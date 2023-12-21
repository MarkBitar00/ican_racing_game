#include "CPP_Vehicle.h"
#include "CPP_Magnet.h"
#include "Net/UnrealNetwork.h"
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
		MaterialPositiveFallbackFile(TEXT("/Game/Materials/NEW_EMISSIVE/EMISSIVE_ROUGE")),
		MaterialNegativeFallbackFile(TEXT("/Game/Materials/NEW_EMISSIVE/EMISSIVE_BLEU"));
	MaterialPositiveFallback = MaterialPositiveFallbackFile.Object;
	MaterialNegativeFallback = MaterialNegativeFallbackFile.Object;
	Mesh->SetIsReplicated(true);

	// Create Collision Sphere and set radius
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(Mesh);
	CollisionSphere->InitSphereRadius(ColliderRadius);

	// Create Spring Arm and attach it to Root Component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Mesh);
	SpringArm->TargetArmLength = CameraInitialZoom;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bDoCollisionTest = false;

	// Create Camera and attach it to Spring Arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->PostProcessBlendWeight = FMath::Clamp(CameraPostProcessBlend, 0, 1);

	// Create and setup Hover Components
	HoverFrontLeft = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverFrontLeft"));
	HoverFrontLeft->SetupAttachment(Mesh);
	HoverFrontRight = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverFrontRight"));
	HoverFrontRight->SetupAttachment(Mesh);
	HoverBackLeft = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverBackLeft"));
	HoverBackLeft->SetupAttachment(Mesh);
	HoverBackRight = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverBackRight"));
	HoverBackRight->SetupAttachment(Mesh);

	// Create and setup Steer locations
	SteerLeftLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SteerLeftLocation"));
	SteerLeftLocation->SetupAttachment(Mesh);
	SteerRightLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SteerRightLocation"));
	SteerRightLocation->SetupAttachment(Mesh);

	// Create Curves and set default
	CurveAcceleration = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveAcceleration"));
	CurveBoostMultiplier = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveBoostMultiplier"));
	CurveBoostDuration = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveBoostDuration"));
	CurveVehicleAttraction = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveVehicleAttraction"));
	CurveVehicleRepulsion = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveVehicleRepulsion"));
	CurveFieldOfView = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveFieldOfView"));
	CurveBlur = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveBlur"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		CurveAccelerationFile(TEXT("/Game/Utils/AccelerationCurve")),
		CurveBoostMultiplierFile(TEXT("/Game/Utils/BoostMultiplierCurve")),
		CurveBoostDurationFile(TEXT("/Game/Utils/BoostDurationCurve")),
		CurveVehicleAttractionFile(TEXT("/Game/Utils/VehicleAttractionCurve")),
		CurveVehicleRepulsionFile(TEXT("/Game/Utils/VehicleRepulsionCurve")),
		CurveFieldOfViewFile(TEXT("/Game/Utils/FieldOfViewCurve")),
		CurveBlurFile(TEXT("/Game/Utils/BlurCurve"));
	CurveAcceleration = CurveAccelerationFile.Object;
	CurveBoostMultiplier = CurveBoostMultiplierFile.Object;
	CurveBoostDuration = CurveBoostDurationFile.Object;
	CurveVehicleAttraction = CurveVehicleAttractionFile.Object;
	CurveVehicleRepulsion = CurveVehicleRepulsionFile.Object;
	CurveFieldOfView = CurveFieldOfViewFile.Object;
	CurveBlur = CurveBlurFile.Object;

	// Add tag for Magnet overlaps
	this->Tags.Add(FName("HoverVehicle"));
}

// Replicate Pawn properties
void ACPP_Vehicle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACPP_Vehicle, Mesh);
	DOREPLIFETIME(ACPP_Vehicle, CollisionSphere);
	DOREPLIFETIME(ACPP_Vehicle, MagneticPolarity);
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
	CameraCurrentFieldOfView = CameraInitialFieldOfView;
	CameraCurrentBlur = CameraInitialBlur;
	CameraCurrentOffset = 0;

	// Setup Camera properties
	SpringArm->SocketOffset = FVector(0, 0, SpringArmTargetOffset);
	Camera->SetRelativeRotation(FRotator(CameraRotation, 0, 0));
	Camera->SetFieldOfView(CameraInitialFieldOfView);
	Camera->PostProcessSettings.bOverride_SceneFringeIntensity = true;
	Camera->PostProcessSettings.SceneFringeIntensity = CameraInitialBlur;

	// Setup Mesh properties
	UMaterialInterface* MatPos = MaterialPositive != nullptr ? MaterialPositive : MaterialPositiveFallback;
	UMaterialInterface* MatNeg = MaterialNegative != nullptr ? MaterialNegative : MaterialNegativeFallback;
	UMaterialInterface* PolarityMaterial = MagneticPolarity == EMagneticPolarity::POSITIVE ? MatPos : MatNeg;
	Mesh->SetMaterial(2, PolarityMaterial);
	Mesh->SetMaterial(4, PolarityMaterial);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetLinearDamping(LinearDamping);
	Mesh->SetAngularDamping(AngularDamping);

	// Initialize Hover Components
	HoverFrontLeft->Init(Mesh, HoverHeight, HoverForce, GravityForce);
	HoverFrontRight->Init(Mesh, HoverHeight, HoverForce, GravityForce);
	HoverBackLeft->Init(Mesh, HoverHeight, HoverForce, GravityForce);
	HoverBackRight->Init(Mesh, HoverHeight, HoverForce, GravityForce);

	// Bind Collider overlap functions
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Vehicle::OverlapBegin);
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &ACPP_Vehicle::OverlapEnd);
}

// Called every frame
void ACPP_Vehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smooth out Spring Arm and Camera properties
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, CameraCurrentZoom, DeltaTime, CameraInterpolationSpeed);
	SpringArm->SocketOffset.Y = FMath::FInterpTo(SpringArm->SocketOffset.Y, CameraCurrentOffset, DeltaTime, CameraInterpolationSpeed);
	Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, CameraCurrentFieldOfView, DeltaTime, CameraInterpolationSpeed);
	Camera->PostProcessSettings.SceneFringeIntensity = FMath::FInterpTo(Camera->PostProcessSettings.SceneFringeIntensity, CameraCurrentBlur, DeltaTime, CameraInterpolationSpeed);

	// Smooth out Center Of Mass Location
	UpdateCenterOfMass();
	Mesh->SetCenterOfMass(FVector(0, 0, FMath::FInterpTo(Mesh->GetCenterOfMass().Z, -CenterOfMassHeight, DeltaTime, 100)));

	// Clamp Mesh rotation
	FRotator CurrentRotation = Mesh->GetComponentRotation();
	CurrentRotation.Roll = FMath::Clamp(CurrentRotation.Roll, -MaxRotation, MaxRotation);
	CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch, -MaxRotation, MaxRotation);
	Mesh->SetWorldRotation(CurrentRotation);

	// Apply magnetic force on players in range
	if (!bActivateVehicleMagnetism) return;
	for (ACPP_Vehicle* VehicleInRange : PlayersInRange)
	{
		UStaticMeshComponent* VehicleInRangeMesh = VehicleInRange->GetMesh();
		FVector VehicleInRangeLocation = VehicleInRange->GetActorLocation();
		FVector Location = GetActorLocation();
		bool bIsSamePolarity = VehicleInRange->GetMagneticPolarity() == MagneticPolarity;

		float LocationsDistance = FVector::Dist(VehicleInRangeLocation, Location);
		float CurveTime = LocationsDistance / ColliderRadius;

		UCurveFloat* Curve = bIsSamePolarity ? CurveVehicleRepulsion : CurveVehicleAttraction;
		float CurveFloatValue = Curve->GetFloatValue(CurveTime);

		FVector Force = (VehicleInRangeLocation - Location) * (bIsSamePolarity ? VehicleMagneticPower : -VehicleMagneticPower) * CurveFloatValue;
		VehicleInRangeMesh->AddForce(Force, NAME_None, true);
	}
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
			EnhancedInputComponent->BindAction(ActionTogglePolarity, ETriggerEvent::Started, this, &ACPP_Vehicle::TogglePolarity);
	}
}

// Collision Sphere overlap functions
void ACPP_Vehicle::OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag(FName("HoverVehicle"))) return;

	ACPP_Vehicle* Player = Cast<ACPP_Vehicle>(OtherActor);
	PlayersInRange.Add(Player);
}

void ACPP_Vehicle::OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor->ActorHasTag(FName("HoverVehicle"))) return;

	ACPP_Vehicle* Player = Cast<ACPP_Vehicle>(OtherActor);
	PlayersInRange.Remove(Player);
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

	if (Hit.Distance <= HoverHeight)
	{
		CenterOfMassHeight = 100;
	}
	else
	{
		CenterOfMassHeight = 0;
	}
}

float ACPP_Vehicle::GetDistanceToMagnet()
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
void ACPP_Vehicle::Accelerate(const struct FInputActionInstance& Instance)
{
	if (!bIsInputActive) return;

	const float AccelerationValue = CurveAcceleration->GetFloatValue(Instance.GetElapsedTime()) * Instance.GetValue().Get<float>();
	FVector ForwardVector = Mesh->GetForwardVector();

	HandleAccelerate(AccelerationValue, ForwardVector);
}

void ACPP_Vehicle::HandleAccelerate_Implementation(float Acceleration, FVector Forward)
{
	Mesh->AddForce(Forward * AccelerationSpeed * Acceleration, NAME_None, true);
}

void ACPP_Vehicle::StartAccelerate()
{
	if (!bIsInputActive) return;

	CameraCurrentZoom = MaxCameraZoom;
}

void ACPP_Vehicle::StopAccelerate(const struct FInputActionInstance& Instance)
{
	if (!bIsInputActive) return;

	const float AccelerationDuration = Instance.GetTriggeredTime();
	float DecelerationTimelinePlayRate = 1 / FMath::Clamp(AccelerationDuration, 0, MaxDecelerationDuration);

	CameraCurrentZoom = CameraInitialZoom;

	StartDeceleration(DecelerationTimelinePlayRate);
}

// Brake Input Action handlers
void ACPP_Vehicle::StartBrake()
{
	if (!bIsInputActive) return;

	AccelerationSpeed = 0;
	CameraCurrentZoom = CameraInitialZoom;
	CameraCurrentOffset = 0;
	
	StopDeceleration();
}

void ACPP_Vehicle::StopBrake()
{
	AccelerationSpeed = InitialAccelerationSpeed;
}

// Steer Input Action handlers
void ACPP_Vehicle::Steer(const struct FInputActionValue& Value)
{
	if (!bIsInputActive) return;

	const float SteerValue = Value.Get<float>();

	if (SteerValue < 0.1 && SteerValue > -0.1)
	{
		StopSteer();
		return;
	}

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
void ACPP_Vehicle::TogglePolarity()
{
	if (!bCanSwitchPolarity || !bIsInputActive) return;

	EMagneticPolarity NewPolarity = MagneticPolarity == EMagneticPolarity::POSITIVE ? EMagneticPolarity::NEGATIVE : EMagneticPolarity::POSITIVE;
	UMaterialInterface* MatPos = MaterialPositive != nullptr ? MaterialPositive : MaterialPositiveFallback;
	UMaterialInterface* MatNeg = MaterialNegative != nullptr ? MaterialNegative : MaterialNegativeFallback;
	UMaterialInterface* NewMaterial = NewPolarity == EMagneticPolarity::POSITIVE ? MatPos : MatNeg;

	HandleTogglePolarity(NewPolarity, NewMaterial);
	OnPolarityToggle();

	bCanSwitchPolarity = false;
	GetWorld()->GetTimerManager().SetTimer(PolarityTimerHandle, this, &ACPP_Vehicle::OnPolarityTimerEnd, 1, false, PolarityDelay);

	if (MagnetInRange == nullptr) return;
	if (MagnetInRange->GetMagneticPolarity() == NewPolarity)
	{
		float BoostDistance = GetDistanceToMagnet();
		float BoostMultiplier = CurveBoostMultiplier->GetFloatValue(BoostDistance);
		float BoostDuration = CurveBoostDuration->GetFloatValue(BoostDistance);
		float FieldOfViewMultiplier = CurveFieldOfView->GetFloatValue(BoostDistance);
		float AddedBlur = CurveBlur->GetFloatValue(BoostDistance);
		float NewAccelerationSpeed = AccelerationSpeed * BoostMultiplier;
		float NewCameraZoom = CameraCurrentZoom * BoostMultiplier;
		float NewFieldOfView = CameraCurrentFieldOfView * FieldOfViewMultiplier;
		float NewBlur = CameraCurrentBlur + AddedBlur;

		AccelerationSpeed = NewAccelerationSpeed > MaxBoostAccelerationSpeed ? MaxBoostAccelerationSpeed : NewAccelerationSpeed;
		CameraCurrentZoom = NewCameraZoom > MaxBoostCameraZoom ? MaxBoostCameraZoom : NewCameraZoom;
		CameraCurrentFieldOfView = NewFieldOfView > MaxCameraFieldOfView ? MaxCameraFieldOfView : NewFieldOfView;
		CameraCurrentBlur = NewBlur > MaxCameraBlur ? MaxCameraBlur : NewBlur;

		OnBoost();
		GetWorld()->GetTimerManager().SetTimer(BoostTimerHandle, this, &ACPP_Vehicle::OnBoostTimerEnd, 1, false, BoostDuration);
	}
	else
	{
		AccelerationSpeed = InitialAccelerationSpeed;
	}
}

void ACPP_Vehicle::HandleTogglePolarity_Implementation(EMagneticPolarity NewPolarity, UMaterialInterface* NewMaterial)
{
	MagneticPolarity = NewPolarity;
	Mesh->SetMaterial(2, NewMaterial);
	Mesh->SetMaterial(4, NewMaterial);

	HandleTogglePolarityServer(NewPolarity, NewMaterial);
}

void ACPP_Vehicle::HandleTogglePolarityServer_Implementation(EMagneticPolarity NewPolarity, UMaterialInterface* NewMaterial)
{
	MagneticPolarity = NewPolarity;
	Mesh->SetMaterial(2, NewMaterial);
	Mesh->SetMaterial(4, NewMaterial);
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
	CameraCurrentFieldOfView = CameraInitialFieldOfView;
	CameraCurrentBlur = CameraInitialBlur;
}
