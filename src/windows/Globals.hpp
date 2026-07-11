#pragma once

#include <windows.h>

extern HINSTANCE g_module;
extern long g_object_count;
extern const CLSID CLSID_WenmoTextService;
extern const GUID GUID_WenmoProfile;

constexpr wchar_t kServiceName[] = L"问墨拼音";
