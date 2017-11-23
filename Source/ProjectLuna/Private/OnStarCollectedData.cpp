#include "ProjectLuna.h"
#include "OnStarCollectedData.h"

OnStarCollectedData::OnStarCollectedData(int levelIndex, int starIndex)
{
	m_levelIndex = levelIndex;
	m_starIndex = starIndex;
}

int OnStarCollectedData::getLevelIndex()
{
	return m_levelIndex;
}

int OnStarCollectedData::getStarIndex()
{
	return m_starIndex;
}