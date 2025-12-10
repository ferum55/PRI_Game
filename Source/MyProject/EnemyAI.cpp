#include "EnemyAI.h"
#include "EnemyController.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Animation/AnimInstance.h"

AEnemyAI::AEnemyAI()
{
    PrimaryActorTick.bCanEverTick = true;
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    TargetLostTime = 5.f;

    HealthBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
    HealthBarWidget->SetupAttachment(GetMesh(), TEXT("head"));
    HealthBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
    HealthBarWidget->SetDrawAtDesiredSize(true);
    HealthBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    HealthBarWidget->SetPivot(FVector2D(0.5f, 1.0f));
    HealthBarWidget->SetDrawSize(FVector2D(200.f, 40.f));

    CurrentHealth = MaxHealth;
}

void AEnemyAI::BeginPlay()
{
    Super::BeginPlay();

    Controller = Cast<AEnemyController>(GetController());
    if (Controller)
    {
        UE_LOG(LogTemp, Warning, TEXT("AI: Controller successfully casted in BeginPlay."));
        UE_LOG(LogTemp, Warning, TEXT("AI: TargetLostTime is %f"), TargetLostTime);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("AI: FAILED to cast AEnemyController in BeginPlay."));
    }

    PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAI::OnActorPerceived);

    WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    RunSpeed = WalkSpeed * 2;

    UE_LOG(LogTemp, Error, TEXT("[HP_UI] BeginPlay: WidgetComponent = %s"),
        HealthBarWidget ? TEXT("VALID") : TEXT("NULL"));

    if (UUserWidget* Widget = HealthBarWidget->GetUserWidgetObject())
    {
        UE_LOG(LogTemp, Warning, TEXT("[HP_UI] UserWidget FOUND: %s"), *Widget->GetName());

        UProgressBar* Bar = Cast<UProgressBar>(Widget->GetWidgetFromName(TEXT("EnemyHealth")));
        if (Bar)
        {
            UE_LOG(LogTemp, Warning, TEXT("[HP_UI] ProgressBar FOUND -> setting %.2f"), CurrentHealth / MaxHealth);
            Bar->SetPercent(CurrentHealth / MaxHealth);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[HP_UI] ProgressBar NOT FOUND! Name mismatch?"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[HP_UI] UserWidget NOT FOUND! Maybe not set in Blueprint?"));
    }

    if (WeaponClass)
    {
        FActorSpawnParameters Params;
        Params.Owner = this;

        WeaponRef = GetWorld()->SpawnActor<AActor>(WeaponClass, Params);

        if (WeaponRef)
        {
            WeaponRef->AttachToComponent(
                GetMesh(),
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                FName("hand_rSocket")
            );
        }
    }
}

void AEnemyAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!Controller) return;
    UBlackboardComponent* BB = Controller->GetBlackboardComponent();
    if (!BB) return;

    if (bHasLineOfSight && (LastLocationElapsed += DeltaTime) >= LastLocationUpdateTime)
    {
        LastLocationElapsed = 0.f;
        if (PerceivedActor)
        {
            FVector PlayerLoc = PerceivedActor->GetActorLocation();
            FVector ToPlayer = PlayerLoc - GetActorLocation();
            float Dist = ToPlayer.Size();

            if (Dist < EffectiveRange)
            {
                PlayerLoc = GetActorLocation() + ToPlayer.GetSafeNormal() * EffectiveRange;
            }

            BB->SetValueAsVector(Controller->GetLastKnownLocationKey(), PlayerLoc);
        }
    }

    if (bHasLineOfSight)
    {
        TargetLostElapsed = 0.f;
    }
    else
    {
        TargetLostElapsed += DeltaTime;

        if (TargetLostElapsed > 0.6f)
        {
            if (BB->GetValueAsBool(Controller->GetHasLineOfSightKey()))
            {
                UE_LOG(LogTemp, Warning, TEXT("[AI] REAL LOS LOST – switching to LKL"));
                BB->SetValueAsBool(Controller->GetHasLineOfSightKey(), false);
            }
        }
    }
}

void AEnemyAI::Walk()
{
    bIsRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    UE_LOG(LogTemp, Warning, TEXT("EnemyAI: Walk() called. Speed=%f"), GetCharacterMovement()->MaxWalkSpeed);
}

void AEnemyAI::Run()
{
    bIsRunning = true;
    GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
    UE_LOG(LogTemp, Warning, TEXT("EnemyAI: Run() called. Speed=%f"), GetCharacterMovement()->MaxWalkSpeed);
}

void AEnemyAI::OnActorPerceived(AActor* Actor, FAIStimulus Stimulus)
{
    UE_LOG(LogTemp, Warning, TEXT("OnActorPerceived CALLED. Actor = %s, SuccessSensed=%d"),
        Actor ? *Actor->GetName() : TEXT("NULL"),
        Stimulus.WasSuccessfullySensed() ? 1 : 0);

    if (!Actor) return;

    AEnemyController* C = Cast<AEnemyController>(GetController());
    if (!C) return;

    UBlackboardComponent* BB = C->GetBlackboardComponent();
    if (!BB) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        FVector Start = GetActorLocation() + FVector(0.f, 0.f, 50.f);
        FVector End = Actor->GetActorLocation() + FVector(0.f, 0.f, 50.f);

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.bReturnPhysicalMaterial = false;
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(Actor);

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit,
            Start,
            End,
            ECC_Visibility,
            Params
        );

        if (bHit && Hit.GetActor() != Actor)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[AI LOS] BLOCKED by %s | Comp=%s"),
                *GetNameSafe(Hit.GetActor()),
                *GetNameSafe(Hit.GetComponent()));

            bHasLineOfSight = false;
            TargetLostElapsed = 0.f;
            return;
        }

        bHasLineOfSight = true;
        TargetLostElapsed = 0.f;

        BB->SetValueAsBool(C->GetHasLineOfSightKey(), true);
        BB->SetValueAsObject(C->GetDetectedEnemyKey(), Actor);
        BB->SetValueAsVector(C->GetLastKnownLocationKey(), Actor->GetActorLocation());

        PerceivedActor = Actor;
    }
    else
    {
        bHasLineOfSight = false;
        TargetLostElapsed = 0.f;

        UE_LOG(LogTemp, Warning, TEXT("[AI] LOS flicker – grace period started"));
    }
}

FGenericTeamId AEnemyAI::GetGenericTeamId() const
{
    return FGenericTeamId(static_cast<uint8>(Team));
}

ETeamAttitude::Type AEnemyAI::GetTeamAttitudeTowards(const AActor& Other) const
{
    if (const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(&Other))
    {
        FGenericTeamId OtherTeamId = TeamAgent->GetGenericTeamId();

        if (OtherTeamId.GetId() == static_cast<uint8>(ETeams::PlayersTeam))
        {
            return ETeamAttitude::Hostile;
        }
    }

    return ETeamAttitude::Neutral;
}

void AEnemyAI::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemyAI::ShootProjectileAtPlayer()
{
    UE_LOG(LogTemp, Warning, TEXT("EnemyAI: ShootProjectileAtPlayer CALLED"));

    if (!ProjectileClass)
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyAI: ProjectileClass is NULL"));
        return;
    }

    if (bIsDodging)
        return;


    if (!bCanAttack) return;

    bCanAttack = false;

    if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
    {
        if (FBoolProperty* Prop = FindFProperty<FBoolProperty>(Anim->GetClass(), TEXT("IsAttacking")))
        {
            Prop->SetPropertyValue_InContainer(Anim, true);
            UE_LOG(LogTemp, Warning, TEXT("EnemyAI: IsAttacking = TRUE"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("EnemyAI: AnimBP has NO IsAttacking variable"));
        }
    }


    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;

    FVector Start = GetActorLocation() + GetActorForwardVector() * 80.f + FVector(0, 0, 50.f);
    FRotator Rot = (Player->GetActorLocation() - Start).Rotation();

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.Instigator = this;

    AProjectile* P = GetWorld()->SpawnActor<AProjectile>(ProjectileClass, Start, Rot, Params);

    if (P)
    {
        if (UProjectileMovementComponent* Move = P->FindComponentByClass<UProjectileMovementComponent>())
        {
            Move->InitialSpeed = ProjectileSpeed;
            Move->Velocity = Rot.Vector() * ProjectileSpeed;
        }

        UE_LOG(LogTemp, Warning, TEXT("EnemyAI: Projectile spawned OK"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("EnemyAI: Projectile spawn FAILED"));
    }


    FTimerHandle ResetAttackAnimTimer;
    GetWorld()->GetTimerManager().SetTimer(
        ResetAttackAnimTimer,
        [this]()
        {
            if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
            {
                if (FBoolProperty* Prop = FindFProperty<FBoolProperty>(Anim->GetClass(), TEXT("IsAttacking")))
                {
                    Prop->SetPropertyValue_InContainer(Anim, false);
                    UE_LOG(LogTemp, Warning, TEXT("EnemyAI: IsAttacking = FALSE"));
                }
            }
        },
        0.9f,
        false
    );

    FTimerHandle CooldownTimer;
    GetWorld()->GetTimerManager().SetTimer(
        CooldownTimer,
        [this]()
        {
            bCanAttack = true;
            UE_LOG(LogTemp, Warning, TEXT("EnemyAI: Attack cooldown finished"));
        },
        AttackCooldown,
        false
    );
}


void AEnemyAI::Dodge()
{
    bIsDodging = true;

    int Dir = (FMath::RandBool() ? 1 : -1);

    float DodgeDistance = 300.f;
    float DodgeSpeed = 300.f;

    FVector Right = GetActorRightVector() * Dir;
    FVector Target = GetActorLocation() + Right * DodgeDistance;

    UE_LOG(LogTemp, Warning, TEXT("[DODGE] Executing dodge to %s"),
        Dir > 0 ? TEXT("RIGHT") : TEXT("LEFT"));

    GetCharacterMovement()->Velocity = Right * DodgeSpeed;

    GetWorld()->GetTimerManager().SetTimer(
        DodgeCooldownTimer,
        [this]()
        {
            bIsDodging = false;
        },
        DodgeCooldown,
        false
    );
}

void AEnemyAI::OnPlayerAimedShot(float ProjectileSpd)
{
    if (!Controller) return;

    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!Player) return;

    float Dist = FVector::Dist(Player->GetActorLocation(), GetActorLocation());

    float Chance = ComputeDodgeChance(Dist);

    UE_LOG(LogTemp, Warning, TEXT("[DODGE] Dist=%.0f  Chance=%.2f"), Dist, Chance);

 
    if (FMath::FRand() > Chance)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DODGE] Chance failed"));
        return;
    }

    if (bIsDodging)
    {
        UE_LOG(LogTemp, Warning, TEXT("[DODGE] Already dodging"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[DODGE] Dodge triggered!"));

    Controller->GetBlackboardComponent()->SetValueAsBool("ShouldDodge", true);
}
float AEnemyAI::ComputeDodgeChance(float Distance) const
{
    if (Distance <= MinDodgeDistance)
        return 0.3f;

    if (Distance >= MaxDodgeDistance)
        return MaxDodgeChance;

    float Alpha = (Distance - MinDodgeDistance) / (MaxDodgeDistance - MinDodgeDistance);

    return Alpha * MaxDodgeChance;
}

void AEnemyAI::SetHealth(float NewHealth)
{
    /*CurrentHealth = FMath::Clamp(NewHealth, 0.f, MaxHealth);

    if (UUserWidget* Widget = HealthBarWidget->GetUserWidgetObject())
        if (UProgressBar* Bar = Cast<UProgressBar>(Widget->GetWidgetFromName(TEXT("EnemyHealth"))))
            Bar->SetPercent(CurrentHealth / MaxHealth);*/

    UE_LOG(LogTemp, Warning, TEXT("[HP] SetHealth CALLED -> NewHealth=%.1f"), NewHealth);

    CurrentHealth = FMath::Clamp(NewHealth, 0.f, MaxHealth);

    UE_LOG(LogTemp, Warning, TEXT("[HP] After clamp -> %.1f / %.1f"), CurrentHealth, MaxHealth);

    UUserWidget* Widget = HealthBarWidget->GetUserWidgetObject();
    if (!Widget)
    {
        UE_LOG(LogTemp, Error, TEXT("[HP_UI] SetHealth: UserWidget = NULL"));
        return;
    }

    UProgressBar* Bar = Cast<UProgressBar>(Widget->GetWidgetFromName(TEXT("EnemyHealth")));
    if (!Bar)
    {
        UE_LOG(LogTemp, Error, TEXT("[HP_UI] SetHealth: ProgressBar NOT FOUND!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[HP_UI] Updating ProgressBar ? %.2f"), CurrentHealth / MaxHealth);
        Bar->SetPercent(CurrentHealth / MaxHealth);
    }

    if (CurrentHealth > 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HP_ANIM] Playing Hit Reaction"));
        PlayHitReaction_BP();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[HP_ANIM] Playing Death Animation"));
        if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
        {
            Anim->Montage_Stop(0.1f);
            GetMesh()->PlayAnimation(DeathAnim, false);
        }

        GetCharacterMovement()->DisableMovement();
        SetActorEnableCollision(false);

        FTimerHandle DeathHandle;
        GetWorldTimerManager().SetTimer(DeathHandle, [this]() { Destroy(); }, 3.0f, false);
    }
}

void AEnemyAI::ApplyPoison()
{
    if (bIsPoisoned)
        return;

    bIsPoisoned = true;

    float NewSpeed = (bIsRunning ? RunSpeed : WalkSpeed) * PoisonSlowMultiplier;
    GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

    UE_LOG(LogTemp, Warning, TEXT("[POISON] Enemy slowed to %.0f"), NewSpeed);

    // Start damage over time
    GetWorld()->GetTimerManager().SetTimer(
        PoisonDamageTimer,
        this,
        &AEnemyAI::PoisonTick,
        1.0f,
        true
    );

    UE_LOG(LogTemp, Warning, TEXT("[POISON] DOT started (1 HP per sec)"));
}

void AEnemyAI::RemovePoison()
{
    if (!bIsPoisoned)
        return;

    bIsPoisoned = false;

    float RestoredSpeed = bIsRunning ? RunSpeed : WalkSpeed;
    GetCharacterMovement()->MaxWalkSpeed = RestoredSpeed;

    UE_LOG(LogTemp, Warning, TEXT("[POISON] Speed restored to %.0f"), RestoredSpeed);

    GetWorld()->GetTimerManager().ClearTimer(PoisonDamageTimer);
}

void AEnemyAI::PoisonTick()
{
    if (!bIsPoisoned)
        return;

    float NewHP = CurrentHealth - PoisonDamagePerTick;
    UE_LOG(LogTemp, Warning, TEXT("[POISON] Damage tick: -%.0f HP"), PoisonDamagePerTick);

    SetHealth(NewHP);

    if (CurrentHealth <= 0.f)
    {
        RemovePoison(); 
    }
}

