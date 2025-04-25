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

	// 적이 플레이어를 Target으로 지정할 때
	if (EnemyState > EEnemyState::EES_Searching)
	{
		CheckCombatTarget();
	}
}

void ABoss::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	ABaseCharacter::GetHit_Implementation(ImpactPoint, Hitter);
}

void ABoss::Die()
{
	ABaseCharacter::Die();
	EnemyState = EEnemyState::EES_Dead;
	bDead = true;
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
	Tags.Add(FName("Enemy"));
}

void ABoss::CheckCombatTarget()
{
	// 걷기 범위 밖일 때
	if (!IsInsideWalkRadius())
	{
		ChaseTarget(ChasingSpeed);	// Boss 이동속도를 ChasingSpeed로 설정
	}
	// 걷기 범위 안 && 공격 범위 밖일 때
	else if (IsInsideWalkRadius() && IsOutsideAttackRadius())
	{
		ClearAttackTimer();
		// 교전 상태 아닐 때
		if (!IsEngaged())
		{
			ChaseTarget(ChasingWalkSpeed);	// Boss 이동속도를 ChasingWalkSpeed로 설정
		}
	}
	// 공격 가능할 때
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

bool ABoss::IsInsideWalkRadius()
{
	return InTargetRange(CombatTarget, WalkRadius);
}

