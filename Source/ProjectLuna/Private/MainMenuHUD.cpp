#include "ProjectLuna.h"
#include "OnLevelLoadedData.h"
#include "MainMenuHUD.h"
#include "ProjectLunaGameMode.h"

void UMainMenuHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UMainMenuHUD::NativeConstruct()
{
	Super::NativeConstruct();

	BlackOverlay->SetOpacity(0);
	BlackOverlay->SetVisibility(ESlateVisibility::Hidden);

	NewGameTitle->SetOpacity(0);
	NewGameTitle->SetVisibility(ESlateVisibility::Hidden);

	/*ContinueTitle->SetOpacity(0);
	ContinueTitle->SetVisibility(ESlateVisibility::Hidden);*/

	LevelSelectTitle->SetOpacity(0);
	LevelSelectTitle->SetVisibility(ESlateVisibility::Hidden);

	/*OptionsTitle->SetOpacity(0);
	OptionsTitle->SetVisibility(ESlateVisibility::Hidden);*/

	ExitTitle->SetOpacity(0);
	ExitTitle->SetVisibility(ESlateVisibility::Hidden);

	CreditsTitle->SetOpacity(0);
	CreditsTitle->SetVisibility(ESlateVisibility::Hidden);

	LeftArrow->SetVisibility(ESlateVisibility::Hidden);
	RightArrow->SetVisibility(ESlateVisibility::Hidden);
	BackButton->SetVisibility(ESlateVisibility::Hidden);
	CreditsList->SetVisibility(ESlateVisibility::Hidden);
	Level1->SetVisibility(ESlateVisibility::Hidden);
	Level2->SetVisibility(ESlateVisibility::Hidden);
	Level3->SetVisibility(ESlateVisibility::Hidden);

	m_gameMode = (AProjectLunaGameMode *)GetWorld()->GetAuthGameMode();

	m_gameMode->getEventManager().subscribe("OnLevelLoaded", this, &UMainMenuHUD::onLevelLoaded);
}

void UMainMenuHUD::onLevelLoaded(OnLevelLoadedData *eventData)
{
	if (eventData->getLevelNumber() != 0)
	{
		BlackOverlay->SetVisibility(ESlateVisibility::Hidden);
		LeftArrow->SetVisibility(ESlateVisibility::Hidden);
		RightArrow->SetVisibility(ESlateVisibility::Hidden);
		BackButton->SetVisibility(ESlateVisibility::Hidden);
	}
}