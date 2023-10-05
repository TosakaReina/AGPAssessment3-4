// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponPickup.h"

#include <functional>

#include "../Characters/PlayerCharacter.h"

void AWeaponPickup::BeginPlay()
{
	Super::BeginPlay();

	GenerateWeaponPickup();
	UpdateWeaponPickupMaterial();
}



void AWeaponPickup::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& HitInfo)
{
	//Super::OnPickupOverlap(OverlappedComponent, OtherActor, OtherComponent, OtherBodyIndex, bFromSweep, HitInfo);
	UE_LOG(LogTemp, Display, TEXT("Overlap event occurred on WeaponPickup"))

	if (ABaseCharacter* Player = Cast<ABaseCharacter>(OtherActor))
	{
		// if (!Player->HasWeapon())
		// {
		// 	Player->EquipWeapon(true, WeaponStats);
		// 	Destroy();
		// }else
		// {
		// 	Player->EquipWeapon(true, WeaponStats);
		// }

		Player->EquipWeapon(true, WeaponStats);
		Destroy();
		
	}
}

void AWeaponPickup::GenerateWeaponPickup()
{
	int32 WeaponChance = FMath::RandRange(0, 100);

	if (WeaponChance <= 5) 
	{
		WeaponRarity = EWeaponRarity::Legendary;
	}
	else if (WeaponChance <= 20) 
	{
		WeaponRarity = EWeaponRarity::Master;
	}
	else if (WeaponChance <= 50) 
	{
		WeaponRarity = EWeaponRarity::Rare;
	}
	else 
	{
		WeaponRarity = EWeaponRarity::Common;
	}

	constexpr FWeaponStatRanges GoodRange = {0.98f, 1.0f,
		0.05f, 0.2f,
		15.0f, 30.0f,
		20.0f, 100.0f,
		0.1f, 1.0f};
	constexpr FWeaponStatRanges BadRange = { 0.9f, 0.98f,
		0.2f, 1.0f,
		5.0f, 15.0f,
		1.0f, 19.99f,
		1.0f, 4.0f};
	
	switch (WeaponRarity) {
	case EWeaponRarity::Legendary:
		AssignRandomStats(GoodRange, BadRange, 4);
		break;
	case EWeaponRarity::Master:
		AssignRandomStats(GoodRange, BadRange, 3);
		break;
	case EWeaponRarity::Rare:
		AssignRandomStats(GoodRange, BadRange, 2);
		break;
	case EWeaponRarity::Common:
		AssignRandomStats( BadRange, BadRange, 0); 
		break;
	}
}

float AWeaponPickup::GetRandomInRange(const FStatRange& Range)
{
	return FMath::RandRange(Range.Min, Range.Max);
}

int32 AWeaponPickup::GenerateRandomIntInRange(const FStatRange& Range)
{
	return static_cast<int32>(FMath::RandRange(Range.Min, Range.Max));
}

void AWeaponPickup::AssignRandomStats(const FWeaponStatRanges& GoodRange,
	const FWeaponStatRanges& BadRange, int32 GoodCount)
{
	UE_LOG(LogTemp, Warning, TEXT("AssignRandomStats started."));
	
	//use lambda function to enhance usability. This statAssigners array is used to store multiple lambda functions
	TArray<TFunction<void(const FWeaponStatRanges&)>> StatAssigners;
	
	StatAssigners.Add([&](const FWeaponStatRanges& range) { WeaponStats.Accuracy = GetRandomInRange(range.AccuracyRange); });
	StatAssigners.Add([&](const FWeaponStatRanges& range) { WeaponStats.FireRate = GetRandomInRange(range.FireRateRange); });
	StatAssigners.Add([&](const FWeaponStatRanges& range) { WeaponStats.BaseDamage = GetRandomInRange(range.BaseDamageRange); });
	StatAssigners.Add([&](const FWeaponStatRanges& range) { WeaponStats.MagazineSize = GenerateRandomIntInRange(range.MagazineSizeRange); });
	StatAssigners.Add([&](const FWeaponStatRanges& range) { WeaponStats.ReloadTime = GenerateRandomIntInRange(range.ReloadTimeRange); });

	UE_LOG(LogTemp, Warning, TEXT("%d"), StatAssigners.Num());
	
	StatAssigners.Sort([](const auto& A, const auto& B) { return FMath::RandBool(); });

	UE_LOG(LogTemp, Warning, TEXT("About to call lambda..."));
	for (int32 i = 0; i < StatAssigners.Num(); ++i) {
		if (i < GoodCount) {
			StatAssigners[i](GoodRange);
		} else {
			StatAssigners[i](BadRange);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Lambda call finished."));
}











