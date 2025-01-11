// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();

	/* <AActor> */
	virtual void Tick(float DeltaTime) override;
	/* </AActor> */

protected:
	/* <AActor> */
	virtual void BeginPlay() override;
	/* </AActor> */

	/* Combat */
	/* <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/* </IHitInterface> */

	virtual	void Attack();
	virtual void Block();
	virtual void Die();
	void DirectionalHitReact(const FVector& ImpactPoint);
	virtual void HandleDamage(float DamageAmount);
	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);
	void DisableCapsule();
	virtual bool CanAttack();
	bool IsAlive();
	void DisableMeshCollision();
	double GetThetaImpactPoint(const FVector& ImpactPoint);
	bool IsFront(const FVector& ImpactPoint);
	/* /Combat */

	/* Montage */
	virtual int32 PlayAttackMontage();
	virtual int32 PlayDeathMontage();
	virtual void PlayBlockMontage();
	virtual void PlayBlockReactMontage();
	virtual void PlayDodgeMontage();
	void PlayHitReactMontage(const FName& SectionName);
	void StopAttackMontage();
	void StopBlockMontage();
	void PlayBlockingCounterMontage();
	void PlayParryingCounterMontage();
	/* /Montage */

	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();

	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UFUNCTION(BlueprintCallable)
	void SetMeshCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

	UPROPERTY(VisibleAnywhere, Category = Weapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(BlueprintReadOnly)
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, Category = Combat)
	double WarpTargetDistance = 75.f;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EDeathPose> DeathPose;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> AttackMontageSections;

	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* AttackMontage;

private:
	/* Montage */



	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);




	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* BlockMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* DodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* CounterMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<FName> DeathMontageSections;
	/* /Montage */

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticles;

public:
	FORCEINLINE TEnumAsByte<EDeathPose> GetDeathPose() const { return DeathPose; }
};
