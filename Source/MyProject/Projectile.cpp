// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Enemy.h"
#include "EnemyAI.h"
#include "MainCharacter.h"

// Sets default values
AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	if (!RootComponent) {
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ProjectileSceneComponent"));
	}
	if (!CollisionComponent) {
		CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
		CollisionComponent->InitSphereRadius(15.0f);
		RootComponent = CollisionComponent;
	}

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	ProjectileMovementComponent->InitialSpeed = 3000.f;
	ProjectileMovementComponent->MaxSpeed = 3000.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->Bounciness = 0.f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	ProjectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMeshComponent"));
	ProjectileMeshComponent->SetupAttachment(RootComponent);

	InitialLifeSpan = LifeSpan;
	CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->SetNotifyRigidBodyCollision(true);
	CollisionComponent->BodyInstance.bUseCCD = true;
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Block);

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	if (!CollisionComponent) {
		UE_LOG(LogTemp, Error, TEXT("No CollisionComponent!"));
		return;
	}

	
}



void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector Loc = GetActorLocation();
	UE_LOG(LogTemp, VeryVerbose, TEXT("Projectile at: %s"), *Loc.ToString());


}

void AProjectile::FireInDirection(const FVector& Direction) {
	ProjectileMovementComponent->Velocity = Direction * ProjectileMovementComponent->InitialSpeed;
}

void AProjectile::OnHit(
    UPrimitiveComponent* HitComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComponent,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    AController* InstigatorCtrl = GetInstigatorController();
    UE_LOG(LogTemp, Warning, TEXT("[Projectile] %s fired by %s hitting %s"),
        *GetName(),
        InstigatorCtrl ? *InstigatorCtrl->GetName() : TEXT("NoController"),
        OtherActor ? *OtherActor->GetName() : TEXT("NULL"));

    // ================== BASIC SAFETY ==================
    if (!OtherActor || OtherActor == this)
    {
        Destroy();
        return;
    }

    // ================== PLAYER DAMAGE ==================
    if (AMainCharacter* Player = Cast<AMainCharacter>(OtherActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("[DMG] Projectile calling TakeDamage on PLAYER"));
        Player->TakeDamage(Damage, FDamageEvent(), GetInstigatorController(), this);
        Destroy();
        return;
    }

    // ================== OLD ENEMY TYPE ==================
    if (AEnemy* Enemy = Cast<AEnemy>(OtherActor))
    {
        float OldHP = Enemy->GetHealth();
        float NewHP = OldHP - Damage;
        UE_LOG(LogTemp, Warning, TEXT("[DMG] Hit AEnemy: OldHP=%.1f NewHP=%.1f"), OldHP, NewHP);

        Enemy->SetHealth(NewHP);
        Destroy();
        return;
    }

    // ================== NEW ENEMY AI ==================
    if (AEnemyAI* EnemyAI = Cast<AEnemyAI>(OtherActor))
    {
        if (IsValid(EnemyAI))
        {
            float OldHP = EnemyAI->GetHealth();
            float NewHP = OldHP - Damage;

            UE_LOG(LogTemp, Warning, TEXT("[DMG] Hit EnemyAI: %s HP %.1f -> %.1f"),
                *EnemyAI->GetName(), OldHP, NewHP);

            EnemyAI->SetHealth(NewHP);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[DMG] EnemyAI pointer INVALID despite cast success"));
        }

        Destroy();
        return;
    }

    // ================== OTHER OBJECTS ==================
    UE_LOG(LogTemp, Warning, TEXT("[DMG] Hit something else: %s"), *OtherActor->GetName());

    // ================== PHYSICS ==================
    if (OtherComponent && OtherComponent->IsSimulatingPhysics())
    {
        constexpr float Impulse = 100.0f;
        OtherComponent->AddImpulseAtLocation(
            ProjectileMovementComponent->Velocity * Impulse,
            Hit.ImpactPoint
        );
    }

    Destroy();
}
