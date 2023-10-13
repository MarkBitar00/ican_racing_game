#include "CPP_Vehicle.h"

// Sets default values
ACPP_Vehicle::ACPP_Vehicle()
{
 	// Set this Pawn to call Tick() every frame (you can turn this off to improve performance if you don't need it)
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Set Root Component with default Scale and Material
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
	SpringArm->TargetArmLength = 400;
	SpringArm->TargetOffset = FVector(0, 0, 100);
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	// Create Camera and attach it to Spring Arm
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

// Called when the game starts or when spawned
void ACPP_Vehicle::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACPP_Vehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ACPP_Vehicle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}
