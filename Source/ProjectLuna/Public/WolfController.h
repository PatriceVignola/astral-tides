// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "Wolf.h"
#include "LunaLog.h"
#include "OnWolfDrownData.h"
#include "RespawnWolfData.h"
#include "OnDeathFadeOverData.h"
#include "ProjectLunaGameMode.h"
#include "LunaLevelScriptActor.h"
#include "OnWolfSpawnedData.h"
#include "MainMenuCamera.h"
#include "OnLevelLoadedData.h"
#include "OnCheckpointReachedData.h"
#include "OnMatineeStartedData.h"
#include "GenericEventData.h"
#include "SetActiveCameraData.h"
#include "WolfController.generated.h"

DECLARE_LUNA_LOG(WolfController);

UCLASS()
class PROJECTLUNA_API AWolfController : public APlayerController
{
	GENERATED_BODY() 

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AWolf> WolfClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AMainMenuCamera> MainMenuCameraClass;

	virtual void BeginPlay() override;
	virtual void Tick(float deltaTime) override;

private:
	UInputComponent *m_mainMenuCameraControls;
	UInputComponent *m_aliveWolfControls;
	UInputComponent *m_deadWolfControls;
	UInputComponent *m_cinematicControls;
	UInputComponent *m_endCreditsControls;
	UInputComponent *m_overviewControls;
	UInputComponent *m_pauseMenuControls;
	UInputComponent *m_goingToMainMenuControls;
	UInputComponent *m_unskippableCinematicControls;

	UInputComponent *m_beforePauseInput;

	UWorld *m_world;
	AWolf *m_wolf;
	AMainMenuCamera *m_mainMenuCamera;
	AProjectLunaGameMode *m_gameMode;
	ALunaLevelScriptActor *m_levelScript;

	FVector m_lastCheckpointPosition;
	FRotator m_lastCheckpointRotation;

	bool m_isPaused;

	float m_matineeSkipCooldown;
	bool m_skipMatineePressed;
	bool m_skipEndCreditsPressed;

	void cacheMainMenuCamera();

	void subscribeEvents();

	void setActiveCamera(SetActiveCameraData *eventData);

	void possesMainMenuCamera();
	void possesWolf();

	// Input bindings
	void bindControls();
	void bindMainMenuCameraControls();
	void bindAliveWolfControls();
	void bindDeadWolfControls();
	void bindCinematicControls();
	void bindEndCreditsControls();
	void bindOverviewControls();
	void bindPauseMenuControls();
	void bindGoingToMainMenuControls();
	void bindUnskippableCinematicControls();

	void quickTeleport();

	void jump();
	void toggleOverview();
	void enableIceBlockInteraction();
	void disableIceBlockInteraction();
	void stopJump();
	void moveRight(float axisValue);
	void moveForward(float axisValue);
	void turnCamera(float axisValue);
	void lookUpCamera(float axisValue);
	void turnCameraMouse(float axisValue);
	void lookUpCameraMouse(float axisValue);
	void howlUp(float axisValue);
	void howlDown(float axisValue);
	void togglePause();
	void skipMatineePressed();
	void skipMatineeReleased();
	void skipEndCreditsPressed();
	void skipEndCreditsReleased();
	void resetGate();
	void quitOverview();
	void showEndGameCredits(GenericEventData *eventData);

	void selectNextPauseMenuOption(float axisValue);
	void acceptPauseMenuOption();

	void acceptMainMenuOption();
	void goToPreviousMainMenu();
	void selectNextMainMenuOption(float axisValue);
	void accelerateCredits(float axisValue);

	void onCheckpointReached(OnCheckpointReachedData *eventData);

	void respawnWolf(RespawnWolfData *eventData);
	void onWolfDrown(OnWolfDrownData *eventData);
	void onDeathFadeOver(OnDeathFadeOverData *eventData);
	void onLevelLoaded(OnLevelLoadedData *eventData);

	void onCinematicStarted(OnMatineeStartedData *eventData);
	void onCinematicEnded(OnMatineeStartedData *eventData);
	void unpauseGame(GenericEventData *eventData);
	void goingToMainMenuFromPause(GenericEventData *eventData);
};
