// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Boss.h"
#include "AIController.h"


ABoss::ABoss()
{

}

void ABoss::Tick(float DeltaTime)
{

}

float ABoss::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{

	return 0;
}

void ABoss::Destroyed()
{

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

}

int32 ABoss::PlayAttackMontage()
{

	return 0;
}

void ABoss::HandleDamage(float DamageAmount) 
{

}

void ABoss::InitializeEnemy()
{
	EnemyController = Cast<AAIController>(GetController());
	SpawnDefaultWeapon();
}