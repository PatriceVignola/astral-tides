#pragma once

#include "EventData.h"

class OnMatineeStartedData : public EventData
{
public:
	OnMatineeStartedData(bool isSkippable);
	bool getIsSkippable();

private:
	bool m_isSkippable;
};
