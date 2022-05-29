# Getting Started

Welcome to the 3D Slicer community. This page contains information that you need to get started with 3D Slicer, including how to install and use basic features and where to find more information.

## System requirements

3D Slicer runs on any Windows, Mac, or Linux computer that was released in the last 5 years. Older computers may work (depending mainly on graphics capabilities).

Slicer can also run on virtual machines and docker containers. For example, [3D Slicer + Jupyter notebook in a web browser](https://mybinder.org/v2/gh/Slicer/SlicerNotebooks/master?filepath=SlicerWeb.ipynb) is available for free via Binder service (no installation needed, the application can run in any web browser).

### Operating system versions

- Windows: Windows 10 or 11, with all recommended updates installed. Windows 10 Version 1903 (May 2019 Update) version or later is required for support of international characters (UTF-8) in filenames and text. Microsoft does not support Windows 8.1 and Windows 7 anymore and Slicer is not tested on these legacy operating system versions, but may still work.
- macOS: macOS High Sierra (10.13) or later (both Intel and ARM based systems). Latest public release is recommended.
- Linux: Ubuntu 18.04 or later<br>CentOS 7 or later. Latest LTS (Long-term-support) version is recommended.

### Recommended hardware configuration
- Memory: more than 4GB (8 or more is recommended). As a general rule, have 10x more memory than the amount of data that you load.
- Display: a minimum resolution of 1024 by 768 (1280 by 1024 or better is recommended).
- Graphics: Dedicated graphics hardware (discrete GPU) memory is recommended for fast volume rendering.
GPU: Graphics must support minimum OpenGL 3.2. Integrated graphics card is sufficient for basic visualization. Discrete graphics card (such as NVidia GPU) is recommended for interactive 3D volume rendering and fast rendering of complex scenes. GPU texture memory (VRAM) should be larger than your largest dataset (e.g., working with 2GB data, get VRAM > 4GB) and check that your images fit in maximum texture dimensions of your GPU hardware. Except rendering, most calculations are performed on CPU, therefore having a faster GPU will generally not impact the overall speed of the application.
- Some computations in 3D Slicer are multi-threaded and will benefit from multi core, multi CPU configurations.
- Interface device: a three button mouse with scroll wheel is recommended. Pen, multi-touchscreen, touchpad, and graphic tablet are supported. All OpenVR-compatible virtual reality headsets are supported for virtual reality display.
- Internet connection to access extensions, Python packages, online documentation, sample data sets, and tutorials.

## Installing 3D Slicer

To download Slicer, click [here](https://download.slicer.org/).

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/getting_started_download.png)

**Notes:**
- The "Preview Release" of 3D Slicer is updated daily (process starts at 11pm ET and takes few hours to complete) and represents the latest development including new features and fixes.
- The "Stable Release" is usually updated a few times a year and is more rigorously tested.
- Slicer is generally simple to install on all platforms. It is possible to install multiple versions of the application on the same user account and they will not interfere with each other. If you run into mysterious problems with your installation you can try deleting the [application settings files](settings.md#settings-file-location).
- Only 64-bit Slicer installers are available to download. Developers can attempt to build 32-bit versions on their own if they need to run Slicer on a 32-bit operating system. That said, this should be carefully considered as many clinical research tasks, such as processing of large CT or MR volumetric datasets, require more memory than can be accommodated with a 32-bit program.

Once downloaded, follow the instructions below to complete installation:

### Windows

- Run the installer.
  - Current limitation: Installation path must only contain English ([ASCII printable](https://en.wikipedia.org/wiki/ASCII#Printable_characters)) characters because otherwise some Python packages may not load correctly (see this [issue](https://github.com/Slicer/Slicer/issues/5383) for more details).
- Run Slicer from the Windows start menu.
- Use "Apps & features" in Windows settings to remove the application.

### Mac

- Open the install package (.dmg file).
- Drag the Slicer application (Slicer.app) to your Applications folder (or other location of your choice).
  - This step is necessary because content of a .dmg file is opened as a read-only volume, and you cannot install extensions or Python packages into a read-only volume.
- Delete the Slicer.app folder to uninstall.

Note for installing a Preview Release: Currently, preview release packages are not signed. Therefore, when the application is started the first time the following message is displayed: "Slicer... can't be opened because it is from an unidentified developer". To resolve this error, locate the application in Finder and right-click (two-finger click) and click `Open`. When it says `This app can’t be opened` go ahead and hit cancel. Right click again and say `Open` (yes, you need to repeat the same as you did before - the outcome will be different than the first time). Click the `Open` (or `Open anyway`) button to start the application. See more explanation and alternative techniques [here](https://support.apple.com/en-my/guide/mac-help/mh40616/mac).

#### Installing using Homebrew

Slicer can be installed with a single terminal command using the [Homebrew](https://brew.sh/) package manager:

```shell
brew install --cask slicer  # to install
brew upgrade slicer         # to upgrade
brew uninstall slicer       # to uninstall
```

This procedure avoids the typical google-download-mount-drag process to install macOS applications.

Preview releases can be installed using [`homebrew-cask-versions`](https://github.com/Homebrew/homebrew-cask-versions):

```shell
brew tap homebrew/cask-versions     # needs to be run once
brew install --cask slicer-preview  # to install
brew upgrade slicer-preview         # to upgrade
brew uninstall slicer-preview       # to uninstall
```

### Linux

- Open the tar.gz archive and copy directory to the location of your choice.
- Installation of additional packages may be necessary depending on the Linux distribution and version, as described in subsections below.
- Run the `Slicer` executable.
- Remove the directory to uninstall.

**Notes:**
- Slicer is expected to work on the vast majority of desktop and server Linux distributions. The system is required to provide at least GLIBC 2.17 and GLIBCCC 3.4.19. For more details, read [here](https://www.python.org/dev/peps/pep-0599/#the-manylinux2014-policy).
- The Extension Manager uses QtWebengine to display the list of extensions. If your linux kernel does not fulfill [sandboxing requirements](https://doc.qt.io/Qt-5/qtwebengine-platform-notes.html#sandboxing-support) then you can turn off sandboxing by this command: `export QTWEBENGINE_DISABLE_SANDBOX=1`
- Getting command-line arguments and process output containing non-ASCII characters requires the system to use a UTF-8 locale. If the system uses a different locale then the `export LANG="C.UTF-8"` command may be used before launching the application to switch to an acceptable locale.

#### Debian / Ubuntu
The following may be needed on fresh debian or ubuntu:

    sudo apt-get install libpulse-dev libnss3 libglu1-mesa
    sudo apt-get install --reinstall libxcb-xinerama0

#### ArchLinux
ArchLinux runs the `strip` utility by default; this needs to be disabled in order to run Slicer binaries.  For more information see [this thread on the Slicer Forum](https://discourse.slicer.org/t/could-not-load-dicom-data/14211/5).

#### Fedora
Install the dependencies:

    sudo dnf install mesa-libGLU libnsl

The included libcrypto.so.1.1 in the Slicer installation is incompatible with the system libraries used by Fedora 35. The fix, until it is updated, is to move/remove the included libcrypto files:

    $SLICER_ROOT/lib/Slicer-4.xx/libcrypto.*

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

Data module's Subject hierarchy tab shows all data sets in the scene. Click the "eye" icon to show/hide an item in all views.

You can customize views (show orientation marker, ruler, change orientation, transparency) by clicking on the push pin in the top left corner of viewer. In the slice viewers, the horizontal bar can be used to scroll through slices or select a slice.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/getting_started_view_controllers.png)

#### Process data

3D Slicer is built on a modular architecture. Choose a module to process or analyze your data. Most important modules are the following (complete list is available in [Modules](modules/index.md) section):

- *Welcome*: The default module when 3D Slicer is started. The panel features options for loading data and customizing 3D Slicer. Below those options are drop-down boxes that contain essential information for using 3D Slicer.
- [Data](modules/data.md): acts as a central data-organizing hub. Lists all data currently in the scene and allows basic operations such as search, rename, delete and move.
- [DICOM](modules/dicom.md): Import and export DICOM objects, such as images, segmentations, structure sets, radiation therapy objects, etc.
- [Volumes](modules/volumes.md): Used for changing the appearance of various volume types.
- [Volume Rendering](modules/volumerendering.md): Provides interactive visualization of 3D image data.
- [Segmentations](modules/segmentations.md): Edit display properties and import/export segmentations.
- [Segment Editor](modules/segmenteditor.md): Segment 3D volumes using various manual, semi-automatic, and automatic tools.
- [Markups](modules/markups.md): Allows the creation and editing of markups associated with a scene.
- [Models](modules/models.md): Loads and adjusts display parameters of models. Allows the user to change the appearance of and organize 3D surface models.
- [Transforms](modules/transforms.md): This module is used for creating and editing transformation matrices. You can establish these relations by moving nodes from the Transformable list to the Transformed list or by dragging the nodes under the Transformation nodes in the Data module.

#### Save data

Data sets loaded into the application can be saved using Save data dialog or exported to DICOM format using DICOM module. Detailes are described in [Data loading and saving section](data_loading_and_saving.md).

#### Extensions

3D Slicer supports plug-ins that are called extensions. An extension could be seen as a delivery package bundling together one or more Slicer modules. After installing an extension, the associated modules will be presented to the user as built-in ones. Extensions can be downloaded from the extensions manager to selectively install features that are useful for the end-user.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/getting_started_module_list.png)

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/getting_started_extensions_manager.png)

For details about downloading extensions, see [Extensions Manager documentation](extensions_manager.md).
Click [here](https://www.slicer.org/wiki/Documentation/Nightly/ModuleExtensionListing/Extensions_by_category) for a full list of extensions. The links on the page will provide documentation for each extension.

Slicer is extensible. If you are interested in customizing or adding functionality to Slicer, click [here](https://www.slicer.org/wiki/Documentation/Nightly/Training#Tutorials_for_software_developers).

### Tutorials

You learn both basic concepts and highly specialized workflows from the numerous available step-by-step and video tutorials.

Try the [Welcome Tutorial](https://www.slicer.org/wiki/Documentation/Nightly/Training#Slicer_Welcome_Tutorial) and the [Data Loading and 3D Visualization Tutorial](https://www.slicer.org/wiki/Documentation/Nightly/Training#Slicer4_Data_Loading_and_3D_Visualization) to learn the basics.

For more tutorials, visit the [Tutorial page](https://www.slicer.org/wiki/Documentation/Nightly/Training).

### User manual

Browse the [User Interface](user_interface.md) section to find quick overview of the application user interface or [Modules](modules/index.md) section for detailed description of each module.

### Ask for help

3D Slicer has been around for many years and many questions have been asked and answered about it already. If you have any questions, then you may start with a web search, for example Google `slicer load jpg` to find out how you can import a stack of jpg images.

The application has a large and very friendly and helpful user community. We have people who will happy to help with simple questions, such as how to do a specific task in Slicer, and we have a large number of engineering and medical experts who can give you advice with how to solve complex problems.

**If you have any questions, go to the [Slicer forum](https://discourse.slicer.org) and ask us!**

## Glossary

Terms used in various fields of medical and biomedical image computing and clinical images are not always consistent. This section defines terms that are commonly used in 3D Slicer, especially those that may have different meaning in other contexts.

- **Annotation**: Simple geometric objects and measurements that the user can place in viewers. Annotations module can be used to create such objects, but it is deprecated - being replaced by Markups module.
- **Bounds**: Describes bounding box of a spatial object along 3 axes. Defined in VTK by 6 floating-point values: `X_min`, `X_max`, `Y_min`, `Y_max`, `Z_min`, `Z_max`.
-** Brightness/contras**t: Specifies linear mapping of voxel values to brightness of a displayed pixel. Brightness is the linear offset, contrast is the multiplier. In medical imaging, this linear mapping is more commonly specified by window/level values.
- **Cell**: Data cells are simple topological elements of meshes, such as lines, polygons, tetrahedra, etc.
- **Color legend** (or color bar, scalar bar): a widget overlaid on slice or 3D views that displays a color legend, indicating meaning of colors.
- **Coordinate system** (or coordinate frame, reference frame, space): Specified by position of origin, axis directions, and distance unit. All coordinate systems in 3D Slicer are right-handed.
- **Extension** (or Slicer extension): A collection of modules that is not bundled with the core application but can be downloaded and installed using the Extensions manager.
- [**Extensions manager**](extensions_manager): A software component of Slicer that allows browsing, installing, uninstalling extensions in the [Extensions catalog (also known as the Slicer app store)](https://extensions.slicer.org) directly from the application.
- [**Extensions index**](https://github.com/Slicer/ExtensionsIndex): A repository that contains description of each extension that the Extension catalog is built from.
- **Extent**: Range of integer coordinates along 3 axes. Defined in VTK by 6 values, for IJK axes: `I_min`, `I_max`, `J_min`, `J_max`, `K_min`, `K_max`. Both minimum and maximum values are inclusive, therefore size of an array is `(I_max - I_min + 1)` x `(J_max - J_min + 1)` x `(K_max - K_min + 1)`.
- **Fiducial**: Represents a point in 3D space. The term originates from image-guided surgery, where "fiducial markers" are used to mark point positions.
- **Frame**: One time point in a time sequence. To avoid ambiguity, this term is not used to refer to a slice of a volume.
- **Geometry**: Specify location and shape of an object in 3D space. See "Volume" term for definition of image geometry.
- **Image intensity**: Typically refers to the value of a voxel. Displayed pixel brightness and color is computed from this value based on the chosen window/level and color lookup table.
- **IJK**: Voxel coordinate system axes. Integer coordinate values correspond to voxel center positions. IJK values are often used as coordinate location within a 3D array. By VTK convention, and I indexes the column, J indexes the row, K indexes the slice. Note that numpy uses the opposite ordering convention, where `a[K][J][I]`. Sometimes this memory layout is described as I being the fastest moving index and K being the slowest moving.
- **ITK**: [Insight Toolkit](https://itk.org/). Software library that Slicer uses for most image processing operations.
- **Labelmap** (or labelmap volume, labelmap volume node): Volume node that has discrete (integer) voxel values. Typically each value corresponds to a specific structure or region. This allows compact representation of non-overlapping regions in a single 3D array. Most software use a single labelmap to store an image segmentation, but Slicer uses a dedicated segmentation node, which can contain multiple representations (multiple labelmaps to allow storing overlapping segments; closed surface representation for quick 3D visualization, etc.).
- **LPS**: Left-posterior-superior anatomical coordinate system. Most commonly used coordinate system in medical image computing. Slicer stores all data in LPS coordinate system on disk (and converts to/from RAS when writing to or reading from disk).
- **Markups**: Simple geometric objects and measurements that the user can place in viewers. [Markups module](modules/markups.md) can be used to create such objects. There are several types, such as point list, line, curve, plane, ROI.
- **Master volume**: Voxel values of this volume is used during segmentation by those effects that rely on intensity of an underlying volume.
- **MRML**: [Medical Reality Markup Language](https://en.wikipedia.org/wiki/Medical_Reality_Markup_Language): Software library for storage, visualization, and processing of information objects that may be used in medical applications. The library is designed to be reusable in various software applications, but 3D Slicer is the only major application that is known to use it.
- **Model** (or model node): MRML node storing surface mesh (consists of triangle, polygon, or other 2D cells) or volumetric mesh (consists of tetrahedral, wedge, or other 3D cells)
- **Module** (or Slicer module): A Slicer module is a software component consisting of a graphical user interface (that is displayed in the module panel when the module is selected), a logic (that implements algorithms that operate on MRML nodes), and may provide new MRML node types, displayable managers (that are responsible for displaying those nodes in views), input/output plugins (that are responsible for load/save MRML nodes in files), and various other plugins. Modules are typically independent and only communicate with each other via modifying MRML nodes, but sometimes a module use features provided by other modules by calling methods in its logic.
- **Node** (or MRML node): One data object in the scene. A node can represent data (such as an image or a mesh), describe how it is displayed (color, opacity, etc.), stored on disk, spatial transformations applied on them, etc. There is a C++ class hierarchy to define the common behaviors of nodes, such as the property of being storable on disk or being geometrically transformable. The structure of this class hierarchy can be inspected in the code or in the [API documentation](https://apidocs.slicer.org/master/classvtkMRMLStorableNode.html).
- **Orientation marker**: Arrow, box, or human shaped marker to show axis directions in slice views and 3D views.
- **RAS**: Right-anterior-superior anatomical coordinate system. Coordinate system used internally in Slicer. It can be converted to/from LPS coordinate system by inverting the direction of the first two axes.
- **Reference**: Has no specific meaning, but typically refers to a secondary input (data object, coordinate frame, geometry, etc.) for an operation.
- **Region of interest (ROI)**: Specifies a box-shaped region in 3D. Can be used for cropping volumes, clipping models, etc.
- **Registration**: The process of aligning objects in space. Result of the registration is a transform, which transforms the "moving" object to the "fixed" object.
- **Resolution**: Voxel size of a volume, typically specified in mm/pixel. It is rarely used in the user interface because its meaning is slightly misleading: high resolution value means large spacing, which means coarse (low) image resolution.
- **Ruler**: It may refer to: 1. View ruler: The line that is displayed as an overlay in viewers to serve as a size reference. 2. Annotation ruler: deprecated distance measurement tool (use "Markups line" instead).
- **Scalar component**: One element of a vector. Number of scalar components means the length of the vector.
- **Scalar value**: A simple number. Typically floating-point.
- **Scene** (or MRML scene): This is the data structure that contains all the data that is currently loaded into the application and additional information about how they should be displayed or used. The term originates [computer graphics](https://en.wikipedia.org/wiki/Rendering_(computer_graphics)).
- **Segment**: Corresponds to single structure in a segmentation. See more information in [Image segmentation](image_segmentation.md) section.
- **Segmentation** (also known as contouring, annotation; region of interest, structure set, mask): Process of delineating 3D structures in images. Segmentation can also refer to the MRML node that is the result of the segmentation process. A segmentation node typically contains multiple segments (each segment corresponds to one 3D structure). Segmentation nodes are not labelmap nodes or model nodes but they can store multiple representations (binary labelmap, closed surface, etc.). See more information in [Image segmentation](image_segmentation) section.
- **Slice**: Intersection of a 3D object with a plane.
- **Slice view annotations**: text in corner of slice views displaying name, and selected DICOM tags of the displayed volumes
- **Spacing**: Voxel size of a volume, typically specified in mm/pixel.
- **Transform** (or transformation): Can transform any 3D object from one coordinate system to another. Most common type is rigid transform, which can change position and orientation of an object. Linear transforms can scale, mirror, shear objects. Non-linear transforms can arbitrarily warp the 3D space. To display a volume in the world coordinate system, the volume has to be resampled, therefore transform *from* the world coordinate system to the volume is needed (it is called the resampling transform). To transform all other node types to the world coordinate system, all points must be transformed *to* the world coordinate system (modeling transform). Since a transform node must be applicable to any nodes, transform nodes can provide both *from* and *to* the parent (store one and compute the other on-the-fly).
- **Volume** (or volume node, scalar volume, image): MRML node storing 3D array of voxels. Indices of the array are typically referred to as IJK. Range of IJK coordinates are called extents. Geometry of the volume is specified by its origin (position of the IJK=(0,0,0) point), spacing (size of a voxel along I, J, K axes), axis directions (direction of I, J, K axes in the reference coordinate system) with respect to a frame of reference. 2D images are single-slice 3D volumes, with their position and orientation specified in 3D space.
- [**Voxel**](https://en.wikipedia.org/wiki/Voxel): One element of a 3D volume. It has a rectangular prism shape. Coordinates of a voxel correspond to the position of its center point. Value of a voxel may be a simple scalar value or a vector.
- **VR**: Abbreviation that can refer to volume rendering or virtual reality. To avoid ambiguity it is generally recommended to use the full term instead (or explicitly define the meaning of the abbreviation in the given context).
- **VTK**: [Visualization Toolkit](https://vtk.org/). Software library that Slicer uses for to data representation and visualization. Since most Slicer classes are derived from VTK classes and they heavily use other VTK classes, Slicer adopted many conventions of VTK style and application programming interface.
- **Window/level** (or window width/window level): Specifies linear mapping of voxel values to brightness of a displayed pixel. Window is the size of the intensity range that is mapped to the full displayable intensity range. Level is the voxel value that is mapped to the center of the full displayable intensity range.
