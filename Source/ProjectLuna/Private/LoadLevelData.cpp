#include "ProjectLuna.h"
#include "LoadLevelData.h"

LoadLevelData::LoadLevelData(int levelNumber)
{
	m_levelNumber = levelNumber;
}

int LoadLevelData::getLevelNumber()
{
	return m_levelNumber;

}