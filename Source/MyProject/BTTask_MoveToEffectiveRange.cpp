#include "BTTask_MoveToEffectiveRange.h"
#include "EnemyAI.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"

// !!! ПЕРЕВІРТЕ: AcceptanceRadiusBuffer має бути оголошений в UBTTask_MoveToEffectiveRange.h, наприклад, 50.0f
// Якщо ви не оголосили AcceptanceRadiusBuffer у .h, використовуйте 50.0f замість змінної у коді.

UBTTask_MoveToEffectiveRange::UBTTask_MoveToEffectiveRange()
{
	NodeName = TEXT("Move To Effective Range");
	bNotifyTick = true;
	// AcceptanceRadiusBuffer = 50.0f; // Якщо ви ініціалізуєте тут, а не у .h
}

EBTNodeResult::Type UBTTask_MoveToEffectiveRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AEnemyController* Controller = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	if (!Controller) return EBTNodeResult::Failed;

	AEnemyAI* Enemy = Cast<AEnemyAI>(Controller->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	AActor* DetectedEnemy = Cast<AActor>(
		BB->GetValueAsObject(DetectedEnemyKey.SelectedKeyName)
	);
	if (!DetectedEnemy) return EBTNodeResult::Failed;

	// Отримання буфера (припускаємо, що він оголошений у .h)
	const float Buffer = 50.0f; // Припущене значення, якщо змінна не працює
	const float TargetRange = Enemy->GetEffectiveRange();
	const float AttackZoneMax = TargetRange + Buffer;

	// --- ПЕРЕВІРКА ---

	float Distance = FVector::Dist(Enemy->GetActorLocation(), DetectedEnemy->GetActorLocation());

	// Якщо ми вже в зоні атаки (Dst <= Range + Buffer)
	if (Distance <= AttackZoneMax)
	{
		// !!! Якщо ми вже в діапазоні, ми зупиняємо рух і завершуємо успіхом !!!
		Controller->StopMovement();
		if (Enemy->GetCharacterMovement())
		{
			Enemy->GetCharacterMovement()->StopMovementImmediately();
		}

		UE_LOG(LogTemp, Warning, TEXT("MoveToEffectiveRange: EXECUTE -> ALREADY IN RANGE. Dst=%f. SUCCESS and STOPPED."), Distance);
		return EBTNodeResult::Succeeded;
	}

	// --- ІНІЦІАЦІЯ РУХУ (Якщо ми далеко) ---
	{
		FAIMoveRequest Request;
		Request.SetGoalActor(DetectedEnemy);

		// Встановлюємо AcceptanceRadius трохи МЕНШЕ за EffectiveRange, 
		// щоб NPC зупинився до входу в зону стрільби
		Request.SetAcceptanceRadius(TargetRange - Buffer);

		// Встановлюємо швидкість бігу
		Enemy->Run();

		FPathFollowingRequestResult MoveResult = Controller->MoveTo(Request);

		UE_LOG(LogTemp, Warning, TEXT("MoveToEffectiveRange: EXECUTE -> MOVEMENT STARTED. TargetRangeMax=%f"), AttackZoneMax);

		if (MoveResult.Code == EPathFollowingRequestResult::RequestSuccessful)
		{
			// Рух успішно ініційовано. Повертаємо InProgress для активації TickTask.
			return EBTNodeResult::InProgress;
		}
		else
		{
			// Не вдалося ініціювати рух
			return EBTNodeResult::Failed;
		}
	}
}

void UBTTask_MoveToEffectiveRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	// Отримуємо об'єкти
	AEnemyController* Controller = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	if (!Controller)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	AEnemyAI* Enemy = Cast<AEnemyAI>(Controller->GetPawn());
	AActor* DetectedEnemy = Cast<AActor>(Controller->GetBlackboardComponent()->GetValueAsObject(DetectedEnemyKey.SelectedKeyName));

	if (!Enemy || !DetectedEnemy)
	{
		Controller->StopMovement();
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const float Buffer = 50.0f;
	const float TargetRange = Enemy->GetEffectiveRange();
	const float AttackZoneMax = TargetRange + Buffer;

	// 1. Обертаємо NPC на кожному тіку, поки він рухається
	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(
		Enemy->GetActorLocation(),
		DetectedEnemy->GetActorLocation()
	);
	Enemy->SetActorRotation(LookRot);

	// 2. Логування (як було домовлено)
	float Distance = FVector::Dist(Enemy->GetActorLocation(), DetectedEnemy->GetActorLocation());
	FVector PlayerPos = DetectedEnemy->GetActorLocation();

	UE_LOG(LogTemp, Warning,
		TEXT("MOVE T: Dst=%6.1f | RangeMax=%6.1f | P=(%6.1f,%6.1f,%6.1f)"),
		Distance,
		AttackZoneMax,
		PlayerPos.X, PlayerPos.Y, PlayerPos.Z
	);

	if (Distance > AttackZoneMax + 50.f)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	// 3. ПЕРЕВІРКА: Умова завершення таски (увійти в Attack Zone)
	if (Distance <= AttackZoneMax)
	{
		// Досягли EffectiveRange. Завершуємо успіхом, щоб Behavior Tree перейшов до Shoot Player.
		Controller->StopMovement();

		// Примусово зупинити рух, скинувши швидкість
		if (Enemy->GetCharacterMovement())
		{
			Enemy->GetCharacterMovement()->StopMovementImmediately();
		}

		UE_LOG(LogTemp, Warning, TEXT("MoveToEffectiveRange: Target reached (%f). SUCCESS and STOPPED."), Distance);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}



	// 4. Перевіряємо, чи контролер рухається (умова застрягання/провалу)
	if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		// Якщо ми мали рухатися, але контролер Idle (застряг), завершуємо невдачею.
		UE_LOG(LogTemp, Warning, TEXT("MoveToEffectiveRange: Controller is Idle (Stuck/Cannot Find Path). FAILED."));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Якщо ми все ще далеко і рухаємося, залишаємось у InProgress.
}