#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "CPP_HoverComponent.h"
#include "CPP_E_MagneticPolarity.h"
#include "CPP_Vehicle.generated.h"

class ACPP_Magnet;

UCLASS()
class ICAN_CAR_GAME_API ACPP_Vehicle : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this Pawn's properties
	ACPP_Vehicle();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Functions
	void TimelineDecelerationUpdate(float Alpha);
	void UpdateCenterOfMass();
	float GetCurveBoostDuration();

	// Getter functions
	// Getter functions (Components)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Components")
	FORCEINLINE class UStaticMeshComponent* GetMesh() const { return Mesh; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Components")
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Components")
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Components")
	FORCEINLINE class USceneComponent* GetSteerLeftComponent() const { return SteerLeftLocation; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Components")
	FORCEINLINE class USceneComponent* GetSteerRightComponent() const { return SteerRightLocation; }

	// Getter functions (Materials)
	FORCEINLINE class UMaterialInstance* GetMaterialPositive() const { return MaterialPositive; }
	FORCEINLINE class UMaterialInstance* GetMaterialNegative() const { return MaterialNegative; }

	// Getter functions (Timelines)
	FORCEINLINE class UTimelineComponent* GetDecelerationTimeline() const { return TimelineDeceleration; }

	// Getter functions (Camera)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Camera")
	float GetCameraCurrentZoom();

	// Getter functions (Movement)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Movement")
	float GetInitialAccelerationSpeed();

	// Getter functions (Magnetism)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Magnetism")
	bool GetCanSwitchPolarity();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Magnetism")
	EMagneticPolarity GetMagneticPolarity();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Magnetism")
	FORCEINLINE class UCurveFloat* GetCurveAttraction() const { return CurveAttraction; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Magnetism")
	FORCEINLINE class UCurveFloat* GetCurveRepulsion() const { return CurveRepulsion; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Magnetism")
	FORCEINLINE class UCurveFloat* GetCurveBoost() const { return CurveBoost; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Magnetism")
	FORCEINLINE class ACPP_Magnet* GetMagnetInRange() const { return MagnetInRange; }

	// Setter functions
	// Setter functions (Camera)
	UFUNCTION(BlueprintCallable, Category = "Setters | Camera")
	virtual void SetCameraCurrentZoom(float Zoom);

	UFUNCTION(BlueprintCallable, Category = "Setters | Camera")
	virtual void SetCameraCurrentOffset(float Pitch);

	// Setter functions (Magnetism)
	UFUNCTION(BlueprintCallable, Category = "Setters | Magnetism")
	virtual void SetCanSwitchPolarity(bool bCanSwitch);

	UFUNCTION(BlueprintCallable, Category = "Setters | Magnetism")
	virtual void SetMagneticPolarity(EMagneticPolarity Polarity);

	UFUNCTION(BlueprintCallable, Category = "Setters | Magnetism")
	virtual void SetMagnetInRange(ACPP_Magnet* Magnet);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Pawn components
	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category="Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	UCPP_HoverComponent* HoverFrontLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	UCPP_HoverComponent* HoverFrontRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	UCPP_HoverComponent* HoverBackLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	UCPP_HoverComponent* HoverBackRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	USceneComponent* SteerLeftLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	USceneComponent* SteerRightLocation;

	// Attributes (Camera)
	UPROPERTY(BlueprintReadonly, Category = "Camera")
	float CameraCurrentZoom = 0;

	UPROPERTY(BlueprintReadonly, Category = "Camera")
	float CameraCurrentOffset = 0;

	// Attributes (Movement)
	UPROPERTY(BlueprintReadonly, Category = "Movement")
	float InitialAccelerationSpeed = 12000;

	// Attributes (Magnetism)
	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	bool bCanSwitchPolarity = true;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	EMagneticPolarity MagneticPolarity = EMagneticPolarity::POSITIVE;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	UCurveFloat* CurveAttraction = nullptr;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	UCurveFloat* CurveRepulsion = nullptr;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	UCurveFloat* CurveBoost = nullptr;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	ACPP_Magnet* MagnetInRange = nullptr;

	// Timeline Components
	UPROPERTY()
	UTimelineComponent* TimelineDeceleration = nullptr;

	UPROPERTY()
	UCurveFloat* CurveTimeline = nullptr;

	FOnTimelineFloat TimelineUpdate{};

public:
	// Public attributes (Camera)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float CameraInitialZoom = 400;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float MaxCameraZoom = 600;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float MaxCameraOffset = 120;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float CameraInterpolationSpeed = 1;

	// Public attributes (Movement)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Movement")
	float AccelerationSpeed = 12000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Movement")
	float SteeringSpeed = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Movement")
	float SteeringRotationForce = 80000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Movement")
	float MaxRotation = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Movement")
	float MaxDecelerationDuration = 3;

	// Public attributes (Hover)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Hover")
	float HoverHeight = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Hover")
	float HoverForce = 150000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Hover")
	float LinearDamping = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Hover")
	float AngularDamping = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Hover")
	float CenterOfMassHeight = -100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Hover")
	float GravityForce = 3000;

	// Public attributes (Magnetism)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Magnetism")
	float PolarityDelay = 1;

private:
	// Materials
	UMaterialInstance* MaterialPositive = nullptr;
	UMaterialInstance* MaterialNegative = nullptr;
};
