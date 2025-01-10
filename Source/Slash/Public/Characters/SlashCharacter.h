// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "CharacterTypes.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class ASoul;
class ATreasure;
class UAnimMontage;
class USlashOverlay;
class USoundBase;

UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();

	/* <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	/* </AActor> */

	/* <APawn> */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	/* </APawn> */

	/* <ACharacter> */
	virtual void Jump() override;
	/* </ACharacter> */

	/* <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/* </IHitInterface> */

	/* <IPickupInterface> */
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;
	/* </IPickupInterface> */

protected:
	/* <AActor> */
	virtual void BeginPlay() override;
	/* </AActor> */

	/* Callbacks for input */
	/* <ABaseCharacter> */
	virtual void Attack() override;
	virtual void Block() override;
	/* </ABaseCharacter> */

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EKeyPressed();
	void Dodge();
	void BlockEnd();
	/* /Callbacks for input */

	/* Combat */
	/* <ABaseCharacter> */
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual bool CanAttack() override;
	/* </ABaseCharacter> */

	void EquipWeapon(AWeapon* Weapon);
	bool CanDisarm();
	bool CanArm();
	void Disarm();
	void Arm();
	void PlayEquipMontage(const FName& SectionName);
	virtual void Die() override;
	bool HasEnoughStamina(const float& Cost);
	bool IsOccupied();
	void PlayBlockAttackSound(const FVector& ImpactPoint);
	void PlayParrySound(const FVector& ImpactPoint);

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();
	
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void FinishEquipping();

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	UFUNCTION(BlueprintCallable)
	void ParryEnd();
	/* /Combat */

private:
	bool IsUnoccupied();
	bool IsFalling();
	void InitializeSlashOverlay();
	void SetHUDHealth();
	void UseStaminaCost(const float& StaminaCost);
	void ChangeStaminaRegenRateBlockingToDefault();
	void ClearStaminaRegenTimer();

	/* Character components */
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, Category = "Hair")
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, Category = "Hair")
	UGroomComponent* Eyebrows;
	/* /Character components */

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY()
	USlashOverlay* SlashOverlay;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* BlockAttackSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* ParrySound;

	float DodgeCost;
	float StartBlockCost;
	float StaminaRegenRate;
	float StartStaminaRegenTime = 3.f;
	FTimerHandle StaminaRegenTimer;
	bool bBlockAttack = false;
	float BlockAttackCost = 10.f;
	float DamageBlocked = 0.f;
	bool bCanParry = false;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE bool GetbCanParry() const { return bCanParry; }
};
