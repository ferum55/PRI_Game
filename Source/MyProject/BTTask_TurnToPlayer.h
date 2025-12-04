#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_TurnToPlayer.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_TurnToPlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_TurnToPlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
