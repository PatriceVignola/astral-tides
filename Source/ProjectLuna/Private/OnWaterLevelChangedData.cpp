#include "ProjectLuna.h"
#include "OnWaterLevelChangedData.h"

OnWaterLevelChangedData::OnWaterLevelChangedData(float newWaterDelta, float minDelta, float maxDelta, float absoluteMaxDelta)
{
	m_newWaterDelta = newWaterDelta;
	m_minDelta = minDelta;
	m_currentMaxDelta = maxDelta;
	m_absoluteMaxDelta = absoluteMaxDelta;
}

float OnWaterLevelChangedData::getNewWaterDelta()
{
	return m_newWaterDelta;
}

float OnWaterLevelChangedData::getMinDelta()
{
	return m_minDelta;
}

float OnWaterLevelChangedData::getCurrentMaxDelta()
{
	return m_currentMaxDelta;
}

float OnWaterLevelChangedData::getAbsoluteMaxDelta()
{
	return m_absoluteMaxDelta;
}