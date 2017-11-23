#pragma once

#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/Image.h"
#include "MainMenuHUD.generated.h"

class OnLevelLoadedData;

class AProjectLunaGameMode;

UCLASS()
class PROJECTLUNA_API UMainMenuHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	enum class MainMenuHUDState {
		None, ShowCredits
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *BlackOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *NewGameTitle;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *ContinueTitle;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *LevelSelectTitle;

	/*UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *OptionsTitle;*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *ExitTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *CreditsTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *LeftArrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *RightArrow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *BackButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *CreditsList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *Level1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *Level2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *Level3;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	AProjectLunaGameMode *m_gameMode;
	MainMenuHUDState m_currentState;

	void onLevelLoaded(OnLevelLoadedData *eventData);
	float m_creditsAcceleration;
};