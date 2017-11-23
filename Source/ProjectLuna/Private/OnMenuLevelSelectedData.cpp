// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "OnMenuLevelSelectedData.h"

OnMenuLevelSelectedData::OnMenuLevelSelectedData(int levelNumber)
{
	m_levelNumber = levelNumber;
}

int OnMenuLevelSelectedData::getLevelNumber()
{
	return m_levelNumber;
}