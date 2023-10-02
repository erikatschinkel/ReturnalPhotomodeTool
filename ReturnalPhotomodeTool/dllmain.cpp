#include <Windows.h>
#include <memory>
#include <stdio.h>

#include "MinHook.h"

#pragma comment(lib, "libMinHook.x64.lib")

static const char* g_gameName = "Returnal";
static const char* g_moduleName = "Returnal-Win64-Shipping.exe";
static const char* g_dllModuleName = "Returnal-Returnal-Win64-Shipping.dll";
static const char* g_engineDllModuleName = "Returnal-Engine-Win64-Shipping.dll";
static const char* g_className = "UnrealWindow";
static const char* g_windowTitle = "Returnal";

HINSTANCE g_gameHandle = NULL;
HINSTANCE g_dllHandle = NULL;
HINSTANCE g_gameDllHandle = NULL;
HINSTANCE g_engineDllHandle = NULL;
HWND g_gameHwnd = NULL;

bool g_shutdown = false;

MH_STATUS status;

class APhotoModeActor {
public:
    __int64 vtablePtr;
    char Pad008[0xe4];
    bool bActorEnableCollision;
    char Pad0ec[0x23c];
    float LeashDistance;

};

class UCameraComponent {
public:
    char Pad000[0x294];
    bool bConstrainAspectRatio;
};

struct FMinimalViewInfo {
    int dummy;
};

typedef void (__fastcall* tTick)(APhotoModeActor*, float, double, double);
typedef void(__fastcall* tGetCameraView)(UCameraComponent*, float, FMinimalViewInfo*);

tTick oTick = nullptr;
tGetCameraView oGetCameraView = nullptr;

void __fastcall hTick(APhotoModeActor* pThis, float a1, double a2, double a3) {

    pThis->bActorEnableCollision = false;
    pThis->LeashDistance = 100000000.00;

    return oTick(pThis, a1, a2, a3);
}

void __fastcall hGetCameraView(UCameraComponent* pThis, float a1, FMinimalViewInfo* a2) {

    pThis->bConstrainAspectRatio = false;

    return oGetCameraView(pThis, a1, a2);
}


void Hook() {

    status = MH_Initialize();
    if (status != MH_OK)
        printf("Failed to initialize MinHook, MH_STATUS 0x%X\n", status);

    __int64 Tick = (__int64)(::GetProcAddress(g_dllHandle, "?Tick@APhotoModeActor@@UEAAXM@Z"));
    __int64 GetCameraView = (__int64)(::GetProcAddress(g_engineDllHandle, "?GetCameraView@UCameraComponent@@UEAAXMAEAUFMinimalViewInfo@@@Z"));

    MH_CreateHook((LPVOID)(Tick), (LPVOID)(hTick), (LPVOID*)(&oTick));
    MH_EnableHook((LPVOID)(Tick));
    MH_CreateHook((LPVOID)(GetCameraView), (LPVOID)(hGetCameraView), (LPVOID*)(&oGetCameraView));
    MH_EnableHook((LPVOID)(GetCameraView));

}

bool Initialize() {

    AllocConsole();
    FILE* pOut;
    freopen_s(&pOut, "CONOUT$", "w", stdout);
    printf("Returnal Photomode Tool Console Initialized\n");

    g_gameHwnd = FindWindowA(g_className, NULL);
    if (g_gameHwnd == NULL)
    {
        g_gameHwnd = FindWindowA(NULL, g_windowTitle);
        if (g_gameHwnd == NULL)
        {
            printf("Failed to retrieve window handle, GetLastError 0x%X\n", GetLastError());
            return false;
        }
    }

    g_gameHandle = ::GetModuleHandleA(g_moduleName);
    if (g_gameHandle == NULL)
    {
        printf("Failed to retrieve module handle, GetLastError 0x%X\n", GetLastError());
        return false;
    }

    g_dllHandle = ::GetModuleHandleA(g_dllModuleName);
    if (g_dllHandle == NULL)
    {
        printf("Failed to retrieve dll module handle, GetLastError 0x%X\n", GetLastError());
        return false;
    }

    g_engineDllHandle = ::GetModuleHandleA(g_engineDllModuleName);
    if (g_engineDllHandle == NULL)
    {
        printf("Failed to retrieve dll module handle, GetLastError 0x%X\n", GetLastError());
        return false;
    }

    Hook();
    if (status != MH_OK) {

        printf("Hook Failed, GetLastError 0x%X\n", GetLastError());
        return false;
    }

    return true;
}


DWORD WINAPI RunCT(LPVOID lpArg)
{
    Initialize();

    return 0;
}

DWORD WINAPI DllMain(_In_ HINSTANCE hInstance, _In_ DWORD fdwReason, _In_ LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        g_dllHandle = hInstance;
        CreateThread(NULL, NULL, RunCT, NULL, NULL, NULL);
    }

    return 1;
}

