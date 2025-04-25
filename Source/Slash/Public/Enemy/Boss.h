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
	/* </AActor> */

	/* <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	/* </IHitInterface> */

protected:

	/* <ABaseCharacter> */
	virtual void Die() override;
	virtual void HandleDamage(float DamageAmount) override;
	/* </ABaseCharacter> */

	/* <AEnemy> */
	virtual void InitializeEnemy() override;
	virtual void CheckCombatTarget() override;
	/* </AEnemy> */

	void ChaseTarget(float Speed);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	FName BossName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat)
	bool bDead = false;


private:
	bool IsInsideWalkRadius();

	UPROPERTY(EditAnywhere, Category = Combat)
	double WalkRadius = 500.f;
};
