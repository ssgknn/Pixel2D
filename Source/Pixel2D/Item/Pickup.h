// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class PIXEL2D_API APickup : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickup();

	//Takes the item to represent and creates the pickup from it. Done on BeginPlay and when a player drops an item on the ground.
	void InitializePickup(const TSubclassOf<class UItem> ItemClass, const int32 Quantity);

	/**Align pickups rotation with ground rotation. */
	UFUNCTION(BlueprintImplementableEvent)
	void AlignWithGround();

	//This is used as a template to create the pickup when spawned in
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Instanced)
	class UItem* ItemTemplate;


protected:

	//The item that will be added to the inventory when this pickup is taken
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, ReplicatedUsing = OnRep_Item)
	class UItem* Item;

	UFUNCTION()
	void OnRep_Item();

	/**If some property on the item is modified, we bind this to OnItemModified and refresh the UI if the item gets modified.*/
	UFUNCTION()
	void OnItemModified();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif 

	//Called when a player takes the pickup
	UFUNCTION()
	void OnTakePickup(class APlayerCharacter* Taker);

	/** A sphere to detect pickups*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class USphereComponent* PickupSphereComponent;

	UPROPERTY()
	class UPaperSpriteComponent* SpriteComponent;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UPaperSprite* PickupSprite;*/

	/*UPROPERTY(EditDefaultsOnly, Category = "Components")
	class UInteractionComponent* InteractionComponent;*/
};
