// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "LunaLevelScriptActor.h"
#include "Runtime/Engine/Classes/Matinee/MatineeActor.h"
#include "ProjectLunaGameMode.h"

DEFINE_LUNA_LOG(LunaLevelScriptActor);

ALunaLevelScriptActor::ALunaLevelScriptActor()
{
}

void ALunaLevelScriptActor::BeginPlay()
{
	Super::BeginPlay();

	LUNA_WARN(LunaLevelScriptActor, "LunaLevelScriptActor started playing");

	m_world = GetWorld();

	m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();

	/*if (AOrquaControllerClass)
	{
		m_orquaController = m_world->SpawnActor<AOrquaController>(AOrquaControllerClass, FVector(), FRotator::ZeroRotator);
	}
	else
	{
		LUNA_WARN(LunaLevelScriptActor, "An OrquaController has to be assigned to the level blueprint");
	}*/

	//initMaxWaterDelta();

	m_gameMode->getEventManager().subscribe("OnLevelLoaded", this, &ALunaLevelScriptActor::onLevelLoadedFromCode);

	LUNA_WARN(LunaLevelScriptActor, "BEGIN PLAY!!!");
	LUNA_WARN(LunaLevelScriptActor, "BEGIN PLAY!!!");
	LUNA_WARN(LunaLevelScriptActor, "BEGIN PLAY!!!");

	// Hack
	//m_levelhasBeenLoaded = true;
}
void ALunaLevelScriptActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	for (TActorIterator<AMatineeActor> ActorItr(m_world); ActorItr; ++ActorItr)
	{
		if (ActorItr->IsA(AMatineeActor::StaticClass()))
		{
			AMatineeActor *matActor = Cast<AMatineeActor>(*ActorItr);
			matActor->Stop();
		}
	}

	m_levelhasBeenLoaded = false;
}
void ALunaLevelScriptActor::showEndGameCredits()
{
	LUNA_WARN(LunaLevelScriptActor, "show end game credits!!!!!!!");
	m_gameMode->getEventManager().raiseEvent("ShowEndGameCredits", new GenericEventData());
}

void ALunaLevelScriptActor::setCurrentCamera(ACameraActor *camera)
{
	m_gameMode->getEventManager().raiseEvent("SetActiveCamera", new SetActiveCameraData(camera));
}

void ALunaLevelScriptActor::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (m_mustRaiseCinematicEvent)
	{
		LUNA_WARN(LunaLevelScriptActor, "MUST RAISE CINEMATIC EVENT!!!!!!!!!!!");
		LUNA_WARN(LunaLevelScriptActor, "MUST RAISE CINEMATIC EVENT!!!!!!!!!!!");
		LUNA_WARN(LunaLevelScriptActor, "MUST RAISE CINEMATIC EVENT!!!!!!!!!!!");

		m_mustRaiseCinematicEvent = false;

		m_gameMode->getEventManager().raiseEvent("OnCinematicStarted", new OnMatineeStartedData(m_skippableCinematic));
	}
}

void ALunaLevelScriptActor::BeginDestroy()
{
	Super::BeginDestroy();

	if (m_gameMode)
	{
		m_gameMode->getEventManager().unsubscribe("OnLevelLoaded", this, &ALunaLevelScriptActor::onLevelLoadedFromCode);
	}
}

void ALunaLevelScriptActor::incrementStarsCount()
{
	++m_collectedStarsCount;

	m_gameMode->getEventManager().raiseEvent("OnStarCollected", new OnStarCollectedData(m_gameMode->getLevelNumber() - 1, m_collectedStarsCount - 1));
}

const int ALunaLevelScriptActor::getCountStarsCollected() const {
	return m_collectedStarsCount;
}

float ALunaLevelScriptActor::getMaxWaterDelta()
{
	return m_maxWaterDelta;
}

float ALunaLevelScriptActor::getCurrentWaterDelta()
{
	// Really not efficient, but it's a last minute hack

	if (m_levelIndex >= LevelStarSteps.Num()) return 0;

	float waterHeight = 0;
	int stepIncrement = 0;

	for (int32 step : LevelStarSteps[m_levelIndex].StarSteps)
	{
		stepIncrement += step;

		if (m_collectedStarsCount >= stepIncrement)
		{
			waterHeight += WaterHeightPerStep;
		}
	}

	return waterHeight;
}

void ALunaLevelScriptActor::onMatineeStarted(bool isSkippable)
{
	LUNA_WARN(LunaLevelScriptActor, "ON MATINEE STARTED!!!");
	LUNA_WARN(LunaLevelScriptActor, "ON MATINEE STARTED!!!");
	LUNA_WARN(LunaLevelScriptActor, "ON MATINEE STARTED!!!");

	if (m_levelhasBeenLoaded)
	{
		m_gameMode->getEventManager().raiseEvent("OnCinematicStarted", new OnMatineeStartedData(isSkippable));
	}
	else
	{
		LUNA_WARN(LunaLevelScriptActor, "SETTING BOOL TO TRUE!!!");
		LUNA_WARN(LunaLevelScriptActor, "SETTING BOOL TO TRUE!!!");
		LUNA_WARN(LunaLevelScriptActor, "SETTING BOOL TO TRUE!!!");

		m_mustRaiseCinematicEvent = true;
		m_skippableCinematic = isSkippable;
	}
}

void ALunaLevelScriptActor::onMatineeEnded()
{
	LUNA_WARN(LunaLevelScriptActor, "ON MATINEE ENDED!!!");
	LUNA_WARN(LunaLevelScriptActor, "ON MATINEE ENDED!!!");
	LUNA_WARN(LunaLevelScriptActor, "ON MATINEE ENDED!!!");

	m_gameMode->getEventManager().raiseEvent("OnCinematicEnded", new OnMatineeStartedData(false));
}

void ALunaLevelScriptActor::onLevelLoaded(int32 levelNumber)
{
	m_gameMode->getEventManager().raiseEvent("OnLevelLoaded", new OnLevelLoadedData(levelNumber, AProjectLunaGameMode::LevelLoadOrigin::Automatic));
}

void ALunaLevelScriptActor::onLevelLoadedFromCode(OnLevelLoadedData *eventData)
{
	if (eventData->getLevelNumber() == 1 || eventData->getLevelNumber() == 2)
	{
		if (eventData->getLevelNumber() > LevelStarSteps.Num()) return;

		m_levelIndex = eventData->getLevelNumber() - 1;

		m_maxWaterDelta = WaterHeightPerStep * LevelStarSteps[eventData->getLevelNumber() - 1].StarSteps.Num();
	}

	m_collectedStarsCount = 0;

	//initMaxWaterDelta();
}

void ALunaLevelScriptActor::skipMatinee()
{
	LUNA_WARN(LunaLevelScriptActor, "SKIP MATINEE!!!");

	if (OnMatineeSkip.IsBound())
	{
		LUNA_WARN(LunaLevelScriptActor, "BOUND!!!");
		// This is to alert the blueprint listeners
		OnMatineeSkip.Broadcast();
	}
}

void ALunaLevelScriptActor::showOverviewPrompt()
{
	LUNA_WARN(LunaLevelScriptActor, "OVERVIEW PROMPT!!!");
	m_gameMode->getEventManager().raiseEvent("ShowOverviewPrompt", new GenericEventData());
}