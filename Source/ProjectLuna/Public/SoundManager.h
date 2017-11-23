// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "GenericEventData.h"
#include "Runtime/Engine/Classes/Sound/SoundCue.h"
#include "LunaLog.h"
#include "SoundManager.generated.h"

DECLARE_LUNA_LOG(SoundManager);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartHowlDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStopHowlDelegate);

class AProjectLunaGameMode;

UCLASS()
class PROJECTLUNA_API ASoundManager : public AActor
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FStartHowlDelegate OnStartHowlUp;

	UPROPERTY(BlueprintAssignable)
	FStopHowlDelegate OnStopHowlUp;

	UPROPERTY(BlueprintAssignable)
	FStartHowlDelegate OnStartHowlDown;

	UPROPERTY(BlueprintAssignable)
	FStopHowlDelegate OnStopHowlDown;

	ASoundManager();
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	void playMainMusic();

private:
	void playSound(USoundCue *sound);

	void onStartHowlUp(GenericEventData *eventData);
	void onStopHowlUp(GenericEventData *eventData);
	void onStartHowlDown(GenericEventData *eventData);
	void onStopHowlDown(GenericEventData *eventData);



	void onStartPush(GenericEventData *eventData);
	void onStopPush(GenericEventData *eventData);
	void onSplash(GenericEventData *eventData);

	UAudioComponent *m_waterSound;
	UAudioComponent *m_pushSound;

	UWorld *m_world;
	AProjectLunaGameMode *m_gameMode;
};
