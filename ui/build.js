const path = require("path");

// Resolve esbuild from lv_binding_js's node_modules
const LV_BINDING_DIR = path.resolve(__dirname, "../deps/lv_binding_js");
const esbuild = require(path.join(LV_BINDING_DIR, "node_modules/esbuild"));

esbuild
    .build({
        entryPoints: [path.resolve(__dirname, "PluginUI.tsx")],
        bundle: true,
        platform: "neutral",
        external: ["tjs:path"],
        outfile: path.resolve(__dirname, "bundle.js"),
        nodePaths: [path.join(LV_BINDING_DIR, "node_modules")],
        define: {
            "process.env.NODE_ENV": '"production"',
        },
    })
    .then(() => console.log("UI bundle built: ui/bundle.js"))
    .catch((e) => {
        console.error(e);
        process.exit(1);
    });
