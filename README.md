# My Windhawk Mods
My collection of Windhawk mods for tweaking the behaviours and UI of Microsoft
Windows and various apps.

## List of Mods

### Mods available on the Windhawk repository
* [Confirm Closing Multiple Tabs in File Explorer](#confirm-closing-multiple-tabs-in-file-explorer)
* [CrystalDiskInfo Smart Auto-Refresh](#crystaldiskinfo-smart-auto-refresh)
* [Ctrl+Backspace Fix for Win32 Text Boxes](#ctrlbackspace-fix-for-win32-text-boxes)
* [Custom Menu Height](#custom-menu-height)
* [Disk Usage Bar in Drive Properties](#disk-usage-bar-in-drive-properties)
* [Never Auto-Expand Explorer Tree Items](#never-auto-expand-explorer-tree-items)
* [Restore AutoRun Icon in Drive Properties](#restore-autorun-icon-in-drive-properties)

### Mods available on this GitHub repo
* [Better file sizes in Explorer details](#better-file-sizes-in-explorer-details)
* [Fix Darkmode ListViews](#fix-darkmode-listviews)
* [Notepad++ Tweaks](#notepad-tweaks)
* [Transparent Idle Desktop Icons](#transparent-idle-desktop-icons)
* [VMware Workstation Library TreeView Tweaks](#vmware-workstation-library-treeview-tweaks)

## Info
* [How to install Mods](#how-to-install-mods)
* [License](#license)

---

<br>
<h2 align="center">List of Mods available on the Windhawk repository</h2>

## Confirm Closing Multiple Tabs in File Explorer
[Install this mod from the Windhawk repository](https://windhawk.net/mods/confirm-closing-multiple-explorer-tabs)
| [C++ source code](/mods/confirm-closing-multiple-explorer-tabs.wh.cpp)

This mod shows a confirmation dialog when you attempt to close a File Explorer
window with multiple tabs open, preventing accidental closure of all tabs.

![](/screenshots/confirm-closing-multiple-explorer-tabs_2026-06-20.png)

### Configuration
* **Tab count threshold:** The minimum number of open tabs required to show
  the confirmation dialog.
* **Default button:** Choose whether "Close Tabs" or "Cancel" is the default
  button in the confirmation dialog.

---

## CrystalDiskInfo Smart Auto-Refresh
[Install this mod from the Windhawk repository](https://windhawk.net/mods/crystaldiskinfo-smart-auto-refresh)
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
[Install this mod from the Windhawk repository](https://windhawk.net/mods/ctrl-backspace-fix-for-win32-text-boxes)
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

## Custom Menu Height
[Install this mod from the Windhawk repository](https://windhawk.net/mods/custom-menu-height)
| [C++ source code](/mods/custom-menu-height.wh.cpp)

Control the height of Win32 context menu items and menu bars. Make classic menus
as spacious as modern WinUI menus.

By default, non-immersive context menu items are shown with a height of 22
pixels, and menu bars with a height of 19 pixels. This mod allows increasing
both menu heights to any custom value.

### Context menu items
| Height: 22px (Windows default) | Height: 32px |
| :----------------------------: | :----------: |
| ![](/screenshots/custom-menu-height_contextMenuItems-22px.png) | ![](/screenshots/custom-menu-height_contextMenuItems-32px.png) |

### Menu bar
| Height: 19px (Windows default) | Height: 32px |
| :----------------------------: | :----------: |
| ![](/screenshots/custom-menu-height_menuBar-19px.png) | ![](/screenshots/custom-menu-height_menuBar-32px.png) |

### Recommended mods
Enhance the overall context menu experience with these complementary mods:
| Mod | Author | Note |
| :-- | :----- | :--- |
| [Classic context menu on Windows 11](https://windhawk.net/mods/explorer-context-menu-classic) | m417z | Requires Windows 11 or later |
| [Custom Window Corner Radius](https://windhawk.net/mods/custom-corner-radius) | m417z | Requires Windows 11 or later |
| [Dark mode context menus](https://windhawk.net/mods/dark-menus) | Mgg Sk | Requires Windows 10 version 1809 or later |
| [Remove Context Menu Items](https://windhawk.net/mods/remove-context-menu-items) | Armaninyow | - |

### Compatibility note: Immersive menus
To ensure the custom context menu item height functions properly within File
Explorer and the taskbar on Windows 10 and later, this mod eradicates immersive
menus system-wide, falling back to standard context menus.

The code for this implementation is based on the
"[Eradicate Immersive Menus](https://windhawk.net/mods/eradicate-immersive-menus)"
mod by **aubymori**.

---

## Disk Usage Bar in Drive Properties
[Install this mod from the Windhawk repository](https://windhawk.net/mods/disk-usage-bar-in-drive-properties)
| [C++ source code](/mods/disk-usage-bar-in-drive-properties.wh.cpp)

This mod replaces the disk usage pie/donut chart in the drive properties dialog
with a usage bar.

![](/screenshots/disk-usage-bar-in-drive-properties.png)

### Features
* Replaces the pie/donut chart with a blue usage bar, like in "This PC".
* Switches the bar colour to red when the disk is almost full.
* Displays the disk usage percentage text below the bar.

### Configuration
* **Show red bar on low space:** Switches the usage bar colour to red when disk
  usage exceeds 90%.
* **Show decimal percentage:** Displays the disk usage percentage text with one
  decimal place (e.g., `64.1%`).
* **Hide storage management button:** Hides the "Details" or "Disk Clean-up"
  button.
  * This button is labeled as "Details" on Windows 11 and later, and as
    "Disk Clean-up" on Windows 10 and earlier.
  * It is recommended to hide this button for localized systems to prevent a UI
    collision with a long "Space used" string for the disk usage percentage
    text.
  * The `Alt+D` keyboard shortcut remains functional.

---

Based on the "[Disk Pie Chart](https://windhawk.net/mods/disk-pie-chart)" mod by
**aubymori**.

---

## Never Auto-Expand Explorer Tree Items
[Install this mod from the Windhawk repository](https://windhawk.net/mods/never-auto-expand-explorer-tree-items)
| [C++ source code](/mods/never-auto-expand-explorer-tree-items.wh.cpp)

File Explorer automatically expands navigation pane items (such as "This PC")
even if the "Expand to current folder" option is off, specifically when:
* Opening any folder inside an external drive in a new tab or window.
* Navigating to any drive after manually expanding and collapsing the "This PC"
  item.

This mod prevents this unwanted auto-expansion behaviour, keeping the navigation
pane tidy.

**Note:** The "Desktop" root item can still auto-expand when the
"Show all folders" option is on, keeping the navigation pane populated.

| Before | After |
| :----: | :---: |
| ![](/screenshots/never-auto-expand-explorer-tree-items_before.png) | ![](/screenshots/never-auto-expand-explorer-tree-items_after.png) |

### Configuration
* **Allow top-level items to auto-expand:** Allows top-level items to
  auto-expand while keeping their nested items collapsed.
  * Enable this option if you want the "This PC" item to auto-expand while
    keeping its drive items collapsed.

---

## Restore AutoRun Icon in Drive Properties
[Install this mod from the Windhawk repository](https://windhawk.net/mods/restore-autorun-icon-in-drive-properties)
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

---

<br>
<h2 align="center">List of Mods available on this GitHub repo</h2>

## Better file sizes in Explorer details
[C++ source code](/mods/explorer-details-better-file-sizes.wh.cpp)

Fork of the
[original mod](https://windhawk.net/mods/explorer-details-better-file-sizes) by
**m417z**

### Original features
This mod offers the following optional improvements to file sizes in File
Explorer's details view:
* **Show folder sizes** - via "Everything" integration or calculated manually
* **Mix files and folders when sorting by size**
* **Use MB/GB for large files** - instead of always KB
* **Use IEC terms** - KB → KiB, MB → MiB, GB → GiB

[Full mod details](https://windhawk.net/mods/explorer-details-better-file-sizes)

### Extended features
This forked mod expands the scope of the original mod with the following
improvements:
* **Folder item counts:** Displays the folder item count alongside the folder
  size.
* **Folder size format:** Customises the display of the folder item count and
  folder size:
  * `%count%` - the folder item count
  * `%size%` - the folder size

**Note:** The "Show folder sizes" setting must be enabled via "Everything"
integration.

![](/screenshots/explorer-details-better-file-sizes.png)

### Important note: Column width
To ensure the folder item count and folder size fit within the "Size" column
without being truncated, you must increase its width:
* Launch File Explorer and navigate to any general folder (e.g., `C:\`).
* Right-click the column header and click **More**.
* In the "Choose Details" dialog, select the **Size** item.
* Set the **Width of selected column (in pixels)** value to at least:
  * `144` for 100% DPI
  * `180` for 125% DPI
  * `216` for 150% DPI
  * `288` for 200% DPI
* Click **OK**.
* Open **"Folder Options"**.
  * Windows 11: **•••** → **Options**
  * Windows 10: **File** → **Options**
* In the "Folder Options" dialog, switch to the **View** tab.
* Click **Apply to Folders**. Doing so saves the specified column width for all
  folders of this type.

Repeat the steps for these folder types:
* **Documents**
* **Downloads**
* **Libraries**
* **Pictures** (optional)
* **Music**
* **Videos** (optional)

After saving the specified column width, the changes may not take effect on
previously accessed directories due to folder view caching (ShellBags). It is
recommended to use
[the script to clear the folder view cache](/misc/clear-folder-view-cache.cmd).

---

## Fix Darkmode ListViews
[C++ source code](/mods/fix-darkmode-listviews.wh.cpp)

Fork of the [original mod](https://windhawk.net/mods/fix-darkmode-listviews) by
**Reabstraction**

Fixes hardcoded text colours in ListViews when using a system-wide dark theme
such as "Rectify11 dark theme".

| Before | After |
| :----: | :---: |
| ![](/screenshots/fix-darkmode-listviews_before.png) | ![](/screenshots/fix-darkmode-listviews_after.png) |
| ![](/screenshots/fix-darkmode-listviews_altColors_before.png) | ![](/screenshots/fix-darkmode-listviews_altColors_after.png) |
| ![](/screenshots/fix-darkmode-listviews_translucentDropDown_before.png) | ![](/screenshots/fix-darkmode-listviews_translucentDropDown_after.png) |

### Extended features
This forked mod significantly expands the scope of the original fix with the
following improvements:
* Added compatibility with dialogs.
* Added compatibility with the
  "[Translucent Windows](https://windhawk.net/mods/translucent-windows)" mod,
  specifically fixing unreadable dark text in the Explorer address bar drop-down
  menu.
* Fixed unreadable blue and green text colours for compressed and encrypted
  files/folders.

### Configuration
**Translucent Windows compatibility:** Fixes unreadable dark text in the
Explorer address bar drop-down menu when using the "Translucent Windows" mod.

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

## Transparent Idle Desktop Icons
[C++ source code](/mods/transparent-idle-desktop-icons.wh.cpp)

Makes desktop icons semi-transparent when they are not actively being interacted
with. When the mouse hovers over the desktop area, the icons will instantly
restore to full opacity. After the cursor leaves the desktop or remains idle
for the configured duration, the icons fade back to the custom opacity level.

![](/screenshots/transparent-idle-desktop-icons.png)

### ⚠️ Important note ⚠️
This mod has a known incompatibility with the
"[Desktop Live Overlay](https://windhawk.net/mods/desktop-live-overlay)" mod.
Running both mods simultaneously will cause the wallpaper to turn completely
black when interacting with the desktop, or artificially darken when idle. For
the best experience, it is highly recommended to use only one of these mods at a
time.

---

## VMware Workstation Library TreeView Tweaks
[C++ source code](/mods/vmware-workstation-library-treeview-tweaks.wh.cpp)

Customise the "Library" tree view sidebar in VMware Workstation.

| Height: 18px (Default) | Height: 40px |
| :--------------------: | :----------: |
| ![](/screenshots/vmware-workstation-library-treeview-tweaks_default.png) | ![](/screenshots/vmware-workstation-library-treeview-tweaks_40px.png) |

### Features & Configuration
You can mix and match the following options in the settings tab:
* **Item Height:** Adjusts the vertical spacing of the virtual machines in the
  list (Default: 18px).
* **Themed TreeView:** Applies the Explorer theme.
* **Full-Row Selection:** Expands the highlight selection box across the entire
  width of the sidebar.
* **Tree Indentation:** Controls the horizontal spacing/indentation of VMs and
  folders (Default: 18px). Lower this to push VMs closer to the left edge.
* **Modern Flat Border:** Replaces the 3D sunken border with a flat 1px line.
* **Remove Expando Buttons:** Hides the expand/collapse arrows completely for a
  minimalist look. You can still expand folders by double-clicking them.
* **Disable ToolTips:** Hides the tooltips that appear when hovering over
  truncated virtual machine names.

---

<br>
<h2 align="center">Info</h2>

## How to install Mods
First, install [Windhawk](https://windhawk.net/) and pick one method below to
install mods:

### From the Windhawk repository
Recommended.
1. Launch Windhawk.
2. Click the "Explore" button.
3. Search one of my mods by typing a mod name shown on this GitHub repo.
4. Click "Details".
5. Click "Install", and the mod is installed.

### Manual installation
Install mods locally only if those mods are not on the Windhawk repository.
1. Launch Windhawk.
   * If you're installing a forked mod, turn off the original mod  to avoid
   conflicts.
2. Click "Create a New Mod" and clear everything in the text editor.
3. Copy the C++ code from this GitHub repo.
4. Paste the C++ code into the text editor.
5. Click "Compile Mod", then "Exit Editing Mode", and the mod is installed.

## License
[MIT License](/LICENSE)

Unless otherwise noted, the mods in this repository are licensed under the MIT
Licence.

### Exceptions
The following mods contain code from or are based on works by other authors
that are licensed under the GNU General Public License v3.0:
* **Better file sizes in Explorer details:** Fork of the original mod by
  **m417z**
