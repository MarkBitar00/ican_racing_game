#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
//#include "CPP_Racer.h"
#include "CPP_Vehicle.h"
#include "CPP_Magnet.h"
#include "CPP_RacerPlayerController.generated.h"

UCLASS()
class ICAN_CAR_GAME_API ACPP_RacerPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	// Input Mapping Context
	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "Player Inputs")
	UInputMappingContext* InputMappingContext = nullptr;

	// Input Actions
	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "Player Inputs")
	UInputAction* ActionAccelerate = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "Player Inputs")
	UInputAction* ActionBrake = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "Player Inputs")
	UInputAction* ActionTogglePolarity = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadonly, Category = "Player Inputs")
	UInputAction* ActionSteer = nullptr;

protected:
	// Pawn possession overrides
	virtual void OnPossess(APawn* aPawn) override;
	virtual void OnUnPossess() override;

	// Input Action handlers
	// Accelerate Input Action handlers
	void HandleAccelerate(const FInputActionValue& InputActionValue);
	void HandleStartAccelerate();
	void HandleStopAccelerate(const FInputActionInstance& InputActionInstance);

	// Brake Input Action handlers
	void HandleStartBrake();
	void HandleStopBrake();

	// Steer Input Action handlers
	void HandleSteer(const FInputActionValue& InputActionValue);
	void HandleStopSteer();

	// Toggle Polarity Input Action handler
	void HandleTogglePolarity();

private:
	// Enhanced Input Component
	UPROPERTY()
	UEnhancedInputComponent* EnhancedInputComponent = nullptr;

	// Controlled Pawn
	UPROPERTY()
	//ACPP_Racer* PlayerCharacter = nullptr;
	ACPP_Vehicle* PlayerCharacter = nullptr;

	// Timer
	FTimerHandle PolarityTimerHandle;
	FTimerHandle BoostTimerHandle;
	void OnPolarityTimerEnd();
	void OnBoostTimerEnd();
};
