#pragma once

#include "EventData.h"

class OnHowledData : public EventData
{
public:
	OnHowledData(float speed, float axisValue);

	float getSpeed();
	float getAxisValue();

private:
	float m_speed;
	float m_axisValue;
};
