// Fill out your copyright notice in the Description page of Project Settings.


#include "ZDPlayerCharacterBase.h"
#include "PaperCharacter.h"
#include "PaperFlipbookComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperTileMapComponent.h"
#include "PaperTileMap.h"
#include "Net/UnrealNetwork.h"

#include "Pickup.h"
#include "Item.h"
#include "InventoryComponent.h"
#include "WorldHandler.h"
#include "ChunkActor.h"
#include "EquippableItem.h"
#include "GearEquippableItem.h"


AZDPlayerCharacterBase::AZDPlayerCharacterBase()
{
	bReplicates = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance
	CameraBoom->bDoCollisionTest = false; // turn off the camera zoom if object between the player and the camera

	PlayerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	PlayerMesh->SetupAttachment(RootComponent);
	
	// Initialize Materials									 C:/GitHub/Pixel2D/Content/2DAssets/CustomizablePixelCharacter/Anim/M_Cloth.uasset
	//static ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialInstanceObject(TEXT("/Game/2DAssets/CustomizablePixelCharacter/Anim/MI_Cloth"));

	//Give the player an inventory with 20 slots, and an 80kg capacity
	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>("PlayerInventory");
	PlayerInventory->SetCapacity(20);
	PlayerInventory->SetWeightCapacity(80.f);
	PlayerInventory->OnItemAdded.AddDynamic(this, &AZDPlayerCharacterBase::ItemAddedToInventory);
	PlayerInventory->OnItemRemoved.AddDynamic(this, &AZDPlayerCharacterBase::ItemRemovedFromInventory);

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	PickupCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("PickupCapsuleComponent"));
	PickupCapsuleComponent->SetupAttachment(RootComponent);
	//PickupCapsuleComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	PickupCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//GetSprite()->SetIsReplicated(true); // Enable replication to SpriteComponent
	SetReplicateMovement(true);
	SetReplicates(true);
}

void AZDPlayerCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZDPlayerCharacterBase, CharacterRotation);
}

void AZDPlayerCharacterBase::BeginPlay()
{

	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		// Enable mouse cursor and disable mouse input in UI only mode
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;
		PlayerController->bEnableMouseOverEvents = true;

		// Set input mode to game and UI
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PlayerController->SetInputMode(InputMode);

	}
	
	// * Initialize player materials *

	MI_Body = UMaterialInstanceDynamic::Create(PlayerMesh->GetMaterial(0), nullptr);
	PlayerMesh->SetMaterial(0, MI_Body);
	PlayerDefaultMaterials.Add(EEquippableSlot::EIS_Chest, MI_Body);
	PlayerMaterials.Add(EEquippableSlot::EIS_Chest, MI_Body);

	MI_HeadGear = UMaterialInstanceDynamic::Create(PlayerMesh->GetMaterial(1), nullptr);
	PlayerMesh->SetMaterial(1, MI_HeadGear);
	PlayerDefaultMaterials.Add(EEquippableSlot::EIS_Helmet, MI_HeadGear);
	PlayerMaterials.Add(EEquippableSlot::EIS_Helmet, MI_HeadGear);

	MI_Hair = UMaterialInstanceDynamic::Create(PlayerMesh->GetMaterial(4), nullptr);
	PlayerMesh->SetMaterial(4, MI_Hair);
	PlayerMaterials.Add(EEquippableSlot::EIS_Head, MI_Hair);

	MI_BodyGear = UMaterialInstanceDynamic::Create(PlayerMesh->GetMaterial(5), nullptr);
	PlayerMesh->SetMaterial(5, MI_BodyGear);
	PlayerDefaultMaterials.Add(EEquippableSlot::EIS_Vest, MI_BodyGear);
	PlayerMaterials.Add(EEquippableSlot::EIS_Vest, MI_BodyGear);

	MI_LegGear = UMaterialInstanceDynamic::Create(PlayerMesh->GetMaterial(6), nullptr);
	PlayerMesh->SetMaterial(6, MI_LegGear);
	PlayerMaterials.Add(EEquippableSlot::EIS_Legs, MI_LegGear);

	MI_FootGear = UMaterialInstanceDynamic::Create(PlayerMesh->GetMaterial(7), nullptr);
	PlayerMesh->SetMaterial(7, MI_FootGear);
	PlayerMaterials.Add(EEquippableSlot::EIS_Feet, MI_FootGear);

	MI_BackPack = UMaterialInstanceDynamic::Create(PlayerMesh->GetMaterial(10), nullptr);
	PlayerMesh->SetMaterial(10, MI_BackPack);
	PlayerMaterials.Add(EEquippableSlot::EIS_Backpack, MI_BackPack);


	GetCharacterMovement()->MaxWalkSpeed = 400.0;
}

void AZDPlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AZDPlayerCharacterBase::Move);

		//Click event
		EnhancedInputComponent->BindAction(PrimaryClickAction, ETriggerEvent::Triggered, this, &AZDPlayerCharacterBase::PrimaryClick);

		//Click event
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Triggered, this, &AZDPlayerCharacterBase::InventoryOpenClose);

	}
}

void AZDPlayerCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*
	float velocity = GetCharacterMovement()->Velocity.Z;
	GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, FString::Printf(TEXT("Velocity: %f"), velocity));
	*/
}

void AZDPlayerCharacterBase::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.X);
		//AddMovementInput(RightDirection, MovementVector.X);

		
		if (MovementVector.X < 0 && !CharacterRotation)
		{
			Server_SetCharacterRotation(1);
		}
		else if(MovementVector.X > 0 && CharacterRotation)
		{
			Server_SetCharacterRotation(0);
		}
		
	}
}

void AZDPlayerCharacterBase::Look(const FInputActionValue& Value)
{

}

void AZDPlayerCharacterBase::PrimaryClick(const FInputActionValue& Value)
{
	//GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, TEXT("AZDPlayerCharacterBase::PrimaryClick()"));
	FVector WorldLocation, WorldDirection;
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
	}

	FVector Start = WorldLocation;
	FVector End = Start + WorldDirection * 1000;

	FCollisionQueryParams TraceParams(FName(TEXT("LineTrace")), true, this);
	TraceParams.bTraceComplex = true;
	FHitResult HitResult;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, TraceParams);

	if (bHit)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, TEXT("AZDPlayerCharacterBase:: -- bHit"));
		if (HitResult.GetActor()->IsA(AChunkActor::StaticClass()))
		{
			AChunkActor* Chunk = Cast<AChunkActor>(HitResult.GetActor());
			if (Chunk)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, TEXT("AZDPlayerCharacterBase:: -- Chunk"));
				FVector HitRelativeLocationToChunk = FVector(FMath::Abs(Chunk->GetActorLocation().X - Start.X), 0.0f, FMath::Abs(Chunk->GetActorLocation().Z - Start.Z));
				Chunk->ModifyBlock(HitRelativeLocationToChunk, 16);
			}
		}
	}
}

void AZDPlayerCharacterBase::InventoryOpenClose(const FInputActionValue& Value)
{
}


void AZDPlayerCharacterBase::OnLootSourceOwnerDestroyed(AActor* DestroyedActor)
{
	//Remove loot source 
	if (HasAuthority() && LootSource && DestroyedActor == LootSource->GetOwner())
	{
		ServerSetLootSource(nullptr);
	}
}

void AZDPlayerCharacterBase::OnRep_LootSource()
{
	////Bring up or remove the looting menu 
	//if (APixel2DPlayerController* PC = Cast<AZDPlayerCharacterBase>(GetController()))
	//{
	//	if (PC->IsLocalController())
	//	{
	//		if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(PC->GetHUD()))
	//		{
	//			if (LootSource)
	//			{
	//				HUD->OpenLootWidget();
	//			}
	//			else
	//			{
	//				HUD->CloseLootWidget();
	//			}
	//		}
	//	}
	//}
}

// -------- OnRep --------
void AZDPlayerCharacterBase::OnRep_CharacterRotation()
{
	if (CharacterRotation)
	{
		//Controller->SetControlRotation(FRotator(0.0, 0.0, 0.0));
		FRotator Rotation = FRotator(0.0f, 180.0f, 0.0f);
		GetSprite()->SetWorldRotation(Rotation);
		PlayerMesh->SetRelativeScale3D(FVector(1, -1, 1));
	}
	else
	{
		//Controller->SetControlRotation(FRotator(0.0, 180.0, 180.0));
		FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
		GetSprite()->SetWorldRotation(Rotation);
		PlayerMesh->SetRelativeScale3D(FVector(1, 1, 1));
	}
}

void AZDPlayerCharacterBase::OnRep_EquippedWeapon()
{
}

void AZDPlayerCharacterBase::ClientShowNotification_Implementation(const FText& Message)
{
	ShowNotification_BP(Message);

	// ** later make a widget class in c++ instead of BP **
	/*if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	{
		HUD->ShowNotification(Message);
	}*/
}

void AZDPlayerCharacterBase::UseItem(UItem* Item)
{
	if (!HasAuthority() && Item)
	{
		Server_UseItem(Item);
	}

	if (HasAuthority())
	{
		if (PlayerInventory && !PlayerInventory->FindItem(Item))
		{
			return;
		}
	}

	if (Item)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("ZDPlayerCharacterBase::UseItem()"));
		Item->OnUse(this);
		Item->Use(this);
	}
}

void AZDPlayerCharacterBase::DropItem(UItem* Item, const int32 Quantity)
{
	if (PlayerInventory && Item && PlayerInventory->FindItem(Item))
	{
		if (!HasAuthority())
		{
			Server_DropItem(Item, Quantity);
			return;
		}

		if (HasAuthority())
		{
			const int32 ItemQuantity = Item->GetQuantity();
			const int32 DroppedQuantity = PlayerInventory->ConsumeItem(Item, Quantity);

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = this;
			SpawnParams.bNoFail = true;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			FVector SpawnLocation = GetActorLocation();
			CharacterRotation ? SpawnLocation.X -= 50 : SpawnLocation.X += 50;
			//SpawnLocation.Z -= GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

			//FTransform SpawnTransform(GetActorRotation(), SpawnLocation);
			FTransform SpawnTransform(FRotator(0.0f, 0.0f, 0.0f), SpawnLocation);

			//ensure(PickupClass);

			if (APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams))
			{
				Pickup->InitializePickup(Item->GetClass(), DroppedQuantity);
			}
		}
	}
}

void AZDPlayerCharacterBase::ItemAddedToInventory(UItem* Item)
{
}

void AZDPlayerCharacterBase::ItemRemovedFromInventory(UItem* Item)
{
}

bool AZDPlayerCharacterBase::EquipItem(UEquippableItem* Item)
{
	EquippedItems.Add(Item->Slot, Item);
	OnEquippedItemsChanged.Broadcast(Item->Slot, Item);
	return true;
}

bool AZDPlayerCharacterBase::UnEquipItem(UEquippableItem* Item)
{
	if (Item)
	{
		if (EquippedItems.Contains(Item->Slot))
		{
			if (Item == *EquippedItems.Find(Item->Slot))
			{
				EquippedItems.Remove(Item->Slot);
				OnEquippedItemsChanged.Broadcast(Item->Slot, nullptr);
				return true;
			}
		}
	}
	return false;
}

void AZDPlayerCharacterBase::EquipGear(UGearEquippableItem* Gear)
{

	if (UMaterialInstanceDynamic* DynamicMaterialInstance = GetSlotMaterialInstance(Gear->Slot))
	{
		DynamicMaterialInstance->SetTextureParameterValue(TEXT("TextureParameter"), Gear->Texture);
		//DynamicMaterialInstance->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor::Yellow);
	}

	/*if (UMaterialInstance* EquippableMaterialInstance = GetSlotMaterialInstance(Gear->Slot))
	{
		UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(EquippableMaterialInstance->GetMaterial(), nullptr);
		if (DynamicMaterialInstance)
		{
			DynamicMaterialInstance->SetTextureParameterValue(TEXT("TextureParameter"), Gear->Texture);
			EquippableMaterialInstance = UMaterialInstance::Create(DynamicMaterialInstance->GetMaterial(), this);
		}
	}
	else
	{
		EquippableMaterialInstance = nullptr;
	}*/
}

void AZDPlayerCharacterBase::UnEquipGear(const EEquippableSlot Slot)
{
	//if (UMaterialInstanceDynamic* EquippableMesh = GetSlotMaterialInstance(Slot))
	//{
	//	if (USkeletalMesh* BodyMesh = *NakedMeshes.Find(Slot))
	//	{
	//		EquippableMesh->SetSkeletalMesh(BodyMesh);

	//		////Put the materials back on the body mesh (Since gear may have applied a different material)
	//		//for (int32 i = 0; i < BodyMesh->Materials.Num(); ++i)
	//		//{
	//		//	if (BodyMesh->Materials.IsValidIndex(i))
	//		//	{
	//		//		EquippableMesh->SetMaterial(i, BodyMesh->Materials[i].MaterialInterface);
	//		//	}
	//		//}
	//	}
	//	else
	//	{
	//		//For some gear like backpacks, there is no naked mesh
	//		EquippableMesh->SetSkeletalMesh(nullptr);
	//	}
	//}
}

void AZDPlayerCharacterBase::EquipWeapon(UWeaponItem* WeaponItem)
{
	//if (WeaponItem && WeaponItem->WeaponClass && HasAuthority())
	//{
	//	if (EquippedWeapon)
	//	{
	//		UnEquipWeapon();
	//	}

	//	//Spawn the weapon in
	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.bNoFail = true;
	//	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	//	SpawnParams.Owner = SpawnParams.Instigator = this;

	//	if (AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponItem->WeaponClass, SpawnParams))
	//	{
	//		Weapon->Item = WeaponItem;

	//		EquippedWeapon = Weapon;
	//		OnRep_EquippedWeapon();

	//		Weapon->OnEquip();
	//	}
	//}
}

void AZDPlayerCharacterBase::UnEquipWeapon()
{
	/*if (HasAuthority() && EquippedWeapon)
	{
		EquippedWeapon->OnUnEquip();
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
		OnRep_EquippedWeapon();
	}*/
}

float AZDPlayerCharacterBase::ModifyHealth(const float Delta)
{
	const float OldHealth = Health;

	Health = FMath::Clamp<float>(Health + Delta, 0.f, MaxHealth);

	return Health - OldHealth;
}

void AZDPlayerCharacterBase::OnRep_Health(float OldHealth)
{
	OnHealthModified(Health - OldHealth);
}

void AZDPlayerCharacterBase::SetLootSource(UInventoryComponent* NewLootSource)
{
	/**If the thing we're looting gets destroyed, we need to tell the client to remove their Loot screen*/
	if (NewLootSource && NewLootSource->GetOwner())
	{
		NewLootSource->GetOwner()->OnDestroyed.AddUniqueDynamic(this, &AZDPlayerCharacterBase::OnLootSourceOwnerDestroyed);
	}

	if (HasAuthority())
	{
		if (NewLootSource)
		{
			//Looting a player keeps their body alive for an extra 2 minutes to provide enough time to loot their items
			if (AZDPlayerCharacterBase* Character = Cast<AZDPlayerCharacterBase>(NewLootSource->GetOwner()))
			{
				Character->SetLifeSpan(120.f);
			}
		}

		LootSource = NewLootSource;
		OnRep_LootSource();
	}
	else
	{
		ServerSetLootSource(NewLootSource);
	}
}

bool AZDPlayerCharacterBase::IsLooting() const
{
	return LootSource != nullptr;
}

UMaterialInstanceDynamic* AZDPlayerCharacterBase::GetSlotMaterialInstance(const EEquippableSlot Slot)
{
	if (PlayerMaterials.Contains(Slot))
	{
		return *PlayerMaterials.Find(Slot);
	}
	return nullptr;
}

void AZDPlayerCharacterBase::BeginLootingPlayer(AZDPlayerCharacterBase* Character)
{
	if (Character)
	{
		Character->SetLootSource(PlayerInventory);
	}
}


// -------- RPC --------
// 
//bool AZDPlayerCharacterBase::Server_SetCharacterRotation_Validate(uint8 NewRotation)
//{
//	// You can add validation logic here if needed
//	return true;
//}

void AZDPlayerCharacterBase::Server_SetCharacterRotation_Implementation(uint8 NewRotation)
{
	// This function is called on the server when a client requests to change CharacterRotation
	if (HasAuthority())
	{
		// Do any server-side logic here
		CharacterRotation = NewRotation;

		// Call OnRep function manually to replicate the change to all clients
		OnRep_CharacterRotation();
	}
}

void AZDPlayerCharacterBase::Server_UseItem_Implementation(class UItem* Item)
{
	UseItem(Item);
}

bool AZDPlayerCharacterBase::Server_UseItem_Validate(class UItem* Item)
{
	return true;
}

void AZDPlayerCharacterBase::Server_DropItem_Implementation(class UItem* Item, const int32 Quantity)
{
	DropItem(Item, Quantity);
}

bool AZDPlayerCharacterBase::Server_DropItem_Validate(class UItem* Item, const int32 Quantity)
{
	return true;
}

void AZDPlayerCharacterBase::ServerSetLootSource_Implementation(class UInventoryComponent* NewLootSource)
{
	SetLootSource(NewLootSource);
}

bool AZDPlayerCharacterBase::ServerSetLootSource_Validate(class UInventoryComponent* NewLootSource)
{
	return true;
}