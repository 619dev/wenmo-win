#include <unknwn.h>
#include <windows.h>

#include "Globals.hpp"
#include "TextService.hpp"

class ClassFactory final : public IClassFactory {
public:
    ClassFactory() { InterlockedIncrement(&g_object_count); }
    ~ClassFactory() override { InterlockedDecrement(&g_object_count); }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override {
        if (!object) return E_INVALIDARG; *object = nullptr;
        if (iid == IID_IUnknown || iid == IID_IClassFactory) *object = static_cast<IClassFactory*>(this);
        if (!*object) return E_NOINTERFACE; AddRef(); return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return static_cast<ULONG>(InterlockedIncrement(&refs_)); }
    ULONG STDMETHODCALLTYPE Release() override { const auto refs = InterlockedDecrement(&refs_); if (!refs) delete this; return static_cast<ULONG>(refs); }
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* outer, REFIID iid, void** object) override {
        if (outer) return CLASS_E_NOAGGREGATION;
        auto* service = new (std::nothrow) TextService(); if (!service) return E_OUTOFMEMORY;
        const HRESULT hr = service->QueryInterface(iid, object); service->Release(); return hr;
    }
    HRESULT STDMETHODCALLTYPE LockServer(BOOL lock) override { lock ? InterlockedIncrement(&g_object_count) : InterlockedDecrement(&g_object_count); return S_OK; }
private: long refs_ = 1;
};

extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID clsid, REFIID iid, void** object) {
    if (clsid != CLSID_WenmoTextService) return CLASS_E_CLASSNOTAVAILABLE;
    auto* factory = new (std::nothrow) ClassFactory(); if (!factory) return E_OUTOFMEMORY;
    const HRESULT hr = factory->QueryInterface(iid, object); factory->Release(); return hr;
}
extern "C" HRESULT __stdcall DllCanUnloadNow() { return g_object_count == 0 ? S_OK : S_FALSE; }
