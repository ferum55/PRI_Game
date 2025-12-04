#include "Movable.h"
#include "Components/StaticMeshComponent.h"

AMovable::AMovable()
{
	PrimaryActorTick.bCanEverTick = false;

	// Порожній StaticMeshComponent — меш вибиратимеш у BP
	CrateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CrateMesh"));
	SetRootComponent(CrateMesh);

	CrateMesh->SetSimulatePhysics(true);
	CrateMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	CrateMesh->SetLinearDamping(0.5f);
	CrateMesh->SetAngularDamping(1.0f);
	CrateMesh->BodyInstance.bUseCCD = true;
}

void AMovable::BeginPlay()
{
	Super::BeginPlay();
	if (CrateMesh)
		CrateMesh->SetMassOverrideInKg(NAME_None, MassKg, true);
}

void AMovable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
