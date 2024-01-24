// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class SLASH_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

private:
	float RunningTime;

	//UPROPERTY(EditDefaultsOnly) : can edit variables in a blueprint only
	//UPROPERTY(EditInstanceOnly) : can edit variables in details of instance only
	//UPROPERTY(EditAnywhere) : can edit variables in a blueprint or details of instance both

	UPROPERTY(EditDefaultsOnly)
	float Amplitude = 0.25f;

	UPROPERTY(EditInstanceOnly)
	float TimeConstant = 5.f;
};
