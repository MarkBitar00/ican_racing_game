#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DrawDebugHelpers.h"
#include "CPP_HoverComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ICAN_CAR_GAME_API UCPP_HoverComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCPP_HoverComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Custom events
	UFUNCTION(BlueprintCallable)
	void Init(UStaticMeshComponent* Parent, float TraceLength, float HoverPower, float GravityPower);

	// Functions
	UFUNCTION(BlueprintCallable)
	void Hover();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	// Attributes (Components)
	UPROPERTY()
	UStaticMeshComponent* HoverParent = nullptr;

	// Attributes (Hover)
	UPROPERTY()
	float HoverHeight = 0;

	UPROPERTY()
	float HoverForce = 0;

	UPROPERTY()
	float GravityForce = 0;

	UPROPERTY()
	UCurveFloat* CurveHover = nullptr;
};
