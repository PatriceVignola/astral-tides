#include "ProjectLuna.h"
#include "ProjectLunaHUD.h"
#include "ProjectLunaGameMode.h"
#include "LunaLevelScriptActor.h"
#include "OnLevelLoadedData.h"

void UProjectLunaHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (m_howlCooldown > 1 && m_tideFillFadeRatio < 1)
	{
		m_tideFillFadeRatio += m_world->DeltaTimeSeconds / 1;
	}
	else if (m_howlCooldown <= 1)
	{
		m_tideFillFadeRatio = m_howlCooldown / 1;
	}

	m_howlCooldown = m_howlCooldown > 0 ? m_howlCooldown - m_world->DeltaTimeSeconds : 0;

	float newAlpha = FMath::Lerp(0.f, 1.f, m_tideFillFadeRatio);

	setTideFillOpacity(TideLevelWidget, newAlpha);

	switch (m_currentState)
	{
		case LunaHUDState::DeathFadingIn:
			fadeInBlackScreen(GetWorld()->DeltaTimeSeconds);

			if (m_fadeRatio >= 1)
			{
				m_fadeRatio = 0;
				m_currentState = LunaHUDState::DeathFadingOut;
				m_gameMode->getEventManager().raiseEvent("RespawnWolf", new RespawnWolfData());
			}
			break;

		case LunaHUDState::DeathFadingOut:
			fadeOutBlackScreen(GetWorld()->DeltaTimeSeconds);

			if (m_fadeRatio >= 1)
			{
				m_fadeRatio = 0;
				m_currentState = LunaHUDState::Playing;
				BlackOverlay->SetVisibility(ESlateVisibility::Hidden);
				m_gameMode->getEventManager().raiseEvent("OnDeathFadeOver", new OnDeathFadeOverData());
			}
			break;

		case LunaHUDState::LoadingLevelFromMenu:
			fadeOutBlackScreen(GetWorld()->DeltaTimeSeconds);

			if (m_fadeRatio >= 1)
			{
				m_currentState = LunaHUDState::Playing;
				BlackOverlay->SetVisibility(ESlateVisibility::Hidden);
				m_fadeRatio = 0;

				// TODO: Maybe raise event to notify the player
			}
			break;

		case LunaHUDState::ShowCredits:
			UCanvasPanelSlot *slot = (UCanvasPanelSlot *)CreditsList->GetParent()->Slot;

			FVector2D prevPos = slot->GetPosition();

			if (prevPos.Y > -13250)
			{
				slot->SetPosition(prevPos - FVector2D(0, GetWorld()->DeltaTimeSeconds * 300));
			}

			break;
	}

	if(m_leftTriggerTimer.isRunning() && m_leftTriggerTimer.hasExpired()) {
		m_leftTriggerTimer.stop();
		LeftTriggerFirst->SetVisibility(ESlateVisibility::Hidden);
		m_leftTriggerCooldown = 0;
	}

	if(m_rightTriggerTimer.isRunning() && m_rightTriggerTimer.hasExpired()) {
		m_rightTriggerTimer.stop();
		RightTriggerFirst->SetVisibility(ESlateVisibility::Hidden);
		m_rightTriggerCooldown = 0;
	}

	if (m_overviewCooldown > 0)
	{
		m_overviewCooldown -= GetWorld()->DeltaTimeSeconds;

		if (m_overviewCooldown <= 0)
		{
			OverviewPrompt->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (m_leftTriggerCooldown > 0)
	{
		m_leftTriggerCooldown -= GetWorld()->DeltaTimeSeconds;

		if (m_leftTriggerCooldown <= 0)
		{
			LeftTriggerFirst->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	if (m_rightTriggerCooldown > 0)
	{
		m_rightTriggerCooldown -= GetWorld()->DeltaTimeSeconds;

		if (m_rightTriggerCooldown <= 0)
		{
			RightTriggerFirst->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UProjectLunaHUD::NativeConstruct()
{
	Super::NativeConstruct();

	BlackOverlay->SetVisibility(ESlateVisibility::Hidden);

	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode*)m_world->GetAuthGameMode();

	m_gameMode->getEventManager().subscribe("OnWolfDrown", this, &UProjectLunaHUD::onWolfDrown);
	m_gameMode->getEventManager().subscribe("OnWolfNearIceBlock", this, &UProjectLunaHUD::onWolfNearIceBlock);
	m_gameMode->getEventManager().subscribe("OnWaterLevelChanged", this, &UProjectLunaHUD::onWaterLevelChanged);
	m_gameMode->getEventManager().subscribe("OnStarCollected", this, &UProjectLunaHUD::onStarCollected);
	m_gameMode->getEventManager().subscribe("OnLevelLoaded", this, &UProjectLunaHUD::onLevelLoaded);
	m_gameMode->getEventManager().subscribe("OnCinematicStarted", this, &UProjectLunaHUD::onCinematicStarted);
	m_gameMode->getEventManager().subscribe("OnCinematicEnded", this, &UProjectLunaHUD::onCinematicEnded);
	m_gameMode->getEventManager().subscribe("OnHowled", this, &UProjectLunaHUD::onHowled);
	m_gameMode->getEventManager().subscribe("ShowOverviewPrompt", this, &UProjectLunaHUD::showOverviewPrompt);
	m_gameMode->getEventManager().subscribe("OnOverviewPressed", this, &UProjectLunaHUD::onOverviewPressed);
	m_gameMode->getEventManager().subscribe("ShowEndGameCredits", this, &UProjectLunaHUD::showEndGameCredits);
	m_gameMode->getEventManager().subscribe("CloseEndGameCredits", this, &UProjectLunaHUD::closeEndGameCredits);

	// We cast the level script to our own, custom level script
	if (m_world->GetLevelScriptActor() && m_world->GetLevelScriptActor()->IsA(ALunaLevelScriptActor::StaticClass()))
	{
		m_levelScript = Cast<ALunaLevelScriptActor>(m_world->GetLevelScriptActor());

		// 0 should be when we start the game from the Persistant or MainMenu map
		if (m_levelScript->StartLevelNumber == 0)
		{
			BlackOverlay->SetVisibility(ESlateVisibility::Visible);
			BlackOverlay->SetOpacity(1);
		}
	}
	else
	{
		LUNA_WARN(ProjectLunaGameMode, "The level blueprint needs to inherit from ALunaLevelScriptActor");
	}

	if (OverviewPrompt)
	{
		OverviewPrompt->SetVisibility(ESlateVisibility::Hidden);
	}

	if (HoldToSkipCredits)
	{
		HoldToSkipCredits->SetVisibility(ESlateVisibility::Hidden);
	}

	resetTideFill();

	m_initialCreditsPos = ((UCanvasPanelSlot *)CreditsList->GetParent()->Slot)->GetPosition();
}

void UProjectLunaHUD::showEndGameCredits(GenericEventData *eventData)
{
	CreditsList->SetVisibility(ESlateVisibility::Visible);
	HoldToSkipCredits->SetVisibility(ESlateVisibility::Visible);
	((UCanvasPanelSlot *)CreditsList->GetParent()->Slot)->SetPosition(m_initialCreditsPos);

	m_currentState = LunaHUDState::ShowCredits;

	UE_LOG(LogTemp, Warning, TEXT("show end credits event caught!!!!!!"));
}

void UProjectLunaHUD::closeEndGameCredits(GenericEventData *eventData)
{
	CreditsList->SetVisibility(ESlateVisibility::Hidden);
	HoldToSkipCredits->SetVisibility(ESlateVisibility::Hidden);
	m_currentState = LunaHUDState::Playing;
}

void UProjectLunaHUD::resetTideFill()
{
	if (!TideLevelWidget) return;

	// Ugly ugly ugly, fuck UMG

	TArray<UImage *> stars;
	TArray<UImage *> bars;
	TArray<UImage *> otherStars;
	TArray<UImage *> otherBars;

	if (m_levelNumber == 1)
	{
		m_firstIceBlockContext = true;
		stars = Level1Stars;
		bars = Level1Bars;

		otherStars = Level2Stars;
		otherBars = Level2Bars;
	}
	else if (m_levelNumber == 2)
	{
		m_firstIceBlockContext = false;
		stars = Level2Stars;
		bars = Level2Bars;

		otherStars = Level1Stars;
		otherBars = Level1Bars;
	}


	for (UImage *star : stars)
	{
		star->SetColorAndOpacity(FLinearColor(1, 1, 1, 1));
		star->SetVisibility(ESlateVisibility::Visible);
	}

	for (UImage *bar : bars)
	{
		bar->SetColorAndOpacity(FLinearColor(0, 0.051, 0.345, 1));
		bar->SetVisibility(ESlateVisibility::Visible);
	}

	for (UImage *star : otherStars)
	{
		star->SetVisibility(ESlateVisibility::Hidden);
	}

	for (UImage *bar : otherBars)
	{
		bar->SetVisibility(ESlateVisibility::Hidden);
	}

	ProgressBars[0]->SetVisibility(ESlateVisibility::Visible);
	DummyImages[0]->SetVisibility(ESlateVisibility::Visible);

	// We reset the ice
	for (int i = 1; i < ProgressBars.Num(); i++)
	{
		ProgressBars[i]->SetVisibility(ESlateVisibility::Hidden);
		DummyImages[i]->SetVisibility(ESlateVisibility::Hidden);
	}

	WaterFillProgress = ProgressBars[0];
	m_dummyImage = DummyImages[0];
}

void UProjectLunaHUD::onWolfDrown(OnWolfDrownData *eventData)
{
	BlackOverlay->SetVisibility(ESlateVisibility::Visible);
	m_currentState = LunaHUDState::DeathFadingIn;
}
void UProjectLunaHUD::fadeOutLevelLoadBlackScreen()
{
	m_currentState = LunaHUDState::LoadingLevelFromMenu;
	BlackOverlay->SetVisibility(ESlateVisibility::Visible);
	BlackOverlay->SetOpacity(1);

	m_fadeRatio = 0;
}

void UProjectLunaHUD::fadeInBlackScreen(float deltaTime)
{
	m_fadeRatio += deltaTime / (DeathFadeTime / 2);

	BlackOverlay->SetOpacity(FMath::Lerp(0.f, 1.f, m_fadeRatio));
}

void UProjectLunaHUD::fadeOutBlackScreen(float deltaTime)
{
	m_fadeRatio += deltaTime / (DeathFadeTime / 2);

	BlackOverlay->SetOpacity(FMath::Lerp(1.f, 0.f, m_fadeRatio));
}

void UProjectLunaHUD::onWolfNearIceBlock(OnWolfNearIceBlockData *eventData)
{
	if(m_firstIceBlockContext) {
		if(eventData->getIsEntering()) {
			IceBlockFirstContext->SetVisibility(ESlateVisibility::Visible);
		} else {
			IceBlockFirstContext->SetVisibility(ESlateVisibility::Hidden);
			m_firstIceBlockContext = false;
		}
	} else {
		if(eventData->getIsEntering()) {
			IceBlockContext->SetVisibility(ESlateVisibility::Visible);
		} else {
			IceBlockContext->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	
}

void UProjectLunaHUD::onWaterLevelChanged(OnWaterLevelChangedData *eventData)
{
	if (m_maxDelta != eventData->getCurrentMaxDelta())
	{
		if (!m_dummyImage)
		{
			m_dummyImage = DummyImages[0];
		}

		WaterFillProgress->SetVisibility(ESlateVisibility::Hidden);
		m_dummyImage->SetVisibility(ESlateVisibility::Hidden);

		m_maxDelta = eventData->getCurrentMaxDelta();
		int unlockedIndex = m_maxDelta / m_levelScript->WaterHeightPerStep;

		WaterFillProgress = ProgressBars[unlockedIndex];
		m_dummyImage = DummyImages[unlockedIndex];

		WaterFillProgress->SetVisibility(ESlateVisibility::Visible);
		m_dummyImage->SetVisibility(ESlateVisibility::Visible);
	}

	float ratio = (eventData->getNewWaterDelta()) / eventData->getAbsoluteMaxDelta();

	// Adjustement
	ratio = ratio * (0.898 - 0.12) + 0.12;

	WaterFillProgress->SetPercent(ratio);
}

void UProjectLunaHUD::onStarCollected(OnStarCollectedData *eventData)
{
	if (!TideLevelWidget) return;

	// Ugly ugly ugly, fuck UMG

	TArray<UImage *> stars;
	TArray<UImage *> bars;

	if (m_levelNumber == 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("LEVEL 1!!!"));

		stars = Level1Stars;
		bars = Level1Bars;
		if(eventData->getStarIndex() == 0) {
			LeftTriggerFirst->SetVisibility(ESlateVisibility::Visible);
			RightTriggerFirst->SetVisibility(ESlateVisibility::Visible);
			m_leftTriggerCooldown = 10;
			m_rightTriggerCooldown = 10;
		}
	}
	else if (m_levelNumber == 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("LEVEL 2!!!"));

		stars = Level2Stars;
		bars = Level2Bars;

	}

	UE_LOG(LogTemp, Warning, TEXT("stars: %d"), stars.Num());
	UE_LOG(LogTemp, Warning, TEXT("bars: %d"), bars.Num());
	UE_LOG(LogTemp, Warning, TEXT("eventData->getStarIndex(): %d"), eventData->getStarIndex());

	if (eventData->getStarIndex() >= stars.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("FAULTY INDEX!!!: %d"), eventData->getStarIndex());
	}
	else
	{
		stars[eventData->getStarIndex()]->SetColorAndOpacity(FLinearColor(1, 0.413, 0, 1));
	}
	

	if (eventData->getStarIndex() > 0)
	{
		bars[eventData->getStarIndex() - 1]->SetColorAndOpacity(FLinearColor(1, 0.413, 0, 1));
	}

	m_howlCooldown = 5;
}

void UProjectLunaHUD::onLevelLoaded(OnLevelLoadedData *eventData)
{
	OverviewPrompt->SetVisibility(ESlateVisibility::Hidden);

	if (eventData->getLevelNumber() == 0)
	{
		TideLevelWidget->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		if (eventData->getLevelNumber() == 1)
		{
			m_overviewPromptAlreadyShown = false;
		}

		TideLevelWidget->SetVisibility(ESlateVisibility::Visible);

		m_levelNumber = eventData->getLevelNumber();
		resetTideFill();
	}

	SkipMatinee->SetVisibility(ESlateVisibility::Hidden);
}

void UProjectLunaHUD::onCinematicStarted(OnMatineeStartedData *eventData)
{
	TideLevelWidget->SetVisibility(ESlateVisibility::Hidden);

	if (eventData->getIsSkippable())
	{
		SkipMatinee->SetVisibility(ESlateVisibility::Visible);
	}
}

void UProjectLunaHUD::onCinematicEnded(OnMatineeStartedData *eventData)
{
	TideLevelWidget->SetVisibility(ESlateVisibility::Visible);
	
	SkipMatinee->SetVisibility(ESlateVisibility::Hidden);
}

void UProjectLunaHUD::onHowled(OnHowledData *eventData) {
	m_howlCooldown = 5;

	float value = eventData->getAxisValue();

	if(value < 0.0001f) {
		//LeftTriggerFirst->SetVisibility(ESlateVisibility::Hidden);
		m_leftTriggerTimer.start(0.1f);
	} else if(value > 0.0001f) {
		//RightTriggerFirst->SetVisibility(ESlateVisibility::Hidden);
		m_rightTriggerTimer.start(0.1f);
	}
}

void UProjectLunaHUD::setTideFillOpacity(UPanelWidget *container, float alpha)
{
	for (UProgressBar *pb : ProgressBars)
	{
		pb->SetFillColorAndOpacity(FLinearColor(0.207, 0.205, 1, alpha));
	}

	for (UImage *dummy : DummyImages)
	{
		dummy->SetOpacity(alpha);
	}

	int childCount = container->GetChildrenCount();

	for (int i = 0; i < childCount; i++)
	{
		UWidget *child = container->GetChildAt(i);

		if (child)
		{
			if ((UImage *)child)
			{
				UImage *image = (UImage *)child;
				image->SetOpacity(alpha);
			}
			else if ((UPanelWidget *)child)
			{
				UPanelWidget *parent = (UPanelWidget *)child;
				setTideFillOpacity(parent, alpha);
			}
		}
	}
}

void UProjectLunaHUD::fadeBlackScreen(float ratio)
{
	if (m_currentState == LunaHUDState::LoadingLevelFromMenu)
	{
		m_currentState = LunaHUDState::Playing;
	}

	m_fadeRatio = ratio;

	BlackOverlay->SetVisibility(ESlateVisibility::Visible);

	BlackOverlay->SetOpacity(FMath::Lerp(0.f, 1.f, ratio));

	if (ratio == 0)
	{
		BlackOverlay->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UProjectLunaHUD::showOverviewPrompt(GenericEventData *eventData)
{
	UE_LOG(LogTemp, Warning, TEXT("SHOWING OVERVIEW PROMPT IN HUD"));

	if (OverviewPrompt && !m_overviewPromptAlreadyShown)
	{
		OverviewPrompt->SetVisibility(ESlateVisibility::Visible);
		m_overviewCooldown = 10;
	}
}

void UProjectLunaHUD::onOverviewPressed(GenericEventData *eventData)
{
	if (OverviewPrompt)
	{
		OverviewPrompt->SetVisibility(ESlateVisibility::Hidden);
		m_overviewPromptAlreadyShown = true;
		m_overviewCooldown = 0;
	}
}