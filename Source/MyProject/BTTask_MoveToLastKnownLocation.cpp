#include "BTTask_MoveToLastKnownLocation.h"
#include "EnemyAI.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_MoveToLastKnownLocation::UBTTask_MoveToLastKnownLocation()
{
	NodeName = TEXT("Move To Last Known Location");
}

EBTNodeResult::Type UBTTask_MoveToLastKnownLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UE_LOG(LogTemp, Warning, TEXT("MoveToLastKnownLocation EXECUTED"));

	AEnemyController* Controller = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	if (!Controller) return EBTNodeResult::Failed;

	AEnemyAI* Enemy = Cast<AEnemyAI>(Controller->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = Controller->GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	FVector LastKnownLocation =
		BB->GetValueAsVector(LastKnownLocationKey.SelectedKeyName);

	if (LastKnownLocation == FVector::ZeroVector)
		return EBTNodeResult::Failed;

	Enemy->Walk();

	FAIMoveRequest MoveReq;
	MoveReq.SetGoalLocation(LastKnownLocation);
	MoveReq.SetAcceptanceRadius(AcceptanceRadius);

	Controller->MoveTo(MoveReq);

	return EBTNodeResult::Succeeded;
}
