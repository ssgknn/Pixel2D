// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EquippableItem.h"
#include "GearEquippableItem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class PIXEL2D_API UGearEquippableItem : public UEquippableItem
{
	GENERATED_BODY()

public:

	UGearEquippableItem();

	virtual bool Equip(class APlayerCharacter* Character) override;
	virtual bool UnEquip(class APlayerCharacter* Character) override;

	///**The skeletal mesh for this gear*/
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gear")
	//class USkeletalMesh* Mesh;

	/**Optional material instance to apply to the gear*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gear")
	UTexture* Texture;

	/**The amount of defence this item provides. 0.2 = 20% less damage taken*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Gear", meta = (ClampMin = 0.0, ClampMax = 1.0))
	float DamageDefenceMultiplier;
	
};
