#include "BTTask_Walk.h"
#include "EnemyAI.h"
#include "AIController.h"

UBTTask_Walk::UBTTask_Walk()
{
	NodeName = TEXT("Walk");
}

EBTNodeResult::Type UBTTask_Walk::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
		return EBTNodeResult::Failed;

	AEnemyAI* Enemy = Cast<AEnemyAI>(AIController->GetPawn());
	if (!Enemy)
		return EBTNodeResult::Failed;

	Enemy->Walk(); // <-- твій метод

	return EBTNodeResult::Succeeded;
}
