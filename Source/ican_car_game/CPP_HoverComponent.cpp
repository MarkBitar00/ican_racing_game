#include "CPP_HoverComponent.h"

// Sets default values for this component's properties
UCPP_HoverComponent::UCPP_HoverComponent()
{
	// Set this Pawn to call Tick() every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Set default attribute values
	HoverParent = nullptr;
	HoverHeight = 0;
	HoverForce = 0;
	GravityForce = 0;

	// Create Curve Hover and set default
	CurveHover = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveHover"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		CurveFile(TEXT("/Game/Utils/Curves/SuspensionCurve"));
	CurveHover = CurveFile.Object;
}

// Called when the game starts
void UCPP_HoverComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UCPP_HoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Hover();
}

// Initialize base attributes for Hover Component
void UCPP_HoverComponent::Init(UStaticMeshComponent* Parent, float TraceLength, float HoverPower, float GravityPower)
{
	SetParent(Parent);
	SetHoverHeight(TraceLength);
	SetHoverForce(HoverPower);
	SetGravityForce(GravityPower);
}

// Hover mechanics
void UCPP_HoverComponent::Hover()
{
	UStaticMeshComponent* Parent = GetParent();
	FVector WorldLocation = GetComponentLocation();
	FVector UpVector = GetUpVector();
	float TraceLength = GetHoverHeight();
	float HoverPower = GetHoverForce();
	float GravityPower = GetGravityForce();

	FHitResult Hit;
	FVector LineTraceEndLocation = (UpVector * -TraceLength) + WorldLocation;
	UWorld* world = GetWorld();

	bool bHit = world->LineTraceSingleByChannel(Hit, WorldLocation, LineTraceEndLocation, ECC_Visibility);
	DrawDebugLine(world, WorldLocation, LineTraceEndLocation, FColor::Red);

	if (bHit)
	{
		float LerpAlpha = (Hit.Location - WorldLocation).Length() / TraceLength;
		float LerpValue = FMath::Lerp(HoverPower, 0, LerpAlpha);
		float CurveValue = CurveHover->GetFloatValue(Hit.Distance / TraceLength);

		FVector Force = Hit.ImpactNormal * LerpValue * CurveValue;
		HoverParent->AddForceAtLocation(Force, WorldLocation);
	}
	else
	{
		FVector Force = WorldLocation + (GravityPower * FVector(0, 0, -100));
		HoverParent->AddForceAtLocation(Force, WorldLocation);
	}
}

// Get Movement properties
float UCPP_HoverComponent::GetHoverHeight()
{
	return HoverHeight;
}

float UCPP_HoverComponent::GetHoverForce()
{
	return HoverForce;
}

float UCPP_HoverComponent::GetGravityForce()
{
	return GravityForce;
}

// Set Components
void UCPP_HoverComponent::SetParent(UStaticMeshComponent* Parent)
{
	HoverParent = Parent;
}

// Set Movement properties
void UCPP_HoverComponent::SetHoverHeight(float Height)
{
	HoverHeight = Height;
}

void UCPP_HoverComponent::SetHoverForce(float Force)
{
	HoverForce = Force;
}

void UCPP_HoverComponent::SetGravityForce(float Force)
{
	GravityForce = Force;
}
