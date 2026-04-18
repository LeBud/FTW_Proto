// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GliderPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FTW_API AGliderPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void SetupInputComponent() override;
	
	UPROPERTY(EditAnywhere, Category="Input")
	class UInputMappingContext* PawnMappingContext;
	
	//UPROPERTY()
	//class UInputAction* RollAction;
	
};
