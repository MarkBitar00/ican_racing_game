#include "CPP_RacerPlayerController.h"

void ACPP_RacerPlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	// Store references to the player's Pawn and the Enhanced Input Component
	PlayerCharacter = Cast<ACPP_Racer>(aPawn);
	EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);

	checkf(PlayerCharacter, TEXT("ACPP_RacerPlayerController derived classes should only possess ACPP_Racer derived Pawns"));
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
		EnhancedInputComponent->BindAction(ActionAccelerate, ETriggerEvent::Triggered, this, &ACPP_RacerPlayerController::HandleAccelerate);
		EnhancedInputComponent->BindAction(ActionAccelerate, ETriggerEvent::Started, this, &ACPP_RacerPlayerController::HandleStartAccelerate);
		EnhancedInputComponent->BindAction(ActionAccelerate, ETriggerEvent::Completed, this, &ACPP_RacerPlayerController::HandleStopAccelerate);
	}

	// Bind Brake Input Action handlers
	if (ActionBrake)
	{
		EnhancedInputComponent->BindAction(ActionBrake, ETriggerEvent::Started, this, &ACPP_RacerPlayerController::HandleStartBrake);
		EnhancedInputComponent->BindAction(ActionBrake, ETriggerEvent::Completed, this, &ACPP_RacerPlayerController::HandleStopBrake);
	}

	// Bind Steer Input Action handlers
	if (ActionSteer)
	{
		EnhancedInputComponent->BindAction(ActionSteer, ETriggerEvent::Triggered, this, &ACPP_RacerPlayerController::HandleSteer);
		EnhancedInputComponent->BindAction(ActionSteer, ETriggerEvent::Canceled, this, &ACPP_RacerPlayerController::HandleStopSteer);
		EnhancedInputComponent->BindAction(ActionSteer, ETriggerEvent::Completed, this, &ACPP_RacerPlayerController::HandleStopSteer);
	}

	// Bind Toggle Polarity Input Action handler
	if (ActionTogglePolarity)
		EnhancedInputComponent->BindAction(ActionTogglePolarity, ETriggerEvent::Started, this, &ACPP_RacerPlayerController::HandleTogglePolarity);
}

void ACPP_RacerPlayerController::OnUnPossess()
{
	EnhancedInputComponent->ClearActionBindings();

	Super::OnUnPossess();
}

// Input Action handlers
// Accelerate Input Action handlers
void ACPP_RacerPlayerController::HandleAccelerate(const FInputActionValue& InputActionValue)
{
	const float AccelerationValue = InputActionValue.Get<float>();
	float AccelerationSpeed = PlayerCharacter->AccelerationSpeed;
	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(PlayerCharacter->GetRootComponent());
	FVector ForwardVector = Mesh->GetForwardVector();

	Mesh->AddForce(ForwardVector * AccelerationSpeed * AccelerationValue, NAME_None, true);
}

void ACPP_RacerPlayerController::HandleStartAccelerate()
{
	PlayerCharacter->SetCameraCurrentZoom(PlayerCharacter->MaxCameraZoom);
}

void ACPP_RacerPlayerController::HandleStopAccelerate(const FInputActionInstance& InputActionInstance)
{
	const float AccelerationDuration = InputActionInstance.GetTriggeredTime();
	float DecelerationTimelinePlayRate = 1 / FMath::Clamp(AccelerationDuration, 0, PlayerCharacter->MaxDecelerationDuration);
	UTimelineComponent* DecelerationTimeline = PlayerCharacter->GetDecelerationTimeline();

	PlayerCharacter->SetCameraCurrentZoom(PlayerCharacter->CameraInitialZoom);
	DecelerationTimeline->SetPlayRate(DecelerationTimelinePlayRate);
	DecelerationTimeline->PlayFromStart();
}

// Brake Input Action handlers
void ACPP_RacerPlayerController::HandleStartBrake()
{
	UTimelineComponent* DecelerationTimeline = PlayerCharacter->GetDecelerationTimeline();

	PlayerCharacter->AccelerationSpeed = 0;
	PlayerCharacter->SetCameraCurrentZoom(PlayerCharacter->CameraInitialZoom);
	PlayerCharacter->SetCameraCurrentOffset(0);
	DecelerationTimeline->Stop();
}

void ACPP_RacerPlayerController::HandleStopBrake()
{
	PlayerCharacter->AccelerationSpeed = PlayerCharacter->GetInitialAccelerationSpeed();
}

// Steer Input Action handlers
void ACPP_RacerPlayerController::HandleSteer(const FInputActionValue& InputActionValue)
{
	const float SteerValue = InputActionValue.Get<float>();
	float SteeringSpeed = PlayerCharacter->SteeringSpeed;
	float SteeringRotationForce = PlayerCharacter->SteeringRotationForce;
	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(PlayerCharacter->GetRootComponent());

	PlayerCharacter->SetCameraCurrentOffset(SteerValue > 0 ? PlayerCharacter->MaxCameraOffset : -PlayerCharacter->MaxCameraOffset);
	Mesh->AddTorqueInDegrees(FVector(0, 0, SteerValue * SteeringSpeed), NAME_None, true);

	/*FVector RightVector = Mesh->GetRightVector();
	USceneComponent* SteerLeft = PlayerCharacter->GetSteerLeftComponent();
	USceneComponent* SteerRight = PlayerCharacter->GetSteerRightComponent();
	FVector SteerLeftLocation = SteerLeft->GetComponentLocation();
	FVector SteerRightLocation = SteerRight->GetComponentLocation();
	FVector Force = RightVector * SteeringRotationForce * (SteerValue > 0 ? 1 : -1);
	FVector Location = SteerValue > 0 ? SteerRightLocation : SteerLeftLocation;

	Mesh->AddForceAtLocation(Force, Location);*/
}

void ACPP_RacerPlayerController::HandleStopSteer()
{
	PlayerCharacter->SetCameraCurrentOffset(0);
}

// Toggle Polarity Input Action handler
void ACPP_RacerPlayerController::HandleTogglePolarity()
{
	if (!PlayerCharacter->GetCanSwitchPolarity()) return;

	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(PlayerCharacter->GetRootComponent());
	EMagneticPolarity CurrentPolarity = PlayerCharacter->GetMagneticPolarity();
	UCurveFloat* CurveBoost = PlayerCharacter->GetCurveBoost();
	ACPP_Magnet* Magnet = PlayerCharacter->GetMagnetInRange();
	float PolarityDelay = PlayerCharacter->PolarityDelay;

	EMagneticPolarity NewPolarity = CurrentPolarity == EMagneticPolarity::POSITIVE ? EMagneticPolarity::NEGATIVE : EMagneticPolarity::POSITIVE;

	PlayerCharacter->SetMagneticPolarity(NewPolarity);
	Mesh->SetMaterial(0, NewPolarity == EMagneticPolarity::POSITIVE ? PlayerCharacter->GetMaterialPositive() : PlayerCharacter->GetMaterialNegative());

	PlayerCharacter->SetCanSwitchPolarity(false);
	GetWorld()->GetTimerManager().SetTimer(PolarityTimerHandle, this, &ACPP_RacerPlayerController::OnPolarityTimerEnd, 1, false, PolarityDelay);

	if (Magnet == nullptr) return;
	if (Magnet->GetMagneticPolarity() == NewPolarity)
	{
		float CurveFloatValue = CurveBoost->GetFloatValue(PlayerCharacter->GetCurveBoostDuration());

		PlayerCharacter->AccelerationSpeed *= CurveFloatValue;
		PlayerCharacter->SetCameraCurrentZoom(PlayerCharacter->GetCameraCurrentZoom() * CurveFloatValue);

		GetWorld()->GetTimerManager().SetTimer(BoostTimerHandle, this, &ACPP_RacerPlayerController::OnBoostTimerEnd, 1, false, CurveFloatValue);
	}
	else
	{
		PlayerCharacter->AccelerationSpeed = PlayerCharacter->GetInitialAccelerationSpeed();
	}
}

void ACPP_RacerPlayerController::OnPolarityTimerEnd()
{
	PlayerCharacter->SetCanSwitchPolarity(true);
}

// Reset Acceleration Speed and Spring Arm Length when boost ends
void ACPP_RacerPlayerController::OnBoostTimerEnd()
{
	PlayerCharacter->AccelerationSpeed = PlayerCharacter->GetInitialAccelerationSpeed();
	PlayerCharacter->SetCameraCurrentZoom(PlayerCharacter->MaxCameraZoom);
}
