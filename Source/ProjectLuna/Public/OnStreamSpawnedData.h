#pragma once

#include "EventData.h"

class AStreamEntity;

class OnStreamSpawnedData : public EventData
{
public:
	OnStreamSpawnedData(AStreamEntity *stream);

	AStreamEntity *getStream();

private:
	AStreamEntity *m_stream;
};
