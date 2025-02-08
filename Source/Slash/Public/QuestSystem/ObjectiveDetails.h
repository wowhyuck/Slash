// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestTypes.h"
#include "ObjectiveDetails.generated.h"

/**
 * 
 */
class SLASH_API ObjectiveDetails
{
public:
	ObjectiveDetails();
	~ObjectiveDetails();
};

USTRUCT(Atomic, BlueprintType)
struct FObjectiveDetails
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ObjectiveName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsOptional;
};
