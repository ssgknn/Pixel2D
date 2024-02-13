// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "InputActionValue.h"
#include "ZDPlayerCharacterBase.generated.h"

/**
 * 
 */
UCLASS()
class PIXEL2D_API AZDPlayerCharacterBase : public APaperZDCharacter
{
	GENERATED_BODY()

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

private:
#pragma region Input
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


public:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterRotation)
	uint8 CharacterRotation;
	UFUNCTION()
	void OnRep_CharacterRotation();
	UFUNCTION(Server, unreliable)
	void Server_SetCharacterRotation(uint8 NewRotation);

	//Items

	UFUNCTION(BlueprintCallable, Category = "Items")
	void UseItem(class UItem* Item);

	/**[Server] Use an item from our inventory.*/
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UseItem(class UItem* Item);

	/**[Server] Drop an item*/
	UFUNCTION(BlueprintCallable, Category = "Items")
	void DropItem(class UItem* Item, const int32 Quantity);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DropItem(class UItem* Item, const int32 Quantity);

	UFUNCTION()
	void ItemAddedToInventory(class UItem* Item);

	UFUNCTION()
	void ItemRemovedFromInventory(class UItem* Item);


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

};
