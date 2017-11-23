#include "ProjectLuna.h"
#include "MainMenuCamera.h"

DEFINE_LUNA_LOG(MainMenuCamera);

// TODO: This whole class has to be refactored

AMainMenuCamera::AMainMenuCamera()
{
 	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->AttachTo(RootComponent);
}

void AMainMenuCamera::BeginPlay()
{
	Super::BeginPlay();

	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();

	if (m_camLocation == FVector::ZeroVector)
	{
		m_camLocation = GetActorLocation();
	}
	else
	{
		SetActorLocation(m_camLocation);
	}

	initMainMenuHUD();
	setInitialRotation();

	m_currentState = MainMenuCameraState::StartingMainMenu;
	m_currentOption = 0;
	m_menuHUD->BlackOverlay->SetVisibility(ESlateVisibility::Visible);
	m_menuHUD->BlackOverlay->SetOpacity(1);

	m_initialCreditsPos = ((UCanvasPanelSlot *)m_menuHUD->CreditsList->GetParent()->Slot)->GetPosition();
}

void AMainMenuCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void AMainMenuCamera::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	switch (m_currentState)
	{
	case MainMenuCameraState::StartingMainMenu:
		fadeOutBlackScreen(deltaTime);
		break;
	case MainMenuCameraState::Lerping:
		lerpToTargetLocRot(deltaTime);
		break;
	case MainMenuCameraState::FadingBlackScreenIn:
		fadeInBlackScreen(deltaTime);
		break;

	case MainMenuCameraState::ShowCredits:
		UCanvasPanelSlot *slot = (UCanvasPanelSlot *)m_menuHUD->CreditsList->GetParent()->Slot;

		FVector2D prevPos = slot->GetPosition();

		if (prevPos.Y > -13250)
		{
			slot->SetPosition(prevPos - FVector2D(0, deltaTime * 300 * (1 + m_creditsAcceleration)));
		}

		break;
	}
}

void AMainMenuCamera::initMainMenuHUD()
{
	if (MenuHUDCLass) // Check the selected UI class is not NULL
	{
		if (!m_menuHUD) // If the widget is not created and == NULL
		{
			m_menuHUD = CreateWidget<UMainMenuHUD>(GetGameInstance(), MenuHUDCLass); // Create Widget

			if (!m_menuHUD) return;

			m_menuHUD->AddToViewport(); // Add it to the viewport so the Construct() method in the UUserWidget:: is run.
			m_menuHUD->SetVisibility(ESlateVisibility::Visible); // Set it to hidden so its not open on spawn.
		}
	}
}

void AMainMenuCamera::setInitialRotation()
{
	SetActorRotation(Rotations[(int)OptionsEnum::NewGame]);

	currentTitle = m_menuHUD->NewGameTitle;
	currentTitle->SetVisibility(ESlateVisibility::Visible);
	currentTitle->SetOpacity(1);
}

void AMainMenuCamera::acceptMainMenuOption()
{
	if (m_currentState == MainMenuCameraState::SelectingLevel)
	{
		// Level 3 is only for showoff right now
		if (m_currentSelectedLevelIndex != 2)
		{
			UImage *prevLevelTitle = m_currentSelectedLevelIndex == 0 ? m_menuHUD->Level1 :
				(m_currentSelectedLevelIndex == 1 ? m_menuHUD->Level2 : m_menuHUD->Level3);

			// Lazy hacks, lazy hacks everywhere
			startLerp(1.5,
				GetActorLocation(),
				GetActorRotation(),
				MainMenuCameraState::LoadLevel,
				true,
				prevLevelTitle);

			//UGameplayStatics::SpawnSoundAttached(StartGameSound, RootComponent);

			if(OnStartGame.IsBound()) {
				OnStartGame.Broadcast();
			}
		}
	}
	else if (m_currentState == MainMenuCameraState::WaitingForInput)
	{
		OptionsEnum enumOption = (OptionsEnum)m_currentOption;

		switch (enumOption)
		{
		case OptionsEnum::NewGame:
			//UGameplayStatics::SpawnSoundAttached(StartGameSound, RootComponent);

			if(OnStartGame.IsBound()) {
				OnStartGame.Broadcast();
			}

			startLerp(NewGameLerpDuration,
				StartGameCameraFinalLocation,
				GetActorRotation(),
				MainMenuCameraState::StartNewGame,
				true,
				m_menuHUD->NewGameTitle);
			break;

		case OptionsEnum::LevelSelect:
			m_currentSelectedLevelIndex = 0;
			startLerp(1.5,
				LevelSelectCameraIntermediateLocation,
				LevelSelectCameraIntermediateRotation,
				MainMenuCameraState::SelectingLevel,
				false,
				m_menuHUD->LevelSelectTitle,
				m_menuHUD->Level1);
			break;
		case OptionsEnum::Credits:
			startLerp(1.5,
				CreditsCameraLocation,
				CreditsCameraRotation,
				MainMenuCameraState::ShowCredits,
				false,
				m_menuHUD->CreditsTitle);
			break;
		case OptionsEnum::Exit:
			startLerp(3,
				ExitCameraLocation,
				ExitCameraRotation,
				MainMenuCameraState::Exit,
				true,
				m_menuHUD->ExitTitle);
			break;
		}

		if (enumOption != OptionsEnum::NewGame)
		{
			if(OnConfirmOption.IsBound()) {
				UE_LOG(LogTemp, Warning, TEXT("Confirm option"));
				OnConfirmOption.Broadcast();
			}
		}
	}
}

void AMainMenuCamera::goToPreviousMainMenu()
{
	if (m_currentState == MainMenuCameraState::WaitingForInput) return;

	// Hack
	((UCanvasPanelSlot *)m_menuHUD->CreditsList->GetParent()->Slot)->SetPosition(m_initialCreditsPos);

	UImage *levelTitle;

	switch (m_currentState)
	{
	case MainMenuCameraState::SelectingLevel:
		levelTitle = m_currentSelectedLevelIndex == 0 ? m_menuHUD->Level1 :
			(m_currentSelectedLevelIndex == 1 ? m_menuHUD->Level2 : m_menuHUD->Level3);

		startLerp(1.5,
			m_camLocation,
			Rotations[m_currentOption],
			MainMenuCameraState::WaitingForInput,
			false,
			levelTitle,
			m_menuHUD->LevelSelectTitle);
		break;

	case MainMenuCameraState::ShowCredits:
		startLerp(1.5,
			m_camLocation,
			Rotations[m_currentOption],
			MainMenuCameraState::WaitingForInput,
			false,
			nullptr,
			m_menuHUD->CreditsTitle);

		m_menuHUD->CreditsList->SetVisibility(ESlateVisibility::Hidden);
	}

	if(OnBackOption.IsBound()) {
		OnBackOption.Broadcast();
	}
}

void AMainMenuCamera::startLerp(float lerpTotalDuration, FVector lerpEndLoc, FRotator lerpEndRot, MainMenuCameraState lerpNextState, bool fadeIn, UImage *prevTitle, UImage *newTitle)
{
	m_lerpStartLoc = GetActorLocation();
	m_lerpStartRot = GetActorQuat();
	m_lerpEndLoc = lerpEndLoc;
	m_lerpEndRot = lerpEndRot;
	m_lerpTotalDuration = lerpTotalDuration;
	m_fadePrevTitle = prevTitle;
	m_fadeNewTitle = newTitle;
	m_lerpNextState = lerpNextState;
	m_lerpFadeInScreen = fadeIn;
	
	if (newTitle)
	{
		newTitle->SetVisibility(ESlateVisibility::Visible);
		newTitle->SetOpacity(0);
	}

	if (fadeIn)
	{
		startBlackScreenFadeIn(lerpTotalDuration, MainMenuCameraState::None);
	}

	setState(MainMenuCameraState::Lerping);
}

void AMainMenuCamera::lerpToTargetLocRot(float deltaTime)
{
	m_lerpElapsedTime += deltaTime;
	float ratio = m_lerpElapsedTime / m_lerpTotalDuration;
	float smoothRatio = smootherRatioStep(ratio);

	FVector newPos = FMath::Lerp(m_lerpStartLoc, m_lerpEndLoc, smoothRatio);
	SetActorLocation(newPos);

	FQuat newRot = FQuat::Slerp(m_lerpStartRot, m_lerpEndRot.Quaternion(), smoothRatio);;
	SetActorRotation(newRot);

	float fadeDuration = (m_lerpTotalDuration / 2) * 0.85;

	if (m_fadePrevTitle)
	{
		float fadeOutRatio = m_lerpElapsedTime / fadeDuration;
		float newFadeOut = FMath::Lerp(1.f, 0.f, fadeOutRatio);
		m_fadePrevTitle->SetOpacity(newFadeOut);
	}

	if (m_fadeNewTitle)
	{
		float fadeInRatio = (m_lerpElapsedTime - (m_lerpTotalDuration - fadeDuration)) / fadeDuration;
		float newFadeIn = FMath::Lerp(0.f, 1.f, fadeInRatio);
		m_fadeNewTitle->SetOpacity(newFadeIn);
	}

	if (m_lerpFadeInScreen)
	{
		fadeInBlackScreen(deltaTime);
	}

	if (ratio >= 1)
	{
		setState(m_lerpNextState);

		if (m_fadePrevTitle)
		{
			m_fadePrevTitle->SetVisibility(ESlateVisibility::Hidden);
		}
		
		m_lerpFadeInScreen = false;
	}
}

void AMainMenuCamera::selectPreviousMainMenuOption()
{
	//  Lazy hotfix that only works for 2 levels
	if (m_currentState == MainMenuCameraState::SelectingLevel)
	{
		//UGameplayStatics::SpawnSoundAttached(NextPreviousOptionSound, RootComponent);

		if(OnSelectPrevOption.IsBound()) {
			OnSelectPrevOption.Broadcast();
		}

		UImage *prevLevelTitle = m_currentSelectedLevelIndex == 0 ? m_menuHUD->Level1 :
			(m_currentSelectedLevelIndex == 1 ? m_menuHUD->Level2 : m_menuHUD->Level3);

		m_currentSelectedLevelIndex = m_currentSelectedLevelIndex == 0 ? LevelRotations.Num() - 1 : m_currentSelectedLevelIndex - 1;
		
		UImage *newLevelTitle = m_currentSelectedLevelIndex == 0 ? m_menuHUD->Level1 :
			(m_currentSelectedLevelIndex == 1 ? m_menuHUD->Level2 : m_menuHUD->Level3);

		startLerp(1.5,
			LevelSelectCameraIntermediateLocation,
			LevelRotations[m_currentSelectedLevelIndex],
			MainMenuCameraState::SelectingLevel,
			false,
			prevLevelTitle,
			newLevelTitle);
	}
	else if (m_currentState == MainMenuCameraState::WaitingForInput)
	{
		//UGameplayStatics::SpawnSoundAttached(NextPreviousOptionSound, RootComponent);

		if(OnSelectPrevOption.IsBound()) {
			OnSelectPrevOption.Broadcast();
		}

		m_prevOption = m_currentOption;
		m_currentOption = m_currentOption == 0 ? OPTIONS_COUNT - 1 : m_currentOption - 1;

		updateSelectionTitle();

		startLerp(RotLerpDuration,
			GetActorLocation(),
			Rotations[m_currentOption],
			MainMenuCameraState::WaitingForInput,
			false,
			prevTitle,
			currentTitle);
	}
}

void AMainMenuCamera::selectNextMainMenuOption()
{
	if (m_currentState == MainMenuCameraState::SelectingLevel)
	{
		//UGameplayStatics::SpawnSoundAttached(NextPreviousOptionSound, RootComponent);
		
		if (OnSelectNextOption.IsBound())
		{
			OnSelectNextOption.Broadcast();
		}

		UImage *prevLevelTitle = m_currentSelectedLevelIndex == 0 ? m_menuHUD->Level1 :
			(m_currentSelectedLevelIndex == 1 ? m_menuHUD->Level2 : m_menuHUD->Level3);

		m_currentSelectedLevelIndex = m_currentSelectedLevelIndex == LevelRotations.Num() - 1 ? 0 : m_currentSelectedLevelIndex + 1;
		
		UImage *newLevelTitle = m_currentSelectedLevelIndex == 0 ? m_menuHUD->Level1 :
			(m_currentSelectedLevelIndex == 1 ? m_menuHUD->Level2 : m_menuHUD->Level3);
		
		startLerp(1.5,
			LevelSelectCameraIntermediateLocation,
			LevelRotations[m_currentSelectedLevelIndex],
			MainMenuCameraState::SelectingLevel,
			false,
			prevLevelTitle,
			newLevelTitle);
	}
	else if (m_currentState == MainMenuCameraState::WaitingForInput)
	{
		//UGameplayStatics::SpawnSoundAttached(NextPreviousOptionSound, RootComponent);

		if(OnSelectNextOption.IsBound()) {
			OnSelectNextOption.Broadcast();
		}

		m_prevOption = m_currentOption;
		m_currentOption = m_currentOption == OPTIONS_COUNT - 1 ? 0 : m_currentOption + 1;

		updateSelectionTitle();

		startLerp(RotLerpDuration,
			GetActorLocation(),
			Rotations[m_currentOption],
			MainMenuCameraState::WaitingForInput,
			false,
			prevTitle,
			currentTitle);
	}
}

void AMainMenuCamera::updateSelectionTitle()
{
	switch ((OptionsEnum)m_prevOption)
	{
	case OptionsEnum::NewGame:
		prevTitle = m_menuHUD->NewGameTitle;
		break;

	/*case OptionsEnum::Continue:
		prevTitle = m_menuHUD->ContinueTitle;
		break;*/

	case OptionsEnum::LevelSelect:
		prevTitle = m_menuHUD->LevelSelectTitle;
		break;

	/*case OptionsEnum::Options:
		prevTitle = m_menuHUD->OptionsTitle;
		break;*/

	case OptionsEnum::Exit:
		prevTitle = m_menuHUD->ExitTitle;
		break;

	case OptionsEnum::Credits:
		prevTitle = m_menuHUD->CreditsTitle;
		break;
	}

	switch ((OptionsEnum)m_currentOption)
	{
	case OptionsEnum::NewGame:
		currentTitle = m_menuHUD->NewGameTitle;
		break;

	/*case OptionsEnum::Continue:
		currentTitle = m_menuHUD->ContinueTitle;
		break;*/

	case OptionsEnum::LevelSelect:
		currentTitle = m_menuHUD->LevelSelectTitle;
		break;

	/*case OptionsEnum::Options:
		currentTitle = m_menuHUD->OptionsTitle;
		break;*/

	case OptionsEnum::Exit:
		currentTitle = m_menuHUD->ExitTitle;
		break;

	case OptionsEnum::Credits:
		currentTitle = m_menuHUD->CreditsTitle;
		break;
	}

	currentTitle->SetVisibility(ESlateVisibility::Visible);
}

float AMainMenuCamera::smootherRatioStep(float ratio)
{
	// Scale, and clamp x to 0..1 range
	ratio = FMath::Clamp(ratio, 0.f, 1.f);
	
	// Evaluate polynomial
	return ratio*ratio*ratio*(ratio*(ratio * 6 - 15) + 10);
}
void AMainMenuCamera::startBlackScreenFadeIn(float duration, MainMenuCameraState newState)
{
	m_blackScreenFadeDuration = duration;
	m_blackScreenFadeNextState = newState;

	// Lazy fix
	m_menuHUD->BlackOverlay->SetVisibility(ESlateVisibility::Visible);
	m_menuHUD->BlackOverlay->SetOpacity(0);

	if (!m_lerpFadeInScreen)
	{
		setState(MainMenuCameraState::FadingBlackScreenIn);
	}
}

void AMainMenuCamera::fadeInBlackScreen(float deltaTime)
{
	m_lerpElapsedTime += deltaTime;

	float ratio = m_lerpElapsedTime / m_blackScreenFadeDuration;

	float newOpacity = FMath::Lerp(0.f, 1.f, ratio);

	m_menuHUD->BlackOverlay->SetOpacity(newOpacity);

	if (!m_lerpFadeInScreen && ratio >= 1)
	{
		setState(m_blackScreenFadeNextState);
		m_lerpElapsedTime = 0;
	}
}

void AMainMenuCamera::fadeOutBlackScreen(float deltaTime)
{
	m_lerpElapsedTime += deltaTime;

	float ratio = m_lerpElapsedTime / MainMenuFadeOutDuration;

	float newOpacity = FMath::Lerp(1.f, 0.f, ratio);

	m_menuHUD->BlackOverlay->SetOpacity(newOpacity);

	if (ratio >= 1)
	{
		setState(MainMenuCameraState::WaitingForInput);
		m_lerpElapsedTime = 0;
	}
}

void AMainMenuCamera::setState(MainMenuCameraState newState)
{
	// Really lazy
	m_menuHUD->LeftArrow->SetVisibility(ESlateVisibility::Hidden);
	m_menuHUD->RightArrow->SetVisibility(ESlateVisibility::Hidden);
	m_menuHUD->BackButton->SetVisibility(ESlateVisibility::Hidden);

	m_lerpElapsedTime = 0;

	switch (newState)
	{
	case MainMenuCameraState::WaitingForInput:
		m_menuHUD->LeftArrow->SetVisibility(ESlateVisibility::Visible);
		m_menuHUD->RightArrow->SetVisibility(ESlateVisibility::Visible);
		break;

	case MainMenuCameraState::StartNewGame:
		m_gameMode->loadLevel(1, AProjectLunaGameMode::LevelLoadOrigin::Player);
		currentTitle->SetVisibility(ESlateVisibility::Hidden);
		break;

	case MainMenuCameraState::FadingBlackScreenIn:
		m_menuHUD->BlackOverlay->SetVisibility(ESlateVisibility::Visible);
		m_menuHUD->BlackOverlay->SetOpacity(0);
		break;

	case MainMenuCameraState::LoadLevel:
		m_gameMode->loadLevel(m_currentSelectedLevelIndex + 1, AProjectLunaGameMode::LevelLoadOrigin::Player);
		break;

	case MainMenuCameraState::SelectingLevel:
		m_menuHUD->BackButton->SetVisibility(ESlateVisibility::Visible);
		m_menuHUD->LeftArrow->SetVisibility(ESlateVisibility::Visible);
		m_menuHUD->RightArrow->SetVisibility(ESlateVisibility::Visible);
		break;

	case MainMenuCameraState::ShowCredits:
		m_menuHUD->BackButton->SetVisibility(ESlateVisibility::Visible);
		m_menuHUD->CreditsList->SetVisibility(ESlateVisibility::Visible);
		((UCanvasPanelSlot *)m_menuHUD->CreditsList->GetParent()->Slot)->SetPosition(m_initialCreditsPos);
		break;

	case MainMenuCameraState::Exit:
		m_world->GetFirstPlayerController()->ConsoleCommand("quit");
		LUNA_WARN(MainMenuCamera, "Quit command called");
		break;
	}

	m_currentState = newState;
}

void AMainMenuCamera::setCredditsAcceleration(float value)
{
	m_creditsAcceleration = value;
}

void AMainMenuCamera::resetLocationRotation()
{
	SetActorLocation(m_camLocation);
	SetActorRotation(Rotations[0]);
}