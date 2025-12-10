#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Walk.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_Walk : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Walk();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
