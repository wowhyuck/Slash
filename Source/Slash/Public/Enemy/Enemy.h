// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Perception/AIPerceptionTypes.h"
#include "Enemy.generated.h"

class UHealthBarComponent;
class UPawnSensingComponent;

UCLASS()
class SLASH_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();

	/* <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	/* </AActor> */

	/* <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/* </IHitInterface> */

protected:
	/* <AActor> */
	virtual void BeginPlay() override;
	/* </AActor> */

	/* <ABaseCharacter> */
	virtual void Die() override;
	virtual	void Attack() override;
	virtual int32 PlayAttackMontage() override;
	virtual bool CanAttack() override;
	virtual void AttackEnd() override;
	virtual void HandleDamage(float DamageAmount) override;
	/* </ABaseCharacter> */

	virtual void InitializeEnemy();
	virtual void CheckCombatTarget();


	void SpawnSoul();
	void SpawnDefaultWeapon();
	bool IsDead();
	void ChaseTarget();
	bool IsOutsideCombatRadius();
	bool IsOutsideAttackRadius();
	bool IsChasing();
	bool IsEngaged();
	void ClearAttackTimer();
	void StartAttackTimer();
	bool InTargetRange(AActor* Target, double Radius);
	void MoveToTarget(AActor* Target);


	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	// AI Perception
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAIPerceptionComponent* AIPerception;

	UPROPERTY()
	class AAIController* EnemyController;

	UPROPERTY(EditAnywhere)
	class USoundBase* ScreamingSound;

	class UAISenseConfig_Hearing* HearingConfig;
	FVector LocationSearched;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	float ChasingSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	float ChasingWalkSpeed = 130.f;

private:
	/* AI Behavior */
	void CheckPatrolTarget();
	void PatrolTimerFinished();
	void HideHealthBar();
	void ShowHealthBar();
	void LoseInterest();
	void StartPatrolling();
	bool IsInsideAttackRadius();
	bool IsAttacking();
	void ClearPatrolTimer();
	bool InLocationRange(FVector Location, double Radius);
	AActor* ChoosePatrolTarget();
	void SearchingLocation();

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);		// Callback for OnPawnSeen in UPawnSensingComponent

	UFUNCTION()
	void SenseNoise(AActor* NoiseActor, FAIStimulus Stimulus);

	void NotSeeTarget();

	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class AWeapon> WeaponClass;

	UPROPERTY(EditAnywhere, Category = Combat)
	double CombatRadius = 500.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AttackRadius = 150.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	double AcceptanceRadius = 150.f;

	// Current patrol target
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	FTimerHandle PatrolTimer;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float PatrolWaitMax = 10.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float PatrollingSpeed = 125.f;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AttackMax = 1.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float DeathLifeSpan = 10.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	TSubclassOf<class ASoul> SoulClass;

	bool bSeeTarget = false;
	FTimerHandle SeeTargetTimer;
};
