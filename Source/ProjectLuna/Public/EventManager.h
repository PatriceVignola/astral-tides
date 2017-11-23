// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EventData.h"

class MemberFunctionHandlerBase
{
public:
	virtual ~MemberFunctionHandlerBase() { };

	// Pure virtual
	virtual void execute(EventData* event) = 0;

	void(*subscribedEvent)(EventData *event);
};

template <class OBJECT, class EventT>
class MemberFunctionHandler : public MemberFunctionHandlerBase
{
public:
	typedef void(OBJECT::*MemberFunction)(EventT*);

	MemberFunctionHandler(OBJECT *instance, MemberFunction memFunc)
	{
		m_memberFunc = memFunc;
		m_instance = instance;
	}

	void execute(EventData* event) override
	{
		(m_instance->*m_memberFunc)(static_cast<EventT*>(event));
	}

	MemberFunction getMemberFunction()
	{
		return m_memberFunc;
	}

private:
	OBJECT *m_instance;
	MemberFunction m_memberFunc;
};



class EventManager
{
public:
	typedef TArray<MemberFunctionHandlerBase*> MemberFunctionHandlerList;
	typedef TMap<FString, MemberFunctionHandlerList> MemberFunctionHandlerMap;

	// Template functions have to be implemented in the header for the compiler
	template <class OBJECT, class EventT>
	void subscribe(FString eventName, OBJECT *obj, void(OBJECT::*newEvent)(EventT*))
	{
		if(!m_memberFunctionMap.Contains(eventName)) {
			//We'll create a new entry into the TMap if it's not present
			m_memberFunctionMap.Add(eventName, MemberFunctionHandlerList());
		}
		m_memberFunctionMap[eventName].Add(new MemberFunctionHandler<OBJECT, EventT>(obj, newEvent));
	}

	// Template functions have to be implemented in the header for the compiler
	template <class OBJECT, class EventT>
	void unsubscribe(FString eventName, OBJECT *obj, void(OBJECT::*subscribedEvent)(EventT*))
	{
		if (!m_memberFunctionMap.Contains(eventName)) return;

		MemberFunctionHandlerList *eventList = &m_memberFunctionMap[eventName];

		// We iterate the list related to the eventName and delete the event handler pointer when we find it
        for (auto it : *eventList)
		{
			MemberFunctionHandler<OBJECT, EventT> *handler = static_cast<MemberFunctionHandler<OBJECT, EventT> *>(it);

			if (handler->getMemberFunction() == subscribedEvent)
			{
				eventList->Remove(it);
                delete it;
				break;
			}
		}
	}

	void raiseEvent(FString eventName, EventData *event);

	~EventManager();

private:
	MemberFunctionHandlerMap m_memberFunctionMap;
};
