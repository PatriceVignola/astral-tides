// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Wolf.h"
#include "LunaLog.h"
#include "OnWolfDrownData.h"
#include "ProjectLunaGameMode.h"
#include "OnWolfSpawnedData.h"
#include "Checkpoint.h"
#include "WaterPlane.generated.h"

DECLARE_LUNA_LOG(WaterPlane);

class ABuoyantObjectRuntime;

UCLASS()
class PROJECTLUNA_API AWaterPlane : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(VisibleAnywhere)
	class USceneComponent *Root;

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent *OverlapMesh;

	AWaterPlane();

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;

	void setWaterHeightDelta(float heightDelta);

	void cacheBuoyantObject(ABuoyantObjectRuntime *buoyantObject);
	void clearCachedBuoyantObjects();
	
	void cacheCurrentLevelIndex(int index);

private:
	UWorld *m_world;
	AProjectLunaGameMode *m_gameMode;
	AWolf *m_wolf;

	int m_levelIndex;

	float initialWaterHeight;

	TArray<ABuoyantObjectRuntime *> m_levelBuoyantObjectRuntimes;

	void cacheLevelActors();
	void checkWolfOverlap();
	void checkBuoyantObjectRuntimesOverlap();

	void onWolfSpawned(OnWolfSpawnedData *eventData);
	void onWolfDestroyed(OnWolfSpawnedData *eventData);
};
