# My Windhawk Mods
My collection of Windhawk mods for tweaking behaviours and UI of Microsoft
Windows and apps.

## Table of contents
* Mods
  * [Confirm Closing Multiple Tabs in File Explorer](#confirm-closing-multiple-tabs-in-file-explorer)
  * [Disk Usage Bar in Drive Properties](#disk-usage-bar-in-drive-properties)
  * [Restore AutoRun Icon in Drive Properties](#restore-autorun-icon-in-drive-properties)
  * [Notepad++ UI Tweaks](#notepad-ui-tweaks)
* Info
  * [How to install these Mods](#how-to-install-these-mods)

---

## Confirm Closing Multiple Tabs in File Explorer
This mod adds a confirmation dialog that spawns when you attempt to close a File
Explorer window with multiple tabs open, preventing accidental closure of all
tabs.

[Install this mod from the Windhawk marketplace](https://windhawk.net/mods/confirm-closing-multiple-explorer-tabs)
| [C++ source code](/mods/confirm-closing-multiple-explorer-tabs.wh.cpp)

![](/screenshots/confirm-closing-multiple-explorer-tabs.png)

### Configuration
**Default button**: Choose whether "Close Tabs" or "Cancel" is the default
button in the confirmation dialog.

---

## Disk Usage Bar in Drive Properties
This mod replaces the disk usage pie/donut chart in the drive properties dialog
with a usage bar.

[Install this mod from the Windhawk marketplace](https://windhawk.net/mods/disk-usage-bar-in-drive-properties)
| [C++ source code](/mods/disk-usage-bar-in-drive-properties.wh.cpp)

![](/screenshots/disk-usage-bar-in-drive-properties.png)

### Features
* Replaces the pie/donut chart with a blue usage bar, like in "This PC".
* Switches the bar colour to red when the disk is almost full.
* Displays the usage percentage text below the bar.

### Supported Windows versions
* Windows 11
* Windows 10
* Windows 8.1

### Configuration
You can enable the option to switch the usage bar colour to red when disk usage
exceeds 90%.

---

Based on the "[Disk Pie Chart](https://windhawk.net/mods/disk-pie-chart)" mod by
**aubymori**.

---

## Restore AutoRun Icon in Drive Properties
Since Windows 2000, the drive properties dialog never displays the AutoRun icon
on the General tab, leaving a blank space.

This mod restores the AutoRun icon back where it belongs.

[Install this mod from the Windhawk marketplace](https://windhawk.net/mods/restore-autorun-icon-in-drive-properties)
| [C++ source code](/mods/restore-autorun-icon-in-drive-properties.wh.cpp)

| Before | After |
| :----: | :---: |
| ![](/screenshots/restore-autorun-icon-in-drive-properties_before.png) | ![](/screenshots/restore-autorun-icon-in-drive-properties_after.png) |

### Compatibility with other mods
Any other mods that hook the `DrawPie` function will conflict with this mod and
prevent it from functioning.

#### Conflicting mods
* [Disk Usage Bar in Drive Properties](https://windhawk.net/mods/disk-usage-bar-in-drive-properties)
  by me
* [Disk Pie Chart](https://windhawk.net/mods/disk-pie-chart)
  by **aubymori**

**Note:** The "Disk Usage Bar in Drive Properties" mod already has the AutoRun
icon restoration code integrated, so you don't need to use both mods together.

### Supported Windows versions
* Windows 11
* Windows 10
* Windows 8.1

---

## Notepad++ UI Tweaks
This mod applies UI tweaks to Notepad++ to improve usability.

[C++ source code](/mods/notepad-plus-plus-ui-tweaks.wh.cpp)

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

---

## How to install these Mods
First, install [Windhawk](https://windhawk.net/) and pick one method below to install mods:

### From the Windhawk marketplace
Recommended.
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
