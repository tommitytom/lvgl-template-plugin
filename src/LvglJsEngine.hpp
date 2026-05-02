#pragma once

extern "C" {
    #include "tjs.h"
    #include "private.h"
    #include "lvgl.h"
}

#include "native/components/window/window.hpp"

// Extends lv_binding_js display data with the TJS runtime pointer.
// Stored on each LVGL display via lv_display_set_user_data().
struct DpfJsDisplayData : LvBindingJsDisplayData {
    TJSRuntime* runtime = nullptr;

    static DpfJsDisplayData* get() {
        lv_display_t* disp = lv_display_get_default();
        return disp ? static_cast<DpfJsDisplayData*>(lv_display_get_user_data(disp)) : nullptr;
    }
};

class LvglJsEngine {
public:
    LvglJsEngine();
    ~LvglJsEngine();

    bool init();
    void tick();
    void shutdown();
    int evalModule(const char* filename);
    int evalString(const char* code);
    JSContext* getContext() const;

private:
    TJSRuntime* qrt = nullptr;
    JSContext* ctx = nullptr;
    DpfJsDisplayData displayData;
    bool initialized = false;
};
