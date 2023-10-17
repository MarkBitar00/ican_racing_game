#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "CPP_HoverComponent.h"
#include "CPP_Vehicle.generated.h"

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
	void SetupHoverComponent(UCPP_HoverComponent* HoverComponent, FVector Location);

	// Getter functions
	// Getter functions (Components)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Components")
	FORCEINLINE class UStaticMeshComponent* GetMesh() const { return Mesh; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Components")
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Components")
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }

	// Getter functions (Camera)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Camera")
	float GetSpringArmLength();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Camera")
	float GetSpringArmSocketOffsetPitch();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Camera")
	float GetCameraCurrentZoom();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Camera")
	float GetCameraCurrentOffset();

	// Getter functions (Movement)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Movement")
	FRotator GetMeshWorldRotation();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Movement")
	float GetInitialAccelerationSpeed();

	// Setter functions
	// Setter functions (Camera)
	UFUNCTION(BlueprintCallable, Category = "Setters | Camera")
	virtual void SetSpringArmLength(float Length);

	UFUNCTION(BlueprintCallable, Category = "Setters | Camera")
	virtual void SetSpringArmSocketOffsetPitch(float Pitch);

	UFUNCTION(BlueprintCallable, Category = "Setters | Camera")
	virtual void SetCameraCurrentZoom(float Zoom);

	UFUNCTION(BlueprintCallable, Category = "Setters | Camera")
	virtual void SetCameraCurrentOffset(float Pitch);

	// Setter functions (Movement)
	UFUNCTION(BlueprintCallable, Category = "Setters | Movement")
	virtual void SetMeshLinearDamping(float Damping);

	UFUNCTION(BlueprintCallable, Category = "Setters | Movement")
	virtual void SetMeshAngularDamping(float Damping);

	UFUNCTION(BlueprintCallable, Category = "Setters | Movement")
	virtual void SetMeshCenterOfMassHeight(float Height);

	UFUNCTION(BlueprintCallable, Category = "Setters | Movement")
	virtual void SetMeshWorldRotation(FRotator Rotation);

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

	// Attributes (Camera)
	UPROPERTY(BlueprintReadonly, Category = "Camera")
	float CameraCurrentZoom;

	UPROPERTY(BlueprintReadonly, Category = "Camera")
	float CameraCurrentOffset;

	// Attributes (Movement)
	UPROPERTY(BlueprintReadonly, Category = "Movement")
	float InitialAccelerationSpeed;

public:
	// Public attributes (Camera)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraInitialZoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxCameraZoom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxCameraOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraInterpolationSpeed;

	// Public attributes (Movement)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float AccelerationSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SteeringSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SteeringRotationForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MaxDecelerationDuration;

	// Public attributes (Hover)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover")
	float HoverHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover")
	float HoverForce;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover")
	float LinearDamping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover")
	float AngularDamping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover")
	float CenterOfMassHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover")
	float GravityForce;
};
