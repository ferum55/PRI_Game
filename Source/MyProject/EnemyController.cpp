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

    UE_LOG(LogTemp, Warning, TEXT("EnemyController: OnPossess CALLED for %s"), *InPawn->GetName());

    if (!BehaviorTree)
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyController: BehaviorTree is NULL!"));
        return;
    }

    UBlackboardComponent* BB;
    UseBlackboard(BehaviorTree->BlackboardAsset, BB);
    RunBehaviorTree(BehaviorTree);

    UE_LOG(LogTemp, Warning, TEXT("EnemyController: BehaviorTree STARTED"));
    UE_LOG(LogTemp, Warning, TEXT("Controller Pawn = %s"), *InPawn->GetName());
    UE_LOG(LogTemp, Warning, TEXT("My BehaviorTree = %s"), BehaviorTree ? *BehaviorTree->GetName() : TEXT("NULL"));
    UE_LOG(LogTemp, Warning, TEXT("Blackboard = %s"), Blackboard ? TEXT("OK") : TEXT("NULL"));

}

