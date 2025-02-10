// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestLogComponent.generated.h"

class AQuest;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API UQuestLogComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UQuestLogComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void AddNewQuest(const FName QuestID);
	void CompleteQuest();
	bool QueryActiveQuest(const FName QuestID);
	void TrackQuest();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere, Category = "Quest")
	TArray<FName> CurrentActiveQuests;

	UPROPERTY(EditAnywhere, Category = "Quest")
	TArray<FName> CompletedQuests;

	UPROPERTY(EditAnywhere, Category = "Quest")
	FName CurrentTracktedQuest;

	UPROPERTY(EditAnywhere, Category = "Quest")
	TArray<AQuest*> CurrentQuests;

	UPROPERTY(EditAnywhere, Category = "Quest")
	TSubclassOf<AQuest> QuestClass;
};
