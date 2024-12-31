#include "Event.h"

namespace Ngine
{
    EventHandler EventHandler::mDefaultHandler;

    EventHandler::EventHandler()
    {}

    EventHandler::~EventHandler()
    {
        mDefaultHandler.ClearEventBuffer();
    }

    void EventHandler::ClearEventBuffer()
    {
        for(auto pEvent : mDefaultHandler.vecEvents)
        {
            if(pEvent) delete pEvent;
        }

        mDefaultHandler.vecEvents.clear();
    }

    void EventHandler::AddEventToBuffer(Event *pEvent, EventType type)
    {
        if(pEvent->mType != type)
        {
            //LOG_F(INFO, "Event types do not match! Forcing second argument as %d...", (uint32_t)type);
            pEvent->mType = type;
        }

        switch (pEvent->mType) {
            case EventType::EventType_Null:
            {
                LOG_F(ERROR, "Event type NULL cannot be added to buffer...");
                break;
            }

            case EventType::EventType_WindowResize:
            {
                EventWindowResize* pCastedEvent = reinterpret_cast<EventWindowResize*>(pEvent);
                mDefaultHandler.vecEvents.push_back(pCastedEvent);
                mDefaultHandler.vecEvents[mDefaultHandler.vecEvents.size() - 1]->mType = type;
                break;
            }


            case EventType::EventType_CursorMove:
            {
                EventCursorMove* pCastedEvent = reinterpret_cast<EventCursorMove*>(pEvent);
                mDefaultHandler.vecEvents.push_back(pCastedEvent);
                mDefaultHandler.vecEvents[mDefaultHandler.vecEvents.size() - 1]->mType = type;
                break;
            }

            case EventType::EventType_KeyAction:
            {
                EventKeyAction* pCastedEvent = reinterpret_cast<EventKeyAction*>(pEvent);
                mDefaultHandler.vecEvents.push_back(pCastedEvent);
                mDefaultHandler.vecEvents[mDefaultHandler.vecEvents.size() - 1]->mType = type;
                break;
            }

            default:
            {
                LOG_F(ERROR, "Unrecoginzed event type was passed!");
            }
        }
    }

    std::vector<Event*>& EventHandler::ObtainEventBuffer()
    {
        return mDefaultHandler.vecEvents;
    }
}