// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestSystem/QuestDetails.h"
#include "QuestSystem/ObjectiveDetails.h"
#include "Quest.generated.h"

UCLASS()
class SLASH_API AQuest : public AActor
{
	GENERATED_BODY()
	
public:	
	AQuest();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FName QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FQuestDetails QuestDetails;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FObjectiveDetails ObejctiveDetails;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	int32 CurrentObejctiveProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	bool IsCompleted;
};
