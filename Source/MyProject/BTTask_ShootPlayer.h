#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ShootPlayer.generated.h"

UCLASS(Blueprintable)
class MYPROJECT_API UBTTask_ShootPlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ShootPlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    virtual uint16 GetInstanceMemorySize() const override { return sizeof(float); }

protected:
    float TimeSinceLastShot = 1.f;
};
