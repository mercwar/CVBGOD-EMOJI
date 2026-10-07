<a target="_self" title="CLICK HERE TO ENTER THE MERCWAR GATEWAY FREE!" href="https://mercwar.github.io/Constellation/index.html">
<img 
    src="https://raw.githubusercontent.com/mercwar/Robo-Knight-Gallery/refs/heads/main/Version%207/image_d2a07390.png" 
    alt="Mercwar Constellation Gateway" 
    style="width:100%; height:auto;"
/>
</a>

#

🌈 # CVBGOD-EMOJI

#### 🗺️ This program runs in your Win64 tray and lets you copy emoji from a list

 - ###### 🪽️ CVBGOD-EMOJI** is a high-performance, tactical Sci-Fi / MechWarrior-themed Windows desktop utility.
 - ###### 🪟️ Designed for lightning-fast emoji browsing, filtering, and clipboard deployment.
 - ###### 🗽️ Built natively in C using the Win32 API and an embedded high-compatibility browser  control.
 - ###### 🚅️ Provides a sleek futuristic HUD for managing Unicode symbols and their hex codes.

<a target="_self" title="CLICK HERE TO ENTER THE MERCWAR GATEWAY FREE!" href="https://github.com/mercwar/CVBGOD-EMOJI/raw/refs/heads/main/CVBGOD-EMOJI.zip">
<img 
    src="ChatGPT%20Image%20Oct%207%2C%202026%2C%2005_43_10%20PM.png" 
    alt="Mercwar FREE Emojiy" 
    style="width:100%; height:auto;"
/>
</a>






---

## 🚀 Key Features

* **Tactical Neon HUD Interface:** Dark-mode cyber-styled aesthetic complete with grid layouts, glowing highlights, and monospaced tech accents.
* **Right-Click Context Menu:** Instantly summon a custom tactical context menu by right-clicking any emoji to copy either:
  * 📋 **The Raw Emoji**
  * 🔢 **The Hexadecimal Code (`U+XXXX`)**
* **System Tray Integration:** Runs quietly in the background. Closing the main application window minimizes it directly to the system tray, with quick-access controls to **Show Window** or **Exit**.
* **Smart Filtering:** Toggle between viewing all code points or filtering strictly for known active emojis.
* **Auto-Navigation:** Automatically jumps straight to the first active page of emojis upon program launch.
* **Custom Branding & Executable Properties:** Ships with customized metadata, version info, and a custom application icon (`me.ico`).

---

## 🛠️ Project Structure

* `main.c` — Win32 application window loop, toolbar controls, system tray management, and event routing.
* `browser_host.c` — COM/OLE initialization, WebBrowser control host wrapper, and absolute URI file navigation.
* `emoji_core.c` — Unicode generation logic, pagination routines, and HTML page builder.
* `version.rc` — Application resource file embedding version information, copyright details, and `me.ico`.
* `favicon.ico` — Custom application icon asset.
* `build.bat` — Automated MSVC compilation script.

---

## ⚙️ Building from Source

To compile the executable (`CVBGOD-EMOJI.exe`) from scratch, ensure you have **Visual Studio** installed with the C++ workload:

1. Open the **Developer Command Prompt for VS**.
2. Navigate to your project source directory containing all source files and `me.ico`.
3. Run the automated build script:

#

   ```
**********************************************************************
** Visual Studio 2022 Developer Command Prompt v17.14.39
** Copyright (c) 2025 Microsoft Corporation
**********************************************************************

C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools>build_emoji.bat

```

The script will automatically compile the resource file (`rc`), compile the C sources (`cl`), and link them into `CVBGOD-EMOJI.exe`.

---

## 🕹️ Usage

1. Launch **`CVBGOD-EMOJI.exe`**.
2. Use the **`< Prev Emoji`** and **`Next Emoji >`** buttons to navigate through sectors.
3. Check or uncheck **`Emoji Only`** to filter out blank or unassigned blocks.
4. **Right-click** any emoji tile to copy its data straight to your clipboard for instant deployment.
5. Close the window anytime to drop it into the system tray. Right-click the tray icon to restore or terminate the program.

---

## 📜 License

Copyright (C) 2026 CVBGOD / Mercwar. All rights reserved.

