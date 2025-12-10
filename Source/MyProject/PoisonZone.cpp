#include "PoisonZone.h"
#include "Components/BoxComponent.h"
#include "EnemyAI.h"

APoisonZone::APoisonZone()
{
    PrimaryActorTick.bCanEverTick = false;

    Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
    RootComponent = Box;

    Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Box->SetCollisionResponseToAllChannels(ECR_Overlap);

    Box->OnComponentBeginOverlap.AddDynamic(this, &APoisonZone::OnOverlapBegin);
    Box->OnComponentEndOverlap.AddDynamic(this, &APoisonZone::OnOverlapEnd);
}

void APoisonZone::BeginPlay()
{
    Super::BeginPlay();
}

void APoisonZone::OnOverlapBegin(
    UPrimitiveComponent* Comp,
    AActor* Other,
    UPrimitiveComponent* OtherComp,
    int32 BodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (AEnemyAI* Enemy = Cast<AEnemyAI>(Other))
    {
        Enemy->ApplyPoison();
    }
}

void APoisonZone::OnOverlapEnd(
    UPrimitiveComponent* Comp,
    AActor* Other,
    UPrimitiveComponent* OtherComp,
    int32 BodyIndex)
{
    if (AEnemyAI* Enemy = Cast<AEnemyAI>(Other))
    {
        Enemy->RemovePoison();
    }
}
