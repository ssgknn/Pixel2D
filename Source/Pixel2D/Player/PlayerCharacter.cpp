// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
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
#include "EngineUtils.h"

#include "../Item/Pickup.h"
#include "../Item/Item.h"
#include "../Components/InventoryComponent.h"
#include "../World/WorldHandler.h"
#include "../World/ChunkActor.h"
#include "../Item/EquippableItem.h"
#include "../Item/GearEquippableItem.h"

APlayerCharacter::APlayerCharacter()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;

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
	PlayerInventory->OnItemAdded.AddDynamic(this, &APlayerCharacter::ItemAddedToInventory);
	PlayerInventory->OnItemRemoved.AddDynamic(this, &APlayerCharacter::ItemRemovedFromInventory);

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

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerCharacter, CharacterRotation);
}

void APlayerCharacter::BeginPlay()
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

	while (WorldHandlerRef == nullptr)
	{
		InitializeChunkVariables();
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		//Click event
		EnhancedInputComponent->BindAction(PrimaryClickAction, ETriggerEvent::Triggered, this, &APlayerCharacter::PrimaryClick);

		//Click event
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Triggered, this, &APlayerCharacter::InventoryOpenClose);

	}
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (ShouldRequestChunkLoad())
	{
		RequestChunkLoad(PlayerID, CenterChunkCoords);
	}
	
	
	/*float velocity = GetCharacterMovement()->Velocity.Z;
	GEngine->AddOnScreenDebugMessage(-1, 0.3f, FColor::Red, FString::Printf(TEXT("Velocity: %f"), velocity));*/
}

void APlayerCharacter::Move(const FInputActionValue& Value)
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

void APlayerCharacter::Look(const FInputActionValue& Value)
{

}

void APlayerCharacter::PrimaryClick(const FInputActionValue& Value)
{
	
	FPlacementData testData;

	FIntPoint dimensions = FIntPoint(2, 2);
	TArray<int32> blockTextureID;
		blockTextureID.Add(443);
		blockTextureID.Add(444);
		blockTextureID.Add(468);
		blockTextureID.Add(469);
	TArray<uint8> bHasCollisiontemp;
		bHasCollisiontemp.Add(1);
		bHasCollisiontemp.Add(1);
		bHasCollisiontemp.Add(1);
		bHasCollisiontemp.Add(1);

	testData.Dimensions = dimensions;
	testData.BlockTextureID = blockTextureID;
	testData.bHasCollision = bHasCollisiontemp;

		CalculateChunkModification(testData);
	
}

void APlayerCharacter::InventoryOpenClose(const FInputActionValue& Value)
{
}

void APlayerCharacter::DEBUG_Key()
{
	//DEBUG

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AChunkActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AChunkActor* ChunkActor = Cast<AChunkActor>(Actor);
		if (ChunkActor)
		{
			UE_LOG(LogTemp, Log, TEXT("ChunkActor Found: %s"), *ChunkActor->GetName());
		}
	}
}

void APlayerCharacter::EnableTick()
{
	SetActorTickEnabled(true);
}

void APlayerCharacter::OnLootSourceOwnerDestroyed(AActor* DestroyedActor)
{
	//Remove loot source 
	if (HasAuthority() && LootSource && DestroyedActor == LootSource->GetOwner())
	{
		ServerSetLootSource(nullptr);
	}
}

void APlayerCharacter::OnRep_LootSource()
{
	////Bring up or remove the looting menu 
	//if (APixel2DPlayerController* PC = Cast<AZDAPlayerCharacterBase>(GetController()))
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
void APlayerCharacter::OnRep_CharacterRotation()
{
	if (CharacterRotation)
	{
		//Controller->SetControlRotation(FRotator(0.0, 0.0, 0.0));
		FRotator Rotation = FRotator(0.0f, 180.0f, 0.0f);
		PlayerMesh->SetRelativeScale3D(FVector(1, -1, 1));
	}
	else
	{
		//Controller->SetControlRotation(FRotator(0.0, 180.0, 180.0));
		FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
		PlayerMesh->SetRelativeScale3D(FVector(1, 1, 1));
	}
}

void APlayerCharacter::OnRep_EquippedWeapon()
{
}

//void AZDAPlayerCharacterBase::PrintMousePositionToWorld()
//{
//	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
//	if (PlayerController)
//	{
//		// Variables to hold mouse position
//		float MouseX, MouseY;
//		if (PlayerController->GetMousePosition(MouseX, MouseY))
//		{
//			FVector WorldLocation, WorldDirection;
//			// Convert the mouse position to a world space position and direction
//			if (PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
//			{
//				if (GEngine)
//				{
//					// Format the string to display
//					FString ScreenMessage = FString::Printf(TEXT("Mouse World Pos: %s, Dir: %s"),
//						*WorldLocation.ToString(), *WorldDirection.ToString());
//
//					// Display the message on the screen
//					GEngine->AddOnScreenDebugMessage(-1, 0.016f, FColor::Yellow, ScreenMessage);
//				}
//			}
//		}
//	}
//}


void APlayerCharacter::InitializeChunkVariables()
{
	for (TActorIterator<AWorldHandler> It(GetWorld()); It; ++It)
	{
		AWorldHandler* FoundHandler = *It;
		if (FoundHandler)
		{
			WorldHandlerRef = FoundHandler;
			break;
		}
	}

	if (WorldHandlerRef)
	{
		ChunkSize_player = WorldHandlerRef->ChunkSize;
		ChunkSizeHalf_player = WorldHandlerRef->ChunkSizeHalf;

		WorldHandlerRef->Server_RegisterPlayerID(this);
	}
}

void APlayerCharacter::SetPlayerID(uint8 ID)
{
	PlayerID = ID;
}

void APlayerCharacter::CalculateChunkModification(FPlacementData placementData)
{
	if (!WorldHandlerRef)
	{
		return;
	}

	/*if (HasAuthority())
	{*/
	int32 chunkElementCount = WorldHandlerRef->ChunkElementCount;
	int32 chunkSize = WorldHandlerRef->ChunkSize;
	int32 chunkSizeHalf = WorldHandlerRef->ChunkSizeHalf;
	int32 blockSize = WorldHandlerRef->BlockSize;

	FVector WorldLocation, WorldDirection;
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
	}

	int32 mousePosX = WorldLocation.X + WorldHandlerRef->ChunkSize / 2;
	int32 mousePosZ = WorldLocation.Z + WorldHandlerRef->ChunkSize / 2;

	//FIntPoint chunkCoord = FIntPoint(WorldLocation.X / WorldHandlerRef->ChunkSize, WorldLocation.Z / WorldHandlerRef->ChunkSize);
	//FIntPoint chunkCoord = FIntPoint(FMath::Floor((mousePosX - chunkSizeHalf) / chunkSize), FMath::Floor((mousePosZ + chunkSizeHalf) / chunkSize));
	FIntPoint chunkCoord = FIntPoint(FMath::Floor((mousePosX) / chunkSize), FMath::Floor((mousePosZ) / chunkSize));

	if (mousePosX < 0)
	{
		chunkCoord.X--;
	}
	if (mousePosZ < 0)
	{
		chunkCoord.Y--;
	}
	//int32 coordX;
	//int32 coordZ;

	/*mousePosX < 0 ? coordX = (FMath::Abs(mousePosX) % WorldHandlerRef->ChunkSize) * (-1) - 1 : coordX = mousePosX % WorldHandlerRef->ChunkSize;
	mousePosZ < 0 ? coordZ = (FMath::Abs(mousePosZ) % WorldHandlerRef->ChunkSize) * (-1) : coordZ = mousePosZ % WorldHandlerRef->ChunkSize;*/

	//FIntPoint chunkCoord = FIntPoint(coordX, coordZ);

	TArray<FChunkChangeData> chunksToUpdate;

	int32 blockStartX;
	int32 blockStopX;
	int32 remainBlocksX; //= (placementData.Dimensions.X - blockStopX) <= 0 ? 0 : placementData.Dimensions.X - blockStopX;

		if (mousePosX >= 0)
		{
			blockStartX = (mousePosX % chunkSize) / blockSize;
		}
		else
		{
			blockStartX = (chunkElementCount - (FMath::Abs(mousePosX) % chunkSize) / blockSize) - 1;
			/*blockStopX = ((-mousePosX % chunkSize) / blockSize) + 1;*/
		}

		if ((blockStartX + placementData.Dimensions.X) <= (chunkElementCount - 1))
		{
			blockStopX = blockStartX + placementData.Dimensions.X - 1;
			remainBlocksX = 0;
		}
		else
		{
			blockStopX = chunkElementCount - 1; //(chunkElementCount - (mousePosX % chunkSize) / blockSize) + 1;
			remainBlocksX = placementData.Dimensions.X - (blockStopX - blockStartX) - 1;
		}

	int32 blockStartZ;
	int32 blockStopZ;
	int32 remainBlocksZ; // = (placementData.Dimensions.Y - blockStopX) <= 0 ? 0 : placementData.Dimensions.Y - blockStopX;

		//if (mousePosZ >= 0)
		//{
		//	blockStartZ = (mousePosZ % chunkSize) / blockSize;
		//	//blockStopZ = (chunkElementCount - (mousePosZ % chunkSize) / blockSize) + 1;
		//}
		//else
		//{
		//	blockStartZ = (chunkElementCount - (-mousePosZ % chunkSize) / blockSize);
		//	//blockStopZ = ((-mousePosZ % chunkSize) / blockSize) + 1;
		//}

		//if ((blockStartZ + placementData.Dimensions.Y) <= (chunkElementCount - 1))
		//{
		//	blockStopZ = blockStartZ + placementData.Dimensions.Y;
		//}
		//else
		//{
		//	blockStopZ = chunkElementCount - 1;
		//	remainBlocksZ = placementData.Dimensions.Y - (blockStopZ - blockStartZ);
		//}

		if (mousePosZ <= 0)
		{
			blockStartZ = (FMath::Abs(mousePosZ) % chunkSize) / blockSize;
			//blockStopZ = (chunkElementCount - (mousePosZ % chunkSize) / blockSize) + 1;
		}
		else
		{
			blockStartZ = (chunkElementCount - (mousePosZ % chunkSize) / blockSize) - 1;
			//blockStopZ = ((-mousePosZ % chunkSize) / blockSize) + 1;
		}

		if ((blockStartZ + placementData.Dimensions.Y) <= (chunkElementCount - 1))
		{
			blockStopZ = blockStartZ + placementData.Dimensions.Y - 1;
			remainBlocksZ = 0;
		}
		else
		{
			blockStopZ = chunkElementCount - 1;
			remainBlocksZ = placementData.Dimensions.Y - (blockStopZ - blockStartZ) - 1;
		}

		/*GEngine->AddOnScreenDebugMessage(-1, 100.3f, FColor::Red, FString::Printf(TEXT("blockStartX: %d"), blockStartX));
		GEngine->AddOnScreenDebugMessage(-1, 100.3f, FColor::Red, FString::Printf(TEXT("blockStopX: %d"), blockStopX));
		GEngine->AddOnScreenDebugMessage(-1, 100.3f, FColor::Red, FString::Printf(TEXT("remainBlocksX: %d"), remainBlocksX));
		GEngine->AddOnScreenDebugMessage(-1, 100.3f, FColor::Red, FString::Printf(TEXT("blockStartZ: %d"), blockStartZ));
		GEngine->AddOnScreenDebugMessage(-1, 100.3f, FColor::Red, FString::Printf(TEXT("blockStopZ: %d"), blockStopZ));
		GEngine->AddOnScreenDebugMessage(-1, 100.3f, FColor::Red, FString::Printf(TEXT("remainBlocksZ: %d"), remainBlocksZ)); */

	FChunkChangeData chunkChange;


	chunkChange.ChunkCoordinate = chunkCoord;

	int32 placementX = 0;
	for (int32 x = blockStartX; x <= blockStopX; x++)
	{
		int32 placementZ = 0;
		for (int32 z = blockStartZ; z <= blockStopZ ; z++)
		{
			int32 cIdx = x + z * chunkElementCount;
			int32 pIdx = placementX + (placementZ * (placementData.Dimensions.Y));

			chunkChange.BlockIdx.Add(cIdx);
			chunkChange.BlockTextureID.Add(placementData.BlockTextureID[pIdx]);
			chunkChange.bHasCollision.Add(placementData.bHasCollision[pIdx]);
			placementZ++;
		}
		placementX++;
	}
	
	chunksToUpdate.Add(chunkChange);

	if (remainBlocksX)
	{	
		chunkChange.ChunkCoordinate = FIntPoint(chunkCoord.X + 1, chunkCoord.Y);
		chunkChange.BlockIdx.Empty();
		chunkChange.BlockTextureID.Empty();
		chunkChange.bHasCollision.Empty();
		
		for (int32 x = 0; x <= remainBlocksX - 1; x++)
		{
			int32 placementZ = 0;
			for (int32 z = blockStartZ; z <= blockStopZ; z++)
			{
				int32 cIdx = x + z * chunkElementCount;
				int32 pIdx = placementX + (placementZ * (placementData.Dimensions.Y));

				chunkChange.BlockIdx.Add(cIdx);
				chunkChange.BlockTextureID.Add(placementData.BlockTextureID[pIdx]);
				chunkChange.bHasCollision.Add(placementData.bHasCollision[pIdx]);
				placementZ++;
			}
			placementX++;
		}

		chunksToUpdate.Add(chunkChange);
	}

	if (remainBlocksZ)
	{
		blockStopZ = remainBlocksZ - 1;

		chunkChange.ChunkCoordinate = FIntPoint(chunkCoord.X, chunkCoord.Y - 1);
		chunkChange.BlockIdx.Empty();
		chunkChange.BlockTextureID.Empty();
		chunkChange.bHasCollision.Empty();

		for (int32 x = blockStartX; x <= blockStopX; x++)
		{
			int32 placementZ = 0;
			for (int32 z = 0; z <= remainBlocksZ - 1; z++)
			{
				int32 cIdx = x + z * chunkElementCount;
				int32 pIdx = placementX + (placementZ * (placementData.Dimensions.Y));

				chunkChange.BlockIdx.Add(cIdx);
				chunkChange.BlockTextureID.Add(placementData.BlockTextureID[pIdx]);
				chunkChange.bHasCollision.Add(placementData.bHasCollision[pIdx]);
				placementZ++;
			}
			placementX++;
		}

		chunksToUpdate.Add(chunkChange);
	}

	if (remainBlocksX && remainBlocksX)
	{
		chunkChange.ChunkCoordinate = FIntPoint(chunkCoord.X + 1, chunkCoord.Y - 1);
		chunkChange.BlockIdx.Empty();
		chunkChange.BlockTextureID.Empty();
		chunkChange.bHasCollision.Empty();

		for (int32 x = 0; x <= remainBlocksX - 1; x++)
		{
			int32 placementZ = 0;
			for (int32 z = 0; z <= remainBlocksZ - 1; z++)
			{
				int32 cIdx = x + z * chunkElementCount;
				int32 pIdx = placementX + (placementZ * (placementData.Dimensions.Y));

				chunkChange.BlockIdx.Add(cIdx);
				chunkChange.BlockTextureID.Add(placementData.BlockTextureID[pIdx]);
				chunkChange.bHasCollision.Add(placementData.bHasCollision[pIdx]);
				placementZ++;
			}
			placementX++;
		}

		chunksToUpdate.Add(chunkChange);
	}
	

	if (HasAuthority())
	{
		WorldHandlerRef->UpdateRegionData(chunksToUpdate);
	}
	else
	{
		Server_RequestRegionUpdate(chunksToUpdate);
	}

}

FIntPoint APlayerCharacter::CalculateChunkCoordPlayerAt()
{
	if (ChunkSize_player > 0)
	{
		FIntPoint toReturn = FIntPoint(FMath::Floor((GetActorLocation().X - ChunkSizeHalf_player) / ChunkSize_player) + 1, FMath::Floor((GetActorLocation().Z + ChunkSizeHalf_player) / ChunkSize_player));
		return toReturn;
	}
	return FIntPoint(0, 0);
}

uint8 APlayerCharacter::ShouldRequestChunkLoad()
{
	if (CalculateChunkCoordPlayerAt() != CenterChunkCoords)
	{
		CenterChunkCoords = CalculateChunkCoordPlayerAt();
		return true;
	}
	return false;
}

void APlayerCharacter::RequestChunkLoad(uint8 playerID, FIntPoint newChenterChunk)
{
	if (HasAuthority())
	{
		WorldHandlerRef->LoadChunks(playerID, newChenterChunk);
	}
	else
	{
		Server_RequestChunkLoad(playerID, newChenterChunk);
	}
}

void APlayerCharacter::Server_RequestChunkLoad_Implementation(uint8 playerID, FIntPoint newChenterChunk)
{
	WorldHandlerRef->LoadChunks(playerID, newChenterChunk);
}

void APlayerCharacter::Server_RequestRegionUpdate_Implementation(const TArray<FChunkChangeData>& ChunkChangeData)
{
	WorldHandlerRef->UpdateRegionData(ChunkChangeData);
}



void APlayerCharacter::ClientShowNotification_Implementation(const FText& Message)
{
	ShowNotification_BP(Message);

	// ** later make a widget class in c++ instead of BP **
	/*if (ASurvivalHUD* HUD = Cast<ASurvivalHUD>(GetHUD()))
	{
		HUD->ShowNotification(Message);
	}*/
}

void APlayerCharacter::UseItem(UItem* Item)
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
		GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("ZDAPlayerCharacterBase::UseItem()"));
		Item->OnUse(this);
		Item->Use(this);
	}
}

void APlayerCharacter::DropItem(UItem* Item, const int32 Quantity)
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

			/*Item->OwningInventory = nullptr;
			Item->InventoryIndexAt = -1;*/

			//ensure(PickupClass);
			if (APickup* Pickup = GetWorld()->SpawnActor<APickup>(PickupClass, SpawnTransform, SpawnParams))
			{
				Pickup->InitializePickup(Item->GetClass(), DroppedQuantity);
			}
		}
	}
}

void APlayerCharacter::ReorderPlayerItems(int idxAt, int newIdx)
{
	if (HasAuthority())
	{
		PlayerInventory->ReorderItems(idxAt, newIdx);
	}
	else
	{
		Server_ReorderPlayerItems(idxAt, newIdx);
	}

}

void APlayerCharacter::Server_ReorderPlayerItems_Implementation(int idxAt, int newIdx)
{
	ReorderPlayerItems(idxAt, newIdx);
}

bool APlayerCharacter::Server_ReorderPlayerItems_Validate(int idxAt, int newIdx)
{
	return true;
}

void APlayerCharacter::ItemAddedToInventory(UItem* Item)
{
}

void APlayerCharacter::ItemRemovedFromInventory(UItem* Item)
{
}

bool APlayerCharacter::EquipItem(UEquippableItem* Item)
{
	if (EquippedItems.Find(Item->Slot))
	{
		UnEquipItem(*EquippedItems.Find(Item->Slot));
	}
	EquippedItems.Add(Item->Slot, Item);
	Item->GetOwningInventory()->RemoveItem(Item);
	OnEquippedItemsChanged.Broadcast(Item->Slot, Item);
	return true;
}

bool APlayerCharacter::UnEquipItem(UEquippableItem* Item)
{
	if (Item)
	{
		if (EquippedItems.Contains(Item->Slot))
		{
			if (Item == *EquippedItems.Find(Item->Slot))
			{
				if (PlayerInventory->TryAddItem(Item).Result == EItemAddResult::IAR_AllItemsAdded)
				{
					EquippedItems.Remove(Item->Slot);
					OnEquippedItemsChanged.Broadcast(Item->Slot, nullptr);
					return true;
				}
			}
		}
	}
	return false;
}

void APlayerCharacter::EquipGear(UGearEquippableItem* Gear)
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

void APlayerCharacter::UnEquipGear(const EEquippableSlot Slot)
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

void APlayerCharacter::EquipWeapon(UWeaponItem* WeaponItem)
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

void APlayerCharacter::UnEquipWeapon()
{
	/*if (HasAuthority() && EquippedWeapon)
	{
		EquippedWeapon->OnUnEquip();
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
		OnRep_EquippedWeapon();
	}*/
}

float APlayerCharacter::ModifyHealth(const float Delta)
{
	const float OldHealth = Health;

	Health = FMath::Clamp<float>(Health + Delta, 0.f, MaxHealth);

	return Health - OldHealth;
}

void APlayerCharacter::OnRep_Health(float OldHealth)
{
	OnHealthModified(Health - OldHealth);
}

void APlayerCharacter::SetLootSource(UInventoryComponent* NewLootSource)
{
	/**If the thing we're looting gets destroyed, we need to tell the client to remove their Loot screen*/
	if (NewLootSource && NewLootSource->GetOwner())
	{
		NewLootSource->GetOwner()->OnDestroyed.AddUniqueDynamic(this, &APlayerCharacter::OnLootSourceOwnerDestroyed);
	}

	if (HasAuthority())
	{
		if (NewLootSource)
		{
			//Looting a player keeps their body alive for an extra 2 minutes to provide enough time to loot their items
			if (APlayerCharacter* Character = Cast<APlayerCharacter>(NewLootSource->GetOwner()))
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

void APlayerCharacter::LootItem(UItem* ItemToGive)
{
	if (HasAuthority())
	{
		if (PlayerInventory &&  ItemToGive /*&& LootSource && LootSource->HasItem(ItemToGive->GetClass(), ItemToGive->GetQuantity())*/)
		{
			const FItemAddResult AddResult = PlayerInventory->TryAddItem(ItemToGive);

			if (AddResult.AmountGiven > 0)
			{
				//LootSource->ConsumeItem(ItemToGive, AddResult.AmountGiven);
			}
			else
			{
				////Tell player why they couldn't loot the item
				//if (ASurvivalPlayerController* PC = Cast<ASurvivalPlayerController>(GetController()))
				//{
				//	PC->ClientShowNotification(AddResult.ErrorText);
				//}
			}
		}
	}
	else
	{
		Server_LootItem(ItemToGive);
	}
}

void APlayerCharacter::Server_LootItem_Implementation(UItem* ItemToLoot)
{
	LootItem(ItemToLoot);
}

bool APlayerCharacter::Server_LootItem_Validate(UItem* ItemToLoot)
{
	return true;
}

bool APlayerCharacter::IsLooting() const
{
	return LootSource != nullptr;
}

UMaterialInstanceDynamic* APlayerCharacter::GetSlotMaterialInstance(const EEquippableSlot Slot)
{
	if (PlayerMaterials.Contains(Slot))
	{
		return *PlayerMaterials.Find(Slot);
	}
	return nullptr;
}

void APlayerCharacter::BeginLootingPlayer(APlayerCharacter* Character)
{
	if (Character)
	{
		Character->SetLootSource(PlayerInventory);
	}
}


// -------- RPC --------
// 
//bool AZDAPlayerCharacterBase::Server_SetCharacterRotation_Validate(uint8 NewRotation)
//{
//	// You can add validation logic here if needed
//	return true;
//}

void APlayerCharacter::Server_SetCharacterRotation_Implementation(uint8 NewRotation)
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

void APlayerCharacter::Server_UseItem_Implementation(class UItem* Item)
{
	UseItem(Item);
}

bool APlayerCharacter::Server_UseItem_Validate(class UItem* Item)
{
	return true;
}

void APlayerCharacter::Server_DropItem_Implementation(class UItem* Item, const int32 Quantity)
{
	DropItem(Item, Quantity);
}

bool APlayerCharacter::Server_DropItem_Validate(class UItem* Item, const int32 Quantity)
{
	return true;
}

void APlayerCharacter::ServerSetLootSource_Implementation(class UInventoryComponent* NewLootSource)
{
	SetLootSource(NewLootSource);
}

bool APlayerCharacter::ServerSetLootSource_Validate(class UInventoryComponent* NewLootSource)
{
	return true;
}