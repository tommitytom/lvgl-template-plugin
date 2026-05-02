import { View, Text, Slider, Button, Render } from "lvgljs-ui";
import React, { useState, useCallback } from "react";

function PluginUI() {
    const [gain, setGain] = useState(0);

    const onGainChange = useCallback((e: any) => {
        setGain(e.value);
    }, []);

    return (
        <View
            style={{
                width: "100%",
                height: "100%",
                "background-color": "#1a1a2e",
                display: "flex",
                "flex-direction": "column",
                "align-items": "center",
                "justify-content": "center",
                padding: 30,
            }}
        >
            <Text
                style={{
                    "text-color": "#e0e0e0",
                    "font-size": 32,
                }}
            >
                LVGL Plugin
            </Text>

            <Text
                style={{
                    "text-color": "#4fc3f7",
                    "font-size": 24,
                    "margin-top": 20,
                }}
            >
                {`Gain: ${gain.toFixed(1)} dB`}
            </Text>

            <Slider
                style={{
                    width: 400,
                    height: 10,
                    "margin-top": 30,
                    "background-color": "#2d2d44",
                }}
                range={[-90, 30]}
                value={gain}
                onChange={onGainChange}
            />

            <View
                style={{
                    display: "flex",
                    "flex-direction": "row",
                    "margin-top": 30,
                }}
            >
                <Button
                    style={{
                        width: 120,
                        height: 50,
                        "background-color": "#4fc3f7",
                        "border-radius": 8,
                    }}
                    onClick={() => setGain(0)}
                >
                    <Text style={{ "text-color": "#1a1a2e", "font-size": 16 }}>
                        Reset
                    </Text>
                </Button>
            </View>

            <Text
                style={{
                    "text-color": "#666680",
                    "font-size": 14,
                    "margin-top": 40,
                }}
            >
                Built with React + LVGL + DPF
            </Text>
        </View>
    );
}

Render.render(<PluginUI />);
