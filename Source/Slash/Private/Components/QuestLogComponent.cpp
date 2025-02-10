// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/QuestLogComponent.h"
#include "QuestSystem/Quest.h"

UQuestLogComponent::UQuestLogComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UQuestLogComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UQuestLogComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UQuestLogComponent::AddNewQuest(const FName QuestID)
{
	UWorld* World = GetWorld();
	if (World && QuestClass)
	{
		CurrentActiveQuests.AddUnique(QuestID);
		AQuest* CurrentQuest = World->SpawnActor<AQuest>(QuestClass);
		CurrentQuests.Add(CurrentQuest);
	}
}

void UQuestLogComponent::CompleteQuest()
{

}

bool UQuestLogComponent::QueryActiveQuest(const FName QuestID)
{
	return CurrentActiveQuests.Contains(QuestID);
}

void UQuestLogComponent::TrackQuest()
{

}
