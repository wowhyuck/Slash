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

	float FrontValue;
	float RightValue;

protected:
	/* <AActor> */
	virtual void BeginPlay() override;
	/* </AActor> */

	/* Callbacks for input */
	/* <ABaseCharacter> */
	virtual void Attack() override;
	virtual void Block() override;
	virtual int32 PlayAttackMontage() override;
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
	void Counter();		// Block 성공 후, Attack

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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsGameFinished = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bClearGame = false;

private:
	void SetVariablesByAttribute();		// Attribute의 Getter로 변수 초기화
	void UpdateSlashOverlay(float DeltaTime);
	bool IsUnoccupied();
	bool IsFalling();
	void InitializeSlashOverlay();		// SlashOverlay의 HealthBar, StaminaBar, Gold 초기화
	void SetHUDHealth();				// HealthBar 업데이트
	void UseStaminaCost(const float& StaminaCost);
	void ChangeStaminaRegenRateBlockingToDefault();		// Blocking->Default의 Stamina Regen Rate로 업데이트
	void ChangebCanCounter();
	void SetDefaultStaminaRegenTimer();
	void ClearStaminaRegenTimer();
	void SetCanCounterTimer();
	void SetResetComboTimer();
	void ClearResetComboTimer();
	void ResetCurrentCombo();		// ResetComboTimer가 끝날 때, CurrentCombo = 0으로 초기화
	void ParryingSuccess(AActor* EnemyWeapon);		// 패링 성공했을 때, 실행되는 함수
	void BlockingSuccess(AActor* EnemyWeapon);							// 막기 성공했을 때, 실행되는 함수

	void LaunchBlockingCharacter();

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

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY()
	USlashOverlay* SlashOverlay;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* BlockAttackSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* ParrySound;

	UPROPERTY(EditAnywhere, Category = Combat)
	float DamageBlocked = 0.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float CanCounterTime = 3.0;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ResetComboTime = 1.f;

	float DodgeCost;
	float StartBlockCost;
	float StaminaRegenRate;
	float StartStaminaRegenTime;
	FTimerHandle StaminaRegenTimer;
	bool bBlockAttack = false;
	float BlockAttackCost;
	bool bCanParry = false;
	bool bCanCounter = false;
	FTimerHandle CanCounterTimer;
	int32 CurrentCombo = 0;
	int32 MaxCombo;
	FTimerHandle ResetComboTimer;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
	FORCEINLINE bool GetbCanParry() const { return bCanParry; }
};
