#include "BTTask_ShootPlayer.h"
#include "EnemyAI.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MainCharacter.h" // Для перевірки GetHealth()
#include "Kismet/KismetMathLibrary.h" // Для FindLookAtRotation
#include "GameFramework/CharacterMovementComponent.h" // Для StopMovementImmediately

UBTTask_ShootPlayer::UBTTask_ShootPlayer()
{
    NodeName = "Shoot Player (Loop Until Lost or Dead)";
    bNotifyTick = true; // Обов'язково для використання TickTask!
}

EBTNodeResult::Type UBTTask_ShootPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AEnemyController* Controller = Cast<AEnemyController>(OwnerComp.GetAIOwner());
    if (!Controller) return EBTNodeResult::Failed;

    AEnemyAI* Enemy = Cast<AEnemyAI>(Controller->GetPawn());
    // Використовуємо GetDetectedEnemyKey() для коректності
    AActor* TargetActor = Cast<AActor>(Controller->GetBlackboardComponent()->GetValueAsObject(Controller->GetDetectedEnemyKey()));

    if (!Enemy || !TargetActor)
    {
        return EBTNodeResult::Failed;
    }

    // При вході в таску, ми онуляємо таймер і одразу зупиняємо будь-який рух від попередніх тасок
    if (Enemy->GetCharacterMovement())
    {
        Enemy->GetCharacterMovement()->StopMovementImmediately();
    }

    TimeSinceLastShot = 0.f;

    // Таска залишається активною до виконання FinishLatentTask
    return EBTNodeResult::InProgress;
}

void UBTTask_ShootPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    // 1. Отримання та перевірка об'єктів
    AEnemyController* Controller = Cast<AEnemyController>(OwnerComp.GetAIOwner());
    if (!Controller)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UBlackboardComponent* BB = Controller->GetBlackboardComponent();
    AEnemyAI* Enemy = Cast<AEnemyAI>(Controller->GetPawn());
    AMainCharacter* Player = Cast<AMainCharacter>(BB->GetValueAsObject(Controller->GetDetectedEnemyKey()));

    // --- Примусова зупинка та Ротація ---

    // Примусова зупинка руху на кожному тіку (для усунення ковзання/інерції)
    if (Enemy && Enemy->GetCharacterMovement())
    {
        Enemy->GetCharacterMovement()->StopMovementImmediately();
    }

    // Обертаємо NPC до цілі
    if (Enemy && Player)
    {
        FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(
            Enemy->GetActorLocation(),
            Player->GetActorLocation()
        );
        Enemy->SetActorRotation(LookRot);
    }

    // 2. Умови ВИХОДУ з таски (Завершення латентності)

    if (!Enemy || !Player)
    {
        // Якщо ціль зникла з Blackboard 
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // A. Умова виходу: Здоров'я гравця (Ціль більше не існує)
    if (Player->GetHealth() <= 0)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // B. Умова виходу: Втрата прямої видимості (LoS)
    bool bHasLoS = BB->GetValueAsBool(Controller->GetHasLineOfSightKey());
    if (!bHasLoS)
    {
        // Не змогли стріляти, тому повертаємо Failed, щоб Behavior Tree переключився на пошук LastKnownLocation
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    TimeSinceLastShot += DeltaSeconds;

    if (TimeSinceLastShot >= Enemy->GetAttackCooldown())
    {
        Enemy->ShootProjectileAtPlayer();
        TimeSinceLastShot = 0.f;
    }
}