// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void RegenStamina(float DeltaTime, float StaminaRegenRate);

protected:
	virtual void BeginPlay() override;


private:
	// Current Health
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;

	// Current Stamina
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Stamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float StartStaminaRegenTime = 3.f;

	UPROPERTY(VisibleAnywhere, Category = "Actor Attributes")
	int32 Gold;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float DodgeCost = 10.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float StartBlockCost = 5.f;									// Block 키를 눌렀을 때 비용

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float BlockAttackCost = 10.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float DefaultStaminaRegenRate = 5.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float BlockingStaminaRegenRate = 1.f;

public:
	void ReceiveDamage(float Damage);
	void UseStamina(float StaminaCost);
	float GetHealthPercent();
	float GetStaminaPercent();
	bool IsAlive();
	void AddHealth(float AmountOfRecovery);
	void AddGold(int32 AmountOfGold);
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE int32 GetGold() const { return Gold; }
	FORCEINLINE float GetDodgeCost() const { return DodgeCost; }
	FORCEINLINE float GetStartBlockCost() const { return StartBlockCost; }
	FORCEINLINE float GetBlockAttackCost() const { return BlockAttackCost; }
	FORCEINLINE float GetStamina() const { return Stamina; }
	FORCEINLINE float GetStartStaminaRegenTime() const { return StartStaminaRegenTime; }
	FORCEINLINE float GetDefaultStaminaRegenRate() const { return DefaultStaminaRegenRate; }
	FORCEINLINE float GetBlockingStaminaRegenRate() const { return BlockingStaminaRegenRate; }
	
};
