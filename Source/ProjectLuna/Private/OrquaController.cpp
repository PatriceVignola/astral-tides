// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "OrquaController.h"
#include "ProjectLunaGameMode.h"

DEFINE_LUNA_LOG(OrquaController);

AOrquaController::AOrquaController()
	: m_world(nullptr)
	, m_orqua(nullptr)
	, m_gameMode(nullptr) {}

void AOrquaController::BeginPlay() {
	Super::BeginPlay();

	m_world = GetWorld();

	m_gameMode = (AProjectLunaGameMode *)m_world->GetAuthGameMode();
	m_gameMode->getEventManager().subscribe("OnLevelLoaded", this, &AOrquaController::onLevelLoaded);
	//m_gameMode->getEventManager().subscribe("OnLevelUnLoaded", this, &AOrquaController::onLevelUnloaded);
}

void AOrquaController::BeginDestroy() {
	Super::BeginDestroy();
	LUNA_LOG(OrquaController, "Begin Destroy()");
	/*if(m_gameMode) {
	m_gameMode->getEventManager().unsubscribe("OnLevelLoaded", this, &AOrquaController::onLevelLoaded);
	m_gameMode->getEventManager().unsubscribe("OnLevelUnloaded", this, &AOrquaController::onLevelUnloaded);
	}*/
}

void AOrquaController::EndPlay(const EEndPlayReason::Type reason) {
	Super::EndPlay(reason);
	LUNA_LOG(OrquaController, "EndPlay()");
	/*this->UnPossess();
	if(m_orqua->Destroy()) {
	LUNA_LOG(OrquaController, "Sucessful destruction ");
	}
	m_orqua = nullptr;*/

	//if(GetPawn()) {
	//	GetPawn()->Destroy();
	//	m_orqua = nullptr;
	//}
	if(m_gameMode) {
		m_gameMode->getEventManager().unsubscribe("OnLevelLoaded", this, &AOrquaController::onLevelLoaded);
		//m_gameMode->getEventManager().unsubscribe("OnLevelUnloaded", this, &AOrquaController::onLevelUnloaded);
	}
}

void AOrquaController::onLevelLoaded(OnLevelLoadedData *eventData) {
	//FVector spawnPosition = FVector(980, -200, 30); //In AINav Scene
	//FVector spawnPosition = FVector(100, 2170, 70); //In testing	
	FVector spawnPosition;
	int level = eventData->getLevelNumber();
	LUNA_LOG(OrquaController, "Level Number is: %d", level);
	if(level == 1) {
		spawnPosition = FVector(280, 2290, 0);
	} else if(level == 2) {
		//spawnPosition = FVector(2240.0, -15980.0, 0);
		spawnPosition = FVector(3960.0, -13790.0, 0);
	}

	/*m_orqua = m_world->SpawnActor<AOrqua>(OrquaClass, spawnPosition, FRotator::ZeroRotator);

	if(m_orqua) {
	m_orqua->setLevelAi(level);
	this->Possess(m_orqua);
	}*/

	/*if(m_orqua) {
	LUNA_WARN(OrquaController, "Orqua already existing, not creating a new one");
	} else {
	m_orqua = m_world->SpawnActor<AOrqua>(OrquaClass, spawnPosition, FRotator::ZeroRotator);
	}

	if(m_orqua) {
	m_orqua->setLevelAi(level);
	this->Possess(m_orqua);
	}*/
	if(m_orqua) {
		LUNA_WARN(OrquaController, "Destroying");
		this->UnPossess();
		if(m_orqua->Destroy()) {
			LUNA_LOG(OrquaController, "Sucessful destruction");
		}
		m_orqua = nullptr;
	}/* else {

	 }*/
	if(level == 1 || level == 2) {
		m_orqua = m_world->SpawnActor<AOrqua>(OrquaClass, spawnPosition, FRotator::ZeroRotator);

		if(m_orqua) {
			m_orqua->setLevelAi(level);
			this->Possess(m_orqua);
		}
	}
}

void AOrquaController::onLevelUnloaded(OnLevelLoadedData *eventData) {
	//if(GetPawn()) {
	//	GetPawn()->Destroy();
	//	m_orqua = nullptr;
	//}
}