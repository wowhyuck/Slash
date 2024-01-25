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
	//UPROPERTY(VisibleDefaultsOnly) : can see variables in a blueprint only, but you can see its default value
	//UPROPERTY(VisibleInstanceOnly) : can see variables in details of instance only, and you can see its value in real time
	//UPROPERTY(VisibleAnywhere) : can see variables in a blueprint and details of instance

	UPROPERTY(VisibleAnywhere)
	float RunningTime;

	//UPROPERTY(EditDefaultsOnly) : can edit variables in a blueprint only
	//UPROPERTY(EditInstanceOnly) : can edit variables in details of instance only
	//UPROPERTY(EditAnywhere) : can edit variables in a blueprint and details of instance

	UPROPERTY(EditDefaultsOnly)
	float Amplitude = 0.25f;

	UPROPERTY(EditInstanceOnly)
	float TimeConstant = 5.f;
};
