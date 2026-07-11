#pragma once

#include <msctf.h>
#include <memory>

#include "wenmo/InputEngine.hpp"

class TextService final : public ITfTextInputProcessor, public ITfKeyEventSink {
public:
    TextService();
    ~TextService() override;

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;
    HRESULT STDMETHODCALLTYPE Activate(ITfThreadMgr* thread_mgr, TfClientId client_id) override;
    HRESULT STDMETHODCALLTYPE Deactivate() override;
    HRESULT STDMETHODCALLTYPE OnSetFocus(BOOL foreground) override;
    HRESULT STDMETHODCALLTYPE OnTestKeyDown(ITfContext*, WPARAM key, LPARAM, BOOL* eaten) override;
    HRESULT STDMETHODCALLTYPE OnTestKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* eaten) override;
    HRESULT STDMETHODCALLTYPE OnKeyDown(ITfContext* context, WPARAM key, LPARAM, BOOL* eaten) override;
    HRESULT STDMETHODCALLTYPE OnKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* eaten) override;
    HRESULT STDMETHODCALLTYPE OnPreservedKey(ITfContext*, REFGUID, BOOL* eaten) override;

private:
    class EditSession;
    bool handles(WPARAM key) const;
    HRESULT request_edit(ITfContext* context, WPARAM key);

    long refs_ = 1;
    TfClientId client_id_ = TF_CLIENTID_NULL;
    ITfThreadMgr* thread_mgr_ = nullptr;
    std::unique_ptr<wenmo::InputEngine> engine_;
};
