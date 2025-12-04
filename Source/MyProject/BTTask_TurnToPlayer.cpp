#include "BTTask_TurnToPlayer.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnemyAI.h"      
#include "MainCharacter.h"

UBTTask_TurnToPlayer::UBTTask_TurnToPlayer()
{
    NodeName = "Turn To Player (C++)";
}

EBTNodeResult::Type UBTTask_TurnToPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AEnemyController* Controller = Cast<AEnemyController>(OwnerComp.GetAIOwner());
    if (!Controller) return EBTNodeResult::Failed;

    UBlackboardComponent* BB = Controller->GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    AEnemyAI* Enemy = Cast<AEnemyAI>(Controller->GetPawn());
    if (!Enemy) return EBTNodeResult::Failed;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject("DetectedEnemy"));
    if (!Target) return EBTNodeResult::Failed;

    FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(
        Enemy->GetActorLocation(),
        Target->GetActorLocation()
    );

    // Повернути ворога лицем до цілі
    Enemy->SetActorRotation(FRotator(0.f, LookRot.Yaw, 0.f));

    return EBTNodeResult::Succeeded;
}
