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

#include "WorldHandler.h"
#include "ChunkActor.h"


AZDPlayerCharacterBase::AZDPlayerCharacterBase()
{
	bReplicates = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance
	CameraBoom->bDoCollisionTest = false; // turn off the camera zoom if object between the player and the camera

	//CameraBoom->bUsePawnControlRotation = false; //set world rotation to the arm instead of relative
	//CameraBoom->bInheritPitch = false;
	//CameraBoom->bInheritYaw = false;
	//CameraBoom->bInheritRoll = false;


	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

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

	//get WorldHandler reference
		/*TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWorldHandler::StaticClass(), FoundActors);

		if (FoundActors.Num() > 0)
		{
			WorldHandlerReference = Cast<AWorldHandler>(FoundActors[0]);
		}*/

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

		// Set input mode to game only
		/*FInputModeGameOnly InputMode;
		PlayerController->SetInputMode(InputMode);*/

	}
	
	//GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, TEXT("ZDPlayerCharacterBase"));
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


// -------- OnRep --------
void AZDPlayerCharacterBase::OnRep_CharacterRotation()
{
	if (CharacterRotation)
	{
		//Controller->SetControlRotation(FRotator(0.0, 0.0, 0.0));
		FRotator Rotation = FRotator(0.0f, 180.0f, 0.0f);
		GetSprite()->SetWorldRotation(Rotation);
	}
	else
	{
		//Controller->SetControlRotation(FRotator(0.0, 180.0, 180.0));
		FRotator Rotation = FRotator(0.0f, 0.0f, 0.0f);
		GetSprite()->SetWorldRotation(Rotation);
	}
}

// -------- RPC --------
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