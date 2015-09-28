#include "cxscmpfr.h"
#include <windows.h>

// a sample exported function
//void DLL_EXPORT SomeFunction(const LPCSTR sometext)
//{
//    MessageBoxA(0, sometext, "DLL Message", MB_OK | MB_ICONINFORMATION);
//}

extern "C" CXSC_MPFR_API BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process
            // return FALSE to fail DLL load
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
