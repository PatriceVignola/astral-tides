// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "WolfController.h"

DEFINE_LUNA_LOG(WolfController);

void AWolfController::BeginPlay()
{
	LUNA_WARN(WolfController, "WolfController started playing");

	if (WolfClass == nullptr)
	{
		LUNA_ERROR(WolfController, "BP_Wolf is not assigned to BP_WolfController");
		return;
	}

	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();

	// We cast the level script to our own, custom level script
	if (m_world->GetLevelScriptActor() && m_world->GetLevelScriptActor()->IsA(ALunaLevelScriptActor::StaticClass()))
	{
		m_levelScript = Cast<ALunaLevelScriptActor>(m_world->GetLevelScriptActor());
	}
	else
	{
		LUNA_WARN(WolfController, "The level blueprint needs to inherit from ALunaLevelScriptActor");
	}

	subscribeEvents();
	bindControls();
	cacheMainMenuCamera();

	// The level number 0 is the main menu
	if (m_gameMode->getLevelNumber() == 0)
	{
		possesMainMenuCamera();
	}
	else
	{
		possesWolf();
	}
}

void AWolfController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (InputComponent != m_cinematicControls && InputComponent != m_endCreditsControls) return;

	if ((m_skipMatineePressed || m_skipEndCreditsPressed) && m_matineeSkipCooldown > 0)
	{
		LUNA_WARN(WolfController, "WANT TO SKIP!!! %f", m_matineeSkipCooldown);

		m_gameMode->fadeBlackScreen(1 - m_matineeSkipCooldown / 1);

		m_matineeSkipCooldown -= deltaTime;

		// HACK HOTFIX
		if (m_matineeSkipCooldown <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Entering choices!"));

			if (m_skipMatineePressed)
			{
				UE_LOG(LogTemp, Warning, TEXT("First Choice!"));
				m_levelScript->skipMatinee();
			}
			else if (m_skipEndCreditsPressed)
			{
				m_skipEndCreditsPressed = false;

				UE_LOG(LogTemp, Warning, TEXT("Second Choice!"));
				m_gameMode->getEventManager().raiseEvent("CloseEndGameCredits", new GenericEventData());
				m_gameMode->loadLevel(0, AProjectLunaGameMode::LevelLoadOrigin::Automatic);
			}
		}
	}
}

void AWolfController::subscribeEvents()
{
	m_gameMode->getEventManager().subscribe("OnWolfDrown", this, &AWolfController::onWolfDrown);
	m_gameMode->getEventManager().subscribe("RespawnWolf", this, &AWolfController::respawnWolf);
	m_gameMode->getEventManager().subscribe("OnDeathFadeOver", this, &AWolfController::onDeathFadeOver);
	m_gameMode->getEventManager().subscribe("OnLevelLoaded", this, &AWolfController::onLevelLoaded);
	m_gameMode->getEventManager().subscribe("OnCheckpointReached", this, &AWolfController::onCheckpointReached);
	m_gameMode->getEventManager().subscribe("OnCinematicStarted", this, &AWolfController::onCinematicStarted);
	m_gameMode->getEventManager().subscribe("OnCinematicEnded", this, &AWolfController::onCinematicEnded);
	m_gameMode->getEventManager().subscribe("UnpauseGame", this, &AWolfController::unpauseGame);
	m_gameMode->getEventManager().subscribe("GoingToMainMenuFromPause", this, &AWolfController::goingToMainMenuFromPause);
	m_gameMode->getEventManager().subscribe("SetActiveCamera", this, &AWolfController::setActiveCamera);
	m_gameMode->getEventManager().subscribe("ShowEndGameCredits", this, &AWolfController::showEndGameCredits);
}

void AWolfController::setActiveCamera(SetActiveCameraData *eventData)
{
	SetViewTarget(eventData->getCamera(), FViewTargetTransitionParams());

	// TEMPORARY SHORTCUT, because now the only thing using this are the end credits

}

void AWolfController::cacheMainMenuCamera()
{
	for (TActorIterator<AActor> ActorItr(m_world); ActorItr; ++ActorItr)
	{
		if (ActorItr->IsA(AMainMenuCamera::StaticClass()))
		{
			m_mainMenuCamera = Cast<AMainMenuCamera>(*ActorItr);
			break;
		}
	}
}

void AWolfController::possesMainMenuCamera()
{
	if (!m_mainMenuCamera) return;

	this->UnPossess();
	this->SetViewTarget(m_mainMenuCamera, FViewTargetTransitionParams());

	InputComponent = m_mainMenuCameraControls;
}

void AWolfController::possesWolf()
{
	if (!m_levelScript) return;

	if (!m_wolf)
	{
		FVector spawnPosition = m_lastCheckpointPosition;
		FRotator spawnRotation = m_lastCheckpointRotation;

		m_wolf = m_world->SpawnActor<AWolf>(WolfClass, spawnPosition, spawnRotation);

		// We notify the listeners that the wolf has finally been spawned
		m_gameMode->getEventManager().raiseEvent("OnWolfSpawned", new OnWolfSpawnedData(m_wolf));
	}

	this->SetViewTarget(m_wolf, FViewTargetTransitionParams());
	this->Possess(m_wolf);

	InputComponent = m_aliveWolfControls;
}

void AWolfController::bindControls()
{
	bindMainMenuCameraControls();
	bindAliveWolfControls();
	bindDeadWolfControls();
	bindCinematicControls();
	bindEndCreditsControls();
	bindOverviewControls();
	bindPauseMenuControls();
	bindGoingToMainMenuControls();
	bindUnskippableCinematicControls();
}

void AWolfController::bindMainMenuCameraControls()
{
	m_mainMenuCameraControls = NewObject<UInputComponent>(this, UInputComponent::StaticClass());

	m_mainMenuCameraControls->BindAction("AcceptOption", IE_Pressed, this, &AWolfController::acceptMainMenuOption);
	m_mainMenuCameraControls->BindAction("GoToPreviousMenu", IE_Pressed, this, &AWolfController::goToPreviousMainMenu);

	m_mainMenuCameraControls->BindAxis("SelectNextOption", this, &AWolfController::selectNextMainMenuOption);
	m_mainMenuCameraControls->BindAxis("AccelerateCredits", this, &AWolfController::accelerateCredits);
}

void AWolfController::bindAliveWolfControls()
{
	m_aliveWolfControls = NewObject<UInputComponent>(this, UInputComponent::StaticClass());

	m_aliveWolfControls->BindAction("Jump", IE_Pressed, this, &AWolfController::jump);
	m_aliveWolfControls->BindAction("Jump", IE_Released, this, &AWolfController::stopJump);
	m_aliveWolfControls->BindAction("PushIceBlock", IE_Pressed, this, &AWolfController::enableIceBlockInteraction);
	m_aliveWolfControls->BindAction("PushIceBlock", IE_Released, this, &AWolfController::disableIceBlockInteraction);
	m_aliveWolfControls->BindAction("ToggleOverview", IE_Pressed, this, &AWolfController::toggleOverview);
	m_aliveWolfControls->BindAction("QuitOverview", IE_Pressed, this, &AWolfController::quitOverview);
	m_aliveWolfControls->BindAction("TogglePause", IE_Pressed, this, &AWolfController::togglePause).bExecuteWhenPaused = true;

	m_aliveWolfControls->BindAxis("MoveRight", this, &AWolfController::moveRight);
	m_aliveWolfControls->BindAxis("MoveForward", this, &AWolfController::moveForward);
	m_aliveWolfControls->BindAxis("CamTurn", this, &AWolfController::turnCamera);
	m_aliveWolfControls->BindAxis("CamLookUp", this, &AWolfController::lookUpCamera);

	m_aliveWolfControls->BindAxis("CamTurnMouse", this, &AWolfController::turnCameraMouse);
	m_aliveWolfControls->BindAxis("CamLookUpMouse", this, &AWolfController::lookUpCameraMouse);
	
	m_aliveWolfControls->BindAxis("HowlUp", this, &AWolfController::howlUp);
	m_aliveWolfControls->BindAxis("HowlDown", this, &AWolfController::howlDown);

	// DEBUG
	m_aliveWolfControls->BindAction("QuickTeleport", IE_Pressed, this, &AWolfController::quickTeleport).bExecuteWhenPaused = true;
}

void AWolfController::quickTeleport()
{
	m_wolf->SetActorLocation(FVector(-4328.625, -5764, 2423));
}

void AWolfController::bindDeadWolfControls()
{
	m_deadWolfControls = NewObject<UInputComponent>(this, UInputComponent::StaticClass());

	m_deadWolfControls->BindAxis("CamTurn", this, &AWolfController::turnCamera);
	m_deadWolfControls->BindAxis("CamLookUp", this, &AWolfController::lookUpCamera);
}

void AWolfController::bindCinematicControls()
{
	m_cinematicControls = NewObject<UInputComponent>(this, UInputComponent::StaticClass());

	m_cinematicControls->BindAction("SkipMatinee", IE_Pressed, this, &AWolfController::skipMatineePressed);
	m_cinematicControls->BindAction("SkipMatinee", IE_Released, this, &AWolfController::skipMatineeReleased);
	m_cinematicControls->BindAction("TogglePause", IE_Pressed, this, &AWolfController::togglePause).bExecuteWhenPaused = true;
}

void AWolfController::bindEndCreditsControls()
{
	m_endCreditsControls = NewObject<UInputComponent>(this, UInputComponent::StaticClass());

	m_endCreditsControls->BindAction("SkipEndCredits", IE_Pressed, this, &AWolfController::skipEndCreditsPressed);
	m_endCreditsControls->BindAction("SkipEndCredits", IE_Released, this, &AWolfController::skipEndCreditsReleased);
}

void AWolfController::bindOverviewControls()
{
	m_overviewControls = NewObject<UInputComponent>(this, UInputComponent::StaticClass());

	m_overviewControls->BindAction("ToggleOverview", IE_Pressed, this, &AWolfController::toggleOverview);
	m_overviewControls->BindAxis("HowlUp", this, &AWolfController::howlUp);
	m_overviewControls->BindAxis("HowlDown", this, &AWolfController::howlDown);
}

void AWolfController::bindPauseMenuControls()
{
	m_pauseMenuControls = NewObject<UInputComponent>(this, UInputComponent::StaticClass());

	m_pauseMenuControls->BindAxis("SelectNextOption", this, &AWolfController::selectNextPauseMenuOption).bExecuteWhenPaused = true;
	m_pauseMenuControls->BindAction("AcceptOption", IE_Pressed, this, &AWolfController::acceptPauseMenuOption).bExecuteWhenPaused = true;
	m_pauseMenuControls->BindAction("TogglePause", IE_Pressed, this, &AWolfController::togglePause).bExecuteWhenPaused = true;
}

void AWolfController::bindGoingToMainMenuControls()
{
	m_goingToMainMenuControls = NewObject<UInputComponent>(this, UInputComponent::StaticClass());
}

void AWolfController::bindUnskippableCinematicControls()
{
	m_unskippableCinematicControls = NewObject<UInputComponent>(this, UInputComponent::StaticClass());
}

void AWolfController::skipMatineePressed()
{
	m_matineeSkipCooldown = 1;
	m_skipMatineePressed = true;
}

void AWolfController::skipMatineeReleased()
{
	m_skipMatineePressed = false;

	m_gameMode->fadeBlackScreen(0);
}

void AWolfController::skipEndCreditsPressed()
{
	UE_LOG(LogTemp, Warning, TEXT("******************************skipEndCreditsPressed*********************************"));

	m_skipMatineePressed = false; // Quick hack
	m_matineeSkipCooldown = 1;
	m_skipEndCreditsPressed = true;
}

void AWolfController::skipEndCreditsReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("******************************skipEndCreditsReleased*********************************"));

	m_skipEndCreditsPressed = false;

	m_gameMode->fadeBlackScreen(0);
}

/*
We need to bind the actions to private classes here since MyWolf might
have not been spawned yet when SetupInputComponent is called
*/

void AWolfController::jump()
{
	if (m_wolf && !m_wolf->isDead())
	{
		m_wolf->Jump();
	}
}

void AWolfController::enableIceBlockInteraction()
{
	if(m_wolf) {
		m_wolf->enableIceBlockInteraction();
	}
}

void AWolfController::disableIceBlockInteraction()
{
	if(m_wolf) {
		m_wolf->disableIceBlockInteraction();
	}
}

void AWolfController::stopJump()
{
	if(m_wolf) {
		m_wolf->StopJumping();
	}
}

void AWolfController::toggleOverview()
{
	if(m_wolf) {
		m_wolf->toggleOverview();
		m_gameMode->getEventManager().raiseEvent("OnOverviewPressed", new GenericEventData());
	}
}

void AWolfController::quitOverview()
{
	if (m_wolf) {
		m_wolf->quitOverview();
	}
}

void AWolfController::moveRight(float axisValue)
{
	if(m_wolf) {
		m_wolf->moveRight(axisValue);
	}
}

void AWolfController::moveForward(float axisValue)
{
	if(m_wolf) {
		m_wolf->moveForward(axisValue);
	}
}

void AWolfController::turnCamera(float axisValue)
{
	if(m_wolf) {
		m_wolf->turnCamera(axisValue);
	}
}

void AWolfController::lookUpCamera(float axisValue)
{
	if(m_wolf) {
		m_wolf->lookUpCamera(axisValue);
	}
}

void AWolfController::turnCameraMouse(float axisValue) {
	if(m_wolf) {
		m_wolf->turnCameraMouse(axisValue);
	}
}

void AWolfController::lookUpCameraMouse(float axisValue) {
	if(m_wolf) {
		m_wolf->lookUpCameraMouse(axisValue);
	}
}

void AWolfController::howlUp(float axisValue)
{
	if(m_wolf) {
		m_wolf->howlUp(axisValue);
	}
}

void AWolfController::howlDown(float axisValue)
{
	if(m_wolf) {
		m_wolf->howlDown(axisValue);
	}
	
}

void AWolfController::togglePause()
{
	LUNA_WARN(WolfController, "togglePause()");

	m_isPaused = !m_isPaused;

	SetPause(m_isPaused);

	if (m_isPaused)
	{
		m_beforePauseInput = InputComponent;
		InputComponent = m_pauseMenuControls;

		m_gameMode->getEventManager().raiseEvent("OnGamePaused", new GenericEventData());
	}
	else
	{
		InputComponent = m_beforePauseInput;

		m_gameMode->getEventManager().raiseEvent("OnGameUnpaused", new GenericEventData());
	}
}

void AWolfController::selectNextPauseMenuOption(float axisValue)
{
	LUNA_WARN(WolfController, "selectNextPauseMenuOption()");

	if (axisValue > 0.7)
	{
		m_gameMode->getEventManager().raiseEvent("SelectNextPauseMenuOption", new GenericEventData());
	}
	else if (axisValue < -0.7)
	{
		m_gameMode->getEventManager().raiseEvent("SelectPreviousPauseMenuOption", new GenericEventData());
	}
}

void AWolfController::acceptPauseMenuOption()
{
	LUNA_WARN(WolfController, "acceptPauseMenuOption()");

	m_gameMode->getEventManager().raiseEvent("AcceptPauseMenuOption", new GenericEventData());
}

void AWolfController::acceptMainMenuOption()
{
	m_mainMenuCamera->acceptMainMenuOption();
}

void AWolfController::goToPreviousMainMenu()
{
	m_mainMenuCamera->goToPreviousMainMenu();
}

void AWolfController::selectNextMainMenuOption(float axisValue)
{
	if (axisValue > 0.7)
	{
		m_mainMenuCamera->selectNextMainMenuOption();
	}
	else if (axisValue < -0.7)
	{
		m_mainMenuCamera->selectPreviousMainMenuOption();
	}
}

void AWolfController::accelerateCredits(float axisValue)
{
	m_mainMenuCamera->setCredditsAcceleration(axisValue);
}

void AWolfController::onWolfDrown(OnWolfDrownData *eventData)
{
	// We disable the controls while the wolf is dying
	if(m_wolf) {
		m_wolf->setDyingState(true);
		m_wolf->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	

	InputComponent = m_deadWolfControls;
}

void AWolfController::respawnWolf(RespawnWolfData *eventData)
{
	m_gameMode->getEventManager().raiseEvent("OnWolfRespawned", new OnWolfRespawnedData());

	// Default position if no player start is found;
	FVector spawnPosition = FVector(0, 0, 500);

	if(m_wolf) {
		m_wolf->SetActorLocation(m_lastCheckpointPosition);
		m_wolf->SetActorRotation(m_lastCheckpointRotation);
		LUNA_WARN(WolfController, "Debug: The wolf has been reset by the controller");

		m_wolf->setDyingState(false);
		m_wolf->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void AWolfController::onDeathFadeOver(OnDeathFadeOverData *eventData)
{
	InputComponent = m_aliveWolfControls;
}

void AWolfController::onLevelLoaded(OnLevelLoadedData *eventData)
{
	if (eventData->getLevelNumber() == 0)
	{
		LUNA_WARN(WolfController, "ON LEVEL LOADED!!!");

		if (m_wolf)
		{
			m_wolf->Destroy();

			m_gameMode->getEventManager().raiseEvent("OnWolfDestroyed", new OnWolfSpawnedData(m_wolf));

			m_wolf = nullptr;

			LUNA_WARN(WolfController, "WOLF DESTROYING!!!");
		}

		if (!m_mainMenuCamera)
		{
			cacheMainMenuCamera();
		}

		possesMainMenuCamera();
		m_mainMenuCamera->resetLocationRotation();
	}
	else
	{
		resetGate();

		possesWolf();
	}
}

void AWolfController::resetGate()
{
	for (TActorIterator<AStaticMeshActor> ActorItr(m_world); ActorItr; ++ActorItr)
	{
		if (ActorItr->GetName().Contains("SM_Gate"))
		{
			LUNA_WARN(WolfController, "FOUND A GATE!!! RESETTING IT!!!");
			ActorItr->SetActorLocation(FVector(-3920.022217, -9718.365234, 2215.701904));
			break;
		}
	}
}

void AWolfController::onCheckpointReached(OnCheckpointReachedData *eventData)
{
	m_lastCheckpointPosition = eventData->getLocation();
	m_lastCheckpointRotation = eventData->getRotation();
}

void AWolfController::onCinematicStarted(OnMatineeStartedData *eventData)
{
	InputComponent = eventData->getIsSkippable() ? m_cinematicControls : m_unskippableCinematicControls;

	if (m_wolf)
	{
		m_wolf->InCinematic = true;
	}
}

void AWolfController::onCinematicEnded(OnMatineeStartedData *eventData)
{
	if (!m_wolf) return;

	InputComponent = m_aliveWolfControls;
	m_wolf->InCinematic = false;

	m_gameMode->fadeBlackScreen(0);
}

void AWolfController::showEndGameCredits(GenericEventData *eventData)
{
	InputComponent = m_endCreditsControls;

	m_wolf->InCinematic = true;
}

void AWolfController::unpauseGame(GenericEventData *eventData)
{
	if (m_isPaused)
	{
		togglePause();
	}
}

void AWolfController::goingToMainMenuFromPause(GenericEventData *eventData)
{
	InputComponent = m_goingToMainMenuControls;

	if (m_wolf)
	{
		m_wolf->setCanDie(false);
	}
}