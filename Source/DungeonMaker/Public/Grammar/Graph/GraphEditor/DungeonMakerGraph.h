#pragma once

#include "CoreMinimal.h"
#include "DungeonMakerNode.h"
#include "GraphOutputGrammar.h"
#include "DungeonMakerGraph.generated.h"

#define LOG_WARNING(FMT, ...) UE_LOG(LogTemp, Warning, (FMT), ##__VA_ARGS__)

UCLASS(Blueprintable)
class DUNGEONMAKER_API UDungeonMakerGraph : public UObject
{
	GENERATED_BODY()

public:
	UDungeonMakerGraph();
	virtual ~UDungeonMakerGraph();

	//////////////////////////////////////////////////////////////////////////
	// uproperties
	UPROPERTY(EditAnywhere, Category = "DungeonMaker")
	FString Name;

	UPROPERTY(EditAnywhere, Category = "DungeonMaker")
	TSubclassOf<UDungeonMakerNode> NodeType;

	UPROPERTY(BlueprintReadOnly, Category = "DungeonMaker")
	TArray<UDungeonMakerNode*> RootNodes;

	UPROPERTY(BlueprintReadOnly, Category = "DungeonMaker")
	TArray<UDungeonMakerNode*> AllNodes;

	UPROPERTY(BlueprintReadOnly, Category = "DungeonMaker")
	TMap<int32, UDungeonMakerNode*> NodeIDLookup;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	class UEdGraph* EdGraph;
#endif

	//////////////////////////////////////////////////////////////////////////
	// ufunctions
	UFUNCTION(BlueprintCallable, Category = "DungeonMaker")
	int32 AddNode(UDungeonMakerNode* NodeToAdd);

	int32 UpdateNodeID(UDungeonMakerNode* NodeToUpdate, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "DungeonMaker")
	void Print(bool ToConsole = true, bool ToScreen = true);

	UFUNCTION(BlueprintCallable, Category = "DungeonMaker")
	int GetLevelNum();

	UFUNCTION(BlueprintCallable, Category = "DungeonMaker")
	void GetNodesByLevel(int Level, TArray<UDungeonMakerNode*>& Nodes);

	void ClearGraph();
	void UpdateIDs();
	FString ToString() const;
	int32 Num() const;
};
