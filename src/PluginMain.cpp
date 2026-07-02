#include "ReqQueuePlugin.h"

CReqQueuePlugin* g_pMyPlugin = NULL;

extern "C"
{
    void __declspec(dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn** ppPlugInInstance)
    {
        *ppPlugInInstance = g_pMyPlugin = new CReqQueuePlugin();
    }

    void __declspec(dllexport) EuroScopePlugInExit(void)
    {
        delete g_pMyPlugin;
        g_pMyPlugin = NULL;
    }
}
