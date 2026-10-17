# Plants vs. Zombies Extracted Assets

This repository contains the extracted raw assets (graphics, audio, animations, and game configurations) from the original macOS version of **Plants vs. Zombies**. 

These assets were extracted from the core game archive (`main.pak`) using a custom Python decryption/unpacking script.

---

## 📁 Repository Structure

The assets are organized into the following directories:

*   **`images/`**: The complete visual asset library of the game, including UI panels, backgrounds, levels, plants, zombies, and effects in standard `.png` and `.jpg` format.
*   **`sounds/`**: Sound effects in `.ogg` format, and background tracks/music in tracker `.mo3` format (which can be played using audio players like XMPlay or decoded to WAV/MP3).
*   **`reanim/`**: Animation files containing bone-sprite mappings and texture coordinates used to drive the smooth, interactive movements of plants, zombies, and characters.
*   **`properties/`**: XML definitions, configurations, gameplay parameters, text strings, and other metadata controlling properties and variables of the game.
*   **`particles/`**: Particle templates defining visual effects like fire, smoke, snow, explosions, and sunshine.
*   **`data/`**: Core system resources, including embedded fonts, base textures, and layout definitions.

---

## 🛠️ How to Use (Game Modding)

In the PopCap **SexyApp Framework** (the engine powering Plants vs. Zombies), if the game doesn't find a `main.pak` file in its resource folder, it will automatically search for loose asset folders (such as `images/`, `sounds/`, `properties/`, etc.) in the same directory.

You can modify these assets directly to create a custom PvZ mod:

1.  **Backup**: Always backup your original game files or your `main.pak` file.
2.  **Edit Assets**: 
    *   **Graphics**: Edit the PNGs/JPGs in `images/` using standard editors like Photoshop, GIMP, or Aseprite. Keep the original filename, dimensions, and transparency settings.
    *   **Configurations**: Open the XMLs in `properties/` with any text editor to modify game strings, stats, or particle behavior.
    *   **Audio**: Replace OGG tracks in `sounds/` with your own sound effects (keep filenames identical).
3.  **Place Loose Folders**: Move or copy your modified folders (`images/`, `properties/`, etc.) into the game's resource directory:
    *   **macOS Path**: `/Applications/Plants vs. Zombies.app/Contents/Resources/`
4.  **Rename/Remove `main.pak`**: Rename `main.pak` in that directory to something like `main.pak.bak` (or move it out of the folder).
5.  **Run the Game**: Start Plants vs. Zombies. The game will boot up using your loose, customized assets!
