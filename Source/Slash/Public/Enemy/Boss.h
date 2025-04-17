// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "Boss.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API ABoss : public AEnemy
{
	GENERATED_BODY()
	
public:
	ABoss();

	/* <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
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
	virtual void HandleDamage(float DamageAmount) override;
	/* </ABaseCharacter> */

	virtual void InitializeEnemy() override;
	virtual void CheckCombatTarget() override;

	void ChaseTarget(float Speed);


private:
	bool IsInsideWalkRadius();

	UPROPERTY(EditAnywhere, Category = Combat)
	double WalkRadius = 500.f;

};
