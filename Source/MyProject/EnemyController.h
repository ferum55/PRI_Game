#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

UCLASS()
class MYPROJECT_API AEnemyController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyController();

    void OnPossess(APawn* InPawn);

    const float GetPatrolRadius() const { return PatrolRadius;}
    const FName& GetPatrolTargetKey() const { return PatrolTargetKey;}
    const FName& GetHasLineOfSightKey() const { return HasLineOfSightKey;}
    const FName& GetDetectedEnemyKey() const { return DetectedEnemyKey; }
    const FName& GetLastKnownLocationKey() const { return LastKnownLocationKey; }


protected:

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float PatrolRadius; 

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
    FName HasLineOfSightKey;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FName DetectedEnemyKey;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FName LastKnownLocationKey;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    FName PatrolTargetKey;

};
