#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/SphereComponent.h"
#include "CPP_HoverComponent.h"
#include "CPP_E_MagneticPolarity.h"
#include "InputActionValue.h"
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

	// Collision Sphere overlap functions
	UFUNCTION()
	void OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Events
	UFUNCTION(BlueprintImplementableEvent)
	void StartDeceleration(float PlayRate);

	UFUNCTION(BlueprintImplementableEvent)
	void StopDeceleration();

	// Functions
	UFUNCTION(BlueprintCallable)
	void TimelineDecelerationUpdate(float Alpha);

	void UpdateCenterOfMass();
	float GetDistanceToMagnet();

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

	// Replicate properties
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Pawn components
	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components", Replicated)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components", Replicated)
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
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

	// Input components
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ActionAccelerate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ActionSteer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ActionBrake;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ActionTogglePolarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ActionRestart;

	// Input Actions handler methods
	void Accelerate(const struct FInputActionInstance& Instance);
	void StartAccelerate();
	void StopAccelerate(const struct FInputActionInstance& Instance);

	void Steer(const struct FInputActionValue& Value);
	void StopSteer();

	void StartBrake();
	void StopBrake();

	void TogglePolarity();

	void HandleRestart();

	// Input Actions server implementations
	UFUNCTION(Server, Unreliable)
	void HandleAccelerate(float Acceleration, FVector Forward);

	UFUNCTION(Server, Unreliable)
	void HandleSteer(float Steer, FVector Force, FVector Location);

	UFUNCTION(NetMulticast, Reliable)
	void HandleTogglePolarity(EMagneticPolarity NewPolarity, UMaterialInterface* NewMaterial);

	UFUNCTION(Server, Reliable)
	void HandleTogglePolarityServer(EMagneticPolarity NewPolarity, UMaterialInterface* NewMaterial);

	// Attributes (Camera)
	UPROPERTY(BlueprintReadonly, Category = "Camera")
	float CameraCurrentZoom = 0;

	UPROPERTY(BlueprintReadonly, Category = "Camera")
	float CameraCurrentOffset = 0;

	// Attributes (Movement)
	UPROPERTY(BlueprintReadonly, Category = "Movement")
	float InitialAccelerationSpeed = 12000;

	UPROPERTY(BlueprintReadonly, Category = "Movement")
	UCurveFloat* CurveAcceleration = nullptr;

	UPROPERTY(BlueprintReadonly, Category = "Movement")
	FVector InitialPosition = FVector(0, 0, 0);

	UPROPERTY(BlueprintReadonly, Category = "Movement")
	FRotator InitialRotation = FRotator();

	// Attributes (Magnetism)
	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	bool bCanSwitchPolarity = true;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism", Replicated)
	EMagneticPolarity MagneticPolarity = EMagneticPolarity::POSITIVE;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	UCurveFloat* CurveVehicleAttraction = nullptr;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	UCurveFloat* CurveVehicleRepulsion = nullptr;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	UCurveFloat* CurveBoostMultiplier = nullptr;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	UCurveFloat* CurveBoostDuration = nullptr;

	UPROPERTY(BlueprintReadonly, Category = "Magnetism")
	ACPP_Magnet* MagnetInRange = nullptr;

	// Timer components
	FTimerHandle PolarityTimerHandle;
	FTimerHandle BoostTimerHandle;
	void OnPolarityTimerEnd();
	void OnBoostTimerEnd();

public:
	// Public attributes (Camera)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float CameraInitialZoom = 400;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float MaxCameraZoom = 600;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float MaxBoostCameraZoom = 800;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float MaxCameraOffset = 120;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float CameraInterpolationSpeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float SpringArmTargetOffset = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float CameraRotation = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Camera")
	float CameraFieldOfView = 100;

	// Public attributes (Movement)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Movement")
	float AccelerationSpeed = 12000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Movement")
	float MaxBoostAccelerationSpeed = 24000;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Magnetism")
	bool bActivateVehicleMagnetism = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Magnetism")
	float ColliderRadius = 800;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Magnetism")
	float VehicleMagneticPower = 500;

	// Public attributes (Materials)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Materials")
	UMaterialInterface* MaterialPositive = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Materials")
	UMaterialInterface* MaterialNegative = nullptr;

private:
	// Attributes (Magnetism)
	UPROPERTY()
	TArray<ACPP_Vehicle*> PlayersInRange;

	// Materials
	UMaterialInterface* MaterialPositiveFallback = nullptr;
	UMaterialInterface* MaterialNegativeFallback = nullptr;
};
