// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "GenericTeamAgentInterface.h"
#include "MainCharacter.h"
#include "Perception/AISenseConfig_Sight.h"
#include "EnemyController.h"
#include "TimerManager.h"
#include "EnemyAI.generated.h"

class AProjectile;
class UWidgetComponent;
class UAnimSequence;

UCLASS()
class MYPROJECT_API AEnemyAI : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
private:
	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent* PerceptionComponent;
	AEnemyController* Controller;
	float TargetLostElapsed = 0.f;
	float LastLocationElapsed = 0.f;
	float LastLocationUpdateTime = 0.5f;
public:
	AEnemyAI();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float EffectiveRange = 1000.f;
	UPROPERTY(EditAnywhere, Category = "UI")
	float HealthBarHeightOffset = 50.f; // ן³הם³לט ÿךשמ ענובא גטשו


protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float WalkSpeed;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float RunSpeed;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	AActor* PerceivedActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teams")
	ETeams Team = ETeams::EnemyTeam;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bHasLineOfSight = false;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	float TargetLostTime;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackCooldown = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ProjectileSpeed = 1500.f;

	bool bCanAttack = true;

	
	float DodgeCooldown = 1.2f;
	FTimerHandle DodgeCooldownTimer;

	// HP / UI / Animation
	UPROPERTY(EditAnywhere, Category = "Stats")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, Category = "UI")
	UWidgetComponent* HealthBarWidget;

	UPROPERTY(EditAnywhere, Category = "Animation")
	UAnimSequence* DeathAnim;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	float MinDodgeDistance = 600.f; // םטזקו צüמדמ – 0%

	UPROPERTY(EditAnywhere, Category = "Dodge")
	float MaxDodgeDistance = 2000.f; // גטשו צüמדמ – 80%

	UPROPERTY(EditAnywhere, Category = "Dodge")
	float MaxDodgeChance = 0.8f;     // 80%

	// Poison state
	bool bIsPoisoned = false;
	FTimerHandle PoisonDamageTimer;

	float PoisonSlowMultiplier = 0.5f; // 50% speed
	float PoisonDamagePerTick = 1.f;   // 1 HP per second


public:
	virtual void Tick(float DeltaTime) override;
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void OnActorPerceived(AActor* Actor, FAIStimulus Stimulus);

	UFUNCTION(BlueprintCallable)
	void ShootProjectileAtPlayer();

	bool bIsRunning = false;
	bool bIsDodging = false;

	float GetAttackCooldown() const { return AttackCooldown; }
	float GetEffectiveRange() const { return EffectiveRange; }

	void Walk();
	void Run();

	UFUNCTION()
	void OnPlayerAimedShot(float ProjectileSpd);

	void Dodge();

	void ApplyPoison();
	void RemovePoison();
	void PoisonTick();


	UFUNCTION(BlueprintCallable)
	void SetHealth(float NewHealth);

	float GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintImplementableEvent)
	void PlayHitReaction_BP();
	float ComputeDodgeChance(float Distance) const;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<AActor> WeaponClass;

	UPROPERTY()
	AActor* WeaponRef;

};
