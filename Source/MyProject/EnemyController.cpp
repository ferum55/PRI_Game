#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

#include "EnemyAI.h"



AEnemyController::AEnemyController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AEnemyController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    if (!BehaviorTree)
    {
        return;
    }

    UBlackboardComponent* BB;
    UseBlackboard(BehaviorTree->BlackboardAsset, BB);
    RunBehaviorTree(BehaviorTree);

}

