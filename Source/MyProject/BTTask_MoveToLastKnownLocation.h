#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_MoveToLastKnownLocation.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_MoveToLastKnownLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_MoveToLastKnownLocation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector LastKnownLocationKey;

	float AcceptanceRadius = 5.f;
};
