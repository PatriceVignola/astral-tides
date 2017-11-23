#pragma once

#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/Widget.h"
#include "Runtime/UMG/Public/Components/Image.h"
#include "Runtime/UMG/Public/Components/ProgressBar.h"
#include "Runtime/UMG/Public/Components/CanvasPanel.h"
#include "Runtime/UMG/Public/Components/CanvasPanelSlot.h"
#include "Runtime/UMG/Public/Components/ContentWidget.h"
#include "OnWolfDrownData.h"
#include "RespawnWolfData.h"
#include "OnDeathFadeOverData.h"
#include "OnWolfNearIceBlockData.h"
#include "OnWaterLevelChangedData.h"
#include "OnStarCollectedData.h"
#include "OnMatineeStartedData.h"
#include "OnHowledData.h"
#include "PauseMenu.h"
#include "LunaLog.h"
#include "Timewatch.h"
#include "GenericEventData.h"
#include "ProjectLunaHUD.generated.h"

class AProjectLunaGameMode;
class ALunaLevelScriptActor;
class OnLevelLoadedData;

UCLASS()
class PROJECTLUNA_API UProjectLunaHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	enum class LunaHUDState {
		Playing, DeathFadingIn, DeathFadingOut, LoadingLevelFromMenu, ShowCredits
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *BlackOverlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *IceBlockContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *IceBlockFirstContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *LeftTriggerFirst;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *RightTriggerFirst;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *SkipMatinee;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *OverviewPrompt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UImage *> DummyImages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UProgressBar *> ProgressBars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UProgressBar *WaterFillProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCanvasPanel *TideLevelWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UImage *> Level1Stars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UImage *> Level2Stars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UImage *> Level1Bars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UImage *> Level2Bars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *CreditsList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *HoldToSkipCredits;

	UPROPERTY(EditAnywhere)
	float DeathFadeTime;

	//UProjectLunaHUD(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void fadeOutLevelLoadBlackScreen();
	void fadeBlackScreen(float ratio);

private:
	LunaHUDState m_currentState;
	AProjectLunaGameMode *m_gameMode;
	UWorld *m_world;
	ALunaLevelScriptActor *m_levelScript;
	float m_fadeRatio;

	float m_maxDelta;

	float m_overviewCooldown;
	float m_leftTriggerCooldown;
	float m_rightTriggerCooldown;

	void fadeInBlackScreen(float deltaTime);
	void fadeOutBlackScreen(float deltaTime);
	
	bool m_overviewPromptAlreadyShown;

	void showEndGameCredits(GenericEventData *eventData);
	void closeEndGameCredits(GenericEventData *eventData);
	void onWolfDrown(OnWolfDrownData *eventData);
	void onWolfNearIceBlock(OnWolfNearIceBlockData *eventData);
	void onWaterLevelChanged(OnWaterLevelChangedData *eventData);
	void onStarCollected(OnStarCollectedData *eventData);
	void onLevelLoaded(OnLevelLoadedData *eventData);
	void onCinematicStarted(OnMatineeStartedData *eventData);
	void onCinematicEnded(OnMatineeStartedData *eventData);
	void onHowled(OnHowledData *eventData);
	void setTideFillOpacity(UPanelWidget *container, float alpha);

	UImage *m_dummyImage;

	FVector2D m_initialCreditsPos;

	void resetTideFill();
	
	float m_howlCooldown;
	float m_tideFillFadeRatio;

	int m_levelNumber;
	bool m_firstIceBlockContext;
	Timewatch m_leftTriggerTimer;
	Timewatch m_rightTriggerTimer;

	void showOverviewPrompt(GenericEventData *eventData);
	void onOverviewPressed(GenericEventData *eventData);
};