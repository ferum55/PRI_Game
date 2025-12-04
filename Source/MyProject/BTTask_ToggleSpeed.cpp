#include "BTTask_ToggleSpeed.h"
#include "EnemyAI.h"
#include "AIController.h"

UBTTask_ToggleSpeed::UBTTask_ToggleSpeed()
{
	NodeName = TEXT("Toggle Speed");
}

EBTNodeResult::Type UBTTask_ToggleSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* Pawn = AIController->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}

	AEnemyAI* Enemy = Cast<AEnemyAI>(Pawn);
	if (!Enemy)
	{
		return EBTNodeResult::Failed;
	}

	if (Enemy->bIsRunning)
	{
		Enemy->Walk();
	}
	else
	{
		Enemy->Run();
	}

	// Finish Execute
	return EBTNodeResult::Succeeded;
}
