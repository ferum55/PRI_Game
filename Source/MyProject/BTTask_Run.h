#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Run.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_Run : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_Run();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
