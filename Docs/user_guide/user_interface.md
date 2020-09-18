# User Interface

## Application overview

Slicer stores all loaded data in a data repository, called the "scene" (or Slicer scene or MRML scene). Each data set, such as an image volume, surface model, or point set, is represented in the scene as a "node".

Slicer provides a large number "modules", each implementing a specific set of functions for creating or manipulating data in the scene. Modules typically do not interact with each other directly: they just all operate on the data nodes in the scene. Slicer package contains over 100 built-in modules and additional modules can be installed by using the Extension Manager.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/user_interface_main_window.png)

### Module Panel

This panel (located by default on the left side of the application main window) displays all the options and features that the current module offers to the user. Current module can be selected using the **Module Selection** toolbar.

**Data Probe** is located at the bottom of the module panel. It displays information about view content at the position of the mouse pointer.

### Views

Slicer displays data in various views. The user can choose between a number of predefined layouts, which may contain slice, 3D, chart, and table views.

The Layout Toolbar provides a drop-down menu of layouts useful for many types of studies. When Slicer is exited normally, the selected layout is saved and restored next time the application is started.

### Application Menu

- **File**: Functions for loading a previouly saved scene or individual datasets of various types, and for downloading sample datasets from the internet. An option for saving scenes and data is also provided here. **Add Data** allows loading data from files. **DICOM** module is recommended to import data from DICOM files and loading of imported DICOM data. **Save** opens the "Save Data" window, which offers a variety of options for saving all data or selected datasets.
- **Edit**: Contains an option for showing Application Settings, which allows users to customize appearance and behavior of Slicer, such as modules displayed in the toolbar, application font size, temporary directory location, location of additional Slicer modules to include.
- **View**: Functions for showing/hiding additional windows and widgets, such as **Extension Manager** for installing extensions from Slicer app store, **Error Log** for checking if the application encountered any potential errors, **Python Interactor** for getting a Python console to interact with the loaded data or modules, **show/hide toolbars**, or **switch view layout**.

### Toolbar

Toolbar provides quick access to commonly used functions. Individual toolbar panels can be shown/hidden using menu: View / Toolbars section.

**Module Selection** toolbar is used for selecting the currently active "module". The toolbar provides options for searching for module names (`Ctrl` + `f` or click on magnify glass icon) or selecting from a menu. **Module history dropdown button** shows the list of recently used modules. **Arrow buttons** can be used for going back to/returning from previously used module.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/user_interface_module_toolbar.png)

**Favorite modules** toolbar contains a list of most frequently used modules. The list can be customized using menu: Edit / Application settings / Modules / Favorite Modules. Drag-and-drop modules from the Modules list to the Favorite Modules list to add a module.

### Status bar

This panel may display application status, such as current operation in progress. Clicking the little **X** icons displays the Erro Log window.

## Interacting with views

### View Cross-Reference

Holding down the `Shift` key while moving the mouse in any slice or 3D view will cause the Crosshair to move to the selected position in all views. By default, when the Crosshair is moved in any views, all slice views are scrolled to the same RAS position indexed by the mouse. This feature is useful when inspecting.

To show/hide the Crosshair position, click crosshair icon
![](../../Libs/MRML/Widgets/Resources/Icons/SlicesCrosshair.png).

To customize behavior and appearance of the Crosshair, click the "down arrow" button on the right side of the crosshair icon.

### Mouse Modes

Slicer has multiple mouse modes: **Transform** (which allows interactive rotate, translate and zoom operations), **Window/Level** to adjust brightness/contrast of the image volumes, and **Place** (which permits objects to be interactively placed in slice and 3D views).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/user_interface_mousemode_toolbar.png)

The toolbar icons that switch between these mouse modes are shown from left to right above, respectively. Place Fiducial is the default place option as shown above; options to place other nodes such as Ruler and Region of Interest Widgets are also available from the drop-down Place Mode menu.

> **Note:** Transform mode is the default interaction mode. By default, Place mode persists for one "place" operation after the Place Mode icon is selected, and then the mode switches back to Transform. Place mode can be made persistent (useful for creating multiple fiducial points, rulers, etc.) by checking the Persistent checkbox shown rightmost in the Mouse Mode Toolbar.

#### Adjusting image window/level

Medical images typically contain thousands of gray levels, but regular computer displays can display only 256 gray levels, and the human eye also has limitation in what minimum contrast difference it can notice (see [Kimpe 2007](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3043920/) for more specific information). Therefore, medical images are displayed with adjustable brightness/contrast (window/level).

By default 3D Slicer uses window/level setting that is specified in the DICOM file. If it is not available then window/level is set so that the entire intensity range of the image (except top/bottom 0.1%, to not let a very thin tail of the intensity distribution to decrease the image contrast too much).

Window/level can be manually adjusted anytime by clicking on "Adjust window/level" button on the toolbar then left-click-and-drag in any of the slice viewers. Optimal window/level can be computed for a chosen area by lef-click-and-dragging while holding down <kbd>Ctrl</kbd> key.

[![](https://img.youtube.com/vi/u1B0F1KcVsk/0.jpg)](https://youtu.be/u1B0F1KcVsk "Demo video of how to adjust image window/level")

### 3D View

Displays a rendered 3D view of the scene along with visual references to specify orientation and scale.

Default orientation axes: A = anterior, P = posterior, R = right, L = left, S = superior and I = inferior.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/user_interface_3d_view_controls.png)

3D View Controls: The blue bar across any 3D View shows a pushpin icon on its left. When the mouse rolls over this icon, a panel for configuring the 3D View is displayed. The panel is hidden when the mouse moves away. For persistent display of this panel, just click the pushpin icon.

### Slice View

Three default slice views are provided (with Red, Yellow and Green colored bars) in which Axial, Saggital, Coronal or Oblique 2D slices of volume images can be displayed. Additional generic slice views have a grey colored bar and an identifying number in their upper left corner.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/user_interface_slice_view_controls.png)

Slice View Controls: The colored bar across any Slice View shows a pushpin icon on its left. When the mouse rolls over this icon, a panel for configuring the slice view is displayed. The panel is hidden when the mouse moves away. For persistent display of this panel, just click the pushpin icon. For more options, click the double-arrow icon.

View Controllers module provides an alternate way of displaying these controllers in the Module Panel.

- **Reset field of view** (small square) centers the slice on the current background volume
- **Link** button synchronizes properties (which volumes are displayed, zoom factor, position of parallel views, opacities, etc.) between all slice views in the same view group. Long-click on the button exposes **hot-linked** option, which controls when properties are synchronized (immediately or when the mouse button is released).
- **Eye** button in the top row can show the current slice in 3D views. Drop-down menu of the button contains advanced options to customize how this slice is rendered: "...match volume" means that the properties are taken from the full volume, while "...match 2D" means that the properties are copied from the current slice view (for example, copies zoom and pan position). Typically these differences are subtle and the settings can be left at default.
- **Orientation Selection** displays allows you to choose the orientation for this slice view.
- **Lightbox** to select a mosiac (a.k.a. contact sheet) view.  Not all operations work in this mode and it may be removed in the future.
- **Reformat** allows interactive manipulation of the slice orientation.
- **Blending options** how foreground and background layers are mixed.
- **Spacing and Field of View** Spacing defines the increment for the slice offset slider.  Field of view sets the zoom level for the slice.
- **Rotate to Volume Plane** changes the orientation of the slice to match the closest acquisition orientation of the displayed volume
- **Show Orientation Marker** controls display of human, cube, etc in lower right corner
- **Ruler** controls display of ruler in slice view


## Mouse & Keyboard Shortcuts

### Generic shortcuts

| Shortcut | Operation |
| -------- | --------- |
| `Ctrl` + `f` | find module by name (hit `Enter` to select) |
| `Ctrl` + `o` | add data from file |
| `Ctrl` + `s` | save data to files |
| `Ctrl` + `w` | close scene |
| `Ctrl` + `0` | show Error Log |
| `Ctrl` + `1` | show Application Help |
| `Ctrl` + `2` | show Application Settings |
| `Ctrl` + `3` | show/hide Python Interactor |
| `Ctrl` + `4` | show Extension Manager |
| `Ctrl` + `5` | show/hide Module Panel |
| `Ctrl` + `h` | open default startup module (configurable in Application Settings) |

### Slice views

The following shortcuts are available when a slice view is active. To
activate a view, click inside the view: if you do not want to change
anything in the view, just activate it then do `right-click` without
moving the mouse. Note that simply hovering over the mouse over a slice
view will not activate the view.

| Shortcut | Operation |
| -------- | --------- |
| `right-click` + `drag up/down` | zoom image in/out |
| `Ctrl` + `mouse wheel` | zoom image in/out |
| `middle-click` + `drag` | pan (translate) view |
| `Shift` + `left-click` + `drag` | pan (translate) view |
| `left arrow` / `right arrow` | move to previous/next slice |
| `b` / `f` | move to previous/next slice |
| `Shift` + `mouse move` | move crosshair in all views |
| `v` | toggle slice visibility in 3D view |
| `r` | reset zoom and pan to default |
| `g` | toggle segmentation or labelmap volume | | | visibility |
| `t` | toggle foreground volume visibility |
| `[` / `]` | use previous/next volume as background |
| `{` / `}` | use previous/next volume as foreeround |

### 3D views

The following shortcuts are available when a 3D view is active. To
activate a view, click inside the view: if you do not want to change
anything in the view, just activate it then do `right-click` without
moving the mouse. Note that simply hovering over the mouse over a slice
view will not activate the view.

| Shortcut | Operation |
| -------- | --------- |
| `Shift` + `mouse move` | move crosshair in all views |
| `left-click` + `drag` | rotate view |
| `left arrow` / `right arrow`  | rotate view |
| `up arrow` / `down arrow` | rotate view |
| `End` or `Keypad 1` | rotate to view from anterior |
| `Shift` + `End` or `Shift` + `Keypad 1` | rotate to view from posterior |
| `Page Down` or `Keypad 3` | rotate to view from left side |
| `Shift` + `Page Down` or `Shift` + `Keypad 3` | rotate to view from right side |
| `Home` or `Keypad 7` | rotate to view from superior |
| `Shift` + `Home` or `Shift` + `Keypad 7`| rotate to view from inferior |
| `right-click` + `drag up/down` | zoom view in/out |
| `Ctrl` + `mouse wheel` | zoom view in/out |
| `+` / `-` | zoom view in/out |
| `middle-click` + `drag` | pan (translate) view |
| `Shift` + `left-click` + `drag` | pan (translate) view |
| `Shift` + `left arrow` / `Shift` + `right arrow` | pan (translate) view |
| `Shift` + `up arrow` / `Shift` + `down arrow` | pan (translate) view |
| `Shift` + `Keypad 2` / `Shift` + `Keypad 4` | pan (translate) view |
| `Shift` + `Keypad 6` / `Shift` + `Keypad 8` | pan (translate) view |
| `Keypad 0` or `Insert` | reset zoom and pan, rotate to nearest standard view |

> **Note:** Simulation if shortcuts not available on your device:
> - One-button mouse: instead of `right-click` do `Ctrl` + `click`
> - Trackpad: instead of `right-click` do `two-finger click`
