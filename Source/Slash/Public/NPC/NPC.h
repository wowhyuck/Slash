// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Characters/BaseCharacter.h"
#include "NPC.generated.h"

class UQuestGiverComponent;

UCLASS()
class SLASH_API ANPC : public ABaseCharacter
{
	GENERATED_BODY()

public:
	ANPC();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

private:	
};
