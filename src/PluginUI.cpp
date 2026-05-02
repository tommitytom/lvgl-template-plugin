/*
 * LVGL plugin example
 * Copyright (C) 2021 Jean Pierre Cimalando <jp-dev@inbox.ru>
 * Copyright (C) 2021-2022 Filipe Coelho <falktx@falktx.com>
 * SPDX-License-Identifier: ISC
 */

#include "DistrhoUI.hpp"
#include "ResizeHandle.hpp"
#include "LvglJsEngine.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------

class LVGLPluginUI : public UI
{
    float fGain = 0.0f;
    ResizeHandle fResizeHandle;
    LvglJsEngine jsEngine;

    // ----------------------------------------------------------------------------------------------------------------

public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */
    LVGLPluginUI()
        : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT),
          fResizeHandle(this)
    {
        const double scaleFactor = getScaleFactor();

        if (d_isNotEqual(scaleFactor, 1.0))
        {
            setGeometryConstraints(DISTRHO_UI_DEFAULT_WIDTH * scaleFactor, DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor);
            setSize(DISTRHO_UI_DEFAULT_WIDTH * scaleFactor, DISTRHO_UI_DEFAULT_HEIGHT * scaleFactor);
        }
        else
        {
            setGeometryConstraints(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT);
        }

        // hide handle if UI is resizable
        if (isResizable())
            fResizeHandle.hide();

        // Initialize JS engine and load React UI bundle
        if (jsEngine.init())
        {
            // TODO: For production, embed the JS bundle as bytecode instead
            // of loading from disk. For now, use an absolute path.
            const char* bundlePath = "/home/tommitytom/code/thirdparty/lvgl-template-plugin/ui/bundle.js";
            if (jsEngine.evalModule(bundlePath) != 0)
                d_stderr("Failed to load %s", bundlePath);
            else
                d_stdout("LvglJsEngine: React UI loaded");
        }
        else
        {
            d_stderr("Failed to initialize LvglJsEngine");
        }
    }

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // DSP/Plugin Callbacks

   /**
      A parameter has changed on the plugin side.@n
      This is called by the host to inform the UI about parameter changes.
    */
    void parameterChanged(uint32_t index, float value) override
    {
        DISTRHO_SAFE_ASSERT_RETURN(index == 0,);

        fGain = value;
        repaint();
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Widget Callbacks

    void uiIdle() override
    {
        jsEngine.tick();
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LVGLPluginUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new LVGLPluginUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
