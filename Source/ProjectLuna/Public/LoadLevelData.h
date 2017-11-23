#pragma once

#include "EventData.h"

class LoadLevelData : public EventData
{
public:
	LoadLevelData(int levelNumber);

	int getLevelNumber();

private:
	int m_levelNumber;
};
