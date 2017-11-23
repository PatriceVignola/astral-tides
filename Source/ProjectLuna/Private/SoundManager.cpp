// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "ProjectLunaGameMode.h"
#include "SoundManager.h"

DEFINE_LUNA_LOG(SoundManager);

// Sets default values
ASoundManager::ASoundManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void ASoundManager::BeginPlay()
{
	Super::BeginPlay();

	m_world = GetWorld();
	m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();

	m_gameMode->getEventManager().subscribe("OnStartHowlUp", this, &ASoundManager::onStartHowlUp);
	m_gameMode->getEventManager().subscribe("OnStopHowlUp", this, &ASoundManager::onStopHowlUp);
	m_gameMode->getEventManager().subscribe("OnStartHowlDown", this, &ASoundManager::onStartHowlDown);
	m_gameMode->getEventManager().subscribe("OnStopHowlDown", this, &ASoundManager::onStopHowlDown);


	m_gameMode->getEventManager().subscribe("OnStartPush", this, &ASoundManager::onStartPush);
	m_gameMode->getEventManager().subscribe("OnStopPush", this, &ASoundManager::onStopPush);

	//TODO Put this back
	//m_gameMode->getEventManager().subscribe("OnSplash", this, &ASoundManager::onSplash);
}

void ASoundManager::BeginDestroy()
{
	Super::BeginDestroy();

	if (m_gameMode)
	{
		m_gameMode->getEventManager().unsubscribe("OnStartHowlUp", this, &ASoundManager::onStartHowlUp);
		m_gameMode->getEventManager().unsubscribe("OnStopHowlUp", this, &ASoundManager::onStopHowlUp);
		m_gameMode->getEventManager().unsubscribe("OnStartHowlDown", this, &ASoundManager::onStartHowlDown);
		m_gameMode->getEventManager().unsubscribe("OnStopHowlDown", this, &ASoundManager::onStopHowlDown);


		m_gameMode->getEventManager().unsubscribe("OnStartPush", this, &ASoundManager::onStartPush);
		m_gameMode->getEventManager().unsubscribe("OnStopPush", this, &ASoundManager::onStopPush);
		//m_gameMode->getEventManager().unsubscribe("OnSplash", this, &ASoundManager::onSplash);
	}
}

void ASoundManager::playMainMusic()
{
	//playSound(MainMusic);
}

void ASoundManager::playSound(USoundCue *sound)
{
	UGameplayStatics::SpawnSoundAttached(sound, RootComponent);
	LUNA_LOG(SoundManager, "Sound playing: %s", *(sound->GetName()));
}

void ASoundManager::onStartHowlUp(GenericEventData *eventData)
{
	UE_LOG(LogTemp, Warning, TEXT("Howling up"));

	if (OnStartHowlUp.IsBound())
	{
		OnStartHowlUp.Broadcast();
	}
}

void ASoundManager::onStopHowlUp(GenericEventData *eventData)
{
	if(OnStopHowlUp.IsBound())
	{
		OnStopHowlUp.Broadcast();
	}
}

void ASoundManager::onStartHowlDown(GenericEventData *eventData) {
	if(OnStartHowlDown.IsBound()) {
		OnStartHowlDown.Broadcast();
	}
}

void ASoundManager::onStopHowlDown(GenericEventData *eventData) {
	if(OnStopHowlDown.IsBound()) {
		OnStopHowlDown.Broadcast();

		UE_LOG(LogTemp, Warning, TEXT("onStopHowlDown"));
	}
}







void ASoundManager::onStartPush(GenericEventData *eventData)
{

}

void ASoundManager::onStopPush(GenericEventData *eventData)
{

}

void ASoundManager::onSplash(GenericEventData *eventData)
{

}