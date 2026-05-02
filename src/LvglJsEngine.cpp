#include "LvglJsEngine.hpp"

extern "C" {
    #include <uv.h>
    #include <quickjs.h>
}

#include "native/bootstrap/render_bootstrap.hpp"
#include "native/components/component.hpp"

// Called by the native component event system (declared in engine.hpp).
// Retrieves the TJSRuntime* for the current plugin instance via the
// LVGL display's user_data — each plugin instance has its own display.
TJSRuntime* GetRuntime() {
    DpfJsDisplayData* data = DpfJsDisplayData::get();
    return data ? data->runtime : nullptr;
}

LvglJsEngine::LvglJsEngine() {}

LvglJsEngine::~LvglJsEngine() {
    shutdown();
}

bool LvglJsEngine::init() {
    if (initialized)
        return true;

    // Initialize txiki.js globals (idempotent if called multiple times)
    static bool tjs_initialized = false;
    if (!tjs_initialized) {
        static char arg0[] = "lvgl-plugin";
        static char* argv[] = { arg0, nullptr };
        TJS_Initialize(1, argv);
        tjs_initialized = true;
    }

    // Create a new JS runtime with default options
    qrt = TJS_NewRuntime();
    if (!qrt)
        return false;

    ctx = TJS_GetJSContext(qrt);

    // Store per-instance data on the LVGL display. This extends
    // LvBindingJsDisplayData (used by lv_binding_js internally for
    // GetWindowInstance()) with the TJS runtime pointer.
    displayData.runtime = qrt;
    lv_display_set_user_data(lv_display_get_default(), &displayData);

    // Set up the lvgljs global symbol and register native LVGL components
    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue render_sym = JS_NewSymbol(ctx, "lvgljs", TRUE);
    JSAtom render_atom = JS_ValueToAtom(ctx, render_sym);
    JSValue render = JS_NewObjectProto(ctx, JS_NULL);

    JS_DefinePropertyValue(ctx, global_obj, render_atom, render, JS_PROP_C_W_E);

    NativeRenderInit(ctx, render);

    JS_FreeAtom(ctx, render_atom);
    JS_FreeValue(ctx, render_sym);
    JS_FreeValue(ctx, global_obj);

    // Initialize the LVGL window object (root container for JS components)
    WindowInit();

    // Start the prepare/check handles so libuv processes JS jobs
    uv_prepare_start(&qrt->jobs.prepare, [](uv_prepare_t* handle) {
        TJSRuntime* qrt = static_cast<TJSRuntime*>(handle->data);
        if (JS_IsJobPending(qrt->rt)) {
            uv_idle_start(&qrt->jobs.idle, [](uv_idle_t*) {});
        }
    });
    uv_unref(reinterpret_cast<uv_handle_t*>(&qrt->jobs.prepare));

    uv_check_start(&qrt->jobs.check, [](uv_check_t* handle) {
        TJSRuntime* qrt = static_cast<TJSRuntime*>(handle->data);
        tjs__execute_jobs(qrt->ctx);
        if (!JS_IsJobPending(qrt->rt)) {
            uv_idle_stop(&qrt->jobs.idle);
        }
    });
    uv_unref(reinterpret_cast<uv_handle_t*>(&qrt->jobs.check));

    initialized = true;
    return true;
}

void LvglJsEngine::tick() {
    if (!initialized)
        return;

    uv_run(TJS_GetLoop(qrt), UV_RUN_NOWAIT);
    tjs__execute_jobs(ctx);
}

void LvglJsEngine::shutdown() {
    if (!initialized)
        return;

    initialized = false;

    lv_display_t* disp = lv_display_get_default();
    if (disp && lv_display_get_user_data(disp) == &displayData)
        lv_display_set_user_data(disp, nullptr);

    displayData.runtime = nullptr;
    displayData.windowInstance = nullptr;

    TJS_FreeRuntime(qrt);
    qrt = nullptr;
    ctx = nullptr;
}

int LvglJsEngine::evalModule(const char* filename) {
    if (!initialized)
        return -1;

    JSValue result = TJS_EvalModule(ctx, filename, true);
    if (JS_IsException(result)) {
        tjs_dump_error(ctx);
        JS_FreeValue(ctx, result);
        return -1;
    }
    JS_FreeValue(ctx, result);
    return 0;
}

int LvglJsEngine::evalString(const char* code) {
    if (!initialized)
        return -1;

    JSValue result = JS_Eval(ctx, code, strlen(code), "<eval>",
                             JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_STRICT);
    if (JS_IsException(result)) {
        tjs_dump_error(ctx);
        JS_FreeValue(ctx, result);
        return -1;
    }
    JS_FreeValue(ctx, result);
    return 0;
}

JSContext* LvglJsEngine::getContext() const {
    return ctx;
}
