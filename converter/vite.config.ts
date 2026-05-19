import { defineConfig } from "vite";

export default defineConfig({
    optimizeDeps: {
        exclude: ["@ffmpeg/ffmpeg"]
    },
    server: {
        headers: {
            "Cross-Origin-Opener-Policy": "same-origin",
            "Cross-Origin-Embedder-Policy": "require-corp"
        }
    }
});