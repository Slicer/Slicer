# Screen Capture

## Overview

This module is for creating videos, image sequences, or lightbox image from 3D and slice view contents.

## Panels and their use

### Input

- **Main view:** This view will be changed during the animation (rotated, sweeped, etc.).
- **Capture all views:** If enabled then all the view in the view layout will be captured. If disabled then only the main view will be captured. By capturing all views, it is possible to see the animated view (such as a moving slice) in 3D and in other slice views.
- **Animation mode:** specifies how the main view will be modified during capture.
  - **3D rotation:** Acquire video of a rotating 3D view. For smooth repeated display of a 360-degree rotation it is recommended to choose 31 or 61 as "Number of images".
  - **slice sweep:** Acquire video while going through selected range of image frames (for slice viewer only).
  - **slice fade:** Acquire video while fading between the foreground and background image (for slice viewer only).).
  - **sequence:** sequence: Acquire video while going through items in the selected sequence browser.

    ````{list-table}
    ---
    header-rows: 1
    ---
    * - 3D rotation
      - Slice sweep
      - Slice fade
      - Sequence
    * - ![rotation animation](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_screencapture_rotation.gif)
      - ![sweep animation](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_screencapture_sweep.gif)
      - ![fade animation](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_screencapture_fade.gif)
      - ![sequence animation](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_screencapture_sequence.gif)
    ````

### Output

- **Output type:**
  - **image series:** Save screnshots as separate image files (in jpg or png file format).
  - **video:** Save animation as a compressed video file. Requires installation of [ffmpeg video encoder](#setting-up-ffmpeg).
  - **lightbox image:** Save screnshots as separate jpg pr png files.

    ![lightbox image](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_screencapture_lightbox.png)

- **Number of images:** Defines how many frames are generated in the specified range. Higher number results in smoother animation but larger video file. If **single** option is enabled then a single image is captured (and counter in the filename is automatically incremented, therefore it can be used to acquire many screenshots manually).
- **Output directory:** Output image or video will be saved in this directory.
- **Output file name:** Output file name for video and lightbox.
- **Video format:**
  - **H264:** modern compressed video format, compatible with current video players.
  - **H264 (high-quality):** H264 with higher-quality setting, results in larger file.
  - **MPEG4:** commonly used compressed video format, mostly compatible with older video players.
  - **MPEG4 (high-quality):** MPEG4 with higher-quality setting, results in larger file.
  - **Animated GIF:** file format that provides lower quality images and large files, but it is more compatible with some legacy image viewers and websites.
  - **Animated GIF (grayscale):** animated GIF, saved as a grayscale image, resulting in slightly smaller files.
- **Video length:** Set total replay time of the video by adjusting the video frame rate.
- **Video frame rate:** Set replay frame rate of the video by adjusting video length.

### Advanced

- **Forward-backward:** After generating images by animating in forward direction, adds animation in reverse direction as well. It removes the "jump" at the end of the animation when it is played repeatedly in a loop.
- **Repeat:** Repeat the entire animation the specified number of times. It is useful for making animations longer (e.g., for uploading to YouTube).
- **ffmpeg executable:** Path to ffmpeg executable. Only used if video export is selected. Requires installation of [ffmpeg video encoder](#setting-up-ffmpeg).
- **Video extra options:** Options for ffmpeg that controls video format and quality. Only used if video export is selected.
  - These parameters are already specified by the module and therefore should not be included in the extra options: `-i (input files) -y (overwrite without asking) -r (frame rate) -start_number`.
  - Information about available options:
    - https://trac.ffmpeg.org/wiki/Encode/H.264
    - https://trac.ffmpeg.org/wiki/Encode/MPEG-4
    - https://trac.ffmpeg.org/wiki/Encode/YouTube
    - https://ffmpeg.org/ffmpeg-all.html
- **Image file name pattern:** Defines image file naming pattern. `%05d` will be replaced by the image number (5 numbers, padded with zeros). This is only used if image series output is selected.
- **Lightbox image columns:** Number of columns in the generated lighbox image.
- **Maximum number of images:** Specifies the maximum range of the "number of images" slider. Useful for creating very long animations.
- **Output volume node:** If a single image output is selected then the output can be saved into the selected volume node.
- **View controllers:** Show view controllers. If unchecked then view controllers will be temporarily hidden during screen capture.
- **Transparent background:** If checked then images will be captured with transparent background.
- **Watermark image:** Adds a watermark image to the captured images.

  ![watermarked image](https://github.com/Slicer/Slicer/releases/download/docs-resources/module_screencapture_watermark.png)

  - **Position:** Position of the watermark image over the captured image.
  - **Size:** Watermark image size, as percentage of the original size.
  - **Opacity:**  Watermark image opacity, larger value makes the watermark more visible (less transparent).

## Setting up ffmpeg

FFmpeg library is not packaged with 3D Slicer due to large package size and licensing requirements for some video compression methods (see https://ffmpeg.org/legal.html). The FFmpeg library has to be downloaded and set up only once and 3D Slicer will remember its location.

### Windows setup instructions

On Windows, Screen Capture model can automatically download and install ffmpeg when needed. Follow these instructions for manual setup:

- Download ffmpeg from here: https://ffmpeg.org/download.html (click Windows icon, select a package, for example `Download FFmpeg64-bit static`)
- Extract downloaded package (for example, to `C:\Users\Public`)
- In Advanced section, ffmpeg executable: select path for ffmpeg.exe (for example, `C:\Users\Public\ffmpeg-20160912-bc7066f-win32-static\bin\ffmpeg.exe`)

### MacOS setup instructions

- Install homebrew (from https://brew.sh/)
- Run:
  ```console
  brew install ffmpeg
  ```
- In Advanced section, ffmpeg executable: select `/usr/local/bin/ffmpeg`

### Linux setup instructions

- Run these commands from the terminal:
  ```console
  git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg
  cd ffmpeg
  sudo apt-get install libx264-dev
  ./configure --enable-gpl --enable-libx264 --prefix=${HOME}
  make install
  ```
- In Advanced section, ffmpeg executable: select `${HOME}/bin/ffmpeg`

## Related modules

- [Animator](https://github.com/SlicerMorph/SlicerMorph/tree/master/Docs/Animator#readme) module in [SlicerMorph extension](https://slicermorph.github.io/) allows creating more complex animations, such as cutting through a volume (by changing region of interest), adjusting volume rendering transfer functions, or exploding view of complex model assembly.
- [Scene Views](sceneviews.md) module can create snapshot of the entire scene content along with a screenshot, which are all saved in the scene.
- Capture Toolbar allows creation simple screenshots that are saved in the scene. The feature may be removed in the future. Screen Capture module's "Output volume node" feature can be used for saving screenshots in the scene.

## Information for developers

- This is a Python scripted module. Source code is available [here](https://github.com/Slicer/Slicer/blob/main/Modules/Scripted/ScreenCapture/ScreenCapture.py).
- Examples of capturing images are available in the [Script Repository](../../developer_guide/script_repository.md#screen-capture)

## Contributors

Andras Lasso (PerkLab, Queen's University)

## Acknowledgements

This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_perklab.png)
![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_ocairo.png)
