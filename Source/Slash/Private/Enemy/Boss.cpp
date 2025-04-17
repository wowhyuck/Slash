// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Boss.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"


ABoss::ABoss()
{

}

void ABoss::Tick(float DeltaTime)
{
	if (IsDead()) return;
	if (EnemyState > EEnemyState::EES_Searching)
	{
		CheckCombatTarget();
	}
}

float ABoss::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	return DamageAmount;
}

void ABoss::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	ABaseCharacter::GetHit_Implementation(ImpactPoint, Hitter);
}

void ABoss::BeginPlay()
{
	Super::BeginPlay();
}

void ABoss::Die()
{
	ABaseCharacter::Die();
	EnemyState = EEnemyState::EES_Dead;
	ClearAttackTimer();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABoss::HandleDamage(float DamageAmount) 
{
	ABaseCharacter::HandleDamage(DamageAmount);
}

void ABoss::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());
	SpawnDefaultWeapon();
}

void ABoss::CheckCombatTarget()
{
	if (!IsInsideWalkRadius())
	{
		ChaseTarget(ChasingSpeed);
	}
	else if (IsInsideWalkRadius() && IsOutsideAttackRadius())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget(ChasingWalkSpeed);
		}
	}
	else if (CanAttack())
	{
		UE_LOG(LogTemp, Warning, TEXT("Attack"));
		StartAttackTimer();
	}
}

void ABoss::ChaseTarget(float Speed)
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = Speed;
	MoveToTarget(CombatTarget);
}

bool ABoss::IsInsideWalkRadius()
{
	return InTargetRange(CombatTarget, WalkRadius);
}

