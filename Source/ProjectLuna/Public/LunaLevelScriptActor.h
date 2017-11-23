// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "TideManager.h"
#include "LunaLog.h"
#include "ProjectLunaGameMode.h"
#include "OnStarCollectedData.h"
#include "StarPiece.h"
//#include "OrquaController.h"
#include "OnMatineeStartedData.h"
#include "OnLevelLoadedData.h"
#include "GenericEventData.h"
#include "Runtime/Engine/Classes/Camera/CameraActor.h"
#include "SetActiveCameraData.h"
#include "LunaLevelScriptActor.generated.h"

DECLARE_LUNA_LOG(LunaLevelScriptActor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSkipMatineeDelegate);

USTRUCT()
struct FLevelInfo
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere)
		TArray<int32> StarSteps;
};

UCLASS()
class PROJECTLUNA_API ALunaLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	int32 StartLevelNumber = 0;

	// In meters
	UPROPERTY(EditAnywhere)
	float WaterHeightPerStep = 100;

	UPROPERTY(EditAnywhere)
	float WaterControlSpeedMin = 2;

	UPROPERTY(EditAnywhere)
	float WaterControlSpeedMax = 6;

	UPROPERTY(EditAnywhere)
	float WaterSpeedMultiplier = 3;

	UPROPERTY(EditAnywhere)
	TArray<FLevelInfo> LevelStarSteps;

	/*UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AOrquaController> AOrquaControllerClass;*/

	UFUNCTION(BlueprintCallable, Category = "Matinee")
	void onMatineeStarted(bool isSkippable);

	UFUNCTION(BlueprintCallable, Category = "EndGame")
	void showEndGameCredits();

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void setCurrentCamera(ACameraActor *camera);

	UFUNCTION(BlueprintCallable, Category = "Matinee")
	void onMatineeEnded();

	UFUNCTION(BlueprintCallable, Category = "LevelStreaming")
	void onLevelLoaded(int32 levelNumber);

	UFUNCTION(BlueprintCallable, Category = "Overview")
	void showOverviewPrompt();

	// This event has to be listened in the blueprint
	UPROPERTY(BlueprintAssignable)
	FSkipMatineeDelegate OnMatineeSkip;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
		UMaterial* StreamSurface;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
		UMaterial* StreamReversed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Materials)
		UMaterial* StreamAir;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void BeginDestroy() override;

	virtual void Tick(float deltaTime) override;

	void incrementStarsCount();
	const int getCountStarsCollected() const;

	float getMaxWaterDelta();
	float getCurrentWaterDelta();

	void skipMatinee();
	

	ALunaLevelScriptActor();

private:
	UWorld *m_world;
	AProjectLunaGameMode *m_gameMode;
	TArray<AStarPiece *> m_starPieceArray;
	//AOrquaController* m_orquaController;
	//AOrqua* m_orqua;

	bool m_levelhasBeenLoaded;
	bool m_mustRaiseCinematicEvent;
	bool m_skippableCinematic;

	float m_maxWaterDelta;

	int m_collectedStarsCount;
	int m_totalStarsCount;

	int m_levelIndex;

	void onLevelLoadedFromCode(OnLevelLoadedData *eventData);
};
