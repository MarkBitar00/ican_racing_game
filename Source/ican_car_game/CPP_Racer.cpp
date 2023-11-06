#include "CPP_Racer.h"
#include "CPP_Magnet.h"

// Sets default values
ACPP_Racer::ACPP_Racer()
{
	// Set this Pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create Capsule Component with Movement Component and set it as Root
	CollisionCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	SetRootComponent(CollisionCapsule);
	CollisionCapsule->SetHiddenInGame(false);

	// Create Mesh, Materials for Magnetic Polarity and set Root Component with default Scale and Material
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);
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
	SpringArm->SetupAttachment(Mesh);
	SpringArm->TargetArmLength = CameraInitialZoom;
	SpringArm->TargetOffset = FVector(0, 0, 100);
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// Create Camera and attach it to Spring Arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// Create and setup Steer locations
	SteerLeftLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SteerLeftLocation"));
	SteerLeftLocation->SetRelativeLocation(FVector(0, -50, 0));
	SteerLeftLocation->SetupAttachment(Mesh);
	SteerRightLocation = CreateDefaultSubobject<USceneComponent>(TEXT("SteerRightLocation"));
	SteerRightLocation->SetRelativeLocation(FVector(0, 50, 0));
	SteerRightLocation->SetupAttachment(Mesh);

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
void ACPP_Racer::BeginPlay()
{
	Super::BeginPlay();

	// Set initial attribute values
	InitialAccelerationSpeed = AccelerationSpeed;
	CameraCurrentZoom = CameraInitialZoom;
	CameraCurrentOffset = 0;

	// Setup Mesh properties
	Mesh->SetSimulatePhysics(true);

	// Bind Timeline functions
	TimelineDeceleration->AddInterpFloat(CurveTimeline, TimelineUpdate);
}

// Called every frame
void ACPP_Racer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smooth out Spring Arm length and offset movements
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, CameraCurrentZoom, DeltaTime, CameraInterpolationSpeed);
	SpringArm->SocketOffset.Y = FMath::FInterpTo(SpringArm->SocketOffset.Y, CameraCurrentOffset, DeltaTime, CameraInterpolationSpeed);

	// Clamp Mesh rotation
	FRotator CurrentRotation = Mesh->GetComponentRotation();
	CurrentRotation.Roll = FMath::Clamp(CurrentRotation.Roll, -MaxRotation, MaxRotation);
	CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch, -MaxRotation, MaxRotation);
	Mesh->SetWorldRotation(CurrentRotation);
}

// Called to bind functionality to input
void ACPP_Racer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

// Called during the Deceleration Timeline's update event
void ACPP_Racer::TimelineDecelerationUpdate(float Alpha)
{
	Mesh->AddForce(Mesh->GetForwardVector() * AccelerationSpeed * Alpha, NAME_None, true);
}

float ACPP_Racer::GetCurveBoostDuration()
{
	if (MagnetInRange == nullptr) return 0;

	FVector Location = GetActorLocation();
	FVector MagnetLocation = MagnetInRange->GetActorLocation();
	FVector VectorProjectionPlaneNormal = FVector(0, 0, 1);
	float LocationsDistance = FVector::Dist(FVector::VectorPlaneProject(Location, VectorProjectionPlaneNormal), FVector::VectorPlaneProject(MagnetLocation, VectorProjectionPlaneNormal));

	return LocationsDistance / MagnetInRange->ColliderRadius;
}

// Get Camera properties
float ACPP_Racer::GetCameraCurrentZoom()
{
	return CameraCurrentZoom;
}

float ACPP_Racer::GetInitialAccelerationSpeed()
{
	return InitialAccelerationSpeed;
}

// Get Magnetism properties
bool ACPP_Racer::GetCanSwitchPolarity()
{
	return bCanSwitchPolarity;
}

EMagneticPolarity ACPP_Racer::GetMagneticPolarity()
{
	return MagneticPolarity;
}

// Set Camera properties
void ACPP_Racer::SetCameraCurrentZoom(float Zoom)
{
	CameraCurrentZoom = Zoom;
}

void ACPP_Racer::SetCameraCurrentOffset(float Pitch)
{
	CameraCurrentOffset = Pitch;
}

// Set Magnetism properties
void ACPP_Racer::SetCanSwitchPolarity(bool bCanSwitch)
{
	bCanSwitchPolarity = bCanSwitch;
}

void ACPP_Racer::SetMagneticPolarity(EMagneticPolarity Polarity)
{
	MagneticPolarity = Polarity;
}

void ACPP_Racer::SetMagnetInRange(ACPP_Magnet* Magnet)
{
	MagnetInRange = Magnet;
}
