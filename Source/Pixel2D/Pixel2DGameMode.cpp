// Copyright Epic Games, Inc. All Rights Reserved.

#include "Pixel2DGameMode.h"
#include "Pixel2DCharacter.h"
#include "UObject/ConstructorHelpers.h"

APixel2DGameMode::APixel2DGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
