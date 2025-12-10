#include "BTTask_Run.h"
#include "EnemyAI.h"
#include "AIController.h"

UBTTask_Run::UBTTask_Run()
{
	NodeName = TEXT("Run");
}

EBTNodeResult::Type UBTTask_Run::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return EBTNodeResult::Failed;

	AEnemyAI* Enemy = Cast<AEnemyAI>(AIController->GetPawn());
	if (!Enemy)
		return EBTNodeResult::Failed;

	Enemy->Run(); // <-- твій метод

	return EBTNodeResult::Succeeded;
}
