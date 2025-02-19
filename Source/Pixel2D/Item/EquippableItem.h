// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "EquippableItem.generated.h"


//All the slots that gear can be equipped to.
UENUM(BlueprintType)
enum class EEquippableSlot : uint8
{
	EIS_Head UMETA(DisplayName = "Head"),
	EIS_Helmet UMETA(DisplayName = "Helmet"),
	EIS_Chest UMETA(DisplayName = "Chest"),
	EIS_Vest UMETA(DisplayName = "Vest"),
	EIS_Legs UMETA(DisplayName = "Legs"),
	EIS_Feet UMETA(DisplayName = "Feet"),
	EIS_Hands UMETA(DisplayName = "Hands"),
	EIS_Backpack UMETA(DisplayName = "Backpack"),
	EIS_PrimaryWeapon UMETA(DisplayName = "Primary Weapon"),
	EIS_Throwable UMETA(DisplayName = "Throwable Item")
};

/**
 * 
 */
UCLASS(Abstract, NotBlueprintable)
class PIXEL2D_API UEquippableItem : public UItem
{
	GENERATED_BODY()
	
public:

	UEquippableItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippables")
	EEquippableSlot Slot;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Use(class APlayerCharacter* Character) override;

	UFUNCTION(BlueprintCallable, Category = "Equippables")
	virtual bool Equip(class APlayerCharacter* Character);

	UFUNCTION(BlueprintCallable, Category = "Equippables")
	virtual bool UnEquip(class APlayerCharacter* Character);

	virtual bool ShouldShowInInventory() const override;
	virtual void AddedToInventory(class UInventoryComponent* Inventory) override;

	UFUNCTION(BlueprintPure, Category = "Equippables")
	bool IsEquipped() { return bEquipped; };

	/**Call this on the server to equip the item*/
	void SetEquipped(bool bNewEquipped);

protected:

	UPROPERTY(ReplicatedUsing = EquipStatusChanged)
	bool bEquipped;

	UFUNCTION()
	void EquipStatusChanged();
};
