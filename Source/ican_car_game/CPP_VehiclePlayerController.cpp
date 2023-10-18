#include "CPP_VehiclePlayerController.h"

void ACPP_VehiclePlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	// Store references to the player's Pawn and the Enhanced Input Component
	PlayerCharacter = Cast<ACPP_Vehicle>(aPawn);
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);

	checkf(PlayerCharacter, TEXT("ACPP_VehiclePlayerController derived classes should only possess ACPP_Vehicle derived Pawns"));
	checkf(EnhancedInputComponent, TEXT("Unable to get reference to the Enhanced Input Component"));
	
	// Get the player's Subsystem, clear mappings and add required mapping
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	checkf(InputSubsystem, TEXT("Unable to get reference to the Enhanced Input Local Player Subsystem"));
	checkf(InputMappingContext, TEXT("Input Mapping Context not specified"));

	InputSubsystem->ClearAllMappings();
	InputSubsystem->AddMappingContext(InputMappingContext, 0);

	// Bind Input Actions to handler functions
	// Bind Accelerate Input Action handlers
	if (ActionAccelerate)
	{
		EnhancedInputComponent->BindAction(ActionAccelerate, ETriggerEvent::Triggered, this, &ACPP_VehiclePlayerController::HandleAccelerate);
		EnhancedInputComponent->BindAction(ActionAccelerate, ETriggerEvent::Started, this, &ACPP_VehiclePlayerController::HandleStartAccelerate);
		EnhancedInputComponent->BindAction(ActionAccelerate, ETriggerEvent::Completed, this, &ACPP_VehiclePlayerController::HandleStopAccelerate);
	}

	// Bind Brake Input Action handlers
	if (ActionBrake)
	{
		EnhancedInputComponent->BindAction(ActionBrake, ETriggerEvent::Started, this, &ACPP_VehiclePlayerController::HandleStartBrake);
		EnhancedInputComponent->BindAction(ActionBrake, ETriggerEvent::Completed, this, &ACPP_VehiclePlayerController::HandleStopBrake);
	}

	// Bind Steer Input Action handlers
	if (ActionSteer)
	{
		EnhancedInputComponent->BindAction(ActionSteer, ETriggerEvent::Triggered, this, &ACPP_VehiclePlayerController::HandleSteer);
		EnhancedInputComponent->BindAction(ActionSteer, ETriggerEvent::Canceled, this, &ACPP_VehiclePlayerController::HandleStopSteer);
		EnhancedInputComponent->BindAction(ActionSteer, ETriggerEvent::Completed, this, &ACPP_VehiclePlayerController::HandleStopSteer);
	}

	// Bind Toggle Polarity Input Action handler
	if (ActionTogglePolarity)
		EnhancedInputComponent->BindAction(ActionTogglePolarity, ETriggerEvent::Started, this, &ACPP_VehiclePlayerController::HandleTogglePolarity);
}

void ACPP_VehiclePlayerController::OnUnPossess()
{
	EnhancedInputComponent->ClearActionBindings();

	Super::OnUnPossess();
}

// Input Action handlers
// Accelerate Input Action handlers
void ACPP_VehiclePlayerController::HandleAccelerate(const FInputActionValue& InputActionValue)
{
	const float AccelerationValue = InputActionValue.Get<float>();
	float AccelerationSpeed = PlayerCharacter->AccelerationSpeed;
	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(PlayerCharacter->GetRootComponent());
	FVector ForwardVector = Mesh->GetForwardVector();

	Mesh->AddForce(ForwardVector * AccelerationSpeed * AccelerationValue, NAME_None, true);
}

void ACPP_VehiclePlayerController::HandleStartAccelerate()
{
	PlayerCharacter->SetCameraCurrentZoom(PlayerCharacter->MaxCameraZoom);
}

void ACPP_VehiclePlayerController::HandleStopAccelerate(const FInputActionInstance& InputActionInstance)
{
	const float AccelerationDuration = InputActionInstance.GetTriggeredTime();
	float DecelerationTimelinePlayRate = 1 / FMath::Clamp(AccelerationDuration, 0, PlayerCharacter->MaxDecelerationDuration);
	UTimelineComponent* DecelerationTimeline = PlayerCharacter->GetDecelerationTimeline();

	PlayerCharacter->SetCameraCurrentZoom(PlayerCharacter->CameraInitialZoom);
	DecelerationTimeline->SetPlayRate(DecelerationTimelinePlayRate);
	DecelerationTimeline->PlayFromStart();
}

// Brake Input Action handlers
void ACPP_VehiclePlayerController::HandleStartBrake()
{
	UTimelineComponent* DecelerationTimeline = PlayerCharacter->GetDecelerationTimeline();

	PlayerCharacter->AccelerationSpeed = 0;
	PlayerCharacter->SetCameraCurrentZoom(PlayerCharacter->CameraInitialZoom);
	PlayerCharacter->SetCameraCurrentOffset(0);
	DecelerationTimeline->Stop();
}

void ACPP_VehiclePlayerController::HandleStopBrake()
{
	PlayerCharacter->AccelerationSpeed = PlayerCharacter->GetInitialAccelerationSpeed();
}

// Steer Input Action handlers
void ACPP_VehiclePlayerController::HandleSteer(const FInputActionValue& InputActionValue)
{
	const float SteerValue = InputActionValue.Get<float>();
	float SteeringSpeed = PlayerCharacter->SteeringSpeed;
	float SteeringRotationForce = PlayerCharacter->SteeringRotationForce;
	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(PlayerCharacter->GetRootComponent());

	PlayerCharacter->SetCameraCurrentOffset(SteerValue > 0 ? PlayerCharacter->MaxCameraOffset : -PlayerCharacter->MaxCameraOffset);
	Mesh->AddTorqueInDegrees(FVector(0, 0, SteerValue * SteeringSpeed), NAME_None, true);

	FVector RightVector = Mesh->GetRightVector();
	USceneComponent* SteerLeft = PlayerCharacter->GetSteerLeftComponent();
	USceneComponent* SteerRight = PlayerCharacter->GetSteerRightComponent();
	FVector SteerLeftLocation = SteerLeft->GetComponentLocation();
	FVector SteerRightLocation = SteerRight->GetComponentLocation();
	FVector Force = RightVector * SteeringRotationForce * (SteerValue > 0 ? 1 : -1);
	FVector Location = SteerValue > 0 ? SteerRightLocation : SteerLeftLocation;

	Mesh->AddForceAtLocation(Force, Location);
}

void ACPP_VehiclePlayerController::HandleStopSteer()
{
	PlayerCharacter->SetCameraCurrentOffset(0);
}

// Toggle Polarity Input Action handler
void ACPP_VehiclePlayerController::HandleTogglePolarity()
{
	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(PlayerCharacter->GetRootComponent());
	EMagneticPolarity Polarity = PlayerCharacter->GetMagneticPolarity();

	PlayerCharacter->SetMagneticPolarity(Polarity == EMagneticPolarity::POSITIVE ? EMagneticPolarity::NEGATIVE : EMagneticPolarity::POSITIVE);
	Mesh->SetMaterial(0, Polarity == EMagneticPolarity::POSITIVE ? PlayerCharacter->GetMaterialNegative() : PlayerCharacter->GetMaterialPositive());

	/* TODO if Magnet In Range, trigger boost */
}
