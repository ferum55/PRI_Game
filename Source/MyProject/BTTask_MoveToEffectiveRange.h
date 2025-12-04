#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType.h"
#include "BTTask_MoveToEffectiveRange.generated.h"

UCLASS(Blueprintable)
class MYPROJECT_API UBTTask_MoveToEffectiveRange : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_MoveToEffectiveRange();

    // Оголошуємо таску як латентну (InProgress)
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    // Перевантажуємо TickTask для постійної перевірки дистанції
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    // Ключ Blackboard для отримання актора гравця
    UPROPERTY(EditAnywhere, Category = Blackboard)
    FBlackboardKeySelector DetectedEnemyKey;

    // Встановлюємо мінімальний радіус, щоб не підходити впритул
    UPROPERTY(EditAnywhere, Category = Movement)
    float AcceptanceRadiusBuffer = 50.0f;
};