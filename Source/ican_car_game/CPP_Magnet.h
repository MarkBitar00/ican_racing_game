#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "CPP_E_MagneticPolarity.h"
#include "CPP_Vehicle.h"
#include "CPP_Magnet.generated.h"

UCLASS()
class ICAN_CAR_GAME_API ACPP_Magnet : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACPP_Magnet();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Construction script
	virtual void OnConstruction(const FTransform& Transform) override;

	// Collision Sphere overlap functions
	UFUNCTION()
	void OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Getter functions
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getters | Magnetism")
	FORCEINLINE EMagneticPolarity GetMagneticPolarity() const { return MagneticPolarity; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Actor components
	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	USphereComponent* Collider;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly, Category = "Components")
	UStaticMeshComponent* Mesh;

public:
	// Public attributes (Magnetism)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Magnetism")
	float MagnetPower = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Magnetism")
	float ColliderRadius = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Magnetism")
	EMagneticPolarity MagneticPolarity = EMagneticPolarity::POSITIVE;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Magnetism")
	UCurveFloat* CurveAttraction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Magnetism")
	UCurveFloat* CurveRepulsion = nullptr;

	// Public attributes (Materials)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Materials")
	UMaterialInterface* MaterialPositive = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes | Materials")
	UMaterialInterface* MaterialNegative = nullptr;

private:
	// Attributes (Magnetism)
	UPROPERTY()
	TArray<ACPP_Vehicle*> VehiclesInRange;

	// Materials
	UMaterialInterface* MaterialPositiveFallback = nullptr;
	UMaterialInterface* MaterialNegativeFallback = nullptr;

	// Curves
	UCurveFloat* CurveAttractionFallback = nullptr;
	UCurveFloat* CurveRepulsionFallback = nullptr;
};
