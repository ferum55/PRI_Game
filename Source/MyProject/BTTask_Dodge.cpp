#include "BTTask_Dodge.h"
#include "EnemyAI.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_Dodge::UBTTask_Dodge()
{
    bNotifyTick = true;
    NodeName = "Dodge Side";
}

EBTNodeResult::Type UBTTask_Dodge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return EBTNodeResult::Failed;

    AEnemyAI* Enemy = Cast<AEnemyAI>(Controller->GetPawn());
    if (!Enemy) return EBTNodeResult::Failed;

    Controller->StopMovement();

    Enemy->bIsDodging = true;

    float Side = FMath::RandBool() ? 1.f : -1.f;
    DodgeDirection = Enemy->GetActorRightVector() * Side;

    Elapsed = 0.f;

    return EBTNodeResult::InProgress;
}

void UBTTask_Dodge::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* Controller = OwnerComp.GetAIOwner();
    if (!Controller) return;

    AEnemyAI* Enemy = Cast<AEnemyAI>(Controller->GetPawn());
    if (!Enemy) return;

    Elapsed += DeltaSeconds;

    Enemy->AddMovementInput(DodgeDirection, 1.0f);

    if (Elapsed >= DodgeDuration)
    {
        Enemy->bIsDodging = false;

        OwnerComp.GetBlackboardComponent()->SetValueAsBool("ShouldDodge", false);


        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
