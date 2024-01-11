// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperTileMapComponent.h"
#include "WorldHandler.generated.h"

UCLASS()
class PIXEL2D_API AWorldHandler : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldHandler();

protected:

	UPROPERTY()
	int ChunkSize;
	
	UPROPERTY()
	FVector CharacterPosition;

	//Chunk refresh bounsary limits
	UPROPERTY()
	double Boundary_L;

	UPROPERTY()
	double Boundary_R;

	UPROPERTY()
	double Boundary_U;

	UPROPERTY()
	double Boundary_D;
		

	UPROPERTY()
	TArray<UPaperTileMapComponent*> TileMapComponent;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tilesets")
	UPaperTileSet* TileSet1;

	UPROPERTY()
	class AZDPlayerCharacterBase* CharacterRef;

private:
	static FName TileSetName1;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void LoadChunks();

	UFUNCTION()
	void RefreshChunks(int direction);

	UFUNCTION()
	void RemoveBlockByIndex(int32 index);

	UFUNCTION()
	void AddBlockByVector(FVector& vector);

};
