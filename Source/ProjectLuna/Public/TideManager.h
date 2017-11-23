// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "WaterPlane.h"
#include "LunaLog.h"
#include "ProjectLunaGameMode.h"
#include "OnHowledData.h"
#include "OnCheckpointReachedData.h"
#include "OnWolfRespawnedData.h"
#include "OnWaterLevelChangedData.h"
#include "OnLevelLoadedData.h"
#include "OnBuoyantObjectSpawnedData.h"
#include "OnStarCollectedData.h"
#include "TideManager.generated.h"

// Forward declaraction
class ALunaLevelScriptActor;

DECLARE_LUNA_LOG(TideManager);

UCLASS()
class PROJECTLUNA_API ATideManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ATideManager();
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

private:
	UWorld *m_world;
	TArray<AWaterPlane *> m_waterPlaneArray;
	ALunaLevelScriptActor *m_levelScript;

	TArray<ABuoyantObjectRuntime *> m_levelBuoyantObjectRuntimes;

	float m_currentHeightDelta;

	bool m_justLoadedLevel;

	void onWolfHowled(OnHowledData *event);
	void updateWaterHeightDelta();
	void onCheckpointReached(OnCheckpointReachedData *eventData);
	void onWolfRespawned(OnWolfRespawnedData *eventData);

	AProjectLunaGameMode *m_gameMode;

	//TODO: Temporary
	float m_lastCheckpointWaterHeightDelta;

	void onBuoyantObjectRuntimeSpawned(OnBuoyantObjectSpawnedData *eventData);
	void onLevelLoaded(OnLevelLoadedData *eventData);
	void onLevelUnloaded(OnLevelLoadedData *eventData);
	void onStarCollected(OnStarCollectedData *eventData);
};
