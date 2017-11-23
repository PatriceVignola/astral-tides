// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AIController.h"
#include "Orqua.h"
#include "LunaLog.h"
#include "OnLevelLoadedData.h"
#include "OrquaController.generated.h"

DECLARE_LUNA_LOG(OrquaController);

class AProjectLunaGameMode;

/**
*
*/
UCLASS()
class PROJECTLUNA_API AOrquaController : public AAIController {
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<class AOrqua> OrquaClass;

	AOrquaController();

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void onLevelLoaded(OnLevelLoadedData *eventData);
	void onLevelUnloaded(OnLevelLoadedData *eventData);

private:
	UWorld* m_world;
	AOrqua* m_orqua;
	AProjectLunaGameMode *m_gameMode;
};
