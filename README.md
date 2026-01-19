# My Windhawk Mods
My collection of Windhawk mods for tweaking behaviours and UI of Microsoft Windows and apps.

---

## Confirm Closing Multiple Tabs in File Explorer
This mod adds a confirmation dialog that spawns when you attempt to close a File Explorer window
with multiple tabs open, preventing accidental closure of all tabs.

[Install this mod from the Windhawk marketplace](https://windhawk.net/mods/confirm-closing-multiple-explorer-tabs)
| [C++ source code](https://github.com/AromaKitsune/My-Windhawk-Mods/blob/main/mods/confirm-closing-multiple-explorer-tabs.wh.cpp)

![Preview](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/confirm-closing-multiple-explorer-tabs.png)

### Configuration
**Default button**: Choose whether "Close Tabs" or "Cancel" is the default button in the confirmation dialog.

---

## Notepad++ UI Tweaks
This mod applies UI tweaks to Notepad++ to improve usability.

### Remove Border from Scintilla
Removes the border from the main text editing area (Scintilla control).

**Why?**: When the border is visible, a 1px gap exists between the vertical scroll bar and the right edge of the screen in a maximised window.
This prevents you from simply flicking your mouse cursor to the right screen edge to grab the scroll bar.
Removing the border eliminates this gap, making the scroll bar easier to grab.

### Remove InfoTip from Document List
Removes the tooltip (InfoTip) that appears when hovering over files in the "Document List" panel.

**Why?**: The default InfoTip can be intrusive; if you hover over file item 1, the tooltip often appears over file item 2, obscuring it.

Additionally, on Windows 11, this prevents the mouse hover effect from triggering on the obscured item, making the list feel unresponsive to cursor movement.
Curiously, this issue also affects several items below it (e.g., items 3, 4, 5), even though the tooltip is only obscuring item 2.
This is a system-wide bug that affects `SysListView32` controls (not just in Notepad++) and does not occur in Windows 10 or earlier.
Removing the InfoTip solves this obstruction.

[C++ source code](https://github.com/AromaKitsune/My-Windhawk-Mods/blob/main/mods/notepad-plus-plus-ui-tweaks.wh.cpp)

---

## How to install these Mods
First, install [Windhawk](https://windhawk.net/) and pick one method below to install mods:

### via Windhawk marketplace (recommended)
1. Launch Windhawk.
2. Click the "Explore" button.
3. Search one of my mods by typing a mod name shown on this GitHub repo.
4. Click "Details".
5. Click "Install", and the mod is installed.

### Manual installation
Install mods locally only if those mods are not on the Windhawk marketplace.
1. Launch Windhawk.
2. Click "Create a New Mod" and clear everything in the text editor.
3. Copy the C++ code from this GitHub repo.
4. Paste the C++ code into the text editor.
5. Click "Compile Mod", then "Exit Editing Mode", and the mod is installed.
