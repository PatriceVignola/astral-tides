// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"
#include "EventManager.h"
#include "ProjectLunaHUD.h"
#include "LunaLog.h"
#include "OnCheckpointReachedData.h"
#include "SoundManager.h"
#include "ProjectLunaGameMode.generated.h"

class OnLevelLoadedData;
class ALunaLevelScriptActor;
class UProjectLunaHUD;
class ATideManager;
class AOrquaController;

DECLARE_LUNA_LOG(ProjectLunaGameMode);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FLevelStreamLoadDelegate, int32, levelNumber, int32, loadOrigin, const TArray<FString>&, loadList, const TArray<FString>&, unloadList);

USTRUCT()
struct FLevelStreamInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FString> LevelsToLoad;

	UPROPERTY(EditAnywhere)
	TArray<FString> LevelsToUnload;
};

UCLASS()
class PROJECTLUNA_API AProjectLunaGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	enum class GameModeState { Normal, GoingToMainMenuFromPause };

	enum class LevelLoadOrigin { Player, Automatic };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class UProjectLunaHUD> MainHUDClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<class ASoundManager> SoundManagerClass;

	// This is the list of levels to load (including intersections) when loading a particular level
	UPROPERTY(EditAnywhere)
	TArray<FLevelStreamInfo> LevelStreamInfoList;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class AOrquaController> AOrquaControllerClass;


	virtual void PreInitializeComponents() override;
	EventManager& getEventManager();

	// This event has to be listened in the blueprint
	UPROPERTY(BlueprintAssignable)
	FLevelStreamLoadDelegate OnLevelStreamLoad;

	virtual void BeginPlay();
	virtual void Tick(float deltaTime) override;

	float getLevelNumber();
	ASoundManager *getSoundManager();

	void loadLevel(int levelNumber, LevelLoadOrigin loadOrigin);

	UFUNCTION(BlueprintCallable, Category="LevelStreaming")
	void onLevelLoaded(int32 levelNumber, int32 loadOrigin);
	
	void fadeBlackScreen(float ratio);

	void setState(GameModeState newState);

private:
	UProjectLunaHUD* m_mainHUD;

	// Static variables are a really bad idea with Unreal, so we should avoid singletons and put all managers here
	EventManager m_eventManager;
	UWorld *m_world;
	ALunaLevelScriptActor *m_levelScript;
	ATideManager *m_tideManager;
	ASoundManager *m_soundManager;
	AOrquaController* m_orquaController;

	GameModeState m_currentState;

	int m_levelNumber;

	float m_fadeRatio;

	void initDefaultCheckpoint();
};
