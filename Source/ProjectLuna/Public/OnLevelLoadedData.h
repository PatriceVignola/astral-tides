// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EventData.h"
#include "ProjectLunaGameMode.h"

class OnLevelLoadedData : public EventData
{
public:
	OnLevelLoadedData(int levelNumber, AProjectLunaGameMode::LevelLoadOrigin loadOrigin);

	int getLevelNumber();
	AProjectLunaGameMode::LevelLoadOrigin getLoadOrigin();

private:
	int m_levelNumber;
	AProjectLunaGameMode::LevelLoadOrigin m_loadOrigin;
};
