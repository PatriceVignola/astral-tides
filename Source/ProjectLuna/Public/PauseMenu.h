// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "Runtime/UMG/Public/Components/Widget.h"
#include "Runtime/UMG/Public/Components/Image.h"
#include "Runtime/UMG/Public/Components/ProgressBar.h"
#include "Runtime/UMG/Public/Components/CanvasPanel.h"
#include "Runtime/UMG/Public/Components/ContentWidget.h"
#include "GenericEventData.h"
#include "PauseMenu.generated.h"

class AProjectLunaGameMode;

UCLASS()
class PROJECTLUNA_API UPauseMenuHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:	
	enum class Options { Resume, MainMenu };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UImage *DimmedBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UImage *> Arrows;

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	const float MAX_COOLDOWN_TIME = 0.3;

	Options m_selectedOption;
	float m_cooldownTime;

	float m_fadeRatio;

	AProjectLunaGameMode *m_gameMode;

	void onGamePaused(GenericEventData *eventData);
	void onGameUnpaused(GenericEventData *eventData);
	void selectNextOption(GenericEventData *eventData);
	void selectPreviousOption(GenericEventData *eventData);
	void acceptOption(GenericEventData *eventData);
};
