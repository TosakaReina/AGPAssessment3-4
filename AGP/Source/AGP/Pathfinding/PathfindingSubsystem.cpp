// Fill out your copyright notice in the Description page of Project Settings.


#include "PathfindingSubsystem.h"

#include "EngineUtils.h"
#include "NavigationNode.h"

TArray<FVector> UPathfindingSubsystem::GetWaypointPositions()
{
	TArray<FVector> AllNodesWPosition;
	for (ANavigationNode* Node : Nodes)
	{
		AllNodesWPosition.Add(Node->GetActorLocation());
	}
	return AllNodesWPosition;
}

void UPathfindingSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	PopulateNodes();
}

TArray<FVector> UPathfindingSubsystem::GetRandomPath(const FVector& StartLocation)
{
	return GetPath(FindNearestNode(StartLocation), GetRandomNode());
}

TArray<FVector> UPathfindingSubsystem::GetPath(const FVector& StartLocation, const FVector& TargetLocation)
{
	return GetPath(FindNearestNode(StartLocation), FindNearestNode(TargetLocation));
}

TArray<FVector> UPathfindingSubsystem::GetPathAway(const FVector& StartLocation, const FVector& TargetLocation)
{
	return GetPath(FindNearestNode(StartLocation), FindFurthestNode(TargetLocation));
}

TArray<FVector> UPathfindingSubsystem::GetPathToBestCover(const FVector& StartLocation,
	const FVector& TargetLocation)
{
	return GetPath(FindNearestNode(StartLocation), FindBestCoverNode(StartLocation, TargetLocation));
}

void UPathfindingSubsystem::PlaceProceduralNodes(const TArray<FVector>& LandscapeVertexData, int32 MapWidth, int32 MapHeight)
{
	// Need to destroy all of the current nodes in the world.
	RemoveAllNodes();
	
	// Then create and place all the nodes and store them in the ProcedurallyPlacedNodes array.
	for (int Y = 0; Y < MapHeight; Y++)
	{
		for (int X = 0; X < MapWidth; X++)
		{
			// Spawn the node in
			if (ANavigationNode* Node = GetWorld()->SpawnActor<ANavigationNode>())
			{
				Node->SetActorLocation(LandscapeVertexData[Y * MapWidth + X]);
				ProcedurallyPlacedNodes.Add(Node);
			} else
			{
				UE_LOG(LogTemp, Error, TEXT("Unable to spawn a node for some reason. This is bad!"))
			}
			
		}
	}
	// Then add connections between all adjacent nodes.
	for (int Y = 0; Y < MapHeight; Y++)
	{
		for (int X = 0; X < MapWidth; X++)
		{
			if (ANavigationNode* CurrentNode = ProcedurallyPlacedNodes[Y * MapWidth + X]) // Make sure it's a valid ptr.
			{
				// ADD CONNECTIONS:
				// Add Left
				if (X != MapWidth-1)
					CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Y * MapWidth + X+1]);
				// Add Up
				if (Y != MapHeight-1)
					CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[(Y+1) * MapWidth + X]);
				// Add Right
				if (X != 0)
					CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[Y * MapWidth + X-1]);
				// Add Down
				if (Y != 0)
					CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[(Y-1) * MapWidth + X]);
				// Add UpLeft
				if (X != MapWidth-1 && Y != MapHeight-1)
					CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[(Y+1) * MapWidth + X+1]);
				// Add UpRight
				if (X != 0 && Y != MapHeight-1)
					CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[(Y+1) * MapWidth + X-1]);
				// Add DownRight
				if (X != 0 && Y != 0)
					CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[(Y-1) * MapWidth+ X-1]);
				// Add DownLeft
				if (X != MapWidth-1 && Y != 0)
					CurrentNode->ConnectedNodes.Add(ProcedurallyPlacedNodes[(Y-1) * MapWidth + X+1]);
			}
		}
	}
}

ANavigationNode* UPathfindingSubsystem::FindBestCoverNode(const FVector& TargetLocation, const FVector& PlayerLocation)
{
	if (Nodes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("The nodes array is empty."))
		return nullptr;
	}

	// find the best cover.
	ANavigationNode* BestCoverNode = nullptr;
	float BestCoverScore = FLT_MAX;
	for (ANavigationNode* Node : Nodes)
	{
		if(Node->bIsCover)
		{
			//Based on some factors such as distance, angle to the player, etc. to get the best cover
			//Smaller means better
			if (const float Score = CalculateCoverScore(TargetLocation, PlayerLocation, Node); Score < BestCoverScore)
			{
				BestCoverScore = Score;
				BestCoverNode = Node;
			}
		}	
	}
	return BestCoverNode;
}


float UPathfindingSubsystem::CalculateJumpCost(const ANavigationNode* FromNode, const ANavigationNode* ToNode)
{
	const float BasicCost = FVector::Distance(FromNode->GetActorLocation(), ToNode->GetActorLocation());

	const float HeightDifference = ToNode->GetActorLocation().Z - FromNode->GetActorLocation().Z;
	const float HeightCost = FMath::Abs(HeightDifference);

	//Encourage or discourage jumping with appropriate multipliers
	return (BasicCost + HeightCost) * 0.7;
}

float UPathfindingSubsystem::CalculateCoverScore(const FVector& EnemyLocation, const FVector& PlayerLocation, ANavigationNode* CoverNode) const
{
	float Score = 0.0f;

	// Calculate the distance to cover as part of the score
	Score += FVector::Distance(EnemyLocation, CoverNode->GetActorLocation());

	// Get the angle between the cover, AI character and enemy
	FVector ToCover = CoverNode->GetActorLocation() - EnemyLocation;
	FVector ToEnemy = PlayerLocation - EnemyLocation;

	// Normalize the vector to calculate the angle
	ToCover.Normalize();
	ToEnemy.Normalize();

	// Covers with smaller angles (closer to a straight line relationship) will receive smaller ratings
	// because they are more likely to be between the enemy and the AI character
	if (const float AngleBetween = FMath::RadiansToDegrees(acosf(FVector::DotProduct(ToCover, ToEnemy))); AngleBetween < MaxAcceptableAngle)
	{
		Score += AngleWeight * AngleBetween;  //AngleWeight is the weight used to adjust the importance of the angle score
	}
	
	return Score;
}

void UPathfindingSubsystem::PopulateNodes()
{
	Nodes.Empty();

	for (TActorIterator<ANavigationNode> It(GetWorld()); It; ++It)
	{
		Nodes.Add(*It);
		UE_LOG(LogTemp, Warning, TEXT("NODE: %s"), *(*It)->GetActorLocation().ToString())
	}
}

void UPathfindingSubsystem::RemoveAllNodes()
{
	Nodes.Empty();
	ProcedurallyPlacedNodes.Empty();

	for (TActorIterator<ANavigationNode> It(GetWorld()); It; ++It)
	{
		GetWorld()->DestroyActor(*It);
	}
}

ANavigationNode* UPathfindingSubsystem::GetRandomNode()
{
	// Failure condition
	if (Nodes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("The nodes array is empty."))
		return nullptr;
	}
	const int32 RandIndex = FMath::RandRange(0, Nodes.Num()-1);
	return Nodes[RandIndex];
}

ANavigationNode* UPathfindingSubsystem::FindNearestNode(const FVector& TargetLocation)
{
	// Failure condition.
	if (Nodes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("The nodes array is empty."))
		return nullptr;
	}

	// Using the minimum programming pattern to find the closest node.
	// What is the Big O complexity of this? Can you do it more efficiently?
	ANavigationNode* ClosestNode = nullptr;
	float MinDistance = UE_MAX_FLT;
	for (ANavigationNode* Node : Nodes)
	{
		const float Distance = FVector::Distance(TargetLocation, Node->GetActorLocation());
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			ClosestNode = Node;
		}
	}

	return ClosestNode;
}

ANavigationNode* UPathfindingSubsystem::FindFurthestNode(const FVector& TargetLocation)
{
	// Failure condition.
	if (Nodes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("The nodes array is empty."))
		return nullptr;
	}

	// Using the minimum programming pattern to find the closest node.
	// What is the Big O complexity of this? Can you do it more efficiently?
	ANavigationNode* FurthestNode = nullptr;
	float MaxDistance = -1.0f;
	for (ANavigationNode* Node : Nodes)
	{
		const float Distance = FVector::Distance(TargetLocation, Node->GetActorLocation());
		if (Distance > MaxDistance)
		{
			MaxDistance = Distance;
			FurthestNode = Node;
		}
	}

	return FurthestNode;
}

TArray<FVector> UPathfindingSubsystem::GetPath(ANavigationNode* StartNode, ANavigationNode* EndNode)
{
	if (!StartNode || !EndNode)
	{
		UE_LOG(LogTemp, Error, TEXT("Either the start or end node are nullptrs."))
		return TArray<FVector>();
	}

	// Setup the open set and add the start node.
	TArray<ANavigationNode*> OpenSet;
	OpenSet.Add(StartNode);

	// StartNode->GScore = UE_MAX_FLT;
	// StartNode->HScore = FVector::Distance(StartNode->GetActorLocation(), EndNode->GetActorLocation());

	// Setup the maps that will hold the GScores, HScores and CameFrom
	TMap<ANavigationNode*, float> GScores, HScores;
	TMap<ANavigationNode*, ANavigationNode*> CameFrom;
	// You could pre-populate the GScores and HScores maps with all of the GScores (at infinity) and HScores here by looping over
	// all the nodes in the Nodes array. However it is more efficient to only calculate these when you need them
	// as some nodes might not be explored when finding a path.

	// Setup the start nodes G and H score.
	GScores.Add(StartNode, 0);
	HScores.Add(StartNode, FVector::Distance(StartNode->GetActorLocation(), EndNode->GetActorLocation()));
	CameFrom.Add(StartNode, nullptr);

	while (!OpenSet.IsEmpty())
	{
		// Find the node in the open set with the lowest FScore.
		ANavigationNode* CurrentNode = OpenSet[0]; // We know this exists because the OpenSet is not empty.
		for (int32 i = 1; i < OpenSet.Num(); i++)
		{
			// We can be sure that all the nodes in the open set have already had their GScores and HScores calculated.
			if (GScores[OpenSet[i]] + HScores[OpenSet[i]] < GScores[CurrentNode] + HScores[CurrentNode])
			{
				CurrentNode = OpenSet[i];
			}
		}


		// Remove the current node from the OpenSet
		OpenSet.Remove(CurrentNode);

		if (CurrentNode == EndNode)
		{
			// Then we have found the path so reconstruct it and get the positions of each of the nodes in the path.
			// UE_LOG(LogTemp, Display, TEXT("PATH FOUND"))
			return ReconstructPath(CameFrom, EndNode);
		}

		//Set tentative array to store nodes to be explored
		TArray<ANavigationNode*> NodesToExplore = CurrentNode->ConnectedNodes;

		// If the current node is a jumping point, add the jumping target point to the node array to be explored.
		if (CurrentNode->bIsJumpPoint && CurrentNode->JumpDestination)
		{
			NodesToExplore.Add(CurrentNode->JumpDestination);
		}
		
		for (ANavigationNode* ConnectedNode : NodesToExplore)
		{
			if (!ConnectedNode) continue; // Failsafe if the ConnectedNode is a nullptr.

			const float Distance = (ConnectedNode == CurrentNode->JumpDestination) ?
				                       CalculateJumpCost(CurrentNode, ConnectedNode) :
				                       FVector::Distance(CurrentNode->GetActorLocation(), ConnectedNode->GetActorLocation());
			
			// const float TentativeGScore = GScores[CurrentNode] + FVector::Distance(CurrentNode->GetActorLocation(), ConnectedNode->GetActorLocation());
			const float TentativeGScore = GScores[CurrentNode] + Distance;

			
			
			// Because we didn't setup all the scores and came from at the start, we need to check if the connected node has a gscore
			// already otherwise set it. If it doesn't have a gscore then it won't have all the other things either so initialise them as well.
			if (!GScores.Contains(ConnectedNode))
			{
				GScores.Add(ConnectedNode, UE_MAX_FLT);
				HScores.Add(ConnectedNode, FVector::Distance(ConnectedNode->GetActorLocation(), EndNode->GetActorLocation()));
				CameFrom.Add(ConnectedNode, nullptr);
			}

			// Then update this nodes scores and came from if the tentative g score is lower than the current g score.
			if (TentativeGScore < GScores[ConnectedNode])
			{
				CameFrom[ConnectedNode] = CurrentNode;
				GScores[ConnectedNode] = TentativeGScore;
				// HScore is already set when adding the node to the HScores map.
				// Then add connected node to the open set if it isn't already in there.
				if (!OpenSet.Contains(ConnectedNode))
				{
					OpenSet.Add(ConnectedNode);
				}
			}
		}
	}

	// If we get here, then no path has been found so return an empty array.
	return TArray<FVector>();
	
}

TArray<FVector> UPathfindingSubsystem::ReconstructPath(const TMap<ANavigationNode*, ANavigationNode*>& CameFromMap, ANavigationNode* EndNode)
{
	TArray<FVector> NodeLocations;

	const ANavigationNode* NextNode = EndNode;
	while(NextNode)
	{
		NodeLocations.Push(NextNode->GetActorLocation());
		NextNode = CameFromMap[NextNode];
	}

	return NodeLocations;
}

