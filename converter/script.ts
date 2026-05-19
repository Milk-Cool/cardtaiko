import * as zip from "@zip.js/zip.js";
import { FFmpeg } from "@ffmpeg/ffmpeg";
import { toBlobURL } from "@ffmpeg/util";

const btn = document.querySelector("#convert") as HTMLButtonElement;
const ffmpeg = new FFmpeg();

btn.addEventListener("click", async () => {
    const { files } = (document.querySelector("#files") as HTMLInputElement);
    if(files === null) return alert("no files were uploaded!!");

    const outData = new zip.Data64URIWriter("application/zip");
    const out = new zip.ZipWriter(outData);

    let pushed: string[] = [];

    for(const file of files) {
        const reader = new zip.ZipReader(new zip.BlobReader(file));
        const entries = await reader.getEntries();
        
        for(const entry of entries) {
            if(entry.directory) continue;
            const aname = file.name.replace(".osz", "") + "/audio.mp3", fname = file.name.replace(".osz", "") + "/" + entry.filename;
            if((entry.filename.endsWith(".mp3") || entry.filename.endsWith(".flac") || entry.filename.endsWith(".wav") || entry.filename.endsWith(".m4a") || entry.filename.endsWith(".ogg")) && !pushed.includes(aname)) {
                const inp = entry.filename === "audio.mp3" ? "audio2.mp3" : entry.filename;
                await ffmpeg.writeFile(inp, await (entry as zip.FileEntry).getData(new zip.Uint8ArrayWriter()));
                await ffmpeg.exec(["-i", inp, "-ar", "8000", "-ac", "1", "-codec:a", "libmp3lame", "-b:a", "64k", "audio.mp3"]);
                const data = await ffmpeg.readFile("audio.mp3");
                await out.add(aname, data instanceof Uint8Array ? new zip.Uint8ArrayReader(data) : new zip.TextReader(data));
                pushed.push(aname);
            } else if(entry.filename.endsWith(".osu")) {
                await out.add(fname, new zip.Uint8ArrayReader(await (entry as zip.FileEntry).getData(new zip.Uint8ArrayWriter())));
                pushed.push(fname);
            }
        }
    }

    await out.close();

    const a = document.createElement("a");
    a.href = await outData.getData();
    a.download = "extract-to-sdcard-slash-taiko-" + Math.floor(Math.random() * 1000000).toString().padStart(6, "0") + ".zip";
    a.click();
    setTimeout(() => a.remove(), 1000);
});

(async () => {
    const base = "https://cdn.jsdelivr.net/npm/@ffmpeg/core@0.12.10/dist/esm";
    await ffmpeg.load({
        coreURL: await toBlobURL(base + "/ffmpeg-core.js", "text/javascript"),
        wasmURL: await toBlobURL(base + "/ffmpeg-core.wasm", "application/wasm")
    });
    btn.disabled = false;
})();