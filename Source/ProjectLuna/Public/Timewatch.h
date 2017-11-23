// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LunaLog.h"
#include "Runtime/Core/Public/Containers/UnrealString.h"

DECLARE_LUNA_LOG(TimeWatch);

/**
 * 
 */
class PROJECTLUNA_API Timewatch
{
public:
	Timewatch(FString name = "");
	~Timewatch();

	bool hasExpired() const;
	/* Start the timewatch
	@param timeToExpire time before the timewatch expires in seconds
	*/
	void start(double timeToExpire);
	bool isRunning() const;
	void stop();

private:
	double m_startTime;
	double m_timeToExpire;
	bool m_isRunning;
	FString m_name;
};
