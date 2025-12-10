#include "BTTask_SetPatrolLocation.h"
#include "EnemyController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SetPatrolLocation::UBTTask_SetPatrolLocation()
{
    NodeName = "Set Patrol Location";
}

EBTNodeResult::Type UBTTask_SetPatrolLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UE_LOG(LogTemp, Warning, TEXT("Patrol Location Called!"));
    AEnemyController* Controller = Cast<AEnemyController>(OwnerComp.GetAIOwner());
    if (!Controller) return EBTNodeResult::Failed;

    APawn* ControlledPawn = Controller->GetPawn();
    if (!ControlledPawn) return EBTNodeResult::Failed;

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(ControlledPawn->GetWorld());
    if (!NavSystem) return EBTNodeResult::Failed;

    FNavLocation ResultLocation;


    bool Found = NavSystem->GetRandomPointInNavigableRadius(
        ControlledPawn->GetActorLocation(),
        PatrolRadius,
        ResultLocation
    );

    if (!Found) return EBTNodeResult::Failed;


    Controller->GetBlackboardComponent()->SetValueAsVector("PatrolLocation", ResultLocation.Location);

    return EBTNodeResult::Succeeded;
}
