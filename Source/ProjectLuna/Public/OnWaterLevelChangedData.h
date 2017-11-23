// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EventData.h"

class OnWaterLevelChangedData : public EventData
{
public:
	OnWaterLevelChangedData(float newWaterDelta, float minDelta, float maxDelta, float absoluteMaxDelta);

	float getNewWaterDelta();
	float getMinDelta();
	float getCurrentMaxDelta();
	float getAbsoluteMaxDelta();

private:
	float m_newWaterDelta;
	float m_minDelta;
	float m_currentMaxDelta;
	float m_absoluteMaxDelta;
};
