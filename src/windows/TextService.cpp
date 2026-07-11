#include "TextService.hpp"

#include <filesystem>
#include <string>

#include "Globals.hpp"

namespace {
std::filesystem::path dictionary_path() {
    wchar_t path[MAX_PATH]{};
    const DWORD length = GetModuleFileNameW(g_module, path, MAX_PATH);
    if (length == 0 || length == MAX_PATH) return {};
    return std::filesystem::path(path).parent_path() / L"resources" / L"cedict_pinyin.tsv";
}

std::wstring utf8_to_wide(const std::string& value) {
    if (value.empty()) return {};
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                                           static_cast<int>(value.size()), nullptr, 0);
    if (count <= 0) return {};
    std::wstring result(static_cast<size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                        static_cast<int>(value.size()), result.data(), count);
    return result;
}
}  // namespace

class TextService::EditSession final : public ITfEditSession {
public:
    EditSession(TextService* service, ITfContext* context, WPARAM key)
        : service_(service), context_(context), key_(key) {
        service_->AddRef(); context_->AddRef();
    }
    ~EditSession() override { context_->Release(); service_->Release(); }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override {
        if (!object) return E_INVALIDARG;
        *object = nullptr;
        if (iid == IID_IUnknown || iid == IID_ITfEditSession) *object = static_cast<ITfEditSession*>(this);
        if (!*object) return E_NOINTERFACE;
        AddRef(); return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return static_cast<ULONG>(InterlockedIncrement(&refs_)); }
    ULONG STDMETHODCALLTYPE Release() override {
        const auto refs = InterlockedDecrement(&refs_); if (!refs) delete this; return static_cast<ULONG>(refs);
    }
    HRESULT STDMETHODCALLTYPE DoEditSession(TfEditCookie cookie) override {
        if (key_ >= 'A' && key_ <= 'Z') { service_->engine_->type(static_cast<char>(key_ - 'A' + 'a')); return S_OK; }
        if (key_ == VK_BACK) { service_->engine_->backspace(); return S_OK; }
        if (key_ != VK_SPACE && key_ != VK_RETURN) return S_OK;
        const auto candidates = service_->engine_->candidates();
        if (candidates.empty()) return S_OK;
        ITfInsertAtSelection* insertion = nullptr;
        HRESULT hr = context_->QueryInterface(IID_PPV_ARGS(&insertion));
        if (SUCCEEDED(hr)) {
            const auto text = utf8_to_wide(candidates.front());
            ITfRange* range = nullptr;
            hr = insertion->InsertTextAtSelection(cookie, TF_IAS_NOQUERY, text.c_str(),
                                                   static_cast<LONG>(text.size()), &range);
            if (range) range->Release();
            insertion->Release();
            if (SUCCEEDED(hr)) service_->engine_->clear();
        }
        return hr;
    }
private:
    long refs_ = 1;
    TextService* service_;
    ITfContext* context_;
    WPARAM key_;
};

TextService::TextService() : engine_(std::make_unique<wenmo::InputEngine>(dictionary_path())) {
    InterlockedIncrement(&g_object_count);
}
TextService::~TextService() { Deactivate(); InterlockedDecrement(&g_object_count); }
HRESULT TextService::QueryInterface(REFIID iid, void** object) {
    if (!object) return E_INVALIDARG; *object = nullptr;
    if (iid == IID_IUnknown || iid == IID_ITfTextInputProcessor) *object = static_cast<ITfTextInputProcessor*>(this);
    else if (iid == IID_ITfKeyEventSink) *object = static_cast<ITfKeyEventSink*>(this);
    if (!*object) return E_NOINTERFACE; AddRef(); return S_OK;
}
ULONG TextService::AddRef() { return static_cast<ULONG>(InterlockedIncrement(&refs_)); }
ULONG TextService::Release() { const auto refs = InterlockedDecrement(&refs_); if (!refs) delete this; return static_cast<ULONG>(refs); }
HRESULT TextService::Activate(ITfThreadMgr* manager, TfClientId id) {
    if (!manager) return E_INVALIDARG; thread_mgr_ = manager; thread_mgr_->AddRef(); client_id_ = id;
    ITfKeystrokeMgr* keys = nullptr; HRESULT hr = manager->QueryInterface(IID_PPV_ARGS(&keys));
    if (SUCCEEDED(hr)) { hr = keys->AdviseKeyEventSink(id, this, TRUE); keys->Release(); }
    return hr;
}
HRESULT TextService::Deactivate() {
    if (thread_mgr_) { ITfKeystrokeMgr* keys = nullptr; if (SUCCEEDED(thread_mgr_->QueryInterface(IID_PPV_ARGS(&keys)))) { keys->UnadviseKeyEventSink(client_id_); keys->Release(); } thread_mgr_->Release(); thread_mgr_ = nullptr; }
    client_id_ = TF_CLIENTID_NULL; engine_->clear(); return S_OK;
}
HRESULT TextService::OnSetFocus(BOOL) { return S_OK; }
bool TextService::handles(WPARAM key) const { return (key >= 'A' && key <= 'Z') || ((key == VK_BACK || key == VK_SPACE || key == VK_RETURN) && !engine_->composition().empty()); }
HRESULT TextService::OnTestKeyDown(ITfContext*, WPARAM key, LPARAM, BOOL* eaten) { if (!eaten) return E_INVALIDARG; *eaten = handles(key); return S_OK; }
HRESULT TextService::OnTestKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* eaten) { if (!eaten) return E_INVALIDARG; *eaten = FALSE; return S_OK; }
HRESULT TextService::request_edit(ITfContext* context, WPARAM key) {
    auto* edit = new (std::nothrow) EditSession(this, context, key); if (!edit) return E_OUTOFMEMORY;
    HRESULT session = E_FAIL; const HRESULT hr = context->RequestEditSession(client_id_, edit, TF_ES_SYNC | TF_ES_READWRITE, &session); edit->Release();
    return FAILED(hr) ? hr : session;
}
HRESULT TextService::OnKeyDown(ITfContext* context, WPARAM key, LPARAM, BOOL* eaten) { if (!eaten || !context) return E_INVALIDARG; *eaten = handles(key); return *eaten ? request_edit(context, key) : S_OK; }
HRESULT TextService::OnKeyUp(ITfContext*, WPARAM, LPARAM, BOOL* eaten) { if (!eaten) return E_INVALIDARG; *eaten = FALSE; return S_OK; }
HRESULT TextService::OnPreservedKey(ITfContext*, REFGUID, BOOL* eaten) { if (!eaten) return E_INVALIDARG; *eaten = FALSE; return S_OK; }
