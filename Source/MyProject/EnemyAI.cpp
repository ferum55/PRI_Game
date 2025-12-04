#include "EnemyAI.h"
#include "EnemyController.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyAI::AEnemyAI()
{
    PrimaryActorTick.bCanEverTick = true;
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    TargetLostTime = 5.f;
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
    RunSpeed = WalkSpeed * 3;
}

void AEnemyAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!Controller) return;
    UBlackboardComponent* BB = Controller->GetBlackboardComponent();
    if (!BB) return;

    // ------------------------
    // Оновлення LKL при видимості
    // ------------------------
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
            // Робимо перехід тільки один раз, коли в BB ще було true
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

    // ------------------------
    // Якщо бачимо
    // ------------------------
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

        // Якщо щось блокує – НЕ перераховуємо втрату видимості зараз
        if (bHit && Hit.GetActor() != Actor)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("[AI LOS] BLOCKED by %s | Comp=%s"),
                *GetNameSafe(Hit.GetActor()),
                *GetNameSafe(Hit.GetComponent()));

            // НЕ очищаємо Blackboard тут!
            // Просто ставимо flag і запускаємо grace-period у Tick
            bHasLineOfSight = false;
            TargetLostElapsed = 0.f;
            return;
        }

        // Чистий LOS
        bHasLineOfSight = true;
        TargetLostElapsed = 0.f;

        BB->SetValueAsBool(C->GetHasLineOfSightKey(), true);
        BB->SetValueAsObject(C->GetDetectedEnemyKey(), Actor);
        BB->SetValueAsVector(C->GetLastKnownLocationKey(), Actor->GetActorLocation());

        PerceivedActor = Actor;
    }
    else
    {
        // Не бачить, але НЕ скидаємо BLACKBOARD
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

    if (!bCanAttack) return;

    bCanAttack = false;

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
        if (auto* Move = P->FindComponentByClass<UProjectileMovementComponent>())
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

    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(
        Timer,
        [this]() { bCanAttack = true; },
        AttackCooldown,
        false
    );
}

void AEnemyAI::Dodge()
{
    bIsDodging = true;

    int Dir = (FMath::RandBool() ? 1 : -1);

    float DodgeDistance = 150.f;   // наскільки ривок
    float DodgeSpeed = 900.f;      // наскільки швидко

    FVector Right = GetActorRightVector() * Dir;
    FVector Target = GetActorLocation() + Right * DodgeDistance;

    UE_LOG(LogTemp, Warning, TEXT("[DODGE] Executing dodge to %s"),
        Dir > 0 ? TEXT("RIGHT") : TEXT("LEFT"));

    // миттєво змінюємо напрям руху
    GetCharacterMovement()->Velocity = Right * DodgeSpeed;

    // повертаємося до звичайного руху через мить
    GetWorld()->GetTimerManager().SetTimer(
        DodgeCooldownTimer,
        [this]()
        {
            bIsDodging = false;
        },
        DodgeCooldown,
        false
    );

    // TODO: Notify AnimBP про ривок:
    // bDodgeLeft / bDodgeRight
}


void AEnemyAI::OnPlayerAimedShot(float ProjectileSpd)
{
    if (!PerceivedActor) return;
    if (!bHasLineOfSight) return;
    if (bIsDodging) return;

    float Dist = FVector::Distance(GetActorLocation(), PerceivedActor->GetActorLocation());

    // Максимальна дистанція, на якій можна встигнути ухилитись
    float MaxDodgeDistance = ProjectileSpeed * 0.7f; // 1500 * 0.7 ? 1050 см

    // Чим ближче – тим менше шансів
    float Chance = FMath::Clamp(Dist / MaxDodgeDistance, 0.f, 1.f);

    float R = FMath::FRand();

    UE_LOG(LogTemp, Warning, TEXT("[DODGE] Dist=%.0f Chance=%.2f Rand=%.2f"),
        Dist, Chance, R);

    if (R <= Chance)
    {
        Dodge();
    }
}

