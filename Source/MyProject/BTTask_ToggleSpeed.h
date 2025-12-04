#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ToggleSpeed.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_ToggleSpeed : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ToggleSpeed();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
