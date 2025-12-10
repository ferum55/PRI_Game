#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Dodge.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_Dodge : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Dodge();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    FVector DodgeDirection;
    float DodgeDuration = 0.3f;
    float Elapsed = 0.f;
};
