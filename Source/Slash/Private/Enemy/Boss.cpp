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

int32 ABoss::PlayAttackMontage()
{

	return 0;
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
	if (!IsInsideJumpAttackRadius())
	{
		ChaseTarget(ChasingSpeed);
	}
	else if(IsInsideJumpAttackRadius())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget(ChasingWalkSpeed);
		}
	}
	else if (CanAttack())
	{
		StartAttackTimer();
	}
}

void ABoss::ChaseTarget(float Speed)
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = Speed;
	MoveToTarget(CombatTarget);
}

bool ABoss::IsInsideJumpAttackRadius()
{
	return InTargetRange(CombatTarget, JumpAttackRadius);
}
