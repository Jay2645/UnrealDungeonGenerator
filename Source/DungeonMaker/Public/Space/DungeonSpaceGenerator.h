// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "BSPLeaf.h"
#include "DungeonSpaceGenerator.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DUNGEONMAKER_API UDungeonSpaceGenerator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDungeonSpaceGenerator();

	UBSPLeaf* RootLeaf;
	UBSPLeaf* StartLeaf;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	const UDungeonTile* DefaultRoomTile;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	const UDungeonMissionSymbol* HallwaySymbol;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MaxGeneratedRooms;
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	bool bDebugDungeon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoomReplacements> PreGenerationRoomReplacementPhases;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRoomReplacements> PostGenerationRoomReplacementPhases;

	// The maximum size of any room in this dungeon, in meters.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MaxRoomSize = 24;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TSet<UBSPLeaf*> MissionLeaves;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TSet<ADungeonRoom*> MissionRooms;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TSet<ADungeonRoom*> UnresolvedHooks;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TMap<const UDungeonTile*, UHierarchicalInstancedStaticMeshComponent*> ComponentLookup;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FDungeonFloor DungeonSpace;

public:	
	void CreateDungeonSpace(int32 DungeonSize, UDungeonMissionNode* Head, FRandomStream& Rng);
	void DrawDebugSpace();

protected:
	bool PairNodesToLeaves(UDungeonMissionNode* Node, TSet<FBSPLink>& AvailableLeaves, FRandomStream& Rng, TSet<UDungeonMissionNode*>& ProcessedNodes, TSet<UBSPLeaf*>& ProcessedLeaves, UBSPLeaf* EntranceLeaf, TSet<FBSPLink>& AllOpenLeaves, bool bIsTightCoupling = false);
	FBSPLink GetOpenLeaf(UDungeonMissionNode* Node, TSet<FBSPLink>& AvailableLeaves, FRandomStream& Rng, TSet<UBSPLeaf*>& ProcessedLeaves);
	void DoFloorWideTileReplacement(FDungeonFloor& DungeonFloor, TArray<FRoomReplacements> ReplacementPhases, FRandomStream &Rng);
};