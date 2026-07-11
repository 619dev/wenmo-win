#include <msctf.h>
#include <string>
#include <windows.h>

#include "Globals.hpp"

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) { g_module = module; DisableThreadLibraryCalls(module); }
    return TRUE;
}

namespace {
HRESULT register_com(bool install) {
    wchar_t clsid[40]{}; StringFromGUID2(CLSID_WenmoTextService, clsid, 40);
    const std::wstring key = std::wstring(L"Software\\Classes\\CLSID\\") + clsid;
    if (!install) return HRESULT_FROM_WIN32(RegDeleteTreeW(HKEY_LOCAL_MACHINE, key.c_str()));
    wchar_t module[MAX_PATH]{}; if (!GetModuleFileNameW(g_module, module, MAX_PATH)) return HRESULT_FROM_WIN32(GetLastError());
    HKEY root = nullptr; LONG error = RegCreateKeyExW(HKEY_LOCAL_MACHINE, key.c_str(), 0, nullptr, 0, KEY_WRITE, nullptr, &root, nullptr);
    if (error != ERROR_SUCCESS) return HRESULT_FROM_WIN32(error);
    RegSetValueExW(root, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(kServiceName), sizeof(kServiceName));
    HKEY server = nullptr; error = RegCreateKeyExW(root, L"InprocServer32", 0, nullptr, 0, KEY_WRITE, nullptr, &server, nullptr);
    if (error == ERROR_SUCCESS) { RegSetValueExW(server, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(module), static_cast<DWORD>((wcslen(module) + 1) * sizeof(wchar_t))); constexpr wchar_t model[] = L"Apartment"; RegSetValueExW(server, L"ThreadingModel", 0, REG_SZ, reinterpret_cast<const BYTE*>(model), sizeof(model)); RegCloseKey(server); }
    RegCloseKey(root); return HRESULT_FROM_WIN32(error);
}

HRESULT register_profile(bool install) {
    ITfInputProcessorProfiles* profiles = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&profiles));
    if (FAILED(hr)) return hr;
    if (install) { hr = profiles->Register(CLSID_WenmoTextService); if (SUCCEEDED(hr)) hr = profiles->AddLanguageProfile(CLSID_WenmoTextService, MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), GUID_WenmoProfile, kServiceName, static_cast<ULONG>(wcslen(kServiceName)), nullptr, 0, 0); }
    else { profiles->RemoveLanguageProfile(CLSID_WenmoTextService, MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED), GUID_WenmoProfile); hr = profiles->Unregister(CLSID_WenmoTextService); }
    profiles->Release(); return hr;
}
}  // namespace

extern "C" HRESULT __stdcall DllRegisterServer() { HRESULT hr = register_com(true); if (FAILED(hr)) return hr; CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); hr = register_profile(true); CoUninitialize(); if (FAILED(hr)) register_com(false); return hr; }
extern "C" HRESULT __stdcall DllUnregisterServer() { CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); register_profile(false); CoUninitialize(); const HRESULT hr = register_com(false); return hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) ? S_OK : hr; }
