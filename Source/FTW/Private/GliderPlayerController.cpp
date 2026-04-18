// Fill out your copyright notice in the Description page of Project Settings.


#include "GliderPlayerController.h"
#include "EnhancedInputSubsystems.h"

void AGliderPlayerController::SetupInputComponent() {
	Super::SetupInputComponent();
	
	if (IsLocalPlayerController()){
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer())) {
			Subsystem->AddMappingContext(PawnMappingContext,0);
		}
	}
}
