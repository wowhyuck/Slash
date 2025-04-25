// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Characters/SlashCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AttributeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/HealthBarComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"
#include "Perception/AIPerceptionTypes.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBarWidget"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(45.f);

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	AIPerception->ConfigureSense(*HearingConfig);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 죽을 때
	if (IsDead()) return;

	// 적을 발견할 때
	if (EnemyState > EEnemyState::EES_Searching)
	{
		CheckCombatTarget();
	}
	// 소리 발생 지점 찾아갈 때
	else if (EnemyState == EEnemyState::EES_Searching)
	{
		SearchingLocation();
	}
	// 정찰할 때
	else
	{
		CheckPatrolTarget();
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	CombatTarget = EventInstigator->GetPawn();

	// 피격 시, 공격 범위 안에 있을 때
	if (IsInsideAttackRadius())
	{
		EnemyState = EEnemyState::EES_Attacking;
	}
	// 피격 시, 공격 범위 밖에 있을 때 -> 추격
	else if (IsOutsideAttackRadius())
	{
		ChaseTarget();
	}
	return DamageAmount;
}

void AEnemy::Destroyed()
{
	// 무기 삭제
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	// 피격 시, HealthBar Widget 띄우기
	if (!IsDead())
	{
		ShowHealthBar();
	}

	// Timer 클리어
	ClearPatrolTimer();
	ClearAttackTimer();
	
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);

	// 공격 중 피격 시, 공격 Montage 중지 후 다시 공격 시작
	StopAttackMontage();
	if (IsInsideAttackRadius())
	{
		if (!IsDead()) StartAttackTimer();
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		ScreamingSound,
		ImpactPoint);

	// 피격 시, Noise 전달해서 AIPerception->OnTargetPerceptionUpdated 부르기
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.f, this);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitializeEnemy();

	// 시야, 소리 감지할 때 호출할 함수 등록
	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}
	if (AIPerception)
	{
		AIPerception->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemy::SenseNoise);
	}
}

void AEnemy::Die()
{
	Super::Die();
	EnemyState = EEnemyState::EES_Dead;
	ClearAttackTimer();
	HideHealthBar();
	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnSoul();
}

void AEnemy::Attack()
{
	Super::Attack();
	if (CombatTarget == nullptr) return;

	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();
}

int32 AEnemy::PlayAttackMontage()
{
	if (bSeeTarget)
	{
		return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
	}
	// 적이 뒤에 있을 때
	else
	{
		PlayMontageSection(AttackMontage, AttackMontageSections[0]);
		return 0;
	}
}

bool AEnemy::CanAttack()
{
	const bool bCanAttack =
		IsInsideAttackRadius() &&
		!IsAttacking() &&
		!IsEngaged() &&
		!IsDead();
	return bCanAttack;
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);

	if (Attributes && HealthBarWidget)
	{
		HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
	}
}

void AEnemy::InitializeEnemy()
{
	Tags.Add(FName("Enemy"));
	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);
	HideHealthBar();
	SpawnDefaultWeapon();
}

void AEnemy::CheckCombatTarget()
{
	// 전투 범위 밖
	if (IsOutsideCombatRadius())
	{
		ClearAttackTimer();		// 공격 타이머 리셋
		LoseInterest();			// 어그로 리셋
		// 교전이 아닐 때
		if (!IsEngaged())
		{
			StartPatrolling();	// 정찰
		}
	}
	// 공격 범위 밖 && 추격 중이 아닐 때
	else if (IsOutsideAttackRadius() && !IsChasing())
	{
		ClearAttackTimer();
		// 교전이 아닐 때
		if (!IsEngaged())
		{
			ChaseTarget();	// 타겟 추격
		}
	}
	// 공격 가능할 때
	else if (CanAttack())
	{
		StartAttackTimer();	// 공격
	}
}

void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();
	if (World && SoulClass)
	{
		ASoul* SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, GetActorLocation(), GetActorRotation());
	}
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

bool AEnemy::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;	// 추격일 때 적 이동속도를 ChasingSpeed로 설정
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideCombatRadius()
{
	return !InTargetRange(CombatTarget, CombatRadius);
}

bool AEnemy::IsOutsideAttackRadius()
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemy::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsEngaged()
{
	return EnemyState == EEnemyState::EES_Engaged;
}

void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (Target == nullptr) return false;

	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	EnemyController->MoveTo(MoveRequest);
}

void AEnemy::CheckPatrolTarget()
{
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		const float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemy::HideHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(false);
	}
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	HideHealthBar();
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed; // 정찰일 때 적 이동속도를 PatrollingSpeed로 설정
	// true  -> 2개 이상 PatrolTarget으로 정찰
	// false -> 등록된 PatrolTarget에 대기 
	PatrolTargets.Num() > 1 ? MoveToTarget(PatrolTarget) : MoveToTarget(PatrolTargets[0]);
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

bool AEnemy::InLocationRange(FVector Location, double Radius)
{
	const double Distance = (Location - GetActorLocation()).Size();
	return Distance <= Radius;
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;	// 등록된 정찰 지점
	for (AActor* Target : PatrolTargets)
	{
		// Target이 PatrolTarget(정찰했던 지점) 다를 때
		if (Target != PatrolTarget)	
		{
			ValidTargets.AddUnique(Target);	// 다음 정찰 지점들 등록
		}
	}

	const int32 NumPatrolTargets = ValidTargets.Num();	// 등록된 정찰 개수
	if (NumPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
		return ValidTargets[TargetSelection];
	}

	return nullptr;
}

void AEnemy::SearchingLocation()
{
	// 정찰 지점 도착할 때
	if (InLocationRange(LocationSearched, PatrolRadius))
	{
		StartPatrolling();	// 다른 정찰 지점을 정찰
	}
	else
	{
		EnemyController->MoveToLocation(LocationSearched);
	}
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	bSeeTarget = true;	// Pawn 볼 때, true로 설정

	// bSeeTarget = false로 초기화
	GetWorldTimerManager().SetTimer(SeeTargetTimer, this, &AEnemy::NotSeeTarget, 0.5);

	// 감지된 Pawn이 플레이어고 Enemy가 공격 시작할 수 있는지
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing &&
		EnemyState < EEnemyState::EES_Attacking&&
		SeenPawn->ActorHasTag(FName("EngageableTarget"));

	if (bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;
		ClearPatrolTimer();
		ChaseTarget();
	}
}

void AEnemy::SenseNoise(AActor* NoiseActor, FAIStimulus Stimulus)
{
	if (EnemyState > EEnemyState::EES_Searching || IsDead()) return;

	// 소리 감지 시, 소리나는 지점으로 정찰
	EnemyState = EEnemyState::EES_Searching;
	LocationSearched = NoiseActor->GetActorLocation();
}

void AEnemy::NotSeeTarget()
{
	bSeeTarget = false;
}
