// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"
#include "DrawDebugHelpers.h"

#include "MyAnimInstance.h"
#include "EnemyAI.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimMontage.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"




// Sets default values
AMainCharacter::AMainCharacter()
{
    PrimaryActorTick.bCanEverTick = true;


    GetMesh()->SetupAttachment(GetCapsuleComponent());
    GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
    GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<USkeletalMesh> BodyMeshRef(
        TEXT("/Game/Mannequin/Character/Mesh/SK_Mannequin.SK_Mannequin")
    );
    if (BodyMeshRef.Succeeded())
    {
        BodyMesh = BodyMeshRef.Object;
        GetMesh()->SetSkeletalMesh(BodyMesh);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Body mesh not found"));
    }

    static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(
        TEXT("/Game/AnimsCharacter/BP_AnimMainCharacter.BP_AnimMainCharacter_C")
    );
    if (AnimBPClass.Succeeded())
    {
        BodyAnimClass = AnimBPClass.Class;
        GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
        GetMesh()->SetAnimInstanceClass(BodyAnimClass);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to load AnimBlueprint"));
    }

    /*FP_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Mesh"));
    FP_Mesh->SetupAttachment(GetMesh());
    FP_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    FP_Mesh->SetOnlyOwnerSee(true);
    FP_Mesh->bCastHiddenShadow = false;
    FP_Mesh->CastShadow = false;

    FP_Mesh->SetSkeletalMesh(GetMesh()->SkeletalMesh);
    FP_Mesh->SetAnimInstanceClass(GetMesh()->AnimClass);*/


    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));

    //Camera->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
    //Camera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));

    Camera->bUsePawnControlRotation = true;

    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
    GetCharacterMovement()->bCrouchMaintainsBaseLocation = true;
    static ConstructorHelpers::FClassFinder<AActor> WeaponBP(
        TEXT("/Game/FPS_Weapon_Bundle/BP_AK74U.BP_AK74U_C")
    );
    if (WeaponBP.Succeeded())
    {
        WeaponClass = WeaponBP.Class;
    }
    static ConstructorHelpers::FClassFinder<AProjectile> ProjectileBP(TEXT("/Game/Projectile/BP_Projectile.BP_Projectile_C"));
    if (ProjectileBP.Succeeded())
    {
        ProjectileClass = ProjectileBP.Class;
    }

    static ConstructorHelpers::FClassFinder<UmyHUD> HUD_BP(TEXT("/Game/UI/BP_HUD.BP_HUD_C"));
    if (HUD_BP.Succeeded())
    {
        HUDClass = HUD_BP.Class;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Failed to find BP_HUD at /Game/UI/BP_HUD.BP_HUD_C"));
    }
    PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

    static ConstructorHelpers::FObjectFinder<UAnimMontage> ReloadMontageRef(
        TEXT("/Game/Anims/AM_Reload.AM_Reload")
    );

    if (ReloadMontageRef.Succeeded())
    {
        ReloadMontage = ReloadMontageRef.Object;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("❌ Reload montage not found"));
    }

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetCapsuleComponent());
    CameraBoom->TargetArmLength = 300.f;
    CameraBoom->bUsePawnControlRotation = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritYaw = false;
    CameraBoom->bInheritRoll = false;
    CameraBoom->bDoCollisionTest = true;
    CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 70.f));

}


void AMainCharacter::BeginPlay()
{
    Super::BeginPlay();


    UE_LOG(LogTemp, Warning, TEXT("=== BeginPlay MainCharacter ==="));

    if (!GetMesh())
    {
        UE_LOG(LogTemp, Error, TEXT("GetMesh() is NULL"));
        return;
    }

    if (!Camera)
    {
        UE_LOG(LogTemp, Error, TEXT("Camera is NULL"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Mesh: %s"), *GetMesh()->GetName());
    UE_LOG(LogTemp, Warning, TEXT("SkeletalMesh asset: %s"),
        GetMesh()->SkeletalMesh ? *GetMesh()->SkeletalMesh->GetName() : TEXT("NULL"));

    const bool bHasCameraSocket = GetMesh()->DoesSocketExist(TEXT("camera"));
    UE_LOG(LogTemp, Warning, TEXT("DoesSocketExist('camera'): %s"), bHasCameraSocket ? TEXT("YES") : TEXT("NO"));

    if (bHasCameraSocket)
    {
        const FTransform SocketWorld = GetMesh()->GetSocketTransform(TEXT("camera"), RTS_World);
        UE_LOG(LogTemp, Warning, TEXT("Socket 'camera' World: Loc=%s Rot=%s"),
            *SocketWorld.GetLocation().ToString(),
            *SocketWorld.Rotator().ToString());
    }

    UE_LOG(LogTemp, Warning, TEXT("Before attach: Camera World Loc=%s Rot=%s"),
        *Camera->GetComponentLocation().ToString(),
        *Camera->GetComponentRotation().ToString());

    UE_LOG(LogTemp, Warning, TEXT("Before attach: Camera Rel Loc=%s Rot=%s"),
        *Camera->GetRelativeLocation().ToString(),
        *Camera->GetRelativeRotation().ToString());

    Camera->AttachToComponent(
        GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("camera")
    );

    UE_LOG(LogTemp, Warning, TEXT("After attach: Camera AttachParent=%s SocketName=%s"),
        Camera->GetAttachParent() ? *Camera->GetAttachParent()->GetName() : TEXT("NULL"),
        *Camera->GetAttachSocketName().ToString());

    UE_LOG(LogTemp, Warning, TEXT("After attach: Camera World Loc=%s Rot=%s"),
        *Camera->GetComponentLocation().ToString(),
        *Camera->GetComponentRotation().ToString());

    UE_LOG(LogTemp, Warning, TEXT("After attach: Camera Rel Loc=%s Rot=%s"),
        *Camera->GetRelativeLocation().ToString(),
        *Camera->GetRelativeRotation().ToString());

    UE_LOG(LogTemp, Warning, TEXT("Mesh World Loc=%s Rot=%s"),
        *GetMesh()->GetComponentLocation().ToString(),
        *GetMesh()->GetComponentRotation().ToString());

    UE_LOG(LogTemp, Warning, TEXT("=== End BeginPlay MainCharacter ==="));

    if (IsLocallyControlled())
    {

        //FP_Mesh->HideBoneByName(TEXT("head"), EPhysBodyOp::PBO_None);
        //FP_Mesh->HideBoneByName(TEXT("neck_01"), EPhysBodyOp::PBO_None);

        //GetMesh()->CastShadow = true;
        //GetMesh()->bCastHiddenShadow = true;
        //GetMesh()->SetOwnerNoSee(true);
    }

    if (WeaponClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = this;

        CurrentWeapon = GetWorld()->SpawnActor<AActor>(WeaponClass, SpawnParams);

        if (CurrentWeapon)
        {

            CurrentWeapon->AttachToComponent(
                GetMesh(),
                FAttachmentTransformRules::SnapToTargetNotIncludingScale,
                TEXT("weapon_socket")
            );
        }
    }

    if (HUDClass)
    {
        HUDWidget = CreateWidget<UmyHUD>(GetWorld(), HUDClass);
        if (HUDWidget)
        {
            HUDWidget->AddToViewport();
            HUDWidget->SetHealth(Health / MaxHealth);
            HUDWidget->SetAmmo(Ammo);
            UE_LOG(LogTemp, Warning, TEXT("HUD added to viewport"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("HUDWidget is null"));
        }
    }
    else UE_LOG(LogTemp, Error, TEXT("HUI"));

    GetCapsuleComponent()->SetCollisionObjectType(ECC_Pawn);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    GetCapsuleComponent()->SetGenerateOverlapEvents(true);
    SetCanBeDamaged(true);

}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (GetMesh() && GetMesh()->GetAnimInstance())
    {
        UMyAnimInstance* Anim = Cast<UMyAnimInstance>(GetMesh()->GetAnimInstance());
        if (Anim && Controller)
        {
            const FRotator ControlRot = Controller->GetControlRotation();
            Anim->AimPitch = FRotator::NormalizeAxis(ControlRot.Pitch);
        }
    }
    if (PhysicsHandle->GrabbedComponent)
    {
        FVector TargetLocation = Camera->GetComponentLocation() + (Camera->GetForwardVector() * HoldDistance);
        PhysicsHandle->SetTargetLocation(TargetLocation);
    }

    if (CameraMode == ECameraMode::ThirdPerson)
    {
        CameraBoom->SetRelativeRotation(
            FRotator(TP_Pitch, TP_Yaw, 0.f)
        );
    }



}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

    PlayerInputComponent->BindAxis("Turn", this, &AMainCharacter::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &AMainCharacter::LookUp);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::OnJumpPressed);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMainCharacter::OnJumpReleased);

    PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMainCharacter::BeginCrouch);
    PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMainCharacter::EndCrouch);
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMainCharacter::Fire);
    PlayerInputComponent->BindAction("Grab", IE_Pressed, this, &AMainCharacter::Grab);
    PlayerInputComponent->BindAction("Grab", IE_Released, this, &AMainCharacter::Release);
    PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMainCharacter::Reload);

    PlayerInputComponent->BindAction("ToggleCamera", IE_Pressed, this, &AMainCharacter::ToggleCameraMode);


}


void AMainCharacter::MoveForward(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void AMainCharacter::MoveRight(float Value)
{
    if (Controller && Value != 0.0f)
    {
        const FRotator YawRotation(0, Controller->GetControlRotation().Yaw, 0);
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}

void AMainCharacter::LookUp(float Value)
{
    if (CameraMode == ECameraMode::ThirdPerson)
    {
        TP_Pitch = FMath::Clamp(TP_Pitch + Value, -80.f, 20.f);
    }
    else
    {
        AddControllerPitchInput(Value);
    }
}

void AMainCharacter::Turn(float Value)
{
    if (CameraMode == ECameraMode::ThirdPerson)
    {
        TP_Yaw += Value;
    }
    else
    {
        AddControllerYawInput(Value);
    }
}

void AMainCharacter::BeginCrouch()
{
    if (CanCrouch()) {
        Crouch();

    }


}

void AMainCharacter::EndCrouch()
{
    UnCrouch();
}

void AMainCharacter::OnJumpPressed()
{
    Jump();
    JumpButtonDown = true;
}

void AMainCharacter::OnJumpReleased()
{
    StopJumping();
    JumpButtonDown = false;
}

void AMainCharacter::Fire()
{
    if (Ammo <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Out of ammo!"));
        return;
    }

    FVector CamLoc = Camera->GetComponentLocation();
    FRotator CamRot = Camera->GetComponentRotation();
    FVector AimDir = CamRot.Vector();

    float MaxAimDistance = 50000.f;
    float AimConeDeg = 5.f;
    float AimConeDot = FMath::Cos(FMath::DegreesToRadians(AimConeDeg));

    //DrawDebugLine(
    //    GetWorld(),
    //    CamLoc,
    //    CamLoc + AimDir * MaxAimDistance,
    //    FColor::Green,
    //    false,
    //    0.2f,
    //    0,
    //    1.f
    //);


    FVector RightEdge = AimDir.RotateAngleAxis(AimConeDeg, FVector::UpVector);
    FVector LeftEdge = AimDir.RotateAngleAxis(-AimConeDeg, FVector::UpVector);

    //DrawDebugLine(GetWorld(), CamLoc, CamLoc + RightEdge * 3000.f, FColor::Yellow, false, 0.2f, 0, 1.f);
    //DrawDebugLine(GetWorld(), CamLoc, CamLoc + LeftEdge * 3000.f, FColor::Yellow, false, 0.2f, 0, 1.f);

    TArray<AActor*> EnemyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyAI::StaticClass(), EnemyActors);

    for (AActor* Actor : EnemyActors)
    {
        FVector ToEnemy = Actor->GetActorLocation() - CamLoc;
        float Dist = ToEnemy.Size();
        if (Dist > MaxAimDistance) continue;

        FVector ToEnemyNorm = ToEnemy.GetSafeNormal();
        float Dot = FVector::DotProduct(AimDir, ToEnemyNorm);

        if (Dot >= AimConeDot)
        {
            /* DrawDebugLine(
                 GetWorld(),
                 CamLoc,
                 Actor->GetActorLocation(),
                 FColor::Red,
                 false,
                 0.2f,
                 0,
                 1.f
             );*/

            if (AEnemyAI* Enemy = Cast<AEnemyAI>(Actor))
            {
                Enemy->OnPlayerAimedShot(1500.f);
            }
        }
    }


    FVector CameraLocation = CamLoc;
    FRotator CameraRotation = CamRot;

    FVector MuzzleLocation =
        CameraLocation +
        CameraRotation.Vector() * 150.f +
        FTransform(CameraRotation).TransformVector(MuzzleOffset);

    FRotator MuzzleRotation = CameraRotation;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
        ProjectileClass,
        MuzzleLocation,
        MuzzleRotation,
        SpawnParams
    );

    if (Projectile)
    {
        Projectile->FireInDirection(MuzzleRotation.Vector());
    }

    Ammo = FMath::Clamp(Ammo - 1, 0, MaxAmmo);

    if (HUDWidget)
    {
        HUDWidget->SetAmmo(Ammo);
    }
}

float AMainCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    FVector StartLocation = GetActorLocation();
    UE_LOG(LogTemp, Error, TEXT("PLAYER DAMAGE: BEFORE damage, Pos: X=%6.1f Y=%6.1f Z=%6.1f"), StartLocation.X, StartLocation.Y, StartLocation.Z);

    float Actual = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (Actual <= 0.f) return 0.f;

    Health -= Actual;
    Camera->PostProcessSettings.bOverride_SceneColorTint = true;
    Camera->PostProcessSettings.SceneColorTint = FLinearColor(1.f, 0.f, 0.f, 1.f);

    FTimerHandle ClearTintHandle;
    GetWorldTimerManager().SetTimer(ClearTintHandle, [this]()
        {
            Camera->PostProcessSettings.SceneColorTint = FLinearColor(1.f, 1.f, 1.f, 1.f);
        }, 0.1f, false);

    if (HUDWidget)
        HUDWidget->SetHealth(Health / MaxHealth);

    if (Health <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Player died"));
    }

    FVector EndLocation = GetActorLocation();

    FVector Displacement = EndLocation - StartLocation;
    float DistanceMoved = Displacement.Size();

    UE_LOG(LogTemp, Error, TEXT("PLAYER DAMAGE: AFTER damage, Pos: X=%6.1f Y=%6.1f Z=%6.1f"), EndLocation.X, EndLocation.Y, EndLocation.Z);
    UE_LOG(LogTemp, Error, TEXT("PLAYER DAMAGE: DISPLACEMENT: Total distance moved due to hit: %f"), DistanceMoved);

    return Actual;
}

#include "DrawDebugHelpers.h"

void AMainCharacter::Grab()
{
    FVector Start = Camera->GetComponentLocation();
    FVector End = Start + (Camera->GetForwardVector() * GrabDistance);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, Start, End, ECC_PhysicsBody, Params);

    if (bHit && Hit.GetActor())
    {
        UPrimitiveComponent* HitComp = Hit.GetComponent();
        if (HitComp && HitComp->IsSimulatingPhysics())
        {
            PhysicsHandle->GrabComponentAtLocation(
                HitComp, NAME_None, HitComp->GetCenterOfMass());
        }
    }
}

void AMainCharacter::Release()
{
    if (PhysicsHandle->GrabbedComponent)
    {
        PhysicsHandle->ReleaseComponent();
    }
}

void AMainCharacter::AddHealth(float Amount)
{
    Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
    if (HUDWidget)
        HUDWidget->SetHealth(Health / MaxHealth);
}

void AMainCharacter::AddAmmo(int32 Amount)
{
    Ammo = FMath::Clamp(Ammo + Amount, 0, MaxAmmo);
    if (HUDWidget)
        HUDWidget->SetAmmo(Ammo);
}

FGenericTeamId AMainCharacter::GetGenericTeamId() const
{
    return FGenericTeamId(static_cast<uint8>(Team));
}

void AMainCharacter::Reload()
{
    UE_LOG(LogTemp, Warning, TEXT("Reload pressed"));

    if (!GetMesh() || !ReloadMontage)
        return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance)
        return;

    if (AnimInstance->Montage_IsPlaying(ReloadMontage))
        return;


    float Len = AnimInstance->Montage_Play(ReloadMontage, 1.f);
    UE_LOG(LogTemp, Warning, TEXT("Montage play result: %f"), Len);

    Ammo = MaxAmmo;
    if (HUDWidget)
        HUDWidget->SetAmmo(Ammo);
}

void AMainCharacter::ToggleCameraMode()
{
    if (CameraMode == ECameraMode::FirstPerson)
    {
        CameraMode = ECameraMode::ThirdPerson;

        Camera->bUsePawnControlRotation = false;
        Camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        Camera->AttachToComponent(
            CameraBoom,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale
        );

        Controller->SetIgnoreLookInput(true);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TP Camera | Yaw=%f Pitch=%f | BoomRot=%s | CamRot=%s | UsePCR=%s"),
            TP_Yaw,
            TP_Pitch,
            *CameraBoom->GetComponentRotation().ToString(),
            *Camera->GetComponentRotation().ToString(),
            Camera->bUsePawnControlRotation ? TEXT("YES") : TEXT("NO")
        );
        CameraMode = ECameraMode::FirstPerson;

        Camera->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        Camera->AttachToComponent(
            GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            TEXT("camera")
        );

        Camera->SetRelativeLocation(FVector::ZeroVector);
        Camera->SetRelativeRotation(FRotator::ZeroRotator);

        Camera->bUsePawnControlRotation = true;
        Controller->SetIgnoreLookInput(false);

    }
}

