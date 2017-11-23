#pragma once

#include "EventData.h"

class OnCheckpointReachedData : public EventData
{
public:
	OnCheckpointReachedData(FVector location, FRotator rotation);

	FVector getLocation();
	FRotator getRotation();

private:
	FVector m_location;
	FRotator m_rotation;
};
