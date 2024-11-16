// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "DrawDebugHelpers.h"
#include "Slash/DebugMacros.h"
#include "Components/SphereComponent.h"
#include "Characters/SlashCharacter.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	RootComponent = ItemMesh;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
	
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	//UE_LOG(LogTemp, Warning, TEXT("Begin Play Called!"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Cyan, FString("Item OnScreen Message!"));
	}

	UWorld* World = GetWorld();

	SetActorLocation(FVector(0.f, 0.f, 50.f));
	SetActorRotation(FRotator(0.f, 45.f, 0.f));

	if (World)
	{
		FVector Location = GetActorLocation();
		//DrawDebugSphere(World, Location, 25.f, 24, FColor::Red, false, 30.f);

		FVector Forward = GetActorForwardVector();
		DrawDebugLine(World, Location, Location + Forward * 100.f, FColor::Red, true, -1.f, 0, 1.f);

		DrawDebugPoint(World, Location + Forward * 100.f, 15.f, FColor::Red, true);
	}

	int32 AvgInt = Avg<int32>(1, 3);
	//UE_LOG(LogTemp, Warning, TEXT("Avg of 1 and 3 : %d"), AvgInt);

	float AvgFloat = Avg<float>(3.45f, 7.86f);
	//UE_LOG(LogTemp, Warning, TEXT("Avg of 3.45 and 7.86 : %f"), AvgFloat);
}

float AItem::TransformedSin(float Value)
{
	return Amplitude * FMath::Sin(Value * TimeConstant);
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

	float DeltaZ = Amplitude * FMath::Sin(DeltaTime * TimeConstant);

	//AddActorWorldOffset(FVector(0, 0, DeltaZ));

	DrawDebugSphere(GetWorld(), GetActorLocation(), 100.f, 12, FColor::Red, false, -1.f);

	//UE_LOG(LogTemp, Warning, TEXT("DeltaTime : %f"), DeltaTime);

	if (GEngine)
	{
		//FString Name = GetName();
		//FString Message = FString::Printf(TEXT("Item Name : %s"), *Name);
		//GEngine->AddOnScreenDebugMessage(1, 60.f, FColor::Cyan, Message);

		//UE_LOG(LogTemp, Warning, TEXT("Item Name : %s"), *Name);

	}
}

