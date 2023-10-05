// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "EngineUtils.h"
#include "HealthComponent.h"
#include "PlayerCharacter.h"
#include "AGP/Pathfinding/PathfindingSubsystem.h"
#include "AGP/Pickups/WeaponPickup.h"
#include "Perception/PawnSensingComponent.h"

// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>("Pawn Sensing Component");
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	PathfindingSubsystem = GetWorld()->GetSubsystem<UPathfindingSubsystem>();
	if (PathfindingSubsystem)
	{
		CurrentPath = PathfindingSubsystem->GetRandomPath(GetActorLocation());
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to find the PathfindingSubsystem"))
	}
	if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemyCharacter::OnSensedPawn);
	}
}

void AEnemyCharacter::MoveAlongPath()
{
	// Execute the path. Should be called each tick.

	// If the path is empty do nothing.
	if (CurrentPath.IsEmpty()) return;
	
	// 1. Move towards the current stage of the path.
	//		a. Calculate the direction from the current position to the target of the current stage of the path.
	FVector MovementDirection = CurrentPath[CurrentPath.Num()-1] - GetActorLocation();
	MovementDirection.Normalize();
	//		b. Apply movement in that direction.
	AddMovementInput(MovementDirection);
	// 2. Check if it is close to the current stage of the path then pop it off.
	if (FVector::Distance(GetActorLocation(), CurrentPath[CurrentPath.Num() - 1]) < PathfindingError)
	{
		CurrentPath.Pop();
	}
}

void AEnemyCharacter::TickPatrol()
{
	if (CurrentPath.IsEmpty())
	{
		if(!HasWeapon())
		{
			if (const AWeaponPickup* NearestWeapon = FindNearestWeapon())
			{
				UE_LOG(LogTemp, Warning, TEXT("X=%f, Y=%f"), NearestWeapon->GetActorLocation().X, NearestWeapon->GetActorLocation().Y)
				CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), NearestWeapon->GetActorLocation());
			}
			CurrentPath = PathfindingSubsystem->GetRandomPath(GetActorLocation());
		}else
		{
			CurrentPath = PathfindingSubsystem->GetRandomPath(GetActorLocation());
		}
	}
	MoveAlongPath();
}

void AEnemyCharacter::TickEngage()
{
	
	if (!SensedCharacter) return;
	
	if (CurrentPath.IsEmpty())
	{
		CurrentPath = PathfindingSubsystem->GetPath(GetActorLocation(), SensedCharacter->GetActorLocation());
	}
	if(WeaponComponent && NeedsReloading())
	{
		WeaponComponent->Reload();
	}
	MoveAlongPath();
	Fire(SensedCharacter->GetActorLocation());
}

void AEnemyCharacter::TickEvade()
{
	// Find the player and return if it can't find it.
	if (!SensedCharacter) return;

	if (CurrentPath.IsEmpty())
	{
		if(HealthComponent->GetCurrentHealthPercentage() <= 0.2f)
		{
			CurrentPath = PathfindingSubsystem->GetPathAway(GetActorLocation(), SensedCharacter->GetActorLocation());
		}else // 0.2f < health percent <= 0.4f. Find Cover
		{
			CurrentPath = PathfindingSubsystem->GetPathToBestCover(GetActorLocation(), SensedCharacter->GetActorLocation());
			if(WeaponComponent && NeedsReloading())
			{
				WeaponComponent->Reload();
			}
			Fire(SensedCharacter->GetActorLocation());
		}
	}
	MoveAlongPath();
}

void AEnemyCharacter::OnSensedPawn(APawn* SensedActor)
{
	if (APlayerCharacter* Player = Cast<APlayerCharacter>(SensedActor))
	{
		SensedCharacter = Player;
		// UE_LOG(LogTemp, Display, TEXT("Sensed Player"))
	}
}

void AEnemyCharacter::UpdateSight()
{
	if (!SensedCharacter) return;
	if (PawnSensingComponent)
	{
		if (!PawnSensingComponent->HasLineOfSightTo(SensedCharacter))
		{
			SensedCharacter = nullptr;
			UE_LOG(LogTemp, Display, TEXT("Lost Player"))
		}
	}
}


// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSight();
	
	switch(CurrentState)
	{
	case EEnemyState::Patrol:
		TickPatrol();
		if (SensedCharacter)
		{
			if (HealthComponent->GetCurrentHealthPercentage() >= 0.4f && HasWeapon())
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Engage;
			} else if(HealthComponent->GetCurrentHealthPercentage() < 0.4f)
			{
				CurrentPath.Empty();
				CurrentState = EEnemyState::Evade;
			}
			//If health percent > 0.4 but do not have weapon, keep patrol to find weapon
		}
		break;
	case EEnemyState::Engage:
		TickEngage();
		if (HealthComponent->GetCurrentHealthPercentage() < 0.4f)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Evade;
		} else if (!SensedCharacter)
		{
			CurrentState = EEnemyState::Patrol;
		}
		break;
	case EEnemyState::Evade:
		TickEvade();
		if (HealthComponent->GetCurrentHealthPercentage() >= 0.4f)
		{
			CurrentPath.Empty();
			CurrentState = EEnemyState::Engage;
		} else if (!SensedCharacter)
		{
			CurrentState = EEnemyState::Patrol;
		}
		break;
	}
}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool AEnemyCharacter::NeedsReloading() const
{
	if(WeaponComponent)
	{
		if(WeaponComponent->RoundsRemaining() <= 0)
		{
			return true;
		}
	}
	return false;
}

APlayerCharacter* AEnemyCharacter::FindPlayer() const
{
	APlayerCharacter* Player = nullptr;
	for (TActorIterator<APlayerCharacter> It(GetWorld()); It; ++It)
	{
		Player = *It;
		break;
	}
	if (!Player)
	{
		UE_LOG(LogTemp, Error, TEXT("Unable to find the Player Character in the world."))
	}
	return Player;
}

AWeaponPickup* AEnemyCharacter::FindNearestWeapon() const
{
	AWeaponPickup* NearestWeapon = nullptr;
	float MinDistance = UE_MAX_FLT;
	for (TActorIterator<AWeaponPickup> It(GetWorld()); It; ++It)
	{
		if(const float Distance = FVector::Distance(GetActorLocation(), It->GetActorLocation()) < MinDistance)
		{
			MinDistance = Distance;
			NearestWeapon = *It;
		}
	}
	return NearestWeapon;
}





