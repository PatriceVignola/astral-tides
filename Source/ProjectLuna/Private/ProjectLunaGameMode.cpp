#include "ProjectLuna.h"
#include "LunaLevelScriptActor.h"
#include "OnLevelLoadedData.h"
#include "ProjectLunaHUD.h"
#include "TideManager.h"
#include "OrquaController.h"

DEFINE_LUNA_LOG(ProjectLunaGameMode);

void AProjectLunaGameMode::PreInitializeComponents()
{
	GetWorldSettings()->GameNetworkManagerClass = nullptr;

	Super::PreInitializeComponents();
}


EventManager& AProjectLunaGameMode::getEventManager()
{
	return m_eventManager;
}

void AProjectLunaGameMode::BeginPlay()
{
	Super::BeginPlay();

	LUNA_WARN(ProjectLunaGameMode, "ProjectLunaGameMode started playing");

	m_world = GetWorld();

	m_tideManager = m_world->SpawnActor<ATideManager>(ATideManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	
	if (SoundManagerClass)
	{
		m_soundManager = m_world->SpawnActor<ASoundManager>(SoundManagerClass, FVector::ZeroVector, FRotator::ZeroRotator);
		m_soundManager->playMainMusic();
	}
	else
	{
		LUNA_WARN(ProjectLunaGameMode, "ProjectLunaGameMode doesnt have a SoundManagerClass assigned");
	}

	if (MainHUDClass)
	{
		if (!m_mainHUD)
		{
			m_mainHUD = CreateWidget<UProjectLunaHUD>(GetGameInstance(), MainHUDClass);
			m_mainHUD->AddToViewport();
			m_mainHUD->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		LUNA_WARN(ProjectLunaGameMode, "ProjectLunaGameMode doesnt have a MainHUDClass assigned");
	}

	if(AOrquaControllerClass) {
		m_orquaController = m_world->SpawnActor<AOrquaController>(AOrquaControllerClass, FVector(), FRotator::ZeroRotator);
	} else {
		LUNA_WARN(ProjectLunaGameMode, "An OrquaController has to be assigned to the level blueprint");
	}

	// We cast the level script to our own, custom level script
	if (m_world->GetLevelScriptActor() && m_world->GetLevelScriptActor()->IsA(ALunaLevelScriptActor::StaticClass()))
	{
		m_levelScript = Cast<ALunaLevelScriptActor>(m_world->GetLevelScriptActor());

		loadLevel(m_levelScript->StartLevelNumber, LevelLoadOrigin::Player);
	}
	else
	{
		LUNA_WARN(ProjectLunaGameMode, "The level blueprint needs to inherit from ALunaLevelScriptActor");
	}
}

void AProjectLunaGameMode::Tick(float deltaTime)
{
	if (m_currentState == GameModeState::GoingToMainMenuFromPause)
	{
		UE_LOG(LogTemp, Warning, TEXT("Options::GoingToMainMenuFromPause"));

		m_fadeRatio += GetWorld()->DeltaTimeSeconds / 1.5;

		fadeBlackScreen(m_fadeRatio);

		if (m_fadeRatio >= 1)
		{
			m_fadeRatio = 0;

			setState(GameModeState::Normal);

			// In case a cinematic is playing
			//m_levelScript->skipMatinee();

			LUNA_WARN(ProjectLunaGameMode, "SKIPPPP!!!!");

			loadLevel(0, AProjectLunaGameMode::LevelLoadOrigin::Automatic);
		}
	}
}

float AProjectLunaGameMode::getLevelNumber()
{
	return m_levelNumber;
}

// TODO: Do a proper LevelManager
void AProjectLunaGameMode::loadLevel(int levelNumber, LevelLoadOrigin loadOrigin)
{
	// This event will be listened in the blueprint
	if (OnLevelStreamLoad.IsBound())
	{
		FLevelStreamInfo levelsInfo = LevelStreamInfoList[levelNumber];

		// This is to alert the blueprint listeners
		OnLevelStreamLoad.Broadcast(levelNumber, (int32)loadOrigin, levelsInfo.LevelsToLoad, levelsInfo.LevelsToUnload);
	
		getEventManager().raiseEvent("OnLevelUnloaded", new OnLevelLoadedData(levelNumber, loadOrigin));
	}
	else
	{
		LUNA_WARN(ProjectLunaGameMode, "The ProjectLunaGameMode blueprint needs to be bound to the OnNewLevelSelected event");
	}
}

void AProjectLunaGameMode::onLevelLoaded(int32 levelNumber, int32 loadOrigin)
{
	if ((LevelLoadOrigin)loadOrigin == LevelLoadOrigin::Player)
	{
		m_mainHUD->fadeOutLevelLoadBlackScreen();
	}


	// Ugly hotfix because we learned at the last minute that level streaming doesn't work well with level scripts (there is only 1 level script for all the levels)
	m_levelNumber = levelNumber;

	//LUNA_WARN(ProjectLunaGameMode, "LEVEL LOADED!!!!!!!!!!!!!!!: %f", levelNumber);

	if (levelNumber != 0)
	{
		initDefaultCheckpoint();
	}
	else
	{
		fadeBlackScreen(0);
	}

	m_mainHUD->LeftTriggerFirst->SetVisibility(ESlateVisibility::Hidden);
	m_mainHUD->RightTriggerFirst->SetVisibility(ESlateVisibility::Hidden);
	m_mainHUD->SkipMatinee->SetVisibility(ESlateVisibility::Hidden);
	m_mainHUD->OverviewPrompt->SetVisibility(ESlateVisibility::Hidden);

	getEventManager().raiseEvent("OnLevelLoaded", new OnLevelLoadedData(m_levelNumber, (LevelLoadOrigin)loadOrigin));
}

void AProjectLunaGameMode::initDefaultCheckpoint()
{
	TSubclassOf<APlayerStart> classToFind;
	TArray<AActor*> playerStarts;
	UGameplayStatics::GetAllActorsOfClass(m_world, classToFind, playerStarts);

	if (playerStarts.Num() > 0)
	{
		getEventManager().raiseEvent("OnCheckpointReached", new OnCheckpointReachedData(playerStarts[0]->GetActorLocation(), playerStarts[0]->GetActorRotation()));
	}
}

ASoundManager *AProjectLunaGameMode::getSoundManager()
{
	return m_soundManager;
}

void AProjectLunaGameMode::fadeBlackScreen(float ratio)
{
	m_mainHUD->fadeBlackScreen(ratio);
}

void AProjectLunaGameMode::setState(GameModeState newState)
{
	m_currentState = newState;
}