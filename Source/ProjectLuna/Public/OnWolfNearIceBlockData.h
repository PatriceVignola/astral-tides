#pragma once

#include "EventData.h"

class OnWolfNearIceBlockData : public EventData
{
public:
	OnWolfNearIceBlockData(bool isEntering, FVector location);
	bool getIsEntering();
	FVector getLocation();

private:
	bool m_isEntering;
	FVector m_location;
};
