#include "BTTask_ShootPlayer.h"
#include "EnemyAI.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MainCharacter.h" 
#include "Kismet/KismetMathLibrary.h" 
#include "GameFramework/CharacterMovementComponent.h" 

UBTTask_ShootPlayer::UBTTask_ShootPlayer()
{
    NodeName = "Shoot Player (Loop Until Lost or Dead)";
    bNotifyTick = true; 
}

EBTNodeResult::Type UBTTask_ShootPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AEnemyController* Controller = Cast<AEnemyController>(OwnerComp.GetAIOwner());
    if (!Controller) return EBTNodeResult::Failed;

    AEnemyAI* Enemy = Cast<AEnemyAI>(Controller->GetPawn());

    AActor* TargetActor = Cast<AActor>(Controller->GetBlackboardComponent()->GetValueAsObject(Controller->GetDetectedEnemyKey()));

    if (!Enemy || !TargetActor)
    {
        return EBTNodeResult::Failed;
    }

    if (Enemy->GetCharacterMovement())
    {
        Enemy->GetCharacterMovement()->StopMovementImmediately();
    }

    TimeSinceLastShot = 0.f;


    return EBTNodeResult::InProgress;
}

void UBTTask_ShootPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{

    AEnemyController* Controller = Cast<AEnemyController>(OwnerComp.GetAIOwner());
    if (!Controller)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UBlackboardComponent* BB = Controller->GetBlackboardComponent();
    AEnemyAI* Enemy = Cast<AEnemyAI>(Controller->GetPawn());
    AMainCharacter* Player = Cast<AMainCharacter>(BB->GetValueAsObject(Controller->GetDetectedEnemyKey()));


    if (Enemy && Enemy->GetCharacterMovement())
    {
        Enemy->GetCharacterMovement()->StopMovementImmediately();
    }


    if (Enemy && Player)
    {
        FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(
            Enemy->GetActorLocation(),
            Player->GetActorLocation()
        );
        Enemy->SetActorRotation(LookRot);
    }

    if (!Enemy || !Player)
    {

        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    if (Player->GetHealth() <= 0)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    bool bHasLoS = BB->GetValueAsBool(Controller->GetHasLineOfSightKey());
    if (!bHasLoS)
    {
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