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

	// Getter functions
	// Getter functions (Components)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Components")
	FORCEINLINE class UStaticMeshComponent* GetParent() const { return HoverParent; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Components")
	FORCEINLINE class UCurveFloat* GetCurveHover() const { return CurveHover; }

	// Getter functions (Hover)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Hover")
	float GetHoverHeight();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Hover")
	float GetHoverForce();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Hover")
	float GetGravityForce();

	// Setter functions
	// Setter functions (Components)
	UFUNCTION(BlueprintCallable, Category = "Setters | Components")
	virtual void SetParent(UStaticMeshComponent* Parent);

	// Setter functions (Hover)
	UFUNCTION(BlueprintCallable, Category = "Setters | Hover")
	virtual void SetHoverHeight(float Height);

	UFUNCTION(BlueprintCallable, Category = "Setters | Hover")
	virtual void SetHoverForce(float Force);

	UFUNCTION(BlueprintCallable, Category = "Setters | Hover")
	virtual void SetGravityForce(float Force);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Attributes (Components)
	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	UStaticMeshComponent* HoverParent = nullptr;

	// Attributes (Hover)
	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Hover")
	float HoverHeight = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Hover")
	float HoverForce = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Hover")
	float GravityForce = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Hover")
	UCurveFloat* CurveHover = nullptr;
};
