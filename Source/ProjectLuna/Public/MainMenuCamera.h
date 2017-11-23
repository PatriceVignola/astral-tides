#pragma once

#include "GameFramework/Actor.h"
#include "LunaLog.h"
#include "MainMenuHUD.h"
#include "OnLevelLoadedData.h"
#include "Runtime/UMG/Public/Components/CanvasPanelSlot.h"
#include "Runtime/UMG/Public/Components/PanelWidget.h"
#include "Runtime/Engine/Classes/Sound/SoundCue.h"
#include "MainMenuCamera.generated.h"

DECLARE_LUNA_LOG(MainMenuCamera);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSelectNextOptionDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSelectPrevOptionDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FConfirmOptionDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBackOptionDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartGameDelegate);

UCLASS()
class PROJECTLUNA_API AMainMenuCamera : public AActor
{
	GENERATED_BODY()
	
public:
	enum class MainMenuCameraState {
		None, WaitingForInput, StartNewGame, ShowCredits, StartingMainMenu, SelectingLevel, Lerping, FadingBlackScreenIn, LoadLevel, Exit
	};

	enum class OptionsEnum { NewGame, /*Continue, */LevelSelect, /*Options, */Exit, Credits };

	UPROPERTY(VisibleAnywhere)
	USceneComponent *Root;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent *Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UMainMenuHUD> MenuHUDCLass;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* StartGameSound;

	UPROPERTY(EditAnywhere, Category = Sound)
	USoundCue* NextPreviousOptionSound;

	UPROPERTY(EditAnywhere)
	float RotLerpDuration = 1;

	UPROPERTY(EditAnywhere)
	float NewGameLerpDuration = 3;

	UPROPERTY(EditAnywhere)
	float MainMenuFadeOutDuration = 1.5;

	UPROPERTY(EditAnywhere)
	float TitleFadeDuration = 0.4;

	UPROPERTY(EditAnywhere)
	float OscillationDelta = 10;

	UPROPERTY(EditAnywhere)
	TArray<FRotator> Rotations;

	UPROPERTY(EditAnywhere)
	TArray<FRotator> LevelRotations;

	UPROPERTY(EditAnywhere)
	FVector StartGameCameraFinalLocation;

	UPROPERTY(EditAnywhere)
	FVector LevelSelectCameraIntermediateLocation;

	UPROPERTY(EditAnywhere)
	FRotator LevelSelectCameraIntermediateRotation;

	UPROPERTY(EditAnywhere)
	FVector CreditsCameraLocation;

	UPROPERTY(EditAnywhere)
	FRotator CreditsCameraRotation;

	UPROPERTY(EditAnywhere)

	FVector ExitCameraLocation;

	UPROPERTY(EditAnywhere)
	FRotator ExitCameraRotation;

	UPROPERTY(BlueprintAssignable)
	FSelectNextOptionDelegate OnSelectNextOption;

	UPROPERTY(BlueprintAssignable)
	FSelectPrevOptionDelegate OnSelectPrevOption;

	UPROPERTY(BlueprintAssignable)
	FConfirmOptionDelegate OnConfirmOption;


	UPROPERTY(BlueprintAssignable)
	FBackOptionDelegate OnBackOption;

	UPROPERTY(BlueprintAssignable)
	FStartGameDelegate OnStartGame;

	AMainMenuCamera();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float deltaTime) override;

	void acceptMainMenuOption();
	void goToPreviousMainMenu();
	void selectPreviousMainMenuOption();
	void selectNextMainMenuOption();

	void setCredditsAcceleration(float value);
	
	void resetLocationRotation();

private:
	UMainMenuHUD* m_menuHUD;
	UImage *prevTitle;
	UImage *currentTitle;

	const int OPTIONS_COUNT = 4;

	UWorld *m_world;
	AProjectLunaGameMode *m_gameMode;

	FVector m_camLocation;

	MainMenuCameraState m_currentState;
	int m_prevOption;
	int m_currentOption;
	float m_lerpElapsedTime;

	int m_currentSelectedLevelIndex;

	float m_oscillationTimer;

	float m_creditsAcceleration;

	FVector2D m_initialCreditsPos;

	FVector m_lerpStartLoc;
	FQuat m_lerpStartRot;
	FVector m_lerpEndLoc;
	FRotator m_lerpEndRot;
	float m_lerpStartAlpha;
	float m_lerpEndAlpha;
	float m_lerpTotalDuration;
	bool m_lerpFadeInScreen;
	MainMenuCameraState m_lerpNextState;

	UImage *m_fadePrevTitle;
	UImage *m_fadeNewTitle;

	float m_blackScreenFadeDuration;
	MainMenuCameraState m_blackScreenFadeNextState;


	void initMainMenuHUD();

	void setInitialRotation();

	void startLerp(float lerpTotalDuration, FVector lerpEndLoc, FRotator lerpEndRot, MainMenuCameraState lerpNextState, bool fadeIn = false, UImage *prevTitle = nullptr, UImage *newTitle = nullptr);
	void lerpToTargetLocRot(float deltaTime);

	void startBlackScreenFadeIn(float duration, MainMenuCameraState newState);

	// Smoother Hermite interporlation (by Ken Perlin)
	float smootherRatioStep(float ratio);

	void updateSelectionTitle();

	void onLevelLoaded(OnLevelLoadedData *eventData);

	void fadeInBlackScreen(float deltaTime);
	void fadeOutBlackScreen(float deltaTime);

	void setState(MainMenuCameraState newState);
};
