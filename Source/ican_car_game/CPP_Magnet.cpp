#include "CPP_Magnet.h"

// Sets default values
ACPP_Magnet::ACPP_Magnet()
{
	// Set this Actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Create Collision Sphere and set Root Component with default Sphere Radius
	Collider = CreateDefaultSubobject<USphereComponent>(TEXT("Collider"));
	SetRootComponent(Collider);
	Collider->InitSphereRadius(ColliderRadius);
	Collider->SetHiddenInGame(false);

	// Create Mesh and set default Scale, Location and Material
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh>
		MeshFile(TEXT("/Engine/BasicShapes/Cylinder"));
	Mesh->SetStaticMesh(MeshFile.Object);
	static ConstructorHelpers::FObjectFinder<UMaterialInterface>
		MaterialPositiveFallbackFile(TEXT("/Game/Materials/MaterialInstances/M_Positive")),
		MaterialNegativeFallbackFile(TEXT("/Game/Materials/MaterialInstances/M_Negative"));
	MaterialPositiveFallback = MaterialPositiveFallbackFile.Object;
	MaterialNegativeFallback = MaterialNegativeFallbackFile.Object;
	Mesh->SetRelativeScale3D(FVector(3, 3, 10));
	Mesh->SetRelativeLocation(FVector(0, 0, 500));
	Mesh->SetupAttachment(RootComponent);
}

void ACPP_Magnet::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Set Magnet Material according to its Magnetic Polarity
	UMaterialInterface* MatPos = MaterialPositive != nullptr ? MaterialPositive : MaterialPositiveFallback;
	UMaterialInterface* MatNeg = MaterialNegative != nullptr ? MaterialNegative : MaterialNegativeFallback;

	Mesh->SetMaterial(0, MagneticPolarity == EMagneticPolarity::POSITIVE ? MatPos : MatNeg);
}

// Called when the game starts or when spawned
void ACPP_Magnet::BeginPlay()
{
	Super::BeginPlay();

	// Bind Collider overlap functions
	Collider->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Magnet::OverlapBegin);
	Collider->OnComponentEndOverlap.AddDynamic(this, &ACPP_Magnet::OverlapEnd);
}

// Called every frame
void ACPP_Magnet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (ACPP_Vehicle* Vehicle : VehiclesInRange)
	{
		UStaticMeshComponent* VehicleMesh = Vehicle->GetMesh();
		FVector VehicleLocation = Vehicle->GetActorLocation();
		FVector MagnetLocation = GetActorLocation();
		bool bIsSamePolarity = Vehicle->GetMagneticPolarity() == MagneticPolarity;

		FVector VectorProjectionPlaneNormal = FVector(0, 0, 1);
		float LocationsDistance = FVector::Dist(FVector::VectorPlaneProject(VehicleLocation, VectorProjectionPlaneNormal), FVector::VectorPlaneProject(MagnetLocation, VectorProjectionPlaneNormal));
		float CurveTime = LocationsDistance / ColliderRadius;

		UCurveFloat* Curve = bIsSamePolarity ? Vehicle->GetCurveRepulsion() : Vehicle->GetCurveAttraction();
		float CurveFloatValue = Curve->GetFloatValue(CurveTime);

		FVector Force = (VehicleLocation - MagnetLocation) * (bIsSamePolarity ? MagnetPower : -MagnetPower) * CurveFloatValue;
		VehicleMesh->AddForce(Force, NAME_None, true);
	}
}

// Collision Sphere overlap functions
void ACPP_Magnet::OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->ActorHasTag(FName("HoverVehicle"))) return;

	ACPP_Vehicle* Vehicle = Cast<ACPP_Vehicle>(OtherActor);
	Vehicle->SetMagnetInRange(this);
	VehiclesInRange.Add(Vehicle);
}

void ACPP_Magnet::OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor->ActorHasTag(FName("HoverVehicle"))) return;

	ACPP_Vehicle* Vehicle = Cast<ACPP_Vehicle>(OtherActor);
	Vehicle->SetMagnetInRange(nullptr);
	VehiclesInRange.Remove(Vehicle);
}
