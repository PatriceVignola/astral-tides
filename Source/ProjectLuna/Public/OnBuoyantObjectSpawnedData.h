#pragma once

#include "EventData.h"

class ABuoyantObjectRuntime;

class OnBuoyantObjectSpawnedData : public EventData
{
public:
	OnBuoyantObjectSpawnedData(ABuoyantObjectRuntime *buoyantObject);
	ABuoyantObjectRuntime *getBuoyantObject();

private:
	ABuoyantObjectRuntime *m_buoyantObject;
};
