// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AttributeComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GroomComponent.h"
#include "Items/Item.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/Weapons/Weapon.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Engine/World.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");

	SetVariablesByAttribute();
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSlashOverlay(DeltaTime);

}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// 적의 공격을 받을 때, 적의 무기가 캐릭터 앞에 있을 때 && 캐릭터가 막기 중일 때 -> 막기 성공 
	bBlockAttack = IsFront(DamageCauser->GetActorLocation()) && ActionState == EActionState::EAS_Blocking;
	
	// 공격 막기 성공했을 경우 -> Weapon Location = ImpactPoint 해서 bBlockAttack = true로 만들기
	if (bBlockAttack)
	{
		bCanCounter = true;
		SetCanCounterTimer();

		// True -> 적의 공격을 패링했을 때, 패링 관련 함수 실행
		// False -> 적의 공격을 막았을 때, 막기 관련 함수 실행
		bCanParry ? ParryingSuccess(DamageCauser) : BlockingSuccess(DamageCauser);

		UseStaminaCost(BlockAttackCost);
		HandleDamage(DamageBlocked);
	}
	else
	{
		HandleDamage(DamageAmount);
	}
	SetHUDHealth();

	return DamageAmount;
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ASlashCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ASlashCharacter::MoveRight);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &ASlashCharacter::Turn);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &ASlashCharacter::LookUp);

	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &ASlashCharacter::Jump);
	PlayerInputComponent->BindAction(FName("Equip"), IE_Pressed, this, &ASlashCharacter::EKeyPressed);
	PlayerInputComponent->BindAction(FName("Attack"), IE_Pressed, this, &ASlashCharacter::Attack);
	PlayerInputComponent->BindAction(FName("Dodge"), IE_Pressed, this, &ASlashCharacter::Dodge);
	PlayerInputComponent->BindAction(FName("Block"), IE_Pressed, this, &ASlashCharacter::Block);
	PlayerInputComponent->BindAction(FName("Block"), IE_Released, this, &ASlashCharacter::BlockEnd);
}

void ASlashCharacter::Jump()
{
	if (IsUnoccupied())
	{
		Super::Jump();
	}
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	// 막기 성공
	if (bBlockAttack)
	{
		// 막기 스태미나가 충분히 있을 경우
		if (HasEnoughStamina(BlockAttackCost))
		{
			PlayBlockReactMontage();
		}
		// 막기 스태미나가 부족할 경우 -> Blocking 풀기
		else
		{
			PlayHitReactMontage("FromFront");
		}
		return;
	}

	// 막기 실패
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && SlashOverlay)
	{
		// Soul을 먹었을 때 -> 캐릭터 Health 회복
		Attributes->AddHealth(Soul->GetRecovery());
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	Tags.Add(FName("EngageableTarget"));

	InitializeSlashOverlay();
	MaxCombo = AttackMontageSections.Num() - 1;
}

void ASlashCharacter::Attack()
{
	if (IsOccupied() || IsFalling()) return;

	ClearResetComboTimer();

	Super::Attack();
	if (CanAttack())
	{
		ActionState = EActionState::EAS_Attacking;

		// True -> 반격
		// False -> 일반 공격
		bCanCounter ? Counter() : PlayAttackMontage();

		// CurrentCombo == MaxCombo일 때, CurrentCombo를 0으로 초기화
		// CurrentCombo != MaxCombo일 때, CurrentCombo에 1 더하기
		CurrentCombo = (CurrentCombo == MaxCombo) ? 0 : FMath::Clamp(CurrentCombo + 1, 0, MaxCombo);
	}
}

void ASlashCharacter::Block()
{
	if (IsOccupied() || IsFalling() || !HasEnoughStamina(StartBlockCost) || CharacterState == ECharacterState::ECS_Unequipped) return;

	bCanParry = true;

	// 막기 중일 때, CapsuleComponent를 WorldDynamic 타입으로 변경하기 
	// -> 적의 Weapon의 BoxComponent와 오버랩하기 위해 
	// -> 적 Weapon이 캐릭터 Mesh와 오버랩할 때, 방향 에러가 많이 발생
	GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	PlayBlockMontage();
	ActionState = EActionState::EAS_Blocking;
	UseStaminaCost(StartBlockCost);

	// 막기 했을 때, 기존 스태미나 회복율을 막기 중 스태미나 회복율로 바꾸기
	ClearStaminaRegenTimer();
	if (Attributes)
	{
		StaminaRegenRate = Attributes->GetBlockingStaminaRegenRate();
	}
}

int32 ASlashCharacter::PlayAttackMontage()
{
	PlayMontageSection(AttackMontage, AttackMontageSections[CurrentCombo]);
	return CurrentCombo;
}

void ASlashCharacter::MoveForward(float Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;

	if (Controller && (Value != 0))
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}

	FrontValue = Value;
}

void ASlashCharacter::MoveRight(float Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;

	if (Controller && (Value != 0))
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}

	RightValue = Value;
}

void ASlashCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void ASlashCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void ASlashCharacter::EKeyPressed()
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		EquipWeapon(OverlappingWeapon);
	}
	else
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();
		}
	}
}

void ASlashCharacter::Dodge()
{
	if (IsOccupied() || IsFalling() || !HasEnoughStamina(DodgeCost)) return;

	ActionState = EActionState::EAS_Dodge;

	PlayDodgeMontage();
	UseStaminaCost(DodgeCost);
}

void ASlashCharacter::BlockEnd()
{
	bBlockAttack = false;
	GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	StopBlockMontage();
	ActionState = Attributes->GetHealth() > 0.f ? EActionState::EAS_Unoccupied : EActionState::EAS_Dead;
	
	// StaminaRegenTimer가 끝나면, DefaultStaminaRegenRate로 바꾸기
	SetDefaultStaminaRegenTimer();
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;

	// 공격이 끝나면, ResetComboTime까지 콤보 가능
	SetResetComboTimer();
}

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	CharacterState = ECharacterState::ECS_EquippedWeapon;
	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;
}

bool ASlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped && 
		EquippedWeapon;
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ECS_EquippedWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::Die()
{
	Super::Die();

	ActionState = EActionState::EAS_Dead;
	IsGameFinished = true;
	DisableMeshCollision();
}

bool ASlashCharacter::HasEnoughStamina(const float& Cost)
{
	return Attributes && (Attributes->GetStamina() > Cost);
}

bool ASlashCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

void ASlashCharacter::PlayBlockAttackSound(const FVector& ImpactPoint)
{
	if (BlockAttackSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BlockAttackSound,
			ImpactPoint);
	}
}

void ASlashCharacter::PlayParrySound(const FVector& ImpactPoint)
{
	if (ParrySound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ParrySound,
			ImpactPoint);
	}
}

void ASlashCharacter::Counter()
{
	bCanParry ? PlayParryingCounterMontage() : PlayBlockingCounterMontage();
}

void ASlashCharacter::ParryEnd()
{
	bCanParry = false;
}

void ASlashCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::SetVariablesByAttribute()
{
	// SlashCharacter의 변수와 Attribute의 변수와 일치시키기
	if (Attributes)
	{
		DodgeCost = Attributes->GetDodgeCost();
		StartBlockCost = Attributes->GetStartBlockCost();
		StaminaRegenRate = Attributes->GetDefaultStaminaRegenRate();
		StartStaminaRegenTime = Attributes->GetStartStaminaRegenTime();
		BlockAttackCost = Attributes->GetBlockAttackCost();
	}
}

void ASlashCharacter::UpdateSlashOverlay(float DeltaTime)
{
	// SlashOverlay의 Widget을 매 Tick마다 업데이트
	if (Attributes && SlashOverlay)
	{
		Attributes->RegenStamina(DeltaTime, StaminaRegenRate);
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

bool ASlashCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::IsFalling()
{
	return GetCharacterMovement()->IsFalling();
}


void ASlashCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();

			// SlashOverlay의 Widget 값들 초기화
			if (SlashOverlay && Attributes)
			{
				SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				SlashOverlay->SetStaminaBarPercent(1.f);
			}
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::UseStaminaCost(const float& StaminaCost)
{
	if (SlashOverlay)
	{
		Attributes->UseStamina(StaminaCost);
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::ChangeStaminaRegenRateBlockingToDefault()
{
	if (Attributes)
	{
		StaminaRegenRate = Attributes->GetDefaultStaminaRegenRate();
	}
}

void ASlashCharacter::ChangebCanCounter()
{
	bCanCounter = false;
}

void ASlashCharacter::SetDefaultStaminaRegenTimer()
{
	GetWorldTimerManager().SetTimer(StaminaRegenTimer, this, &ASlashCharacter::ChangeStaminaRegenRateBlockingToDefault, StartStaminaRegenTime);
}

void ASlashCharacter::ClearStaminaRegenTimer()
{
	GetWorldTimerManager().ClearTimer(StaminaRegenTimer);
}

void ASlashCharacter::SetCanCounterTimer()
{
	GetWorldTimerManager().SetTimer(CanCounterTimer, this, &ASlashCharacter::ChangebCanCounter, CanCounterTime);
}

void ASlashCharacter::SetResetComboTimer()
{
	GetWorldTimerManager().SetTimer(ResetComboTimer, this, &ASlashCharacter::ResetCurrentCombo, ResetComboTime);
}

void ASlashCharacter::ClearResetComboTimer()
{
	GetWorldTimerManager().ClearTimer(ResetComboTimer);
}

void ASlashCharacter::ResetCurrentCombo()
{
	CurrentCombo = 0;
}

void ASlashCharacter::ParryingSuccess(AActor* EnemyWeapon)
{
	PlayParrySound(EnemyWeapon->GetActorLocation());
	ChangeStaminaRegenRateBlockingToDefault();
	
	// 적이 패링 당했을 때 Attack Animation 중지시키기
	IHitInterface* HitInterface = Cast<IHitInterface>(EnemyWeapon->GetOwner());
	if (HitInterface)
	{
		HitInterface->Execute_GetHit(EnemyWeapon->GetOwner(), EnemyWeapon->GetActorLocation(), this);
	}
}

void ASlashCharacter::BlockingSuccess(AActor* EnemyWeapon)
{
	PlayBlockAttackSound(EnemyWeapon->GetActorLocation());

	LaunchBlockingCharacter();
}

void ASlashCharacter::LaunchBlockingCharacter()
{
	FVector LaunchDir = -1 * GetActorForwardVector();
	FVector LaunchVelocity = 100 * LaunchDir;
	LaunchCharacter(LaunchVelocity, false, false);
}
