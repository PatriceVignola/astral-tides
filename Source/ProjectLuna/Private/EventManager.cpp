#include "ProjectLuna.h"
#include "EventManager.h"

EventManager::~EventManager()
{
	for (auto &kvp : m_memberFunctionMap)
	{
		for (auto &memberFunctionList : kvp.Value)
		{
			// We delete all pointers to functions allocated on the heap
			delete memberFunctionList;
		}
	}

	// Clearing the map automatically clears the vector containers
	m_memberFunctionMap.Empty();
}


void EventManager::raiseEvent(FString eventName, EventData *event)
{

	if (!m_memberFunctionMap.Contains(eventName)) return;

	MemberFunctionHandlerList *functionsList = &m_memberFunctionMap[eventName];

	for (MemberFunctionHandlerBase *subscribedEvent : *functionsList)
	{
		subscribedEvent->execute(event);
	}

	delete event;
}