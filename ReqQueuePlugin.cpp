#include "ReqQueuePlugin.h"
#include <cstdio>

using namespace EuroScopePlugIn;

CReqQueuePlugin::CReqQueuePlugin(void)
    : CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
              MY_PLUGIN_NAME,
              MY_PLUGIN_VERSION,
              MY_PLUGIN_AUTHOR,
              MY_PLUGIN_COPYRIGHT)
{
    m_currentSecond = 0;

    // Controllers add this as a TAG item column in their DEP/GND list layout.
    RegisterTagItemType("REQ Queue Status", TAG_ITEM_QUEUE_STATUS);
    // Controllers bind this to a mouse click on that column to open the reorder menu.
    RegisterTagItemFunction("REQ Queue Menu", TAG_FUNC_QUEUE_MENU);
}

CReqQueuePlugin::~CReqQueuePlugin(void)
{
}

int CReqQueuePlugin::FindQueueIndex(const std::string& callsign) const
{
    for (size_t i = 0; i < m_queue.size(); i++)
    {
        if (m_queue[i].callsign == callsign)
            return static_cast<int>(i);
    }
    return -1;
}

void CReqQueuePlugin::RemoveFromQueue(const std::string& callsign)
{
    int idx = FindQueueIndex(callsign);
    if (idx >= 0)
        m_queue.erase(m_queue.begin() + idx);
}

void CReqQueuePlugin::MoveEntry(int fromIndex, int toIndex)
{
    if (fromIndex < 0 || fromIndex >= static_cast<int>(m_queue.size()))
        return;

    if (toIndex < 0)
        toIndex = 0;
    if (toIndex >= static_cast<int>(m_queue.size()))
        toIndex = static_cast<int>(m_queue.size()) - 1;
    if (fromIndex == toIndex)
        return;

    QueueEntry entry = m_queue[fromIndex];
    m_queue.erase(m_queue.begin() + fromIndex);
    m_queue.insert(m_queue.begin() + toIndex, entry);
}

// Called once per second by EuroScope. This is where we auto-detect ground
// state changes since the SDK has no "state changed" event to hook into -
// polling every second is the standard approach other plugins use too.
void CReqQueuePlugin::OnTimer(int Counter)
{
    m_currentSecond = Counter;

    // 1) Add newly-taxiing aircraft to the back of the queue,
    //    and drop aircraft that have left the TAXI state.
    for (CFlightPlan fp = FlightPlanSelectFirst(); fp.IsValid(); fp = FlightPlanSelectNext(fp))
    {
        const char* gs = fp.GetGroundState();
        std::string callsign = fp.GetCallsign();

        bool isTaxiing = (gs != NULL) && (std::string(gs) == GROUND_STATE_TAXI);
        int idx = FindQueueIndex(callsign);

        if (isTaxiing && idx < 0)
        {
            QueueEntry e;
            e.callsign = callsign;
            e.enteredAtSecond = Counter;
            m_queue.push_back(e);
        }
        else if (!isTaxiing && idx >= 0)
        {
            // Progressed to DEPA, reverted to PUSH, or state cleared -> done with the queue.
            m_queue.erase(m_queue.begin() + idx);
        }
    }

    // 2) Prune entries for callsigns that vanished entirely (disconnects, etc.)
    //    without a clean ground-state transition.
    for (int i = static_cast<int>(m_queue.size()) - 1; i >= 0; i--)
    {
        CFlightPlan fp = FlightPlanSelect(m_queue[i].callsign.c_str());
        if (!fp.IsValid())
            m_queue.erase(m_queue.begin() + i);
    }
}

void CReqQueuePlugin::OnGetTagItem(CFlightPlan FlightPlan,
                                    CRadarTarget RadarTarget,
                                    int ItemCode,
                                    int TagData,
                                    char sItemString[16],
                                    int* pColorCode,
                                    COLORREF* pRGB,
                                    double* pFontSize)
{
    if (ItemCode != TAG_ITEM_QUEUE_STATUS)
        return;

    if (!FlightPlan.IsValid())
        return;

    int idx = FindQueueIndex(FlightPlan.GetCallsign());
    if (idx < 0)
        return; // not in the queue -> leave the tag item blank

    int waitSeconds = m_currentSecond - m_queue[idx].enteredAtSecond;
    if (waitSeconds < 0)
        waitSeconds = 0;

    int mins = waitSeconds / 60;
    int secs = waitSeconds % 60;

    // Max 15 chars + null terminator. e.g. "#2 04:12" = position 2, waited 4m12s.
    sprintf_s(sItemString, 16, "#%d %02d:%02d", idx + 1, mins, secs);

    *pColorCode = TAG_COLOR_DEFAULT;
}

void CReqQueuePlugin::OnFunctionCall(int FunctionId,
                                      const char* sItemString,
                                      POINT Pt,
                                      RECT Area)
{
    if (FunctionId == TAG_FUNC_QUEUE_MENU)
    {
        // EuroScope invokes this on whichever flight plan's tag was clicked;
        // the reliably correlated aircraft at that moment is the ASEL'd one.
        CFlightPlan fp = FlightPlanSelectASEL();
        if (!fp.IsValid())
            return;

        m_menuTargetCallsign = fp.GetCallsign();

        OpenPopupList(Area, "Request Queue", 1);
        AddPopupListElement("Move to Top",       "", FUNC_QUEUE_MOVE_TOP);
        AddPopupListElement("Move Up",           "", FUNC_QUEUE_MOVE_UP);
        AddPopupListElement("Move Down",         "", FUNC_QUEUE_MOVE_DOWN);
        AddPopupListElement("Remove from Queue", "", FUNC_QUEUE_REMOVE);
        return;
    }

    if (m_menuTargetCallsign.empty())
        return;

    int idx = FindQueueIndex(m_menuTargetCallsign);
    if (idx < 0)
        return;

    switch (FunctionId)
    {
        case FUNC_QUEUE_MOVE_TOP:
            MoveEntry(idx, 0);
            break;
        case FUNC_QUEUE_MOVE_UP:
            MoveEntry(idx, idx - 1);
            break;
        case FUNC_QUEUE_MOVE_DOWN:
            MoveEntry(idx, idx + 1);
            break;
        case FUNC_QUEUE_REMOVE:
            RemoveFromQueue(m_menuTargetCallsign);
            break;
        default:
            break;
    }

    m_menuTargetCallsign.clear();
}
