// Fill out your copyright notice in the Description page of Project Settings.


#include "GearEquippableItem.h"
#include "ZDPlayerCharacterBase.h"


UGearEquippableItem::UGearEquippableItem()
{
	DamageDefenceMultiplier = 0.0f;
}

bool UGearEquippableItem::Equip(AZDPlayerCharacterBase* Character)
{
	bool bEquipSuccessful = Super::Equip(Character);

	if (bEquipSuccessful && Character)
	{
		Character->EquipGear(this);
	}

	return bEquipSuccessful;
}

bool UGearEquippableItem::UnEquip(AZDPlayerCharacterBase* Character)
{
	bool bUnEquipSuccessful = Super::UnEquip(Character);

	if (bUnEquipSuccessful && Character)
	{
		Character->UnEquipGear(Slot);
	}

	return bUnEquipSuccessful;
}
