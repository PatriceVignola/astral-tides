#include "ProjectLuna.h"
#include "OnMatineeStartedData.h"

OnMatineeStartedData::OnMatineeStartedData(bool isSkippable)
{
	m_isSkippable = isSkippable;
}

bool OnMatineeStartedData::getIsSkippable()
{
	return m_isSkippable;
}