// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EventData.h"

class OnMenuLevelSelectedData : public EventData
{
public:
	OnMenuLevelSelectedData(int levelNumber);

	int getLevelNumber();

private:
	int m_levelNumber;
};
