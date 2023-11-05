#include "CPP_Vehicle.h"
#include "CPP_Magnet.h"

// Sets default values
ACPP_Vehicle::ACPP_Vehicle()
{
 	// Set this Pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create Mesh, Materials for Magnetic Polarity and set Root Component with default Scale and Material
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		MeshFile(TEXT("/Engine/BasicShapes/Cube"));
	Mesh->SetStaticMesh(MeshFile.Object);
	Mesh->SetRelativeScale3D(FVector(2.4, 1.6, 0.8));
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
	SpringArm->TargetOffset = FVector(0, 0, 100);
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// Create Camera and attach it to Spring Arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// Create and setup Hover Components
	FVector HoverVector = FVector(50, -50, 0);
	HoverFrontLeft = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverFrontLeft"));
	SetupHoverComponent(HoverFrontLeft, HoverVector);
	HoverFrontRight = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverFrontRight"));
	SetupHoverComponent(HoverFrontRight, HoverVector);
	HoverBackLeft = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverBackLeft"));
	SetupHoverComponent(HoverBackLeft, HoverVector);
	HoverBackRight = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverBackRight"));
	SetupHoverComponent(HoverBackRight, HoverVector);

	// Create and setup Steer locations
	SteerLeftLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SteerLeftLocation"));
	SteerLeftLocation->SetRelativeLocation(FVector(0, -50, 0));
	SteerLeftLocation->SetupAttachment(RootComponent);
	SteerRightLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SteerRightLocation"));
	SteerRightLocation->SetRelativeLocation(FVector(0, 50, 0));
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

	// Add "Vehicle" tag
	this->Tags.Add(FName("Vehicle"));
}

// Called when the game starts or when spawned
void ACPP_Vehicle::BeginPlay()
{
	Super::BeginPlay();

	// Set initial attribute values
	InitialAccelerationSpeed = AccelerationSpeed;
	CameraCurrentZoom = CameraInitialZoom;
	CameraCurrentOffset = 0;

	// Setup Mesh properties
	Mesh->SetSimulatePhysics(true);
	Mesh->SetLinearDamping(LinearDamping);
	Mesh->SetAngularDamping(AngularDamping);
	//Mesh->SetCenterOfMass(FVector(0, 0, CenterOfMassHeight));

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
}

// Set Hover Component relative location and attach it to Root Component
void ACPP_Vehicle::SetupHoverComponent(UCPP_HoverComponent* HoverComponent, FVector Location)
{
	HoverComponent->SetRelativeLocation(Location);
	HoverComponent->SetupAttachment(RootComponent);
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
		CenterOfMassHeight = 100.f;
	}
	else
	{
		CenterOfMassHeight = 0.f;
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
void ACPP_Vehicle::SetMagneticPolarity(EMagneticPolarity Polarity)
{
	MagneticPolarity = Polarity;
}

void ACPP_Vehicle::SetMagnetInRange(ACPP_Magnet* Magnet)
{
	MagnetInRange = Magnet;
}
