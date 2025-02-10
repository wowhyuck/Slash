// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Interfaces/InteractionInterface.h"
#include "QuestGiverComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API UQuestGiverComponent : public UActorComponent, public IInteractionInterface
{
	GENERATED_BODY()

public:	
	UQuestGiverComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:
	virtual void BeginPlay() override;

	void DisplayQuest();

	virtual FString InteractWith_Implementation() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FDataTableRowHandle QuestData;
};
