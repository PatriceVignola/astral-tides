#include "ProjectLuna.h"
#include "OnBuoyantObjectSpawnedData.h"
#include "BuoyantObjectRuntime.h"

OnBuoyantObjectSpawnedData::OnBuoyantObjectSpawnedData(ABuoyantObjectRuntime *buoyantObject)
{
	m_buoyantObject = buoyantObject;
}

ABuoyantObjectRuntime *OnBuoyantObjectSpawnedData::getBuoyantObject()
{
	return m_buoyantObject;
}