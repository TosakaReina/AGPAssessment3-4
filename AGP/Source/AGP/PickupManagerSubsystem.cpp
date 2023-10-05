// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupManagerSubsystem.h"

#include "AGPGameInstance.h"
#include "Pathfinding/PathfindingSubsystem.h"



void UPickupManagerSubsystem::PopulateSpawnLocations()
{
	PossibleSpawnLocations.Empty();
	PossibleSpawnLocations = GetWorld()->GetSubsystem<UPathfindingSubsystem>()->GetWaypointPositions();
}

void UPickupManagerSubsystem::SpawnWeaponPickup()
{
	//make sure the PossibleSpawnLocations array is not empty
	if (PossibleSpawnLocations.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to spawn weapon pickup."))
		return;
	}
	
	//make sure there is a valid UAGPGameInstance
	if (const UAGPGameInstance* GameInstance =
	 GetWorld()->GetGameInstance<UAGPGameInstance>())
	{
		//randomly choose a spawn position
		FVector SpawnPosition =
		PossibleSpawnLocations[FMath::RandRange(0, PossibleSpawnLocations.Num()-
	   1)];
		PossibleSpawnLocations.Remove(SpawnPosition);
		//modify Z value
		SpawnPosition.Z += 50.0f;
		//spawn weapon pickup instance using SpawnActor function with param: class, location, rotation
		AWeaponPickup* Pickup = GetWorld()->SpawnActor<AWeaponPickup>(
		GameInstance->GetWeaponPickupClass(),SpawnPosition,FRotator::ZeroRotator);
		UE_LOG(LogTemp, Display, TEXT("Weapon Pickup Spawned"))
	}
}

void UPickupManagerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(PossibleSpawnLocations.IsEmpty())
	{
		PopulateSpawnLocations();
	}
	
	TimeSinceLastSpawn += DeltaTime;
	if(TimeSinceLastSpawn >= PickupSpawnRate)
	{
		SpawnWeaponPickup();
		TimeSinceLastSpawn = 0.0f;
	}
}
