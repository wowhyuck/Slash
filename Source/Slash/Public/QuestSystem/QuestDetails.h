// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ObjectiveDetails.h"
#include "QuestDetails.generated.h"

/**
 * 
 */
class SLASH_API QuestDetails
{
public:
	QuestDetails();
	~QuestDetails();
};

USTRUCT(Atomic, BlueprintType)
struct FQuestDetails : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText LogDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText TrackingDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FObjectiveDetails Objective;
};
