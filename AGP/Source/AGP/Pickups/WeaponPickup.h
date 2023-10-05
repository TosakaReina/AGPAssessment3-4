// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupBase.h"
#include "AGP/WeaponComponent.h"
#include "WeaponPickup.generated.h"

/**
 * 
 */

USTRUCT()
struct FStatRange {
	GENERATED_BODY()
	
	float Min;
	float Max;
};

USTRUCT()
struct FWeaponStatRanges {
	GENERATED_BODY()
	
	FStatRange AccuracyRange;
	FStatRange FireRateRange;
	FStatRange BaseDamageRange;
	FStatRange MagazineSizeRange;
	FStatRange ReloadTimeRange;
};

UENUM(BlueprintType)
enum class EWeaponRarity : uint8
{
	Common,
	Rare,
	Master,
	Legendary
};

UCLASS()
class AGP_API AWeaponPickup : public APickupBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateWeaponPickupMaterial();

	UPROPERTY(BlueprintReadOnly)
	EWeaponRarity WeaponRarity = EWeaponRarity::Common;
	FWeaponStats WeaponStats;
	
	virtual void OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& HitInfo) override;

private:
	void GenerateWeaponPickup();

	static float GetRandomInRange(const FStatRange& Range);
	static int32 GenerateRandomIntInRange(const FStatRange& Range); 
	void AssignRandomStats(const FWeaponStatRanges& GoodRange, const FWeaponStatRanges& BadRange, int32 GoodCount);
	
};
