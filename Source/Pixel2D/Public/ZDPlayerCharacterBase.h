// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "InputActionValue.h"
#include "ZDPlayerCharacterBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItemsChanged, const EEquippableSlot, Slot, const UEquippableItem*, Item);

/**
 * 
 */
UCLASS()
class PIXEL2D_API AZDPlayerCharacterBase : public APaperZDCharacter
{
	GENERATED_BODY()

public:
	AZDPlayerCharacterBase();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


protected:

	virtual void BeginPlay();

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/** Called for Clicking input */
	void PrimaryClick(const FInputActionValue& Value);

	/** Called for Inventory open/close input */
	void InventoryOpenClose(const FInputActionValue& Value);

public:

	// Items
	UFUNCTION(BlueprintCallable, Category = "Items")
	void UseItem(class UItem* Item);

	/**[Server] Use an item from our inventory.*/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UseItem(class UItem* Item);

	UFUNCTION(BlueprintCallable, Category = "Items")
	void DropItem(class UItem* Item, const int32 Quantity);

	/**[Server] Drop an item*/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DropItem(class UItem* Item, const int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Items")
	void ReorderPlayerItems(int idxAt, int newIdx);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ReorderPlayerItems(int idxAt, int newIdx);

	UFUNCTION()
	void ItemAddedToInventory(class UItem* Item);

	UFUNCTION()
	void ItemRemovedFromInventory(class UItem* Item);

	/**Handle equipping an equippable item*/
	bool EquipItem(class UEquippableItem* Item);
	bool UnEquipItem(class UEquippableItem* Item);

	/**These should never be called directly - UGearEquippableItem and UWeaponItem call these on top of EquipItem*/
	void EquipGear(class UGearEquippableItem* Gear);
	void UnEquipGear(const EEquippableSlot Slot);
	void EquipWeapon(class UWeaponItem* WeaponItem);
	void UnEquipWeapon();

	//Modify the players health by either a negative or positive amount. Return the amount of health actually removed
	float ModifyHealth(const float Delta);

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION(BlueprintImplementableEvent)
	void OnHealthModified(const float HealthDelta);

	UFUNCTION(BlueprintCallable)
	void SetLootSource(class UInventoryComponent* NewLootSource);

	UFUNCTION(BlueprintCallable, Category = "Looting")
	void LootItem(class UItem* ItemToGive);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_LootItem(class UItem* ItemToLoot);

	UFUNCTION(BlueprintPure, Category = "Looting")
	bool IsLooting() const;

	/**Called to update the inventory*/
	UPROPERTY(BlueprintAssignable, Category = "Items")
	FOnEquippedItemsChanged OnEquippedItemsChanged;

	/**Return the skeletal mesh component for a given slot*/
	UMaterialInstanceDynamic* GetSlotMaterialInstance(const EEquippableSlot Slot);

	UFUNCTION(BlueprintPure)
	FORCEINLINE TMap<EEquippableSlot, UEquippableItem*> GetEquippedItems() const { return EquippedItems; }

	//UFUNCTION(BlueprintCallable, Category = "Weapons")
	//FORCEINLINE class AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	/**needed this because the pickups use a blueprint base class*/
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<class APickup> PickupClass;

protected:

	//Begin being looted by a player
	UFUNCTION()
	void BeginLootingPlayer(class AZDPlayerCharacterBase* Character);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable)
	void ServerSetLootSource(class UInventoryComponent* NewLootSource);

	/**The inventory that we are currently looting from. */
	UPROPERTY(ReplicatedUsing = OnRep_LootSource, BlueprintReadOnly)
	UInventoryComponent* LootSource;

	UFUNCTION()
	void OnLootSourceOwnerDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void OnRep_LootSource();
	

	UFUNCTION()
	void OnRep_CharacterRotation();

	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION(Server, unreliable)
	void Server_SetCharacterRotation(uint8 NewRotation);

public:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** A capsule to detect pickups*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UCapsuleComponent* PickupCapsuleComponent;

	/** Our players inventory*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UInventoryComponent* PlayerInventory;

	/** Our players inventory*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* PlayerMesh;

	//The players body meshes.
	UPROPERTY(BlueprintReadOnly, Category = Mesh)
	TMap<EEquippableSlot, UMaterialInstanceDynamic*> PlayerMaterials;

	UPROPERTY(BlueprintReadOnly, Category = Mesh)
	TMap<EEquippableSlot, UMaterialInstance*> PlayerDefaultMaterials;

	UPROPERTY()
	UMaterialInstanceDynamic* MI_Body;

	UPROPERTY()
	UMaterialInstanceDynamic* MI_HeadGear;

	UPROPERTY()
	UMaterialInstanceDynamic* MI_Hair;

	UPROPERTY()
	UMaterialInstanceDynamic* MI_BodyGear;
	
	UPROPERTY()
	UMaterialInstanceDynamic* MI_LegGear;

	UPROPERTY()
	UMaterialInstanceDynamic* MI_FootGear;

	UPROPERTY()
	UMaterialInstanceDynamic* MI_Emote;

	UPROPERTY()
	UMaterialInstanceDynamic* MI_BackPack;

	
protected:

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "Health")
	float Health;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth;

	//Allows for efficient access of equipped items
	UPROPERTY(VisibleAnywhere, Category = "Items")
	TMap<EEquippableSlot, UEquippableItem*> EquippedItems;

	/*UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;*/

	UPROPERTY(ReplicatedUsing = OnRep_CharacterRotation)
	uint8 CharacterRotation;

	
//Chunk
public:

	UFUNCTION()
	void CalculateChunkModification(FPlacementData placementData);


	UFUNCTION(Server, Reliable)
	void Server_RequestRegionUpdate(const TArray<FChunkChangeData>& chunksToUpdate);

	//UFUNCTION(Server, reliable)
	//void Server_ModifyChunks(FPlacementData placementData);

	UPROPERTY()
	class AWorldHandler* WorldHandlerRef;

	//void PrintMousePositionToWorld();

#pragma region Input
private:

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* PrimaryClickAction;

	/** Open/Close the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* InventoryAction;

#pragma endregion Input

#pragma region PlayerController

public:

	UFUNCTION(Client, Reliable, BlueprintCallable)
	void ClientShowNotification(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowNotification_BP(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void ShowInGameUI();

#pragma endregion PlayerController
	
};
