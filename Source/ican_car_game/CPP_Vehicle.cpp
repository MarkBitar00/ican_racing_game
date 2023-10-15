#include "CPP_Vehicle.h"

// Sets default values
ACPP_Vehicle::ACPP_Vehicle()
{
 	// Set this Pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Create Mesh and set Root Component with default Scale and Material
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		MeshFile(TEXT("/Engine/BasicShapes/Cube"));
	Mesh->SetStaticMesh(MeshFile.Object);
	Mesh->SetRelativeScale3D(FVector(2.4, 1.6, 0.8));
	static ConstructorHelpers::FObjectFinder<UMaterialInstance>
		MaterialFile(TEXT("/Game/Materials/MaterialInstances/M_Positive"));
	Mesh->SetMaterial(0, MaterialFile.Object);

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
	HoverFrontLeft = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverFrontLeft"));
	SetupHoverComponent(HoverFrontLeft, FVector(45, -45, -50));
	HoverFrontRight = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverFrontRight"));
	SetupHoverComponent(HoverFrontRight, FVector(45, 45, -50));
	HoverBackLeft = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverBackLeft"));
	SetupHoverComponent(HoverBackLeft, FVector(-45, -45, -50));
	HoverBackRight = CreateDefaultSubobject<UCPP_HoverComponent>(TEXT("HoverBackRight"));
	SetupHoverComponent(HoverBackRight, FVector(-45, 45, -50));
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
	SetMeshLinearDamping(LinearDamping);
	SetMeshAngularDamping(AngularDamping);
	SetMeshCenterOfMassHeight(CenterOfMassHeight);

	// Initialize Hover Components
	HoverFrontLeft->Init(Mesh, HoverHeight, HoverForce, GravityForce);
	HoverFrontRight->Init(Mesh, HoverHeight, HoverForce, GravityForce);
	HoverBackLeft->Init(Mesh, HoverHeight, HoverForce, GravityForce);
	HoverBackRight->Init(Mesh, HoverHeight, HoverForce, GravityForce);
}

// Called every frame
void ACPP_Vehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Smooth out Spring Arm length movements
	SetSpringArmLength(FMath::FInterpTo(GetSpringArmLength(), CameraCurrentZoom, DeltaTime, CameraInterpolationSpeed));

	// Smooth out Spring Arm offset movements
	SetSpringArmSocketOffsetPitch(FMath::FInterpTo(GetSpringArmSocketOffsetPitch(), CameraCurrentOffset, DeltaTime, CameraInterpolationSpeed));

	// Clamp Mesh rotation
	FRotator CurrentRotation = GetMeshWorldRotation();
	CurrentRotation.Roll = FMath::Clamp(CurrentRotation.Roll, -MaxRotation, MaxRotation);
	CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch, -MaxRotation, MaxRotation);
	SetMeshWorldRotation(CurrentRotation);
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

// Get Camera properties
float ACPP_Vehicle::GetSpringArmLength()
{
	return SpringArm->TargetArmLength;
}

float ACPP_Vehicle::GetSpringArmSocketOffsetPitch()
{
	return SpringArm->SocketOffset.Y;
}

// Get Movement properties
FRotator ACPP_Vehicle::GetMeshWorldRotation()
{
	return Mesh->GetComponentRotation();
}

float ACPP_Vehicle::GetInitialAccelerationSpeed()
{
	return InitialAccelerationSpeed;
}

// Set Camera properties
void ACPP_Vehicle::SetSpringArmLength(float Length)
{
	SpringArm->TargetArmLength = Length;
}

void ACPP_Vehicle::SetSpringArmSocketOffsetPitch(float Pitch)
{
	SpringArm->SocketOffset.Y = Pitch;
}

// Set Movement properties
void ACPP_Vehicle::SetMeshLinearDamping(float Damping)
{
	Mesh->SetLinearDamping(Damping);
}

void ACPP_Vehicle::SetMeshAngularDamping(float Damping)
{
	Mesh->SetAngularDamping(Damping);
}

void ACPP_Vehicle::SetMeshWorldRotation(FRotator Rotation)
{
	Mesh->SetWorldRotation(Rotation);
}

void ACPP_Vehicle::SetMeshCenterOfMassHeight(float Height)
{
	Mesh->SetCenterOfMass(FVector(0, 0, Height));
}
