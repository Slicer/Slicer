# Getting Started

Welcome to the 3D Slicer community. This page contains information that you need to get started with 3D Slicer, including how to install and use basic features and where to find more information.

## System requirements

3D Slicer runs on any Windows, Mac, or Linux computer that was released in the last 5 years. Older computers may work (depending mainly on graphics capabilites). Slicer can also run on virtual machines and docker containers.

### Recommended hardware configuration:
- Memory: more than 4GB (8 or more is recommended). As a general rule, have 10x more memory than the amount of data that you load.
- Display: a minimum resolution of 1024 by 768 (1280 by 1024 or better is recommended).
- Graphics: . Dedicated graphics hardware (discrete GPU) memory is recommended for fast volume rendering.
GPU: Graphics must support minimum OpenGL 3.2. Integrated graphics card is sufficient for basic visualization. Discrete graphics card (such as NVidia GPU) is recommended for interactive 3D volume rendering and fast rendering of complex scenes. GPU texture memory (VRAM) should be larger than your largest dataset (e.g., working with 2GB data, get VRAM > 4GB) and check that your images fit in maximum texture dimensions of your GPU hardware. Except rendering, most calculations are performed on CPU, therefore having a faster GPU will generally not impact the overall speed of the application.
- Some computations in 3D Slicer are multi-threaded and will benefit from multi core, multi CPU configurations. Minimum Intel Core i5 CPU is recommended. On desktop computers, Intel Core i7 CPU is recommended.
- Interface device: a three button mouse with scroll wheel is recommended. Pen, multi-touchscreen, touchpad, and graphic tablet are supported. All OpenVR-compatible virtual reality headsets are supported for virtual reality display.
- Internet connection to access extensions, Python packages, online documentation, sample data sets, and tutorials.

### Recommended operating system versions
- Windows 10, 64-bit
- Mac OS X Lion
  - On Mac OS X Maverick - Make sure to install this update: http://support.apple.com/kb/DL1754
- Linux: recent versions of popular distributions should work. Ubuntu and Fedora are the most widely used distribution among the developers. The SlicerPreview nightly build system runs CentOS 7.

32 bit versus 64 bit: Many clinical research tasks, such as processing of large CT or MR volumetric datasets, require more memory than can be accommodated with a 32 bit program. Therefore, we only make 64-bit Slicer versions available. Developers can build 32-bit version on their own if they need to run Slicer on a 32-bit operating system.

## Installing 3D Slicer

To download Slicer, click [here](http://download.slicer.org/).

![SlicerInstallers-2020-03-25](https://github.com/Slicer/Slicer/releases/download/docs-resources/getting_started_download.png)

The "Preview Release" of 3D Slicer is updated every night and contains latest developments: new features and fixes. The "Stable Release" is updated about once a year nightly and is more rigorously tested.

Slicer is generally simple to install on all platforms. It is possible to install multiple versions of the application on the same user account and they will not interfere with each other. If you run into mysterious problems with your installation you can try deleting the [Settings Files](settings.html#settings-file-location).

Once downloaded, follow the instructions below to complete installation:

### Windows

- Run the installer
- Run Slicer from the Windows start menu
- Use "Apps & features" in Windows settings to remove the application

### Mac

- Drag the Slicer application to your Applications folder or other location of your choice.
- You cannot install extensions into the read-only volume so you must copy before installing extensions.
- Delete the Slicer.app folder to uninstall

### Linux

- Open the tar.gz archive and copy directory to the location of your choice. Run the Slicer executable.
- Remove the directory to uninstall

The following may be needed on fresh debian or ubuntu:

    sudo apt-get install libpulse-dev libnss3 libglu1-mesa

## Using Slicer

3D Slicer offers lots of features and gives users great flexibility in how to use them. As a result, new users may be overwhelmed with the number of options and have difficulty figuring out how to perform even simple operations. This is normal and many users successfully crossed this difficult stage by investing some time into learning how to use this software.

How to learn Slicer?

### Quick start

You may try to figure out how the application works by loading data sets and explore what you can do.

#### Load data

Open 3D Slicer and using the Welcome panel either load your own data or download sample data to explore. Sample data is often useful for trying the features of 3D Slicer if you don't have data of your own.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/getting_started_load_data.png)

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/getting_started_sample_data.png)

#### View data

You can customize views by clicking on the push pin in the top left corner of viewer. In the slice viewers, the horizontal bar can be used to scroll through slices or select a slice.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/getting_started_view_controllers.png)

#### Process data

3D Slicer is built on a modular architecture. Choose a module to process or analyze your data. Most important modules are the followings (complete list is available in [Modules](modules/index) section):

- [Welcome](modules/welcome): The default module when 3D Slicer is started. The panel features options for loading data and customizing 3D Slicer. Below those options are drop-down boxes that contain essential information for using 3D Slicer.
- [Data](modules/data): acts as a central data-organizing hub. Lists all data currently in the scene and allows basic operations such as search, rename, delete and move.
- [DICOM](modules/dicom): Import and export DICOM objects, such as images, segmentations, strucutre sets, radiation therapy objects, etc.
- [Volumes](modules/volumes):: Used for changing the appearance of various volume types.
- [Volume Rendering](modules/volumerendering): Provides interactive visualization of 3D image data.
- [Segmentations](modules/segmentations): Edit display properties and import/export segmentations.
- [Segment Editor](modules/segmenteditor): Segment 3D volumes using various manual, semi-automatic, and automatic tools.
- [Markups](modules/markups): Allows the creation and editing of markups associated with a scene. Currently, lists of fiducially are supported as markups.
- [Models](modules/models): Loads and adjusts display parameters of models. Allows the user to change the appearance of and organize 3D surface models.
- [Transforms](modules/transforms): This module is used for creating and editing transformation matrices. You can establish these relations by moving nodes from the Transformable list to the Transformed list or by dragging the nodes under the Transformation nodes in the Data module.

#### Extensions

3D Slicer supports plug-ins that are called extensions. An extension could be seen as a delivery package bundling together one or more Slicer modules. After installing an extension, the associated modules will be presented to the user as built-in ones. Extensions can be downloaded from the extension manager to selectively install features that are useful for the end-user.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/getting_started_module_list.png)

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/getting_started_extensions_manager.png)

For details about downloading extensions, see [Extension Manager documentation](extension_manager).
Click [here](https://www.slicer.org/wiki/Documentation/Nightly/ModuleExtensionListing/Extensions_by_category) for a full list of extensions. The links on the page will provide documentation for each extension.

Slicer is extensible. If you are interested in customizing or adding functionality to Slicer, click [here](https://www.slicer.org/wiki/Documentation/Nightly/Training#Tutorials_for_software_developers).

### Tutorials

You learn both basic concepts and highly specialized workflows from the numerous available step-by-step and video tutorials.

Try the [Welcome Tutorial](https://www.slicer.org/wiki/Documentation/Nightly/Training#Slicer_Welcome_Tutorial) and the [Data Loading and 3D Visualization Tutorial](https://www.slicer.org/wiki/Documentation/Nightly/Training#Slicer4_Data_Loading_and_3D_Visualization) to learn the basics.

For more tutorials, visit the [Tutorial page](https://www.slicer.org/wiki/Documentation/Nightly/Training).

### User manual

Browse the [User Guide](user_interface) section to find quick overview of the application user interface or [Modules](modules/index) section for detailed description of each module.

### Ask for help

3D Slicer has been around for many years and many questions have been asked and answered about it already. If you have any questions, then you may start with a web search, for example Google `slicer load jpg` to find out how you can import a stack of jpg images.

The application has a large and very friendly and helpful user community. We have poeple who will happy to help with simple questions, such as how to do a specific task in Slicer, and we have a large number of engineering and medical experts who can give you advice with how to solve complex problems.

**If you have any questions, go to the [Slicer forum](https://discourse.slicer.org) and ask us!**
