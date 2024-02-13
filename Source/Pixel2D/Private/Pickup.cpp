// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"

#include "Item.h"
#include "InventoryComponent.h"
#include "ZDPlayerCharacterBase.h"
#include "Pixel2DPlayerController.h"

// Sets default values
APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	PickupSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphereComponent"));
	PickupSphereComponent->SetupAttachment(RootComponent);
	PickupSphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PickupSphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create sprite component
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("PickupSpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);
}

void APickup::InitializePickup(const TSubclassOf<class UItem> ItemClass, const int32 Quantity)
{
	if (HasAuthority() && ItemClass && Quantity)
	{
		Item = NewObject<UItem>(this, ItemClass);
		Item->SetQuantity(Quantity);

		OnRep_Item();

		Item->MarkDirtyForReplication();
	}

}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() && ItemTemplate && bNetStartup)
	{
		InitializePickup(ItemTemplate->GetClass(), ItemTemplate->GetQuantity());

		PickupSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		PickupSphereComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		PickupSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
	
	}


	/**If pickup was spawned in at runtime, ensure that it matches the rotation of the ground that it was dropped on
	If we dropped a pickup on a 20 degree slope, the pickup would also be spawned at a 20 degree angle*/
	if (!bNetStartup)
	{
		AlignWithGround();
	}

	if (Item)
	{
		Item->MarkDirtyForReplication();
	}
}

void APickup::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(APickup, Item);

}

void APickup::OnRep_Item()
{
	if (Item)
	{
		//PickupMesh->SetStaticMesh(Item->PickupMesh);
		//InteractionComponent->InteractableNameText = Item->DisplayName;
		SpriteComponent->SetSprite(PickupSprite);

		//Clients bind to this delegate in order to refresh the interaction widget if item quantity changes
		Item->OnItemModified.AddDynamic(this, &APickup::OnItemModified);
	}


	//If any replicated properties on the item are changed, we refresh the widget
	//InteractionComponent->RefreshWidget();
}

void APickup::OnItemModified()
{
	/*if (InteractionComponent)
	{
		InteractionComponent->RefreshWidget();
	}*/
}

bool APickup::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (Item && Channel->KeyNeedsToReplicate(Item->GetUniqueID(), Item->RepKey))
	{
		bWroteSomething |= Channel->ReplicateSubobject(Item, *Bunch, *RepFlags);
	}

	return bWroteSomething;
}

void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AZDPlayerCharacterBase* PlayerCharacterActor = Cast<AZDPlayerCharacterBase>(OtherActor);

	if (PlayerCharacterActor)
	{
		OnTakePickup(PlayerCharacterActor);
	}
}

#if WITH_EDITOR
void APickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	//If a new pickup is selected in the property editor, change the mesh to reflect the new item being selected
	if (PropertyName == GET_MEMBER_NAME_CHECKED(APickup, ItemTemplate))
	{
		if (ItemTemplate)
		{
			//PickupMesh->SetStaticMesh(ItemTemplate->PickupMesh);
			PickupSprite = ItemTemplate->PickupSprite;
		}
	}
}
#endif

void APickup::OnTakePickup(class AZDPlayerCharacterBase* Taker)
{
	if (!Taker)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pickup was taken but player was not valid. "));
		return;
	}

	//Not 100% sure Pending kill check is needed but should prevent player from taking a pickup another player has already tried taking
	if (HasAuthority() && !IsPendingKillPending() && Item)
	{
		if (UInventoryComponent* PlayerInventory = Taker->PlayerInventory)
		{
			const FItemAddResult AddResult = PlayerInventory->TryAddItem(Item);

			if (AddResult.AmountGiven < Item->GetQuantity())
			{
				Item->SetQuantity(Item->GetQuantity() - AddResult.AmountGiven);
			}
			else if (AddResult.AmountGiven >= Item->GetQuantity())
			{
				Destroy();
			}

			if (!AddResult.ErrorText.IsEmpty())
			{
				if (APixel2DPlayerController* PC = Cast<APixel2DPlayerController>(Taker->GetController()))
				{
					// need to move controller logic to player controller
					//PC->ClientShowNotification(AddResult.ErrorText);
				}
			}
		}
	}
}