#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Movable.generated.h"

UCLASS()
class MYPROJECT_API AMovable : public AActor
{
	GENERATED_BODY()

public:
	AMovable();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movable")
	UStaticMeshComponent* CrateMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movable")
	float MassKg = 50.0f;
};
