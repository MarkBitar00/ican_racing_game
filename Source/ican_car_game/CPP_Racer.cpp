#include "CPP_Racer.h"
#include "CPP_Magnet.h"

// Sets default values
ACPP_Racer::ACPP_Racer()
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
	SpringArm->SetupAttachment(Mesh);
	SpringArm->TargetArmLength = CameraInitialZoom;
	SpringArm->TargetOffset = FVector(0, 0, 100);
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// Create Camera and attach it to Spring Arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	// Create Pawn Movement and set its values
	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	Movement->MaxSpeed = MovementSpeed;
	Movement->Acceleration = AccelerationSpeed;
	Movement->Deceleration = DecelerationSpeed;

	// Create Curves and set default
	CurveAttraction = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveAttraction"));
	CurveRepulsion = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveRepulsion"));
	CurveBoost = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveBoost"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		CurveAttractionFile(TEXT("/Game/Utils/Curves/AttractionCurve")),
		CurveRepulsionFile(TEXT("/Game/Utils/Curves/RepulsionCurve")),
		CurveBoostFile(TEXT("/Game/Utils/Curves/BoostCurve"));
	CurveAttraction = CurveAttractionFile.Object;
	CurveRepulsion = CurveRepulsionFile.Object;
	CurveBoost = CurveBoostFile.Object;

	// Add "Vehicle" tag
	this->Tags.Add(FName("Vehicle"));
}

// Called when the game starts or when spawned
void ACPP_Racer::BeginPlay()
{
	Super::BeginPlay();

	// Set initial attribute values
	InitialMovementSpeed = MovementSpeed;
	CameraCurrentZoom = CameraInitialZoom;
	CameraCurrentOffset = 0;
}

// Called every frame
void ACPP_Racer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smooth out Spring Arm length and offset movements
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, CameraCurrentZoom, DeltaTime, CameraInterpolationSpeed);
	SpringArm->SocketOffset.Y = FMath::FInterpTo(SpringArm->SocketOffset.Y, CameraCurrentOffset, DeltaTime, CameraInterpolationSpeed);

	// Keep Rotation parallel to ground and ajust height of Pawn
	UpdateGravity(DeltaTime);
}

// Called to bind functionality to input
void ACPP_Racer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACPP_Racer::UpdateGravity(float Delta)
{
	FVector Location = GetActorLocation();
	FVector UpVector = GetActorUpVector();

	FHitResult Hit;
	FVector LineTraceEndLocation = (UpVector * -10000) + Location;
	UWorld* World = GetWorld();

	bool bHit = World->LineTraceSingleByChannel(Hit, Location, LineTraceEndLocation, ECC_Visibility);
	DrawDebugLine(World, Location, LineTraceEndLocation, FColor::Yellow);

	if (bHit)
	{
		FVector ForwardVector = GetActorForwardVector();

		FRotator Rotation = UKismetMathLibrary::MakeRotFromXZ(FVector::VectorPlaneProject(ForwardVector, Hit.Normal), Hit.Normal);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), Rotation, Delta, 10));

		if (Hit.Distance > HoverHeight)
		{
			SetActorLocation(FMath::VInterpTo(Location, FVector(Location.X, Location.Y, Location.Z - (Hit.Distance - HoverHeight)), Delta, MovementInterpolationSpeed));
		}
		else if (Hit.Distance < HoverHeight)
		{
			SetActorLocation(FMath::VInterpTo(Location, FVector(Location.X, Location.Y, Location.Z + (HoverHeight - Hit.Distance)), Delta, MovementInterpolationSpeed));
		}
	}
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

// Get Movement properties
float ACPP_Racer::GetInitialMovementSpeed()
{
	return InitialMovementSpeed;
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
