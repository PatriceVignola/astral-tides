#pragma once

#include "EventData.h"
#include "Wolf.h"

class OnWolfSpawnedData : public EventData
{
public:
	OnWolfSpawnedData(AWolf *wolf);
	AWolf* getWolf();

private:
	AWolf *m_wolf;
};
