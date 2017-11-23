// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectLuna.h"
#include "Timewatch.h"

DEFINE_LUNA_LOG(TimeWatch);

Timewatch::Timewatch(FString name)
	: m_startTime(0.f)
	, m_timeToExpire(0)
	, m_isRunning(false)
	, m_name(name)
{

}

Timewatch::~Timewatch()
{
}

bool Timewatch::hasExpired() const {
	auto t = FPlatformTime::Seconds();
	if((m_startTime + m_timeToExpire) <= t) {
		LUNA_LOG(TimeWatch, "TimeWatch %s has expired", *m_name);
	}
	return (m_startTime + m_timeToExpire) <= t;
}

void Timewatch::start(double timeToExpire) {
	m_timeToExpire = timeToExpire;
	m_startTime = FPlatformTime::Seconds();
	m_isRunning = true;
}

bool Timewatch::isRunning() const {
	return m_isRunning;
}

void Timewatch::stop() {
	m_isRunning = false;
}