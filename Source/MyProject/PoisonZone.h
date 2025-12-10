#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoisonZone.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class APoisonZone : public AActor
{
    GENERATED_BODY()

public:
    APoisonZone();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    UBoxComponent* Box;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* ZoneMesh;
    UPROPERTY()
    UMaterialInterface* GreenMat;


    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* Comp, AActor* Other,
        UPrimitiveComponent* OtherComp, int32 BodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* Comp, AActor* Other,
        UPrimitiveComponent* OtherComp, int32 BodyIndex);
};
