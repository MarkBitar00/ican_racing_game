#include "CPP_HoverComponent.h"

// Sets default values for this component's properties
UCPP_HoverComponent::UCPP_HoverComponent()
{
	// Set this Pawn to call Tick() every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Create Curve Hover and set default
	CurveHover = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveHover"));
	CurveGravity = CreateDefaultSubobject<UCurveFloat>(TEXT("CurveGravity"));
	static ConstructorHelpers::FObjectFinder<UCurveFloat>
		CurveHoverFile(TEXT("/Game/Utils/SuspensionCurve")),
		CurveGravityFile(TEXT("/Game/Utils/GravityCurve"));
	CurveHover = CurveHoverFile.Object;
	CurveGravity = CurveGravityFile.Object;
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
	HoverParent = Parent;
	HoverHeight = TraceLength;
	HoverForce = HoverPower;
	GravityForce = GravityPower;
}

// Hover mechanics
void UCPP_HoverComponent::Hover()
{
	FVector WorldLocation = GetComponentLocation();
	FVector UpVector = GetUpVector();

	FHitResult Hit;
	FVector LineTraceEndLocation = (UpVector * -HoverHeight * 100) + WorldLocation;
	UWorld* World = GetWorld();

	bool bHit = World->LineTraceSingleByChannel(Hit, WorldLocation, LineTraceEndLocation, ECC_Visibility);
	DrawDebugLine(World, WorldLocation, LineTraceEndLocation, FColor::Red);

	if (Hit.Distance <= HoverHeight)
	{
		float LerpAlpha = (Hit.Location - WorldLocation).Length() / HoverHeight;
		float LerpValue = FMath::Lerp(HoverForce, 0, LerpAlpha);
		float CurveHoverValue = CurveHover->GetFloatValue(Hit.Distance / HoverHeight);

		FVector Force = Hit.ImpactNormal * LerpValue * CurveHoverValue;
		HoverParent->AddForceAtLocation(Force, WorldLocation);
	}
	else
	{
		float CurveGravityValue = CurveGravity->GetFloatValue(Hit.Distance / HoverHeight);

		FVector Force = WorldLocation + (GravityForce * FVector(0, 0, -Hit.Distance / CurveGravityValue));
		HoverParent->AddForceAtLocation(Force, WorldLocation);
	}
}
