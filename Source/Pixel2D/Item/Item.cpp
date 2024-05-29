// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"

#include "Net/UnrealNetwork.h"
#include "../Components/InventoryComponent.h"


#define LOCTEXT_NAMESPACE "Item"

void UItem::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItem, Quantity);
	DOREPLIFETIME(UItem, InventoryIndexAt);
	DOREPLIFETIME(UItem, OwningInventory);
}

bool UItem::IsSupportedForNetworking() const
{
	return true;
}

class UWorld* UItem::GetWorld() const
{
	return World;
}

#if WITH_EDITOR
void UItem::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	//UPROPERTY clamping doesn't support using a variable to clamp so we do in here instead
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItem, Quantity))
	{
		Quantity = FMath::Clamp(Quantity, 1, bStackable ? MaxStackSize : 1);
	}
	else if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItem, bStackable))
	{
		if (!bStackable)
		{
			Quantity = 1;
		}
	}
}
#endif

UItem::UItem()
{
	DisplayName = LOCTEXT("ItemName", "Item");
	UseActionText = LOCTEXT("ItemUseActionText", "Use");
	Weight = 0.f;
	bStackable = true;
	Quantity = 1;
	MaxStackSize = 2;
	RepKey = 0;

	InventoryIndexAt = -1;
}

void UItem::OnRep_Modified()
{
	OnItemModified.Broadcast();
}

void UItem::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 0, bStackable ? MaxStackSize : 1);
		MarkDirtyForReplication();
	}
}

void UItem::SetInventoryIndexAt(const int32 NewIndex)
{
	if (NewIndex != InventoryIndexAt)
	{
		InventoryIndexAt = NewIndex;
		MarkDirtyForReplication();
	}
}

void UItem::SetOwningInventory(UInventoryComponent* NewInvenotry)
{
	if (NewInvenotry != OwningInventory)
	{
		OwningInventory = NewInvenotry;
		MarkDirtyForReplication();
	}
}

bool UItem::ShouldShowInInventory() const
{
	return true;
}

void UItem::Use(class AZDPlayerCharacterBase* Character)
{

}

void UItem::AddedToInventory(class UInventoryComponent* Inventory)
{

}

void UItem::MarkDirtyForReplication()
{
	//Mark this object for replication
	++RepKey;

	//Mark the array for replication
	if (OwningInventory)
	{
		++OwningInventory->ReplicatedItemsKey;
	}
}

#undef LOCTEXT_NAMESPACE