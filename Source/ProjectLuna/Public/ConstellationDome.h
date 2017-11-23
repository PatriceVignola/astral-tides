// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "StarLink.h"
#include "StarComponent.h"
#include "LunaLog.h"
#include "ProjectLunaGameMode.h"
#include "OnStarCollectedData.h"
#include "OnCheckpointReachedData.h"
#include "OnWolfRespawnedData.h"
#include "OnLevelLoadedData.h"
#include "OnMenuLevelSelectedData.h"
#include "ConstellationDome.generated.h"

DECLARE_LUNA_LOG(ConstellationDome);

UCLASS()
class PROJECTLUNA_API AConstellationDome : public AActor
{
	GENERATED_BODY()
	
public:
	// TODO: This is only for prototype
	// TODO: Eventually create classes for constellation containers
	// TODO: Eventually create a more dynamic way of adding constellation containers directly from the editor
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent *InvisibleDome;

	UPROPERTY(VisibleAnywhere)
	USceneComponent *LupusWolfConstellation;

	UPROPERTY(VisibleAnywhere)
	USceneComponent *MenuCassiopeiaConstellation;

	UPROPERTY(VisibleAnywhere)
	USceneComponent *MenuUrsaMajorConstellation;

	UPROPERTY(VisibleAnywhere)
	USceneComponent *MenuLeoConstellation;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AStarLink> StarLinkGreyClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AStarLink> StarLinkWhiteClass;

	AConstellationDome();
	virtual void BeginPlay() override;

private:
	UWorld *m_world;
	AProjectLunaGameMode *m_gameMode;

	// TODO: Move to custom constellation script
	TMap<FString, UStarComponent*> m_stars;
	TMap<FString, bool> m_checkpointStates;

	TMap<FString, UStarComponent*> m_menuCassiopeiaStars;
	TMap<FString, UStarComponent*> m_menuUrsaMajorStars;
	TMap<FString, UStarComponent*> m_menuLeoStars;

	void cacheStars();

	void cacheCassiopeiaStars();
	void cacheUrsaMajorStars();
	void cacheLeoStars();

	void initStars();
	void onStarCollected(OnStarCollectedData *eventData);
	void onCheckpointReached(OnCheckpointReachedData *eventData);
	void onWolfRespawned(OnWolfDrownData *eventData);
	void onLevelLoaded(OnLevelLoadedData *eventData);
	//void onMenuLevelSelected(OnMenuLevelSelectedData *eventData);
};
