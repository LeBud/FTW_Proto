// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "GameFramework/Pawn.h"
#include "GliderPawn.generated.h"

UCLASS()
class FTW_API AGliderPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGliderPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category="Inputs")
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category="Inputs")
	UInputAction* RollAction;
	UPROPERTY(EditAnywhere, Category="Inputs")
	UInputAction* PitchAction;
	UPROPERTY(EditAnywhere, Category="Inputs")
	UInputAction* YawAction;
	UPROPERTY(EditAnywhere, Category="Inputs")
	UInputAction* UpgradeAction;

	UPROPERTY(EditAnywhere, Category="Rotation Sensibility")
	float RollSensibility = 400.f;
	UPROPERTY(EditAnywhere, Category="Rotation Sensibility")
	float PitchSensibility = 100.f;
	UPROPERTY(EditAnywhere, Category="Rotation Sensibility")
	float YawSensibility = 40.f;
	UPROPERTY(EditAnywhere, Category="Rotation Sensibility", meta=(ClampMin= 0.1f, ClampMax=0.9f))
	float RotationCompensation = .5f;
	
	UPROPERTY(EditAnywhere, Category="Lift", meta=(ClampMin= 0.f, ClampMax=1.f))
	float LiftEfficiency = .8f;
	UPROPERTY(EditAnywhere, Category="Lift")
	float LiftUpForce = 100.f;
	UPROPERTY(EditAnywhere, Category="Lift")
	float LiftMinSpeedThreshold = 100.f;
	UPROPERTY(EditAnywhere, Category="Lift")
	float LiftMaxForce = 200.f;
	UPROPERTY(EditAnywhere, Category="Lift")
	UCurveFloat* C_LiftAoAEfficiency;
	UPROPERTY(EditAnywhere, Category="Lift")
	UCurveFloat* C_LiftSpeedEfficiency;
	UPROPERTY(EditAnywhere, Category="Lift")
	UCurveFloat* C_RollLiftAmount;
	
	UPROPERTY(EditAnywhere, Category="Drag", meta=(ClampMin= 0.f, ClampMax=1.f))
	float DragPercent = .5f;
	UPROPERTY(EditAnywhere, Category="Drag")
	float DragMinSpeedThreshold = 100.f;
	UPROPERTY(EditAnywhere, Category="Drag")
	float DragMaxForce = 1000.f;

	UPROPERTY(EditAnywhere, Category="Velocity Correction")
	float ForwardMinThreshold = 600.f;
	UPROPERTY(EditAnywhere, Category="Velocity Correction")
	float DownSpeedThreshold = 300.f;
	UPROPERTY(EditAnywhere, Category="Velocity Correction")
	float SideMinThreshold = 100.f;
	UPROPERTY(EditAnywhere, Category="Velocity Correction", meta = (ClampMin= 0.f, ClampMax=1.f))
	float VerticalVelocityRestitution = 1.f;
	UPROPERTY(EditAnywhere, Category="Velocity Correction", meta = (ClampMin= 0.f, ClampMax=1.f))
    float ForwardVelocityRestitution = 0.6f;
    UPROPERTY(EditAnywhere, Category="Velocity Correction", meta = (ClampMin= 0.f, ClampMax=1.f))
    float SideVelocityRestitution = 1.f;
	UPROPERTY(EditAnywhere, Category="Velocity Correction")
	float MaxVerticalRestitutionForce = 2000.f;
	UPROPERTY(EditAnywhere, Category="Velocity Correction")
	float MaxForwardRestitutionForce = 1500.f;
	UPROPERTY(EditAnywhere, Category="Velocity Correction")
	float GliderOrientationCorrectionSpeed = 100.f;
	UPROPERTY(EditAnywhere, Category="Velocity Correction", meta = (ClampMin = 0.f, ClampMax=1.f))
	float ForwardAddedVelocityPercent = 0.1f;
	UPROPERTY(EditAnywhere, Category="Velocity Correction")
	UCurveFloat* C_VelocityRestitution;
	UPROPERTY(EditAnywhere, Category="Velocity Correction")
    UCurveFloat* C_AddedRollAmountCorrection;
    UPROPERTY(EditAnywhere, Category="Velocity Correction")
    UCurveFloat* C_RollAmount;
	UPROPERTY(EditAnywhere, Category="Velocity Correction")
	UCurveFloat* C_PitchCorrectionAmount;
	UPROPERTY(EditAnywhere, Category="Velocity Correction")
	UCurveFloat* C_YawCorrectionAmount;

	UPROPERTY(EditAnywhere, Category="Upgrade")
	float UpgradeDuration = 2.f;
	UPROPERTY(EditAnywhere, Category="Upgrade")
	float UpgradeThrusterForce = 200.f;
	
	UPROPERTY(EditAnywhere, Category="Camera Settings")
	UCurveFloat* C_TargetArmLenght;
	UPROPERTY(EditAnywhere, Category="Camera Settings")
    UCurveFloat* C_TargetArmOffset;
	UPROPERTY(EditAnywhere, Category="Camera Settings")
    float MinCameraFov = 85.f;
    UPROPERTY(EditAnywhere, Category="Camera Settings")
    float MaxCameraFov = 105.f;
    UPROPERTY(EditAnywhere, Category="Camera Settings")
    float CameraFovLerpSpeed = 3000.f;
	
	FVector GetVelocity();
	FVector ForwardDir;
	FVector RightDir;
	FVector UpDir;

	UPROPERTY(BlueprintReadOnly)
	float ForwardSpeed;

	UPROPERTY(BlueprintReadWrite)
	bool bIsInCloudWind;
	
	UPROPERTY()
	bool bIsUsingController;

	bool bIsUsingUpgrade;
	float UpgradeUseTimer;
	
	
	void CameraLook(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);
	void Pitch(const FInputActionValue& Value);
	void Yaw(const FInputActionValue& Value);
	void UpgradeUse(const FInputActionValue& Value);

	void UpdateDrag();
	void UpdateLift();
	void UpdateVelocity();
	void UpdateCamera();
	
	UFUNCTION()
	void OnHardwareDeviceChanged(const FPlatformUserId UserId, const FInputDeviceId DeviceId);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USphereComponent* Sphere;
	
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* Mesh;
	
	UPROPERTY(EditAnywhere)
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera;
};
