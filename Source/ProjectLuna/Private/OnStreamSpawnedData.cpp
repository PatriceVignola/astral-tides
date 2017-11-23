#include "ProjectLuna.h"
#include "OnStreamSpawnedData.h"
#include "StreamEntity.h"

OnStreamSpawnedData::OnStreamSpawnedData(AStreamEntity *stream)
{
	m_stream = stream;
}

AStreamEntity* OnStreamSpawnedData::getStream()
{
	return m_stream;
}