#pragma once

#include "EventData.h"

class OnStarCollectedData : public EventData
{
public:
	OnStarCollectedData(int levelIndex, int starIndex);
	int getLevelIndex();
	int getStarIndex();

private:
	int m_levelIndex;
	int m_starIndex;
};
