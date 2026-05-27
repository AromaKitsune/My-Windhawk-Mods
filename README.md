# My Windhawk Mods
My collection of Windhawk mods for tweaking the behaviours and UI of Microsoft
Windows and various apps.

<h2>List of Mods</h2>
<table>
  <thead>
    <tr>
      <th>Mod</th>
      <th>Description</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>
        <a href="#confirm-closing-multiple-tabs-in-file-explorer">
          Confirm Closing Multiple Tabs in File Explorer
        </a>
      </td>
      <td>
        Shows a confirmation dialog when closing a File Explorer window with
        multiple tabs open
      </td>
    </tr>
    <tr>
      <td>
        <a href="#crystaldiskinfo-smart-auto-refresh">
          CrystalDiskInfo Smart Auto-Refresh
        </a>
      </td>
      <td>
        Temporarily pauses the Auto-Refresh function whenever the application's
        main window is visible
      </td>
    </tr>
    <tr>
      <td>
        <a href="#ctrlbackspace-fix-for-win32-text-boxes">
          Ctrl+Backspace Fix for Win32 Text Boxes
        </a>
      </td>
      <td>
        Makes Ctrl+Backspace delete the previous word in Win32 text boxes
        instead of inserting a control character
      </td>
    </tr>
    <tr>
      <td>
        <a href="#disk-usage-bar-in-drive-properties">
          Disk Usage Bar in Drive Properties
        </a>
      </td>
      <td>
        Replaces the pie/donut chart in drive properties with a usage bar
      </td>
    </tr>
    <tr>
      <td>
        <a href="#never-auto-expand-explorer-tree-items">
          Never Auto-Expand Explorer Tree Items
        </a>
      </td>
      <td>
        Stops the unwanted auto-expansion of navigation pane items even if the
        "Expand to current folder" option is off
      </td>
    </tr>
    <tr>
      <td>
        <a href="#notepad-tweaks">
          Notepad++ Tweaks
        </a>
      </td>
      <td>
        Applies tweaks to Notepad++ to improve usability
      </td>
    </tr>
    <tr>
      <td>
        <a href="#restore-autorun-icon-in-drive-properties">
          Restore AutoRun Icon in Drive Properties
        </a>
      </td>
      <td>
        Restores the missing AutoRun icon in drive properties
      </td>
    </tr>
  </tbody>
</table>

## Info
* [How to install Mods](#how-to-install-mods)

---

## Confirm Closing Multiple Tabs in File Explorer
[Install this mod from the Windhawk marketplace](https://windhawk.net/mods/confirm-closing-multiple-explorer-tabs)
| [C++ source code](/mods/confirm-closing-multiple-explorer-tabs.wh.cpp)

This mod adds a confirmation dialog that spawns when you attempt to close a File
Explorer window with multiple tabs open, preventing accidental closure of all
tabs.

![](/screenshots/confirm-closing-multiple-explorer-tabs.png)

### Configuration
**Default button**: Choose whether "Close Tabs" or "Cancel" is the default
button in the confirmation dialog.

---

## CrystalDiskInfo Smart Auto-Refresh
[Install this mod from the Windhawk marketplace](https://windhawk.net/mods/crystaldiskinfo-smart-auto-refresh)
| [C++ source code](/mods/crystaldiskinfo-smart-auto-refresh.wh.cpp)

CrystalDiskInfo includes an optional Auto-Refresh feature that updates disk
information at specified intervals, which is useful for background monitoring
while the application is minimised to the system tray. However, when the
application's main window is visible, automatic refreshes can interfere with the
analysis of current disk status, as shifting attribute values make tracking
specific metrics more difficult.

This mod temporarily pauses the Auto-Refresh function by blocking its disk
polling cycle whenever the application's main window is visible, allowing for an
uninterrupted analysis of current disk status.

The Auto-Refresh function resumes once the application is minimised to the
taskbar or system tray.

**Note:** If CrystalDiskInfo is already running when the mod is loaded, pick one
of the following options to activate it:
* Restart the application completely.
* Select an interval in **Function** → **Auto Refresh**, including the one
  currently set.

---

## Ctrl+Backspace Fix for Win32 Text Boxes
[Install this mod from the Windhawk marketplace](https://windhawk.net/mods/ctrl-backspace-fix-for-win32-text-boxes)
| [C++ source code](/mods/ctrl-backspace-fix-for-win32-text-boxes.wh.cpp)

Win32 text boxes often lack previous-word deletion functionality, resulting in
the `Ctrl+Backspace` hotkey inserting the `Delete` control character instead of
deleting the previous word.

This mod resolves this behaviour by adding previous-word deletion functionality
to those `Edit` controls.

It also supports wrapped `Edit` controls used in .NET WinForms and Delphi VCL
applications.

| Before |
| :----- |
| ![](/screenshots/ctrl-backspace-fix-for-win32-text-boxes_before.gif) |

| After |
| :---- |
| ![](/screenshots/ctrl-backspace-fix-for-win32-text-boxes_after.gif) |

### Notes
* In hotkey text boxes, pressing `Ctrl+Backspace` deletes the text instead of
  assigning the hotkey.
* In masked text boxes, pressing `Ctrl+Backspace` deletes the placeholder
  characters, breaking the input mask and preventing further input into those
  missing slots.
* Custom-drawn text boxes, such as those in Qt applications, are unaffected
  because they do not utilise standard Win32 `Edit` controls. Most custom-drawn
  UI frameworks already handle their own previous-word deletion functionality.

---

## Disk Usage Bar in Drive Properties
[Install this mod from the Windhawk marketplace](https://windhawk.net/mods/disk-usage-bar-in-drive-properties)
| [C++ source code](/mods/disk-usage-bar-in-drive-properties.wh.cpp)

This mod replaces the disk usage pie/donut chart in the drive properties dialog
with a usage bar.

![](/screenshots/disk-usage-bar-in-drive-properties.png)

### Features
* Replaces the pie/donut chart with a blue usage bar, like in "This PC".
* Switches the bar colour to red when the disk is almost full.
* Displays the disk usage percentage text below the bar.

### Configuration
This mod provides the following options:
* **Show red bar on low space**: Switches the usage bar colour to red when disk
  usage exceeds 90%.
* **Show decimal percentage**: Displays the disk usage percentage text with one
  decimal place (e.g., `64.1%`).
* **Hide storage management button**: Hides the "Details" (Windows 11) or "Disk
  Clean-up" (Windows 8.1/10) button.
  * It is recommended to hide this button for localised systems to prevent a UI
    collision with a long "Space used" string for the disk usage percentage
    text.
  * The `Alt+D` keyboard shortcut remains functional.

### Supported Windows versions
* Windows 11
* Windows 10
* Windows 8.1

---

Based on the "[Disk Pie Chart](https://windhawk.net/mods/disk-pie-chart)" mod by
**aubymori**.

---

## Never Auto-Expand Explorer Tree Items
[Install this mod from the Windhawk marketplace](https://windhawk.net/mods/never-auto-expand-explorer-tree-items)
| [C++ source code](/mods/never-auto-expand-explorer-tree-items.wh.cpp)

File Explorer automatically expands navigation pane items (such as "This PC")
even if the "Expand to current folder" option is off, specifically when:
* Opening any folder inside an external drive in a new tab or window.
* Navigating to any drive after manually expanding and collapsing the "This PC"
  item.

This mod prevents this unwanted auto-expansion behaviour, keeping the navigation
pane tidy.

**Note:** The top-level "Desktop" item can still auto-expand when the
"Show all folders" option is on, keeping the navigation pane populated.

| Before | After |
| :----: | :---: |
| ![](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/never-auto-expand-explorer-tree-items_before.png) | ![](https://raw.githubusercontent.com/AromaKitsune/My-Windhawk-Mods/main/screenshots/never-auto-expand-explorer-tree-items_after.png) |

---

## Notepad++ Tweaks
[C++ source code](/mods/notepad-plus-plus-tweaks.wh.cpp)

This mod applies tweaks to Notepad++ to improve usability.

### Remove Border from Scintilla
Removes the border from the main text editing area (Scintilla control).

**Why?**: When the border is visible, a 1px gap exists between the vertical
scroll bar and the right edge of the screen in a maximised window. This prevents
you from simply flicking your mouse cursor to the right screen edge to grab the
scroll bar. Removing the border eliminates this gap, making the scroll bar
easier to grab.

### Remove InfoTip from Document List
Removes the tooltip (InfoTip) that appears when hovering over files in the
"Document List" panel.

**Why?**: The default InfoTip can be intrusive; if you hover over file item 1,
the tooltip often appears over file item 2, obscuring it.

Additionally, on Windows 11, this prevents the mouse hover effect from
triggering on the obscured item, making the list feel unresponsive to cursor
movement. Curiously, this issue also affects several items below it (e.g., items
3, 4, 5), even though the tooltip is only obscuring item 2. This is a
system-wide bug that affects `SysListView32` controls (not just in Notepad++)
and does not occur in Windows 10 or earlier. Removing the InfoTip solves this
obstruction.

---

## Restore AutoRun Icon in Drive Properties
[Install this mod from the Windhawk marketplace](https://windhawk.net/mods/restore-autorun-icon-in-drive-properties)
| [C++ source code](/mods/restore-autorun-icon-in-drive-properties.wh.cpp)

Since Windows 2000, the drive properties dialog never displays the AutoRun icon
on the General tab, leaving a blank space.

This mod restores the AutoRun icon back where it belongs.

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

## How to install Mods
First, install [Windhawk](https://windhawk.net/) and pick one method below to
install mods:

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
