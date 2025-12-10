#include "BTTask_MoveToEffectiveRange.h"
#include "EnemyAI.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "GameFramework/CharacterMovementComponent.h"


UBTTask_MoveToEffectiveRange::UBTTask_MoveToEffectiveRange()
{
	NodeName = TEXT("Move To Effective Range");
	bNotifyTick = true;
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


	const float Buffer = 50.0f; 
	const float TargetRange = Enemy->GetEffectiveRange();
	const float AttackZoneMax = TargetRange + Buffer;

	float Distance = FVector::Dist(Enemy->GetActorLocation(), DetectedEnemy->GetActorLocation());

	if (Distance <= AttackZoneMax)
	{
		Controller->StopMovement();
		if (Enemy->GetCharacterMovement())
		{
			Enemy->GetCharacterMovement()->StopMovementImmediately();
		}

		UE_LOG(LogTemp, Warning, TEXT("MoveToEffectiveRange: EXECUTE -> ALREADY IN RANGE. Dst=%f. SUCCESS and STOPPED."), Distance);
		return EBTNodeResult::Succeeded;
	}


	{
		FAIMoveRequest Request;
		Request.SetGoalActor(DetectedEnemy);

		Request.SetAcceptanceRadius(TargetRange - Buffer);


		Enemy->Run();

		FPathFollowingRequestResult MoveResult = Controller->MoveTo(Request);

		UE_LOG(LogTemp, Warning, TEXT("MoveToEffectiveRange: EXECUTE -> MOVEMENT STARTED. TargetRangeMax=%f"), AttackZoneMax);

		if (MoveResult.Code == EPathFollowingRequestResult::RequestSuccessful)
		{
			return EBTNodeResult::InProgress;
		}
		else
		{

			return EBTNodeResult::Failed;
		}
	}
}

void UBTTask_MoveToEffectiveRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{

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

	FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(
		Enemy->GetActorLocation(),
		DetectedEnemy->GetActorLocation()
	);
	Enemy->SetActorRotation(LookRot);

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
	if (Distance <= AttackZoneMax)
	{
		Controller->StopMovement();

		if (Enemy->GetCharacterMovement())
		{
			Enemy->GetCharacterMovement()->StopMovementImmediately();
		}

		UE_LOG(LogTemp, Warning, TEXT("MoveToEffectiveRange: Target reached (%f). SUCCESS and STOPPED."), Distance);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}



	if (Controller->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("MoveToEffectiveRange: Controller is Idle (Stuck/Cannot Find Path). FAILED."));
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

}