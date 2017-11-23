// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "PauseMenu.h"
#include "ProjectLunaGameMode.h"


void UPauseMenuHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (m_cooldownTime > 0)
	{
		m_cooldownTime -= InDeltaTime;
	}
}

void UPauseMenuHUD::NativeConstruct()
{
	Super::NativeConstruct();

	m_gameMode = (AProjectLunaGameMode*)GetWorld()->GetAuthGameMode();

	m_gameMode->getEventManager().subscribe("OnGamePaused", this, &UPauseMenuHUD::onGamePaused);
	m_gameMode->getEventManager().subscribe("OnGameUnpaused", this, &UPauseMenuHUD::onGameUnpaused);
	m_gameMode->getEventManager().subscribe("SelectNextPauseMenuOption", this, &UPauseMenuHUD::selectNextOption);
	m_gameMode->getEventManager().subscribe("SelectPreviousPauseMenuOption", this, &UPauseMenuHUD::selectPreviousOption);
	m_gameMode->getEventManager().subscribe("AcceptPauseMenuOption", this, &UPauseMenuHUD::acceptOption);
}

void UPauseMenuHUD::onGamePaused(GenericEventData *eventData)
{
	SetVisibility(ESlateVisibility::Visible);

	Arrows[0]->SetVisibility(ESlateVisibility::Visible);
	m_selectedOption = (Options)0;
	m_cooldownTime = 0;
}

void UPauseMenuHUD::onGameUnpaused(GenericEventData *eventData)
{
	SetVisibility(ESlateVisibility::Hidden);

	for (UImage *arrow : Arrows)
	{
		arrow->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPauseMenuHUD::selectNextOption(GenericEventData *eventData)
{
	if (m_cooldownTime > 0 || Arrows.Num() == 0) return;

	int prevIndex = (int)m_selectedOption;

	int selectedIndex = prevIndex == Arrows.Num() - 1 ? 0 : prevIndex + 1;
	m_selectedOption = (Options)selectedIndex;

	Arrows[prevIndex]->SetVisibility(ESlateVisibility::Hidden);
	Arrows[selectedIndex]->SetVisibility(ESlateVisibility::Visible);

	m_cooldownTime = MAX_COOLDOWN_TIME;
}

void UPauseMenuHUD::selectPreviousOption(GenericEventData *eventData)
{
	if (m_cooldownTime > 0 || Arrows.Num() == 0) return;

	int prevIndex = (int)m_selectedOption;

	int selectedIndex = prevIndex == 0 ? Arrows.Num() - 1 : prevIndex - 1;
	m_selectedOption = (Options)selectedIndex;

	Arrows[prevIndex]->SetVisibility(ESlateVisibility::Hidden);
	Arrows[selectedIndex]->SetVisibility(ESlateVisibility::Visible);

	m_cooldownTime = MAX_COOLDOWN_TIME;
}

void UPauseMenuHUD::acceptOption(GenericEventData *eventData)
{
	switch (m_selectedOption)
	{
	case Options::Resume:
		m_gameMode->getEventManager().raiseEvent("UnpauseGame", new GenericEventData());
		break;

	case Options::MainMenu:
		m_gameMode->getEventManager().raiseEvent("UnpauseGame", new GenericEventData());
		m_gameMode->getEventManager().raiseEvent("GoingToMainMenuFromPause", new GenericEventData());

		m_gameMode->setState(AProjectLunaGameMode::GameModeState::GoingToMainMenuFromPause);
		break;
	}
}