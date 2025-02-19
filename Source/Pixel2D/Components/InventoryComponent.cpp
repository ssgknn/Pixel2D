// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "../Item/Item.h"

#define LOCTEXT_NAMESPACE "Inventory"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	OnItemAdded.AddDynamic(this, &UInventoryComponent::ItemAdded);
	OnItemRemoved.AddDynamic(this, &UInventoryComponent::ItemRemoved);

	SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	Items.SetNum(Capacity, false);
	Items.Init(nullptr, Capacity);
	
}

FItemAddResult UInventoryComponent::TryAddItem(class UItem* Item)
{
	return TryAddItem_Internal(Item);
}

FItemAddResult UInventoryComponent::TryAddItemFromClass(TSubclassOf<class UItem> ItemClass, const int32 Quantity /*=1*/)
{
	UItem* Item = NewObject<UItem>(GetOwner(), ItemClass);
	Item->SetQuantity(Quantity);
	return TryAddItem_Internal(Item);
}

void UInventoryComponent::ReorderItems(int idxAt, int newIdx)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Items[idxAt] && Items[newIdx] == nullptr)
		{
			Items[idxAt]->SetInventoryIndexAt(newIdx);
			Items[newIdx] = Items[idxAt];
			Items[idxAt] = nullptr;
		}
		else if (Items[idxAt] && Items[newIdx])
		{
			if (Items[idxAt]->GetClass() == Items[newIdx]->GetClass())
			{
				if ((Items[idxAt]->GetQuantity() + Items[newIdx]->GetQuantity()) <= Items[newIdx]->MaxStackSize)
				{
					Items[newIdx]->SetQuantity(Items[idxAt]->GetQuantity() + Items[newIdx]->GetQuantity());
					Items[idxAt] = nullptr;
				}
				else
				{
					Items[idxAt]->SetQuantity((Items[idxAt]->GetQuantity() + Items[newIdx]->GetQuantity()) - Items[idxAt]->MaxStackSize);
					Items[newIdx]->SetQuantity(Items[newIdx]->MaxStackSize);
				}
			}
			else
			{
				Items[idxAt]->SetInventoryIndexAt(newIdx);
				Items[newIdx]->SetInventoryIndexAt(idxAt);

				UItem* tempItem = Items[newIdx];

				Items[newIdx] = Items[idxAt];
				Items[idxAt] = tempItem;
			}
		}
		OnRep_Items();
		ReplicatedItemsKey++;
		OnInventoryUpdated.Broadcast();
	}
}

int32 UInventoryComponent::ConsumeItem(class UItem* Item)
{
	if (Item)
	{
		ConsumeItem(Item, Item->GetQuantity());
	}
	return 0;
}

int32 UInventoryComponent::ConsumeItem(class UItem* Item, const int32 Quantity)
{
	if (GetOwner() && GetOwner()->HasAuthority() && Item)
	{
		const int32 RemoveQuantity = FMath::Min(Quantity, Item->GetQuantity());

		//We shouldn't have a negative amount of the item after the drop
		ensure(!(Item->GetQuantity() - RemoveQuantity < 0));

		//We now have zero of this item, remove it from the inventory
		Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);

		if (Item->GetQuantity() <= 0)
		{
			RemoveItem(Item);
		}
		else
		{
			ClientRefreshInventory();
		}

		return RemoveQuantity;
	}

	return 0;
}

bool UInventoryComponent::RemoveItem(class UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Item)
		{
			//Items.RemoveSingle(Item);
			Items[Item->GetInventoryIndexAt()] = nullptr;
			Item->SetInventoryIndexAt(-1);
			OnItemRemoved.Broadcast(Item);

			OnRep_Items();

			ReplicatedItemsKey++;

			return true;
		}
	}

	return false;
}

bool UInventoryComponent::HasItem(TSubclassOf <class UItem> ItemClass, const int32 Quantity /*= 1*/) const
{
	if (UItem* ItemToFind = FindItemByClass(ItemClass))
	{
		return ItemToFind->GetQuantity() >= Quantity;
	}
	return false;
}

UItem* UInventoryComponent::FindItem(class UItem* Item) const
{
	if (Item)
	{
		for (UItem* InvItem : Items)
		{
			if (InvItem && InvItem->GetClass() == Item->GetClass())
			{
				return InvItem;
			}
		}
	}
	return nullptr;
}

UItem* UInventoryComponent::FindItemStackNotFull(class UItem* Item) const
{
	if (Item)
	{
		for (UItem* InvItem : Items)
		{
			if (InvItem && InvItem->GetClass() == Item->GetClass() && !InvItem->IsStackFull())
			{
				return InvItem;
			}
		}
	}
	return nullptr;
}

UItem* UInventoryComponent::FindItemByClass(TSubclassOf<class UItem> ItemClass) const
{
	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == ItemClass)
		{
			return InvItem;
		}
	}
	return nullptr;
}

TArray<UItem*> UInventoryComponent::FindItemsByClass(TSubclassOf<class UItem> ItemClass) const
{
	TArray<UItem*> ItemsOfClass;

	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass()->IsChildOf(ItemClass))
		{
			ItemsOfClass.Add(InvItem);
		}
	}

	return ItemsOfClass;
}

float UInventoryComponent::GetCurrentWeight() const
{
	float Weight = 0.f;

	for (auto& Item : Items)
	{
		if (Item)
		{
			Weight += Item->GetStackWeight();
		}
	}

	return Weight;
}

void UInventoryComponent::SetWeightCapacity(const float NewWeightCapacity)
{
	WeightCapacity = NewWeightCapacity;
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SetCapacity(const int32 NewCapacity)
{
	Capacity = NewCapacity;
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::ClientRefreshInventory_Implementation()
{
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryComponent, Items);
}

bool UInventoryComponent::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	//Check if the array of items needs to replicate
	if (Channel->KeyNeedsToReplicate(0, ReplicatedItemsKey))
	{
		for (auto& Item : Items)
		{
			if (Item != nullptr)
			{
				if (Channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey))
				{
					bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
				}
			}
			
		}
	}

	return bWroteSomething;
}

UItem* UInventoryComponent::AddNewItem(class UItem* Item, const int32 Quantity)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		UItem* NewItem = NewObject<UItem>(GetOwner(), Item->GetClass());
		NewItem->World = GetWorld();
		NewItem->SetQuantity(Quantity);
		NewItem->SetOwningInventory(this);
		NewItem->AddedToInventory(this);
		int32 idx = FindFirstEmptyIdx();

		if (idx >= 0)
		{
			NewItem->SetInventoryIndexAt(idx);
			Items[idx] = NewItem;
		}
		
		Items[idx]->MarkDirtyForReplication();
		OnItemAdded.Broadcast(NewItem);
		OnRep_Items();
		return NewItem;
	}

	return nullptr;
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();

	for (auto& Item : Items)
	{
		//On the client the world won't be set initially, so it set if not
		if (Item && !Item->World)
		{
			OnItemAdded.Broadcast(Item);
			Item->World = GetWorld();
		}
	}
}

FItemAddResult UInventoryComponent::TryAddItem_Internal(class UItem* Item)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		if (Item->bStackable)
		{
			//Somehow the items quantity went over the max stack size. This shouldn't ever happen
			ensure(Item->GetQuantity() <= Item->MaxStackSize);

			// enters if we have an item in inventory which is not full
			if (UItem* ExistingItem = FindItemStackNotFull(Item))
			{
				const int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / Item->Weight);
				const int32 QuantityMaxAddAmount = FMath::Min(ExistingItem->MaxStackSize - ExistingItem->GetQuantity(), Item->GetQuantity());
				const int32 AddAmount = FMath::Min(WeightMaxAddAmount, QuantityMaxAddAmount);

				// enters if we can add all items from pickup to the not full stack
				if (AddAmount == Item->GetQuantity())
				{
					ExistingItem->SetQuantity(ExistingItem->GetQuantity() + AddAmount);
					return FItemAddResult::AddedAll(Item->GetQuantity());
				}
				else
				{
					ExistingItem->SetQuantity(ExistingItem->GetQuantity() + AddAmount);
					return FItemAddResult::AddedSome(Item->GetQuantity(), AddAmount, LOCTEXT("StackAddedSomeFullText", "Couldn't add all of stack to inventory."));
				}
			}
			else //we want to add a stackable item that doesn't exist in the inventory
			{
				if (CountValidElements(Items) + 1 > GetCapacity())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("InventoryCapacityFullText", "Couldn't add {ItemName} to Inventory. Inventory is full."), Item->DisplayName));
				}

				const int32 WeightMaxAddAmount = FMath::FloorToInt((WeightCapacity - GetCurrentWeight()) / Item->Weight);
				const int32 QuantityMaxAddAmount = FMath::Min(Item->MaxStackSize, Item->GetQuantity());
				const int32 AddAmount = FMath::Min(WeightMaxAddAmount, QuantityMaxAddAmount);

				AddNewItem(Item, AddAmount);
				return AddAmount >= Item->GetQuantity() ? FItemAddResult::AddedAll(Item->GetQuantity()) : FItemAddResult::AddedSome(Item->GetQuantity(), AddAmount, LOCTEXT("StackAddedSomeFullText", "Couldn't add all of stack to inventory."));
			}
		}
		else //item isnt stackable
		{
			if (CountValidElements(Items) + 1 > GetCapacity())
			{
				return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("InventoryCapacityFullText", "Couldn't add {ItemName} to Inventory. Inventory is full."), Item->DisplayName));
			}

			//Items with a weight of zero dont require a weight check
			if (!FMath::IsNearlyZero(Item->Weight))
			{
				if (GetCurrentWeight() + Item->Weight > GetWeightCapacity())
				{
					return FItemAddResult::AddedNone(Item->GetQuantity(), FText::Format(LOCTEXT("StackWeightFullText", "Couldn't add {ItemName}, too much weight."), Item->DisplayName));
				}
			}

			//Non-stackables should always have a quantity of 1
			ensure(Item->GetQuantity() == 1);

			AddNewItem(Item, 1);

			return FItemAddResult::AddedAll(Item->GetQuantity());
		}
	}

	//AddItem should never be called on a client
	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));
}

void UInventoryComponent::ItemAdded(class UItem* Item)
{
	FString RoleString = GetOwner()->HasAuthority() ? "server" : "client";
	UE_LOG(LogTemp, Warning, TEXT("Item added: %s on %s"), *GetNameSafe(Item), *RoleString);
}

void UInventoryComponent::ItemRemoved(class UItem* Item)
{
	FString RoleString = GetOwner()->HasAuthority() ? "server" : "client";
	UE_LOG(LogTemp, Warning, TEXT("Item Removed: %s on %s"), *GetNameSafe(Item), *RoleString);
}

int32 UInventoryComponent::CountValidElements(const TArray<class UItem*>& InArray)
{
	int32 ValidCount = 0;
	for (int32 i = 0; i < InArray.Num(); ++i)
	{
		if (InArray[i] != nullptr)
		{
			++ValidCount;
		}
	}
	return ValidCount;
}

int32 UInventoryComponent::FindFirstEmptyIdx()
{
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i] == nullptr)
		{
			return i;
		}
	}
	return -1;
}

#undef LOCTEXT_NAMESPACE

