#pragma once

#include "EuroScopePlugIn.h"
#include <string>
#include <vector>

#define MY_PLUGIN_NAME      "CZVR Request Queue"
#define MY_PLUGIN_VERSION   "0.1"
#define MY_PLUGIN_AUTHOR    "Sean / CZVR"
#define MY_PLUGIN_COPYRIGHT "Internal CZVR tool - not for redistribution outside VATCAN"

// Tag item + tag function codes.
// These IDs only need to be unique WITHIN this plugin.
const int TAG_ITEM_QUEUE_STATUS = 1;
const int TAG_FUNC_QUEUE_MENU   = 1;

// Function IDs used inside the popup reorder menu (passed back via OnFunctionCall)
const int FUNC_QUEUE_MOVE_TOP  = 101;
const int FUNC_QUEUE_MOVE_UP   = 102;
const int FUNC_QUEUE_MOVE_DOWN = 103;
const int FUNC_QUEUE_REMOVE    = 104;

// The ground state string EuroScope uses for "taxiing out".
// Confirmed from EuroScopePlugIn.h: GetGroundState() returns "PUSH", "TAXI", "DEPA", or "".
const char* const GROUND_STATE_TAXI = "TAXI";

struct QueueEntry
{
    std::string callsign;
    int         enteredAtSecond; // value of OnTimer's Counter when it joined the queue
};

class CReqQueuePlugin : public EuroScopePlugIn::CPlugIn
{
public:
    CReqQueuePlugin(void);
    virtual ~CReqQueuePlugin(void);

    virtual void OnGetTagItem(EuroScopePlugIn::CFlightPlan FlightPlan,
                               EuroScopePlugIn::CRadarTarget RadarTarget,
                               int ItemCode,
                               int TagData,
                               char sItemString[16],
                               int* pColorCode,
                               COLORREF* pRGB,
                               double* pFontSize);

    virtual void OnFunctionCall(int FunctionId,
                                 const char* sItemString,
                                 POINT Pt,
                                 RECT Area);

    virtual void OnTimer(int Counter);

private:
    std::vector<QueueEntry> m_queue;       // order = priority; index 0 = next to go
    int                     m_currentSecond;
    std::string             m_menuTargetCallsign; // which AC the open popup menu refers to

    int  FindQueueIndex(const std::string& callsign) const;
    void RemoveFromQueue(const std::string& callsign);
    void MoveEntry(int fromIndex, int toIndex);
};
