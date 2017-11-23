#include "ProjectLuna.h"
#include "OnWolfSpawnedData.h"

OnWolfSpawnedData::OnWolfSpawnedData(AWolf *wolf)
{
	m_wolf = wolf;
}

AWolf* OnWolfSpawnedData::getWolf()
{
	return m_wolf;
}