// Copyright Epic Games, Inc. All Rights Reserved.

#include "Pixel2DGameMode.h"
#include "../Player/PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"

APixel2DGameMode::APixel2DGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Characters/Player/BP_PlayerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
