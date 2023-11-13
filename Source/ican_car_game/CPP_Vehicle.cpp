#include "CPP_Vehicle.h"
#include "CPP_Magnet.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
ACPP_Vehicle::ACPP_Vehicle()
{
	// Set Pawn replication
	SetReplicates(true);
	SetReplicateMovement(true);

 	// Set this Pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create Mesh, Materials for Magnetic Polarity and set Root Component with default Scale and Material
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UMaterialInstance>
		MaterialPositiveFile(TEXT("/Game/Materials/MaterialInstances/M_Positive")),
		MaterialNegativeFile(TEXT("/Game/Materials/MaterialInstances/M_Negative"));
	MaterialPositive = MaterialPositiveFile.Object;
	MaterialNegative = MaterialNegativeFile.Object;
	Mesh->SetMaterial(0, MaterialPositiveFile.Object);

	// Create Spring Arm and attach it to Root Component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = CameraInitialZoom;
	SpringArm->SocketOffset = FVector(0, 0, SpringArmTargetOffset);
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// Create Camera and attach it to Spring Arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetRelativeRotation(FRotator(CameraRotation, 0, 0));
	Camera->SetFieldOfView(CameraFieldOfView);

	// Create and setup Hover Components
	HoverFrontLeft = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverFrontLeft"));
	HoverFrontLeft->SetupAttachment(RootComponent);
	HoverFrontRight = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverFrontRight"));
	HoverFrontRight->SetupAttachment(RootComponent);
	HoverBackLeft = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverBackLeft"));
	HoverBackLeft->SetupAttachment(RootComponent);
	HoverBackRight = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverBackRight"));
	HoverBackRight->SetupAttachment(RootComponent);

	// Create and setup Steer locations
	SteerLeftLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SteerLeftLocation"));
	SteerLeftLocation->SetupAttachment(RootComponent);
	SteerRightLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SteerRightLocation"));
	SteerRightLocation->SetupAttachment(RootComponent);

	// Create Curves and set default
	CurveAttraction = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveAttraction"));
	CurveRepulsion = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveRepulsion"));
	CurveBoost = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveBoost"));
	CurveTimeline = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveTimeline"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		CurveAttractionFile(TEXT("/Game/Utils/Curves/AttractionCurve")),
		CurveRepulsionFile(TEXT("/Game/Utils/Curves/RepulsionCurve")),
		CurveBoostFile(TEXT("/Game/Utils/Curves/BoostCurve")),
		CurveTimelineFile(TEXT("/Game/Utils/Curves/TimelineFloatDescCurve"));
	CurveAttraction = CurveAttractionFile.Object;
	CurveRepulsion = CurveRepulsionFile.Object;
	CurveBoost = CurveBoostFile.Object;
	CurveTimeline = CurveTimelineFile.Object;

	// Create Timeline and bind function to it
	TimelineDeceleration = CreateDefaultSubobject<UTimelineComponent>(TEXT("TimelineDeceleration"));
	TimelineUpdate.BindUFunction(this, FName{ TEXT("TimelineDecelerationUpdate") });

	// Add tag for Magnet overlaps
	this->Tags.Add(FName("HoverVehicle"));
}

// Called when the game starts or when spawned
void ACPP_Vehicle::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set initial attribute values
	InitialAccelerationSpeed = AccelerationSpeed;
	CameraCurrentZoom = CameraInitialZoom;
	CameraCurrentOffset = 0;

	// Setup Mesh properties
	Mesh->SetSimulatePhysics(true);
	Mesh->SetLinearDamping(LinearDamping);
	Mesh->SetAngularDamping(AngularDamping);

	// Initialize Hover Components
	HoverFrontLeft->Init(Mesh, HoverHeight, HoverForce, GravityForce);
	HoverFrontRight->Init(Mesh, HoverHeight, HoverForce, GravityForce);
	HoverBackLeft->Init(Mesh, HoverHeight, HoverForce, GravityForce);
	HoverBackRight->Init(Mesh, HoverHeight, HoverForce, GravityForce);

	// Bind Timeline functions
	TimelineDeceleration->AddInterpFloat(CurveTimeline, TimelineUpdate);
}

// Called every frame
void ACPP_Vehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smooth out Spring Arm length and offset movements
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, CameraCurrentZoom, DeltaTime, CameraInterpolationSpeed);
	SpringArm->SocketOffset.Y = FMath::FInterpTo(SpringArm->SocketOffset.Y, CameraCurrentOffset, DeltaTime, CameraInterpolationSpeed);

	// Smooth out Center Of Mass Location
	UpdateCenterOfMass();
	Mesh->SetCenterOfMass(FVector(0, 0, FMath::FInterpTo(Mesh->GetCenterOfMass().Z, -CenterOfMassHeight, DeltaTime, 100)));

	// Clamp Mesh rotation
	FRotator CurrentRotation = Mesh->GetComponentRotation();
	CurrentRotation.Roll = FMath::Clamp(CurrentRotation.Roll, -MaxRotation, MaxRotation);
	CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch, -MaxRotation, MaxRotation);
	Mesh->SetWorldRotation(CurrentRotation);
}

// Called to bind functionality to input
void ACPP_Vehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up Input Action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Bind Accelerate Input Action handler methods
		if (ActionAccelerate)
		{
			EnhancedInputComponent->BindAction(ActionAccelerate, ETriggerEvent::Triggered, this, &ACPP_Vehicle::Accelerate);
			EnhancedInputComponent->BindAction(ActionAccelerate, ETriggerEvent::Started, this, &ACPP_Vehicle::StartAccelerate);
			EnhancedInputComponent->BindAction(ActionAccelerate, ETriggerEvent::Completed, this, &ACPP_Vehicle::StopAccelerate);
		}

		// Bind Brake Input Action handler methods
		if (ActionBrake)
		{
			EnhancedInputComponent->BindAction(ActionBrake, ETriggerEvent::Started, this, &ACPP_Vehicle::StartBrake);
			EnhancedInputComponent->BindAction(ActionBrake, ETriggerEvent::Completed, this, &ACPP_Vehicle::StopBrake);
		}

		// Bind Steer Input Action handler methods
		if (ActionSteer)
		{
			EnhancedInputComponent->BindAction(ActionSteer, ETriggerEvent::Triggered, this, &ACPP_Vehicle::Steer);
			EnhancedInputComponent->BindAction(ActionSteer, ETriggerEvent::Canceled, this, &ACPP_Vehicle::StopSteer);
			EnhancedInputComponent->BindAction(ActionSteer, ETriggerEvent::Completed, this, &ACPP_Vehicle::StopSteer);
		}

		// Bind Toggle Polarity Input Action handler method
		if (ActionTogglePolarity)
			EnhancedInputComponent->BindAction(ActionTogglePolarity, ETriggerEvent::Started, this, &ACPP_Vehicle::HandleTogglePolarity);
	}
}

// Called during the Deceleration Timeline's update event
void ACPP_Vehicle::TimelineDecelerationUpdate(float Alpha)
{
	Mesh->AddForce(Mesh->GetForwardVector() * AccelerationSpeed * Alpha, NAME_None, true);
}

void ACPP_Vehicle::UpdateCenterOfMass()
{
	FVector WorldLocation = Mesh->GetComponentLocation();
	FVector UpVector = Mesh->GetUpVector();

	FHitResult Hit;
	FVector LineTraceEndLocation = (UpVector * -HoverHeight * 100) + WorldLocation;
	UWorld* World = GetWorld();

	bool bHit = World->LineTraceSingleByChannel(Hit, WorldLocation, LineTraceEndLocation, ECC_Visibility);
	DrawDebugLine(World, WorldLocation, LineTraceEndLocation, FColor::Yellow);

	if (Hit.Distance <= HoverHeight)
	{
		CenterOfMassHeight = 100;
	}
	else
	{
		CenterOfMassHeight = 0;
	}
}

float ACPP_Vehicle::GetCurveBoostDuration()
{
	if (MagnetInRange == nullptr) return 0;

	FVector Location = GetActorLocation();
	FVector MagnetLocation = MagnetInRange->GetActorLocation();
	FVector VectorProjectionPlaneNormal = FVector(0, 0, 1);
	float LocationsDistance = FVector::Dist(FVector::VectorPlaneProject(Location, VectorProjectionPlaneNormal), FVector::VectorPlaneProject(MagnetLocation, VectorProjectionPlaneNormal));

	return LocationsDistance / MagnetInRange->ColliderRadius;
}

// Get Camera properties
float ACPP_Vehicle::GetCameraCurrentZoom()
{
	return CameraCurrentZoom;
}

float ACPP_Vehicle::GetInitialAccelerationSpeed()
{
	return InitialAccelerationSpeed;
}

// Get Magnetism properties
bool ACPP_Vehicle::GetCanSwitchPolarity()
{
	return bCanSwitchPolarity;
}

EMagneticPolarity ACPP_Vehicle::GetMagneticPolarity()
{
	return MagneticPolarity;
}

// Set Camera properties
void ACPP_Vehicle::SetCameraCurrentZoom(float Zoom)
{
	CameraCurrentZoom = Zoom;
}

void ACPP_Vehicle::SetCameraCurrentOffset(float Pitch)
{
	CameraCurrentOffset = Pitch;
}

// Set Magnetism properties
void ACPP_Vehicle::SetCanSwitchPolarity(bool bCanSwitch)
{
	bCanSwitchPolarity = bCanSwitch;
}

void ACPP_Vehicle::SetMagneticPolarity(EMagneticPolarity Polarity)
{
	MagneticPolarity = Polarity;
}

void ACPP_Vehicle::SetMagnetInRange(ACPP_Magnet* Magnet)
{
	MagnetInRange = Magnet;
}

// Input Action handlers
// Accelerate Input Action handlers
void ACPP_Vehicle::Accelerate(const struct FInputActionValue& Value)
{
	const float AccelerationValue = Value.Get<float>();
	FVector ForwardVector = Mesh->GetForwardVector();

	HandleAccelerate(AccelerationValue, ForwardVector);
}

void ACPP_Vehicle::HandleAccelerate_Implementation(float Acceleration, FVector Forward)
{
	Mesh->AddForce(Forward * AccelerationSpeed * Acceleration, NAME_None, true);
}

void ACPP_Vehicle::StartAccelerate()
{
	CameraCurrentZoom = MaxCameraZoom;
}

void ACPP_Vehicle::StopAccelerate(const struct FInputActionInstance& Instance)
{
	const float AccelerationDuration = Instance.GetTriggeredTime();
	float DecelerationTimelinePlayRate = 1 / FMath::Clamp(AccelerationDuration, 0, MaxDecelerationDuration);

	CameraCurrentZoom = CameraInitialZoom;
	TimelineDeceleration->SetPlayRate(DecelerationTimelinePlayRate);
	TimelineDeceleration->PlayFromStart();
}

// Brake Input Action handlers
void ACPP_Vehicle::StartBrake()
{
	AccelerationSpeed = 0;
	CameraCurrentZoom = CameraInitialZoom;
	CameraCurrentOffset = 0;
	TimelineDeceleration->Stop();
}

void ACPP_Vehicle::StopBrake()
{
	AccelerationSpeed = InitialAccelerationSpeed;
}

// Steer Input Action handlers
void ACPP_Vehicle::Steer(const struct FInputActionValue& Value)
{
	const float SteerValue = Value.Get<float>();

	if (SteerValue < 0.1 && SteerValue > -0.1) return;

	CameraCurrentOffset = SteerValue > 0 ? MaxCameraOffset : -MaxCameraOffset;

	FVector RightVector = Mesh->GetRightVector();
	FVector LeftLocation = SteerLeftLocation->GetComponentLocation();
	FVector RightLocation = SteerRightLocation->GetComponentLocation();
	FVector Force = RightVector * SteeringRotationForce * (SteerValue > 0 ? 1 : -1);
	FVector Location = SteerValue > 0 ? RightLocation : LeftLocation;

	HandleSteer(SteerValue, Force, Location);
}

void ACPP_Vehicle::HandleSteer_Implementation(float Steer, FVector Force, FVector Location)
{
	Mesh->AddTorqueInDegrees(FVector(0, 0, Steer * SteeringSpeed), NAME_None, true);
	Mesh->AddForceAtLocation(Force, Location);
}

void ACPP_Vehicle::StopSteer()
{
	CameraCurrentOffset = 0;
}

// Toggle Polarity Input Action handler
void ACPP_Vehicle::HandleTogglePolarity()
{
	if (!bCanSwitchPolarity) return;

	EMagneticPolarity NewPolarity = MagneticPolarity == EMagneticPolarity::POSITIVE ? EMagneticPolarity::NEGATIVE : EMagneticPolarity::POSITIVE;

	MagneticPolarity = NewPolarity;
	Mesh->SetMaterial(0, NewPolarity == EMagneticPolarity::POSITIVE ? MaterialPositive : MaterialNegative);

	bCanSwitchPolarity = false;
	GetWorld()->GetTimerManager().SetTimer(PolarityTimerHandle, this, &ACPP_Vehicle::OnPolarityTimerEnd, 1, false, PolarityDelay);

	if (MagnetInRange == nullptr) return;
	if (MagnetInRange->GetMagneticPolarity() == NewPolarity)
	{
		float CurveFloatValue = CurveBoost->GetFloatValue(GetCurveBoostDuration());

		AccelerationSpeed *= CurveFloatValue;
		CameraCurrentZoom = CameraCurrentZoom * CurveFloatValue;

		GetWorld()->GetTimerManager().SetTimer(BoostTimerHandle, this, &ACPP_Vehicle::OnBoostTimerEnd, 1, false, CurveFloatValue);
	}
	else
	{
		AccelerationSpeed = InitialAccelerationSpeed;
	}
}

void ACPP_Vehicle::OnPolarityTimerEnd()
{
	bCanSwitchPolarity = true;
}

// Reset Acceleration Speed and Spring Arm Length when boost ends
void ACPP_Vehicle::OnBoostTimerEnd()
{
	AccelerationSpeed = InitialAccelerationSpeed;
	CameraCurrentZoom = MaxCameraZoom;
}
