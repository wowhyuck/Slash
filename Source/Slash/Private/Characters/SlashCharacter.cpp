// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"
#include "Animation/AnimMontage.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AttributeComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Items/Item.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/Weapons/Weapon.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"

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

	if (Attributes)
	{
		DodgeCost = Attributes->GetDodgeCost();
		StartBlockCost = Attributes->GetStartBlockCost();
		StaminaRegenRate = Attributes->GetDefaultStaminaRegenRate();
	}
}

void ASlashCharacter::Tick(float DeltaTime)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->RegenStamina(DeltaTime, StaminaRegenRate);
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	bBlockAttack = IsFront(DamageCauser->GetActorLocation()) && ActionState == EActionState::EAS_Blocking;
	
	// 공격 막기 성공했을 때 -> Weapon Location = ImpactPoint 해서 bBlockAttack = true로 만들기
	if (bBlockAttack)
	{
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
		PlayBlockReactMontage();
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
		Attributes->AddHealth(Soul->GetRecovery());
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGold(Attributes->GetGold());
	}
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	Tags.Add(FName("EngageableTarget"));

	InitializeSlashOverlay();
}

void ASlashCharacter::Attack()
{
	if (IsOccupied() || IsFalling()) return;

	Super::Attack();
	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::Block()
{
	if (IsOccupied() || IsFalling() || !HasEnoughStamina(StartBlockCost) || CharacterState != ECharacterState::ECS_EquippedOneHandedWeapon) return;

	GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	ClearStaminaRegenTimer();
	PlayBlockMontage();
	ActionState = EActionState::EAS_Blocking;
	UseStaminaCost(StartBlockCost);
	if (Attributes)
	{
		StaminaRegenRate = Attributes->GetBlockingStaminaRegenRate();
	}
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

	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;

	UseStaminaCost(DodgeCost);
}

void ASlashCharacter::BlockEnd()
{
	bBlockAttack = false;
	GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	StopBlockMontage();
	ActionState = Attributes->GetHealth() > 0.f ? EActionState::EAS_Unoccupied : EActionState::EAS_Dead;
	GetWorldTimerManager().SetTimer(StaminaRegenTimer, this, &ASlashCharacter::ChangeStaminaRegenRateBlockingToDefault, StartStaminaRegenTime);
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
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
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
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
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
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
			if (SlashOverlay && Attributes)
			{
				SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
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


void ASlashCharacter::ClearStaminaRegenTimer()
{
	GetWorldTimerManager().ClearTimer(StaminaRegenTimer);
}