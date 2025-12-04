#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTTask_Patrol.generated.h"

UCLASS()
class MYPROJECT_API UBTTask_Patrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Patrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:

    // Äמהא‏ Ò²ËÜÊÈ צו, ב³כüרו ם³קמדמ.
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector PatrolTargetKey;
};
