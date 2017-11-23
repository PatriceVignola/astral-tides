// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "TideManager.h"
#include "EventManager.h"
#include "LunaLevelScriptActor.h"

DEFINE_LUNA_LOG(TideManager);

ATideManager::ATideManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void ATideManager::BeginPlay()
{
	Super::BeginPlay();

	LUNA_WARN(TideManager, "TideManager started playing");

	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode*)m_world->GetAuthGameMode();

	// This shouldn't be too slow, but if it is, we might need to put them manually in the editor
	for (TActorIterator<AActor> ActorItr(m_world); ActorItr; ++ActorItr)
	{
		if (ActorItr->IsA(AWaterPlane::StaticClass()))
		{
			AWaterPlane *waterPlane = Cast<AWaterPlane>(*ActorItr);
			m_waterPlaneArray.Add(waterPlane);
		}
	}

	// We cast the level script to our own, custom level script
	if (m_world->GetLevelScriptActor() && m_world->GetLevelScriptActor()->IsA(ALunaLevelScriptActor::StaticClass()))
	{
		m_levelScript = Cast<ALunaLevelScriptActor>(m_world->GetLevelScriptActor());
	}
	else
	{
		LUNA_WARN(TideManager, "The level blueprint needs to inherit from ALunaLevelScriptActor");
	}

	m_gameMode->getEventManager().subscribe("OnHowled", this, &ATideManager::onWolfHowled);
	m_gameMode->getEventManager().subscribe("OnCheckpointReached", this, &ATideManager::onCheckpointReached);
	m_gameMode->getEventManager().subscribe("OnWolfRespawned", this, &ATideManager::onWolfRespawned);
	m_gameMode->getEventManager().subscribe("OnBuoyantObjectRuntimeSpawned", this, &ATideManager::onBuoyantObjectRuntimeSpawned);
	m_gameMode->getEventManager().subscribe("OnLevelLoaded", this, &ATideManager::onLevelLoaded);
	m_gameMode->getEventManager().subscribe("OnLevelUnloaded", this, &ATideManager::onLevelUnloaded);
	m_gameMode->getEventManager().subscribe("OnStarCollected", this, &ATideManager::onStarCollected);
}

void ATideManager::BeginDestroy()
{
	Super::BeginDestroy();

	if (m_gameMode)
	{
		m_gameMode->getEventManager().unsubscribe("OnHowled", this, &ATideManager::onWolfHowled);
		m_gameMode->getEventManager().unsubscribe("OnCheckpointReached", this, &ATideManager::onCheckpointReached);
		m_gameMode->getEventManager().unsubscribe("OnWolfRespawned", this, &ATideManager::onWolfRespawned);
		m_gameMode->getEventManager().unsubscribe("OnBuoyantObjectRuntimeSpawned", this, &ATideManager::onBuoyantObjectRuntimeSpawned);
		m_gameMode->getEventManager().unsubscribe("OnLevelLoaded", this, &ATideManager::onLevelLoaded);
		m_gameMode->getEventManager().unsubscribe("OnLevelUnloaded", this, &ATideManager::onLevelUnloaded);
		m_gameMode->getEventManager().unsubscribe("OnStarCollected", this, &ATideManager::onStarCollected);
	}
}

// TODO: Maybe put a callback to let the wolf know if it reached the limits
void ATideManager::onWolfHowled(OnHowledData *event)
{
	if (!m_levelScript) return;


	float currentMaxDelta = m_levelScript->getCurrentWaterDelta();

	float speed = m_levelScript->WaterSpeedMultiplier * event->getSpeed() * event->getAxisValue();

	UE_LOG(LogTemp, Warning, TEXT("%f"), speed);

	m_currentHeightDelta += speed;

	if (m_currentHeightDelta < 0)
	{
		m_currentHeightDelta = 0;
	}
	else if (m_currentHeightDelta > currentMaxDelta)
	{
		m_currentHeightDelta = currentMaxDelta;
	}

	updateWaterHeightDelta();
}

void ATideManager::updateWaterHeightDelta()
{
	for (AWaterPlane *waterPlane : m_waterPlaneArray)
	{
		waterPlane->setWaterHeightDelta(m_currentHeightDelta);
	}

	float currentMaxDelta = m_levelScript->getCurrentWaterDelta();

	m_gameMode->getEventManager().raiseEvent("OnWaterLevelChanged", new OnWaterLevelChangedData(m_currentHeightDelta, 0, currentMaxDelta, m_levelScript->getMaxWaterDelta()));
}

void ATideManager::onCheckpointReached(OnCheckpointReachedData *eventData)
{
	LUNA_WARN(TideManager, "CHECKPOINT REACHED!!!");

	m_lastCheckpointWaterHeightDelta = m_currentHeightDelta;
}

void ATideManager::onWolfRespawned(OnWolfRespawnedData *eventData)
{
	LUNA_WARN(TideManager, "Resetting water level");

	m_currentHeightDelta = m_lastCheckpointWaterHeightDelta;

	updateWaterHeightDelta();
}

void ATideManager::onBuoyantObjectRuntimeSpawned(OnBuoyantObjectSpawnedData *eventData)
{
	m_levelBuoyantObjectRuntimes.Add(eventData->getBuoyantObject());

	for (AWaterPlane *waterPlane : m_waterPlaneArray)
	{
		waterPlane->cacheBuoyantObject(eventData->getBuoyantObject());
	}
}

void ATideManager::onLevelLoaded(OnLevelLoadedData *eventData)
{
	m_currentHeightDelta = 0;

	updateWaterHeightDelta();

	LUNA_WARN(TideManager, "LEVEL LOADED!!! %d", eventData->getLevelNumber());

	for (AWaterPlane *waterPlane : m_waterPlaneArray)
	{
		waterPlane->cacheCurrentLevelIndex(eventData->getLevelNumber());
	}
}

void ATideManager::onLevelUnloaded(OnLevelLoadedData *eventData)
{
	/*
	for (ABuoyantObjectRuntime * buoyantObject : m_levelBuoyantObjectRuntimes)
	{
		buoyantObject->Destroy();
	}*/

	for (AWaterPlane *waterPlane : m_waterPlaneArray)
	{
		waterPlane->clearCachedBuoyantObjects();
	}

	m_levelBuoyantObjectRuntimes.Empty();
}

void ATideManager::onStarCollected(OnStarCollectedData *eventData)
{
	updateWaterHeightDelta();
}