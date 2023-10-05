// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavigationNode.generated.h"

UCLASS()
class AGP_API ANavigationNode : public AActor
{
	GENERATED_BODY()

	friend class UPathfindingSubsystem;
	
public:	
	// Sets default values for this actor's properties
	ANavigationNode();

	virtual bool ShouldTickIfViewportsOnly() const override;
	float GScore;
	float HScore;
	ANavigationNode* CameFrom;
	float FScore() const
	{
		return GScore + HScore;
	}

	//whether is jump point
	UPROPERTY(EditAnywhere, Category = "Jumping")
	bool bIsJumpPoint = false;
	//the jumping destination node
	UPROPERTY(EditAnywhere, Category = "Jumping")
	ANavigationNode* JumpDestination = nullptr;
	UPROPERTY(EditAnywhere, Category = "Cover")
	bool bIsCover = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<ANavigationNode*> ConnectedNodes;
	UPROPERTY(VisibleAnywhere)
	USceneComponent* LocationComponent;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
