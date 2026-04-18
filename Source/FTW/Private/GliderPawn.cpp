// Fill out your copyright notice in the Description page of Project Settings.


#include "GliderPawn.h"

#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AGliderPawn::AGliderPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	SetRootComponent(Sphere);
	Sphere->SetSimulatePhysics(true);
	Sphere->SetEnableGravity(false);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Sphere);
	
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Sphere);
	SpringArm->TargetArmLength = 1200.f;
	SpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
}

// Called when the game starts or when spawned
void AGliderPawn::BeginPlay()
{
	Super::BeginPlay();

	UInputDeviceSubsystem* InputDeviceSubsystem = GEngine->GetEngineSubsystem<UInputDeviceSubsystem>();
	InputDeviceSubsystem->OnInputHardwareDeviceChanged.AddDynamic(this, &AGliderPawn::OnHardwareDeviceChanged);
	bIsUsingController = InputDeviceSubsystem->GetMostRecentlyUsedHardwareDevice(GetWorld()->GetFirstPlayerController()->GetPlatformUserId()).PrimaryDeviceType == EHardwareDevicePrimaryType::Gamepad;
}

void AGliderPawn::OnHardwareDeviceChanged(const FPlatformUserId UserId, const FInputDeviceId DeviceId) {
	UInputDeviceSubsystem* InputDeviceSubsystem = GEngine->GetEngineSubsystem<UInputDeviceSubsystem>();
	bIsUsingController = InputDeviceSubsystem->GetMostRecentlyUsedHardwareDevice(UserId).PrimaryDeviceType == EHardwareDevicePrimaryType::Gamepad;
}

#pragma region InputsFunction
// Called to bind functionality to input
void AGliderPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGliderPawn::CameraLook);
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &AGliderPawn::Roll);
		EnhancedInputComponent->BindAction(PitchAction, ETriggerEvent::Triggered, this, &AGliderPawn::Pitch);
		EnhancedInputComponent->BindAction(YawAction, ETriggerEvent::Triggered, this, &AGliderPawn::Yaw);
	}
	
}

void AGliderPawn::CameraLook(const FInputActionValue& Value) {
	FVector2D LookAxis = Value.Get<FVector2D>();
	
	GetWorld()->GetFirstPlayerController()->GetPawn()->AddControllerYawInput(LookAxis.X);
	
	if (bIsUsingController) GetWorld()->GetFirstPlayerController()->GetPawn()->AddControllerPitchInput(LookAxis.Y);
	else GetWorld()->GetFirstPlayerController()->GetPawn()->AddControllerPitchInput(-LookAxis.Y);
}

void AGliderPawn::Roll(const FInputActionValue& Value) {
	float value = Value.Get<float>();
	
	Sphere->AddTorqueInDegrees(Sphere->GetForwardVector() * -value * RollSensibility, "None", true);
}

void AGliderPawn::Pitch(const FInputActionValue& Value) {
	float value = Value.Get<float>();

	Sphere->AddTorqueInDegrees(Sphere->GetRightVector() * value * PitchSensibility, "None", true);
}

void AGliderPawn::Yaw(const FInputActionValue& Value) {
	float value = Value.Get<float>();

	Sphere->AddTorqueInDegrees(Sphere->GetUpVector() * value * YawSensibility, "None", true);
}

#pragma endregion InputsFunction

// Called every frame
void AGliderPawn::Tick(float DeltaTime)
{
	//==== Les fonctions de DEBUG ====
	// if(GEngine)
	// 	GEngine->AddOnScreenDebugMessage(1, 15.0f, FColor::Yellow,
	// 		FString::Printf(TEXT("World delta for current frame equals %f %f %f"), drag.X, drag.Y, drag.Z));
	// DrawDebugLine(GetWorld(), Sphere->GetComponentLocation(), Sphere->GetComponentLocation() + drag, FColor::Yellow);
	
	Super::Tick(DeltaTime);

	ForwardDir = Sphere->GetForwardVector();
	RightDir = Sphere->GetRightVector();
	UpDir = Sphere->GetUpVector();
	ForwardSpeed = FVector::DotProduct(GetVelocity(), ForwardDir);

	//Gravity
	Sphere->AddForce(FVector(0,0,-980.f), "None", true);
	
	//Rotation Limiter
	FVector rotationLimit = -Sphere->GetPhysicsAngularVelocityInDegrees() / RotationCompensation;
	Sphere->AddTorqueInDegrees(rotationLimit, "None", true);

	//Gestion de l'avion a basse vitesse catastrophique pour les courants ascendants
	//Retravailler la maniabilité globale
	//Ajouté une gestion de l'AoA sur le drag
	//Regarder pour ajouter les force de Rudder (aile arrière)
	//Regarder le short sur le channel wip-GP
	
	UpdateDrag();
	UpdateLift();
	UpdateVelocity();
	UpdateCamera();

	if (bIsInCloudWind){
		Sphere->AddForce(FVector::UpVector * 400.f, "None", true);
		if (ForwardSpeed < ForwardMinThreshold) {
			Sphere->AddForce(ForwardDir * ForwardMinThreshold , "None", true);
		}
	}
	
	if(GEngine)
        GEngine->AddOnScreenDebugMessage(1, 15.0f, FColor::Orange,
        FString::Printf(TEXT("Forward Speed %f"), ForwardSpeed));
}

void AGliderPawn::UpdateDrag() {
	if (ForwardSpeed < DragMinSpeedThreshold) return;
	
	FVector WorldDragDir = -GetVelocity();
	FVector LocalDragDir = -ForwardDir;
	
	FVector DragDir = (WorldDragDir + LocalDragDir).GetSafeNormal();
	
	float DragForce = ForwardSpeed * DragPercent;
	
	if (DragForce > DragMaxForce) DragForce = DragMaxForce;
	
	Sphere->AddForce(DragDir * DragForce, "None", true);
	DrawDebugLine(GetWorld(), Sphere->GetComponentLocation(), Sphere->GetComponentLocation() + DragDir * DragForce, FColor::Blue);
	if(GEngine)
    	GEngine->AddOnScreenDebugMessage(2, 15.0f, FColor::Blue,
    	FString::Printf(TEXT("Drag Force %f"), DragForce));
}

void AGliderPawn::UpdateLift() {
	//Je pense qu'il faudra que je modifie pour que le lift change en fonction du roll de l'avion
	//Que le lift soit moins fort quand il est penché, et qu'il n'y en ai pas du tout quand l'avion est à l'envers
	//Je me dis en additionnant le vecteur Up et Right ça peut aider à avoir encore plus l'effet de tournant
	if (ForwardSpeed < LiftMinSpeedThreshold) return;
	
	float LiftForce = LiftUpForce * ForwardSpeed * LiftEfficiency;
	
	LiftForce = FMath::Clamp(LiftForce, -LiftMaxForce, LiftMaxForce);
	
	LiftForce *= C_LiftAoAEfficiency->GetFloatValue(Sphere->GetComponentRotation().Pitch);
	LiftForce *= C_LiftSpeedEfficiency->GetFloatValue(ForwardSpeed);
	
	Sphere->AddForce(UpDir * LiftForce, "None", true);
	
	DrawDebugLine(GetWorld(), Sphere->GetComponentLocation(), Sphere->GetComponentLocation() + UpDir * LiftForce * C_LiftSpeedEfficiency->GetFloatValue(ForwardSpeed), FColor::Green);
	if(GEngine)
        	GEngine->AddOnScreenDebugMessage(3, 15.0f, FColor::Green,
        	FString::Printf(TEXT("Lift Force %f"), LiftForce * C_LiftSpeedEfficiency->GetFloatValue(ForwardSpeed)));
}

void AGliderPawn::UpdateVelocity() {
	
	if (GetVelocity().Size() < 0.f) return;

	//Correction d'orientation par rapport à l'angle et à la vélocité du planeur
	//Potentiellement le déplacer et le lier aux fonctions de rotations ??
	//Essayer d'ajouter un facteur d'angle entre la vélocité et la direction pour ajuster plus ou moins fort la direction
	//Peut être check la magnitude du vecteur pour faire l'effet plus ou moins fort
	
	FVector Axis = FVector::CrossProduct(ForwardDir, GetVelocity());
	Sphere->AddTorqueInDegrees(Axis * GliderOrientationCorrectionSpeed, "None", true);

	//Add Yaw and Pitch to the plane when Roll
	FVector YawCorrection = FVector(0, 0, C_YawCorrectionAmount->GetFloatValue(ForwardSpeed));
	FVector PitchCorrection = FVector(0, C_PitchCorrectionAmount->GetFloatValue(ForwardSpeed), 0);
	Sphere->AddTorqueInDegrees((YawCorrection * C_RollAmount->GetFloatValue(Sphere->GetComponentRotation().Roll)
		+ PitchCorrection * FMath::Abs(C_RollAmount->GetFloatValue(Sphere->GetComponentRotation().Roll)))
		* C_AddedRollAmountCorrection->GetFloatValue(ForwardSpeed), "None", true);
	
	if (ForwardSpeed < ForwardMinThreshold) return;
	
	float DownSpeed = FVector::DotProduct(GetVelocity(), -UpDir);
	float RightSpeed =  FVector::DotProduct(GetVelocity(), RightDir);
	float LeftSpeed = FVector::DotProduct(GetVelocity(), -RightDir);
	
	float SideCompensatedForce = 0;
	float VerticalCompensatedForce = 0;
	float ForwardCompensatedForce = 0;
	
	FVector ForcesSum = FVector::ZeroVector;
	
	//Force restitution, to loose less speed over time/distance
	Sphere->AddForce(ForwardDir * ForwardSpeed * ForwardAddedVelocityPercent, "None", true);
	
	if (DownSpeed > DownSpeedThreshold){
		//Potentiellement ajouter la compensation pour le up dir ??
		VerticalCompensatedForce = DownSpeed * VerticalVelocityRestitution;
		ForwardCompensatedForce = DownSpeed * ForwardVelocityRestitution;

		VerticalCompensatedForce = FMath::Clamp(VerticalCompensatedForce, 0, MaxVerticalRestitutionForce);
		ForwardCompensatedForce = FMath::Clamp(ForwardCompensatedForce, 0, MaxForwardRestitutionForce);
		
		VerticalCompensatedForce *= C_VelocityRestitution->GetFloatValue(ForwardSpeed);
		ForwardCompensatedForce *= C_VelocityRestitution->GetFloatValue(ForwardSpeed);
		
		ForcesSum += UpDir * VerticalCompensatedForce + ForwardDir * ForwardCompensatedForce;
		
		//Sphere->AddForce(UpDir * VerticalCompensatedForce, "None", true);
		//Sphere->AddForce(ForwardDir * ForwardCompensatedForce, "None", true);
	}
	
	if (RightSpeed > SideMinThreshold){
		SideCompensatedForce = -RightSpeed * SideVelocityRestitution;
		//Sphere->AddForce(RightDir * SideCompensatedForce, "None", true);
		ForcesSum += RightDir * SideCompensatedForce;
	}
	else if (LeftSpeed > SideMinThreshold) {
		SideCompensatedForce = LeftSpeed * SideVelocityRestitution;
		//Sphere->AddForce(RightDir * SideCompensatedForce, "None", true);
		ForcesSum += RightDir * SideCompensatedForce;
	}

	Sphere->AddForce(ForcesSum, "None", true);

	DrawDebugLine(GetWorld(), Sphere->GetComponentLocation(), Sphere->GetComponentLocation() + UpDir * VerticalCompensatedForce, FColor::Red);
	DrawDebugLine(GetWorld(), Sphere->GetComponentLocation(), Sphere->GetComponentLocation() + ForwardDir * ForwardCompensatedForce, FColor::Red);
	DrawDebugLine(GetWorld(), Sphere->GetComponentLocation(), Sphere->GetComponentLocation() + RightDir * SideCompensatedForce, FColor::Yellow);
	
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(4, 15.0f, FColor::Red,
		FString::Printf(TEXT("Compensated Forces : Up %f / Forward %f"), VerticalCompensatedForce, ForwardCompensatedForce));
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(5, 15.0f, FColor::Yellow,
		FString::Printf(TEXT("Side Compensated Force %f"), SideCompensatedForce));
}

void AGliderPawn::UpdateCamera() {
	SpringArm->TargetArmLength = C_TargetArmLenght->GetFloatValue(ForwardSpeed);
	SpringArm->SocketOffset.Y = C_TargetArmOffset->GetFloatValue(ForwardSpeed);
		
    Camera->FieldOfView = FMath::Clamp(FMath::Lerp(MinCameraFov, MaxCameraFov, ForwardSpeed / CameraFovLerpSpeed), MinCameraFov, MaxCameraFov);
}


FVector AGliderPawn::GetVelocity() {
 return Sphere->GetPhysicsLinearVelocity();
}



