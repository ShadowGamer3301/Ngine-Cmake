#pragma once
#include "Core.hxx"

namespace Ngine
{
    enum class EventType : uint32_t 
    {
        EventType_Null = 0x0000,
        EventType_WindowResize = 0x0001,
        EventType_CursorMove = 0x0002,
        EventType_KeyAction = 0x0003,
    };

    struct Event
    {
    public:
        EventType mType = EventType::EventType_Null;
    };

    struct EventWindowResize : Event
    {
    public:
        EventType mType = EventType::EventType_WindowResize;
        uint32_t mNewWidth = 0;
        uint32_t mNewHeight = 0;
    };

    struct EventCursorMove : Event
    {
    public:
        EventType mType = EventType::EventType_CursorMove;
        double pos_x = 0.0;
        double pos_y = 0.0;
    };

    struct EventKeyAction : Event
    {
    public:
        EventType mType = EventType::EventType_KeyAction;
        int mKey = 0;
        bool mPressed = false;
    };

    class EventHandler
    {
    public:
        static std::vector<Event*>& ObtainEventBuffer();
        static void ClearEventBuffer();
        static void AddEventToBuffer(Event* pEvent, EventType type);

    private:
        EventHandler();
        ~EventHandler();

        static EventHandler mDefaultHandler;
        
        std::vector<Event*> vecEvents;

    };
}