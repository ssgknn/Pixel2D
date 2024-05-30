// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"

TArray<FString> UMainMenuWidget::GetWorldNames()
{
    TArray<FString> WorldNames;
    FString WorldsDirectory = FPaths::ProjectSavedDir() / TEXT("WORLDS");

    // Get the platform file manager
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    // Check if the WORLDS directory exists
    if (PlatformFile.DirectoryExists(*WorldsDirectory))
    {
        // Lambda function to process each directory entry
        PlatformFile.IterateDirectory(*WorldsDirectory, [&](const TCHAR* FilenameOrDirectory, bool bIsDirectory) -> bool
            {
                if (bIsDirectory)
                {
                    FString WorldName = FPaths::GetBaseFilename(FilenameOrDirectory);
                    WorldNames.Add(WorldName);
                }
                return true; // Continue iterating
            });
    }

    return WorldNames;
}
