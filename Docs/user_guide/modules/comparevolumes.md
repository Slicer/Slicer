# Compare Volumes

## Overview


The Compare Volumes module manages the layout and linking of multiple volumes for you and gives other options. It is meant for comparing registration results but is also good for looking at multiple MR contrasts.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_compare-volumes-3-over-3.jpeg)

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_compare-volumes-side-by-side.jpeg)

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_compare-volumes-reveal-cursor.jpeg)


## Use Cases

Most frequently used for these scenarios:

* **Compare the results of image registration**.

* **Look at all series in a DICOM study**

* **Compare timepoints**

* **View the same segmentation over multiple volumes**

## Panels and their use

The **Volumes** section lists all the volumes in the scene. You can select the ones you want to see and can drag them to set the layout order.

The **Orientation** radio buttons lets you pick between one view or a row of three per volume.

The **Common Background** option lets you overlay all selected volumes in the foreground compared to a reference volume in the background (used with the Visualization Fade/Rock/Flicker modes). If you have just two volumes you can select one here and uncheck it in the Volumes section and you will get a layout with one set of views with the volumes in foreground and background.

**Common Label** isn’t useful as much now because Segmentations are shown over all views, but if you have a labelmap you can choose to display it.

**Hot Link** sets up linking mode so that pan/zoom/scroll happens as you drag vs on mouse release.

**Visualization** mode allows you to crossfade between foreground and background when you have common background enabled. The Rock and Flicker modes animate this action so you can look for subtle changes without needing to use the mouse. You can also pan/zoom/scroll while the animation modes are active to explore the volumes.

The **+ - Fit** buttons control the field of view of the slice views.

**Compare Checked Volumes** is the button to apply the Volumes, Orientation, and Common Background/Label options in a custom layout. Note that in current Slicer it seems you sometimes need to click the Fit button after applying a new layout.

The **Layer Reveal Cursor** show a real-time checkerboard of the foreground and background as you move the mouse over the slice views.

## Contributors

- Steve Pieper (Isomics, Inc.)

## Acknowledgments

This work was supported by NIH grants R01CA111288 and U01CA151261, NA-MIC, NAC and the Slicer Community.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_nac.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_namic.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ncigt.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_spl.png)
