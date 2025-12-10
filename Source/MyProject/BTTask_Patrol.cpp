#include "BTTask_Patrol.h"
#include "EnemyController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Patrol::UBTTask_Patrol()
{
    NodeName = "Patrol Location";
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UE_LOG(LogTemp, Warning, TEXT("Patrolling!"));
    AEnemyController* Controller = Cast<AEnemyController>(OwnerComp.GetAIOwner());
    if (!Controller) return EBTNodeResult::Failed;

    UBlackboardComponent* BB = Controller->GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    float PatrolRadius = Controller->GetPatrolRadius();
    if (PatrolRadius <= 0.f) return EBTNodeResult::Failed;

    FNavLocation ResultLocation;
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetNavigationSystem(&OwnerComp);
    if (!NavSys) return EBTNodeResult::Failed;

    if (NavSys->GetRandomReachablePointInRadius(Controller->GetNavAgentLocation(), PatrolRadius, ResultLocation))
    {
        // Мінімальна заміна: використовуємо BlackboardKeySelector
        BB->SetValueAsVector(PatrolTargetKey.SelectedKeyName, ResultLocation.Location);

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}
