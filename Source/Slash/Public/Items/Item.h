// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class USphereComponent;


UCLASS()
class SLASH_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sine Parameters")
	float Amplitude = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters")
	float TimeConstant = 5.f;

	UFUNCTION(BlueprintCallable)
	float TransformedSin(float Value);

	UFUNCTION(BlueprintPure)
	float TransformedCosin();

	template<typename T>
	T Avg(T First, T Second);

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float RunningTime;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ItemMesh;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* Sphere;

	// ------ Property Specifiers(프로퍼티 지정자) ----- //
	// // --- UPROPERTY --- //
	// EditDefaultsOnly : can edit a variable in a blueprint only.
	// EditInstanceOnly : can edit a variable in details of instance only.
	// EditAnywhere : can edit a variable in a blueprint and details of instance.
	// VisibleDefaultsOnly : can see a variable in a blueprint only, but you can see its default value.
	// VisibleInstanceOnly : can see a variable in details of instance only, and you can see its value in real time.
	// VisibleAnywhere : can see a variable in a blueprint and details of instance.
	//
	// BlueprintReadOnly : can use a getter of a variable in an Event Graph, but the variable is not private.
	// BlueprintReadWrite : can use a getter and a setter of a variable in a Event Granph, but the variable is not private.
	// 
	// Category : can put a variable to category where you set.
	//
	// meta = (AllowPrivateAccess = "true") : can use a variable with BlueprintReadOnly and BlueprintReadWrite, though the variable is private. 
	// // ----------------- //
	// // --- UFUNCTION --- //
	// BlueprintCallable : The function can be executed in a Blueprint.
	// BlueprintPure : The function does not affect the owning object in any way and can be executed in a Blueprint.
	// ------------------------------------------------ //
};

template<typename T>
inline T AItem::Avg(T First, T Second)
{
	return (First + Second) / 2;
}
