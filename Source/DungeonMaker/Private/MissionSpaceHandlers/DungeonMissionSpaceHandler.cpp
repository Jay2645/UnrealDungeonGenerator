// Fill out your copyright notice in the Description page of Project Settings.

#include "DungeonMissionSpaceHandler.h"
#include "DungeonSpaceGenerator.h"
#include "DungeonMissionSymbol.h"

void UDungeonMissionSpaceHandler::InitializeDungeonFloor(UDungeonSpaceGenerator* SpaceGenerator, TArray<int32> LevelSizes)
{
	DungeonSpaceGenerator = SpaceGenerator;

	DungeonSpaceGenerator->DungeonSpace = FDungeonSpace(LevelSizes);
}

bool UDungeonMissionSpaceHandler::CreateDungeonSpace(UDungeonMissionNode* Head, FIntVector StartLocation,
	int32 SymbolCount, FRandomStream& Rng)
{
	const int32 MAX_ATTEMPTS = 20;
	int32 currentAttempts = 0;
	do
	{
		// Create space for each room on the DungeonFloor
		RoomCount = 0;
		GenerateDungeonRooms(Head, StartLocation, Rng, SymbolCount);

		if (RoomCount != SymbolCount - 1)
		{
			UE_LOG(LogSpaceGen, Warning, TEXT("Room count didn't match symbol count! Rooms: %d, Symbols: %d"), RoomCount, SymbolCount);

			TArray<int32> levelSizes;
			for (int i = 0; i < DungeonSpaceGenerator->DungeonSpace.Num(); i++)
			{
				levelSizes.Add(DungeonSpaceGenerator->DungeonSpace[i].XSize());
			}
			InitializeDungeonFloor(DungeonSpaceGenerator, levelSizes);

			if (currentAttempts < MAX_ATTEMPTS)
			{
				UE_LOG(LogSpaceGen, Error, TEXT("Ran out of attempts to make room count match symbol count! Rooms: %d, Symbols: %d"), RoomCount, SymbolCount);
				return false;
			}
		}
		else
		{
			ProcessRoomNeighbors();
		}

		currentAttempts++;
	} while (RoomCount != SymbolCount - 1);

	return true;
}

void UDungeonMissionSpaceHandler::DrawDebugSpace()
{
	for (int i = 0; i < DungeonSpaceGenerator->DungeonSpace.Num(); i++)
	{
		DungeonSpaceGenerator->DungeonSpace[i].DrawDungeonFloor(GetOwner(), i);
	}
}

TSet<FIntVector> UDungeonMissionSpaceHandler::GetAvailableLocations(FIntVector Location, 
	TSet<FIntVector> IgnoredLocations /*= TSet<FIntVector>()*/)
{
	TSet<FIntVector> availableLocations;

	if (DungeonSpaceGenerator->IsLocationValid(Location) && DungeonSpaceGenerator->DungeonSpace[Location.Z][Location.Y][Location.X].DungeonSymbol.Symbol != NULL)
	{
		UDungeonMissionSymbol* symbol = (UDungeonMissionSymbol*)DungeonSpaceGenerator->DungeonSpace[Location.Z][Location.Y][Location.X].DungeonSymbol.Symbol;
		if (!symbol->bAllowedToHaveChildren)
		{
			// Not allowed to have children; return empty set
			return availableLocations;
		}
	}

	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int z = -1; z <= 1; z++)
			{
				// Don't count ourselves
				if (x == 0 && y == 0 && z == 0)
				{
					continue;
				}
				// Don't count diagonals
				if ((x == 1 || x == -1) && (y == 1 || y == -1))
				{
					continue;
				}
				// Z only counts directly above or directly below
				if ((z == 1 || z == -1) && x != 0 && y != 0)
				{
					continue;
				}
				// We only want to include Z if we have a valid Z neighboring us
				if (z == 1 && Location.Z + 1 < DungeonSpaceGenerator->DungeonSpace.Num() || z == -1 && Location.Z - 1 >= 0)
				{
					continue;
				}

				FIntVector possibleLocation = Location + FIntVector(x, y, z);
				if (IgnoredLocations.Contains(possibleLocation))
				{
					// Ignoring this location
					continue;
				}

				if (!DungeonSpaceGenerator->IsLocationValid(possibleLocation))
				{
					// Out of range
					continue;
				}
				if (DungeonSpaceGenerator->DungeonSpace[possibleLocation.Z][possibleLocation.Y][possibleLocation.X].RoomClass != NULL)
				{
					// Already placed
					continue;
				}

				availableLocations.Add(possibleLocation);
			}
		}
	}
	return availableLocations;
}

FFloorRoom UDungeonMissionSpaceHandler::MakeFloorRoom(UDungeonMissionNode* Node, FIntVector Location, 
	FRandomStream& Rng, int32 TotalSymbolCount)
{
	FFloorRoom room = FFloorRoom();
	UDungeonMissionSymbol* symbol = (UDungeonMissionSymbol*)Node->NodeType;
	room.RoomClass = symbol->GetRoomType(Rng);
	room.Location = Location;
	room.Difficulty = Node->NodeID / (float)TotalSymbolCount;
	room.DungeonSymbol = Node->ToGraphSymbol();
	room.RoomNode = Node;
	room.MaxRoomSize = RoomSize;

	return room;
}

void UDungeonMissionSpaceHandler::SetRoom(FFloorRoom Room)
{
	DungeonSpaceGenerator->SetRoom(Room);
	RoomCount++;
}

void UDungeonMissionSpaceHandler::GenerateDungeonRooms(UDungeonMissionNode* Head, FIntVector StartLocation, FRandomStream &Rng, int32 SymbolCount)
{
	// Empty
}

TKeyValuePair<FIntVector, FIntVector> UDungeonMissionSpaceHandler::GetOpenRoom(UDungeonMissionNode* Node, 
	TMap<FIntVector, FIntVector>& AvailableRooms, FRandomStream& Rng, TSet<FIntVector>& ProcessedRooms)
{
	TSet<UDungeonMissionNode*> nodesToCheck;
	for (UDungeonMakerNode* neighborNode : Node->ChildrenNodes)
	{
		if (neighborNode->bTightlyCoupledToParent)
		{
			nodesToCheck.Add((UDungeonMissionNode*)neighborNode);
		}
	}

	FIntVector roomLocation = FIntVector(-1, -1, -1);
	FIntVector parentLocation = FIntVector(-1, -1, -1);

	do
	{
		if (AvailableRooms.Num() == 0)
		{
			// Out of rooms; return an invalid input
			UE_LOG(LogSpaceGen, Warning, TEXT("Ran out of rooms when trying to place %s"), *Node->ToString(0, false));
			return TKeyValuePair<FIntVector, FIntVector>(FIntVector(-1, -1, -1), FIntVector(-1, -1, -1));
		}
		int32 leafIndex = Rng.RandRange(0, AvailableRooms.Num() - 1);
		TArray<FIntVector> allAvailableRooms;
		AvailableRooms.GetKeys(allAvailableRooms);
		roomLocation = allAvailableRooms[leafIndex];
		parentLocation = AvailableRooms[roomLocation];
		AvailableRooms.Remove(roomLocation);

		if (ProcessedRooms.Contains(roomLocation))
		{
			// Already processed this leaf
			roomLocation = FIntVector(-1, -1, -1);
			parentLocation = FIntVector(-1, -1, -1);
			continue;
		}

		TSet<FIntVector> neighbors = GetAvailableLocations(roomLocation, ProcessedRooms);

		if (neighbors.Num() < nodesToCheck.Num())
		{
			// This leaf wouldn't have enough neighbors to attach all our tightly-coupled nodes
			roomLocation = FIntVector(-1, -1, -1);
			parentLocation = FIntVector(-1, -1, -1);
			continue;
		}
	} while (roomLocation.X == -1 && roomLocation.Y == -1 && roomLocation.Z == -1);
	return TKeyValuePair<FIntVector, FIntVector>(roomLocation, parentLocation);
}

void UDungeonMissionSpaceHandler::ProcessRoomNeighbors()
{
	for (int z = 0; z < DungeonSpaceGenerator->DungeonSpace.Num(); z++)
	{
		for (int y = 0; y < DungeonSpaceGenerator->DungeonSpace[z].YSize(); y++)
		{
			for (int x = 0; x < DungeonSpaceGenerator->DungeonSpace[z].XSize(); x++)
			{
				for (FIntVector neighbor : DungeonSpaceGenerator->DungeonSpace[z][y][x].NeighboringRooms)
				{
					DungeonSpaceGenerator->DungeonSpace[neighbor.Z][neighbor.Y][neighbor.X].NeighboringRooms.Add(FIntVector(x, y, z));
				}
				for (FIntVector neighbor : DungeonSpaceGenerator->DungeonSpace[z][y][x].NeighboringTightlyCoupledRooms)
				{
					DungeonSpaceGenerator->DungeonSpace[neighbor.Z][neighbor.Y][neighbor.X].NeighboringTightlyCoupledRooms.Add(FIntVector(x, y, z));
				}
			}
		}
	}
}