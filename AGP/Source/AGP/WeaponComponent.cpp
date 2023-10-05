// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"

#include "Characters/BaseCharacter.h"
#include "Characters/HealthComponent.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	TimeSinceLastShot += DeltaTime;
}

bool UWeaponComponent::Fire(const FVector& BulletStart, const FVector& FireAtLocation)
{
	// Determine if the character is able to fire.
	if (TimeSinceLastShot < WeaponStats.FireRate)
	{
		return false;
	}
	RoundsRemainingInMagazine -= 1;
	if(RoundsRemainingInMagazine <= 0)
	{
		return false;
	}
	if(bIsReloading)
	{
		return false;
	}

	// Optional Component
	// FVector FireDirection = (FireAtLocation - BulletStart).GetSafeNormal();
	// FVector RandomDirection = FMath::VRand();
	// FireDirection = FMath::Lerp(RandomDirection, FireDirection, WeaponStats.Accuracy);
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	if (GetWorld()->LineTraceSingleByChannel(HitResult, BulletStart, FireAtLocation, ECC_WorldStatic, QueryParams))
	{
		if (ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(HitResult.GetActor()))
		{
			if (UHealthComponent* HitCharacterHealth = HitCharacter->GetComponentByClass<UHealthComponent>())
			{
				HitCharacterHealth->ApplyDamage(WeaponStats.BaseDamage);
				
			}
			DrawDebugLine(GetWorld(), BulletStart, HitResult.ImpactPoint, FColor::Green, false, 1.0f);
		}
		else
		{
			DrawDebugLine(GetWorld(), BulletStart, HitResult.ImpactPoint, FColor::Orange, false, 1.0f);
		}
		
	}
	else
	{
		DrawDebugLine(GetWorld(), BulletStart, FireAtLocation, FColor::Red, false, 1.0f);
	}

	TimeSinceLastShot = 0.0f;
	return true;
}

void UWeaponComponent::SetWeaponStats(const FWeaponStats& NewWeaponStats)
{
	WeaponStats = NewWeaponStats;
	RoundsRemainingInMagazine = WeaponStats.MagazineSize;
}

void UWeaponComponent::Reload()
{
	if (!bIsReloading)
	{
		bIsReloading = true;
		GetWorld()->GetTimerManager().SetTimer(ReloadTimerHandle, this, &UWeaponComponent::FinishReloading, WeaponStats.ReloadTime, false);
		UE_LOG(LogTemp, Warning, TEXT("Reloading"))
	}
}

void UWeaponComponent::FinishReloading()
{
	RoundsRemainingInMagazine = WeaponStats.MagazineSize;
	bIsReloading = false;
	UE_LOG(LogTemp, Warning, TEXT("Finish Reloading"))
}

int32 UWeaponComponent::RoundsRemaining() const
{
	return RoundsRemainingInMagazine;
}

