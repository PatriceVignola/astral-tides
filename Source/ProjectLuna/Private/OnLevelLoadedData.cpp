// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "OnLevelLoadedData.h"

OnLevelLoadedData::OnLevelLoadedData(int levelNumber, AProjectLunaGameMode::LevelLoadOrigin loadOrigin)
{
	m_levelNumber = levelNumber;
	m_loadOrigin = loadOrigin;
}

int OnLevelLoadedData::getLevelNumber()
{
	return m_levelNumber;
}

AProjectLunaGameMode::LevelLoadOrigin OnLevelLoadedData::getLoadOrigin()
{
	return m_loadOrigin;
}