#include "PoisonZone.h"
#include "EnemyAI.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionScalarParameter.h"


APoisonZone::APoisonZone()
{
    PrimaryActorTick.bCanEverTick = false;

    Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
    RootComponent = Box;

    Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Box->SetCollisionObjectType(ECC_WorldDynamic);
    Box->SetCollisionResponseToAllChannels(ECR_Ignore);
    Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

    Box->OnComponentBeginOverlap.AddDynamic(this, &APoisonZone::OnOverlapBegin);
    Box->OnComponentEndOverlap.AddDynamic(this, &APoisonZone::OnOverlapEnd);

    // ----- Visible Zone Mesh -----
    ZoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneMesh"));
    ZoneMesh->SetupAttachment(Box);
    ZoneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ZoneMesh->SetCastShadow(false);
    ZoneMesh->SetHiddenInGame(false);

    // Load Engine cube mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        ZoneMesh->SetStaticMesh(CubeMesh.Object);
    }
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MatFinder(
        TEXT("Material'/Game/EnemyAI/M_Green.M_Green'")
    );


    if (MatFinder.Succeeded())
    {
        GreenMat = MatFinder.Object;
    }

}

//void APoisonZone::BeginPlay()
//{
//    Super::BeginPlay();
//
//    FVector Extent = Box->GetScaledBoxExtent();
//    FVector Scale;
//    Scale.X = (Extent.X * 2.f) / 100.f;
//    Scale.Y = (Extent.Y * 2.f) / 100.f;
//    Scale.Z = (Extent.Z * 2.f) / 100.f;
//
//    ZoneMesh->SetWorldScale3D(Scale);
//
//    // ---- Create green translucent material ----
//    UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(
//        ZoneMesh->GetMaterial(0), this);
//
//    if (Mat)
//    {
//        Mat->SetVectorParameterValue("BaseColor", FLinearColor(0.f, 1.f, 0.f, 1.f));
//        Mat->SetScalarParameterValue("Opacity", 0.25f); // 25% transparent
//        ZoneMesh->SetMaterial(0, Mat);
//    }
//}
//

void APoisonZone::BeginPlay()
{
    Super::BeginPlay();

    FVector Extent = Box->GetScaledBoxExtent();
    FVector Scale;
    Scale.X = (Extent.X * 2.f) / 100.f;
    Scale.Y = (Extent.Y * 2.f) / 100.f;
    Scale.Z = (Extent.Z * 2.f) / 100.f;
    ZoneMesh->SetWorldScale3D(Scale);

    // Create basic translucent material at runtime
    UMaterial* BaseMat = NewObject<UMaterial>(this);
    BaseMat->BlendMode = BLEND_Translucent;
    BaseMat->TwoSided = true;
    BaseMat->SetShadingModel(MSM_Unlit);

    // Create dynamic instance
    UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);

    // Set color (green, 25% opacity)
    DynMat->SetVectorParameterValue(FName("Color"), FLinearColor(0.f, 1.f, 0.f, 0.25f));

    ZoneMesh->SetMaterial(0, DynMat);
    if (GreenMat)
    {
        ZoneMesh->SetMaterial(0, GreenMat);
    }

}


void APoisonZone::OnOverlapBegin(
    UPrimitiveComponent* Comp, AActor* Other,
    UPrimitiveComponent* OtherComp, int32 BodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    if (AEnemyAI* Enemy = Cast<AEnemyAI>(Other))
    {
        Enemy->ApplyPoison();
    }
}

void APoisonZone::OnOverlapEnd(
    UPrimitiveComponent* Comp, AActor* Other,
    UPrimitiveComponent* OtherComp, int32 BodyIndex)
{
    if (AEnemyAI* Enemy = Cast<AEnemyAI>(Other))
    {
        Enemy->RemovePoison();
    }
}
