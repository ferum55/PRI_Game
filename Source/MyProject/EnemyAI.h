// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "GenericTeamAgentInterface.h"
#include "MainCharacter.h"
#include "Perception/AISenseConfig_Sight.h"
#include "EnemyController.h"

#include "EnemyAI.generated.h"

class AProjectile;

UCLASS()
class MYPROJECT_API AEnemyAI : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
private:
	UPROPERTY(Category=Character,VisibleAnywhere,BlueprintReadOnly,meta=(AllowPrivateAccess="true"))
	UAIPerceptionComponent* PerceptionComponent;
	AEnemyController* Controller;
	float TargetLostElapsed = 0.f;
	float LastLocationElapsed = 0.f;
	float LastLocationUpdateTime = 0.5;
public:
	AEnemyAI();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float EffectiveRange = 1000.f;


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
	//
	bool bIsDodging = false;
	float DodgeCooldown = 1.2f;
	FTimerHandle DodgeCooldownTimer;
	



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

	float GetAttackCooldown() const { return AttackCooldown; }
	float GetEffectiveRange() const { return EffectiveRange; }
	void Walk();
	void Run();
	UFUNCTION()
	void OnPlayerAimedShot(float ProjectileSpd);
	void Dodge();
	

};
