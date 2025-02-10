// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/QuestGiverComponent.h"
#include "Components/QuestLogComponent.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UQuestGiverComponent::UQuestGiverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UQuestGiverComponent::BeginPlay()
{
	Super::BeginPlay();

}


// Called every frame
void UQuestGiverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UQuestGiverComponent::DisplayQuest()
{

}

FString UQuestGiverComponent::InteractWith_Implementation()
{
	ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	if (SlashCharacter && SlashCharacter->GetQuestLog())
	{
		bool IsActiveQuest = SlashCharacter->GetQuestLog()->QueryActiveQuest(QuestData.RowName);
		FString ObjectiveID = GetOwner()->GetName();

		if (IsActiveQuest)
		{
			DisplayQuest();
		}
		else
		{
			FString PrintMessage = FString::Printf(TEXT("Already on Quest."));
		}
		return ObjectiveID;
	}

	return FString();
}
