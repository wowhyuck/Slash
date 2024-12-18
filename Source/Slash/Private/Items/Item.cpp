// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "DrawDebugHelpers.h"
#include "Slash/DebugMacros.h"
#include "Components/SphereComponent.h"
#include "Characters/SlashCharacter.h"
#include "NiagaraComponent.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = ItemMesh;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());

	EmbersEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Embers"));
	EmbersEffect->SetupAttachment(GetRootComponent());
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
}

float AItem::TransformedSin()
{
	return Amplitude * FMath::Sin(RunningTime * TimeConstant);
}

float AItem::TransformedCosin()
{
	return Amplitude * FMath::Cos(RunningTime * TimeConstant);
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter)
	{
		SlashCharacter->SetOverlappingItem(this);
	}

	//const FString OtherActorName = OtherActor->GetName();

	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red, OtherActorName);
	//}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (SlashCharacter)
	{
		SlashCharacter->SetOverlappingItem(nullptr);
	}

	//const FString OtherActorName = FString("Ending Overlap with: ") + OtherActor->GetName();
	//if (GEngine)
	//{
	//	GEngine->AddOnScreenDebugMessage(1, 30.f, FColor::Red, OtherActorName);
	//}
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	if(ItemState == EItemState::EIS_Hovering)
	{
		AddActorWorldOffset(FVector(0, 0, TransformedSin()));
	}
}

