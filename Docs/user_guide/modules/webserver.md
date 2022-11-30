# WebServer

## Overview

Creates a fairly simple but powerful web server that can respond to http(s) requests with data from the current application state or modify the application state.

This module is meant to be the basis for implementing web applications that use Slicer as a remote render / computation engine or for controlling your Slicer instance for interaction with other system code like shell scripts or other applications.

There are three types of endpoints:

| Type | Description | Path | Handler |
|--|--|--|--|
| [Static](#static-endpoints) | Hosts files out of the module's `docroot` like any standard http server. | `/` | [StaticPagesRequestHandler][StaticPagesRequestHandler] |
| [DICOMweb](#dicom-endpoints) | Exposes the Slicer dicom database as a DICOMweb services | `/dicom` | [DICOMRequestHandler][DICOMRequestHandler] |
| [Slicer](#slicer-endpoints) | Give read/write access to features in Slicer's MRML scene and GUI.</br>This interface also exposes the Python interpreter so that arbitrary python code may be executed in the Slicer application. | `/slicer` | [SlicerRequestHandler][SlicerRequestHandler] |

[StaticPagesRequestHandler]: https://github.com/Slicer/Slicer/blob/main/Modules/Scripted/WebServer/WebServerLib/StaticPagesRequestHandler.py
[DICOMRequestHandler]: https://github.com/Slicer/Slicer/blob/main/Modules/Scripted/WebServer/WebServerLib/DICOMRequestHandler.py
[SlicerRequestHandler]: https://github.com/Slicer/Slicer/blob/main/Modules/Scripted/WebServer/WebServerLib/SlicerRequestHandler.py

:::{note}
The web server is integrated with the Qt event loop so it can be used together with the interactive session.
:::

:::{warning}
This module should be considered somewhat experimental and a likely security risk. Do not expose web server endpoints on the public internet without careful consideration.

Because the web server uses standard http, there are many off-the-shelf security options, such as firewalls, ssh-tunneling, and authenticating proxies, that can be used to improve security of installations.
:::

## Panels and their use

- Start server: Launches web server listening on port `2016`. If the default port is in use, other ports are checked sequentially until an open port is found, allowing more than one Slicer web server to run concurrently.
- Stop server: Stop web server.
- Open static page in external browser: Display `docroot` using default operating system web browser found using `qt.QDesktopServices`.
- Open static page in internal browser: Display `docroot` using Slicer built-in web browser instantiated using `slicer.qSlicerWebWidget()`.
- Log output: If `Log to GUI` is enabled, access log and execution results are logged. Logs are cleared periodically.
- Clear Log: Clear log output displayed in the module panel.
- Advanced:
  - CORS: Enable/disable [Cross-Origin Resource Sharing](https://en.wikipedia.org/wiki/Cross-origin_resource_sharing).
  - Slicer API: Enable/disable use of [Slicer endpoints](#slicer-endpoints) associated with the `/slicer` path.
  - Slicer API exec: Enable/disable remote execution of python code through `/slicer/exec` endpoint. See [Remote Control][#remote-control].
  - DICOMweb API: Enable/disable support of [DICOMWeb endpoints](#dicomweb-endpoints) associated with the `/dicom` path.
  - Static pages: Enable/disable serving of static files found in the `docroot`  associated with the `/` path.
  - Log to Console: Enable/disable the logging of messages in the console.
  - Log to GUI: Enable/disable the logging of messages in the module panel.

:::{note}
Logging to the console and/or the GUI is useful for learning about the software and for debugging, but slows down request handling and should be disabled for routine use.
:::

:::{warning}
The `Slicer API exec` option exposes the full python interface of Slicer running with the same permissions as the Slicer app itself.  This means that users of that API can install arbitrary code on the system and execute it with the user's rights.  In practice this means that the user of the API can perform actions such as deleting files, sending emails, or installing system software.  Exposing these capabilities is intentional and aligned with the design of the module, but users should be aware that enabling this feature is effectively the same as giving the user of the API the password to whatever account is running Slicer.

Note also that even with the `Slicer API exec` disabled, it is possible that other endpoints expose vulnerabilities such as buffer overruns that could lead to server exploits.  It is suggested that only trusted users be granted access to any of the API endpoints.
:::

:::{warning}
Cross-Origin Resource Sharing allows browser-based code
hosted from any origin to access the Slicer API.  That is,
any javascript in a site opened in a browser on the machine
running the server would have access to the API.  While this
feature is useful for some development or specific scenarios
it should be used with caution.  Note that CORS is enforced
by the user's web browser, so even with CORS turned off it's possible
for other software running with access to the port to access
the API even if CORS is turned off.
:::


## Static endpoints

Hosts files out of the module's `docroot` like any standard http server.

Currently this is used just for examples, but note that this server can be used to host [web applications](https://en.wikipedia.org/wiki/Single-page_application) of significant complexity with the option of interacting with the Slicer API.

OHIF DICOM viewer is included as an example (available at <http://localhost:2016/browse>). If the DICOMweb endpoint is enabled then this viewer can be used to quickly share content of the Slicer DICOM database with other computers on the same network. All users that can access the computer are assumed to be trusted - if the server is accessible to non-trusted people then it is recommended to restrict access by setting up a firewall or proxy server.


## DICOMweb endpoints

Exposes the Slicer dicom database as a [DICOMweb endpoint](https://www.dicomstandard.org/using/dicomweb).

This version implements a subset of the `QIDO-RS` and `WADO-RS` specifications allowing to host a web app such as the [OHIF Viewer](https://ohif.org/).

Supported QIDO requests:
- `/dicom/studies`: get list of studies as json, optional query parameters: `offset`, `limit`, `PatientID`
- `/dicom/studies/<studyuid>/metadata`: get DICOM tags of the specified study as json
- `/dicom/studies/<studyuid>/series`: get list of series for a study as json
- `/dicom/studies/<studyuid>/series/<seriesuid>/metadata`: get DICOM tags of the specified series as json
- `/dicom/studies/<studyuid>/series/<seriesuid>/instances`: get list of instances for a series as json
- `/dicom/studies/<studyuid>/series/<seriesuid>/instances/<sopinstanceuid>`: download the instance
- `/dicom/studies/<studyuid>/series/<seriesuid>/instances/<sopinstanceuid>/metadata`: get DICOM tags of the specified instance as json

Supported WADO requests:
- `/dicom?object=<sopinstanceuid>`: downloads the specified instance

For OHIF version 2, change the `platform/viewer/public/config/default.js`, set the `servers` configuration key as follows.

```
  servers: {
    dicomWeb: [
      {
        name: 'DCM4CHEE',
        wadoUriRoot: 'http://localhost:2016/dicom',
        qidoRoot: 'http://localhost:2016/dicom',
        wadoRoot: 'http://localhost:2016/dicom',
        qidoSupportsIncludeField: true,
        imageRendering: 'wadouri',
        thumbnailRendering: 'wadouri',
        enableStudyLazyLoad: true,
        supportsFuzzyMatching: true,
      },
    ],
  },
```

## Slicer endpoints

Full specification of the Slicer API is available at the bottom of this page. The API is subject to change.

### Remote Control

The web server can also be accessed via other commands such as `curl`. A `dict` can be returned as a json object by setting it in the `__execResult` variable and enabling the `Slicer API exec` feature in the `Advanced` section.

For example, these commands may be used to download the MRHead sample data, change the screen layout and return a dictionary including the ID of the loaded volume:

```
curl -X POST localhost:2016/slicer/exec --data "import SampleData; volumeNode = SampleData.SampleDataLogic().downloadMRHead(); slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView); __execResult = {'volumeNodeID': volumeNode.GetID()}"
```

:::{note}

See the [Script Repository](../../developer_guide/script_repository.md) for other examples of python code you could remotely execute.

:::

### Remote Rendering

There are several endpoints to get png images from slice views and 3D views. These endpoints allow control of slice offset or 3D camera view (see method doc strings in the source code for options since there is currently no auto-generated api documentation).  These endpoints can be used as the `src` or `href` for html `img` or `a` tags or as WebGL textures as shown in the demo scripts.

### Data Access

Give read/write access to features in Slicer's MRML scene and GUI.

`http` `GET`, `POST`, `DELETE` operations can be used to query, load, save, delete data in the scene.

#### Usage via slicerio Python package

The API can be conveniently used via the [slicerio Python package](https://pypi.org/project/slicerio/) **in any Python environment**. For example, to open an image file and a segmentation file in a single Slicer instance (without restarting a new instance for each file), run this code snippet:

```python
import slicerio.server
slicerio.server.file_load("c:/tmp/MRHead.nrrd")
slicerio.server.file_load("c:/tmp/Segmentation.nrrd", "SegmentationFile")
```

note: If Slicer application is not running (or the server API is disabled) then `slicerio.server.file_load` will start a new Slicer instance by launching the Slicer executable specified either in the `slicer_executable` function argument or in the `SLICER_EXECUTABLE` environment variable.

Data nodes can be retrieved from Slicer by saving into local file:

```python
slicerio.server.file_save("c:/tmp/MRHeadOutput.nrrd", name="MRHead", properties={'useCompression': False})
```

Properties of nodes can be queried using `slicerio.server.node_properties`:

```python
properties = slicerio.server.node_properties(name="Segmentation")[0]
segments = properties["Segmentation"]["Segments"]
for segmentId in segments:
    segment = segments[segmentId]
    print(f"{segment['Name']} color: {segment['Color']}")
```

Full specification and more examples are available in the `slicerio` package documentation.

#### Usage via curl

For example, to save a nrrd version of a volume in Slicer, you can use:
```
curl -v http://localhost:2016/slicer/volume\&id='MRHead' -o /tmp/local.nrrd
```

Currently only limited forms are supported (scalar volumes and grid transforms).

Other endpoints allow get/set of transforms and fiducials.

## Slicer REST API

### Remote control (exec)

#### GET /exec

Run script in Slicer's Python interpreter, as if it was run in the application's Python console.
It can be used to implement a Read Eval Print Loop (REPL).

Parameters:
- `source`: Python code to run

Return:
- 200 (application/json): Result of code running as json string. The result must be set in a `__execResult` variable (`dict` object)
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

Example:

    curl -X POST localhost:2016/slicer/exec --data "slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView)"

### MRML data access

Get information on MRML nodes in the scene or load/save nodes.

Common parameters for data selection for all methods:
- `id`: select node that has this id.
- `class`: select nodes of this class (select nodes of this class (e.g., vtkMRMLVolumeNode, vtkMRMLSegmentationNode)
- `name`: select nodes of this name

If `id` is specified then `class` and `name` parameters are ignored.
If both `class` and `name` are specified then those nodes will be selected that have fulfill both selection criteria.

#### GET /mrml and GET /mrml/names

Get names of the selected nodes.

Parameters:
- `id`: as described above
- `class`: as described above
- `name`: as described above

Return:
- 200 (application/json): list of node names.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /mrml/ids

Get ids of the selected nodes.

Parameters:
- `id`: as described above
- `class`: as described above
- `name`: as described above

Return:
- 200 (application/json): list of node ids.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /mrml/properties

Get properties of the selected nodes as a json object.

Parameters:
- `id`: as described above
- `class`: as described above
- `name`: as described above

Return:
- 200 (application/json): dictionary object, key is the node id, value is the node properties object.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /mrml/file

Save node to local file. Query parameters must be specified so that only a single node is selected.

Parameters:
- `id`: as described above
- `class`: as described above
- `name`: as described above
- `localfile`: filename to save the node to
- `useCompression`: specifies if the file should be saved using compression (`true` or `false`)

Return:
- 200 (application/json): JSON object, with `success` property set to true.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### POST /mrml/file

Load node from URL or local file into a node.

Parameters:
- `localfile`: Local filename to load the node from. If specified then `url` is ignored.
- `url`: Local or remote URL to load the file from.
- `filetype`: Specifies how to interpret the selected file. For example `VolumeFile`, `SegmentationFile`, `ModelFile`, `MarkupsFile`, `TransformFile`, `SceneFile`.

Return:
- 200 (application/json): JSON object, "success" property is set true and `loadedNodeIDs` property contains a list of loaded node ids.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### DELETE /mrml/file

Remove nodes from the scene. If no query parameters are specified then the whole scene is cleared.

Parameters:
- `id`: as described above
- `class`: as described above
- `name`: as described above
- `localfile`: filename to save the node to
- `useCompression`: specifies if the file should be saved using compression (`true` or `false`)

Return:
- 200 (application/json): JSON object, with `success` property set to true.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### PUT /mrml

Reload node that was originally loaded from a file, from the same file.
This is useful if the input file is changed since it was last loaded into Slicer.

Parameters:
- `id`: as described above
- `class`: as described above
- `name`: as described above

Return:
- 200 (application/json): JSON object, "success" property is set true and `reloadedNodeIDs` property contains a list of reloaded node ids.
  "success" property is set to false if it was not possible to reload any nodes.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

### User interface

#### PUT /gui

Shutdown the application.

Parameters:
- `contents`: show full application GUI (`full`) or viewers only (`viewers`)
- `viewersLayout`: set view layout, such as `fourup`, `oneup3d` (names derived from slicer.vtkMRMLLayoutNode constants - SlicerLayout...View)

Return:
- 200 (application/json): JSON object, with `success` property set to true.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /screenshot

Get screenshot of the application main window.

Return:
- 200 (image/png): screenshot image
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /slice

Get screenshot of a slice view after applying parameters.

Parameters:
- `view`: `red`, `yellow`, or `green`
- `scrollTo`: 0 to 1 for slice position within volume
- `offset`: mm offset relative to slice origin (position of slice slider)
- `size`: pixel size of output png
- `copySliceGeometryFrom`: view name of other slice to copy from
- `orientation`: `axial`, `sagittal`, `coronal`

Return:
- 200 (image/png): screenshot image
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /threeD

Get screenshot of the first 3D view after applying parameters.

Parameters:
- `lookFromAxis`: `L`, `R`, `A`, `P`, `I`, `S`

Return:
- 200 (image/png): screenshot image
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /timeimage

For timing and debugging - return an image with the current time rendered as text down to the hundredth of a second.

Parameters:
- `color`: hex encoded RGB of dashed border (default 333 for dark gray)

Return:
- 200 (image/png): rendered image
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

### Other functions

#### GET /system/version

Get application version information as a json object.

Return:
- 200 (application/json): version information object.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### DELETE /system

Shutdown the application.

Return:
- 200 (application/json): JSON object, with `success` property set to true.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /tracking

Display/update position of a cursor (position marker cube) in the 3D view.
This can be used to display position of a tracked object.

Parameters:
- `m`: 4x4 transformation matrix in column major order (position is last row).
  Matrix is overwritten if position or quaternion are provided
- `q`: quaternion in WXYZ order
- `p`: position (last column of transform)

Return:
- 200 (text/plain): plain text message for the user
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /sampledata

Load a sample data set into the scene.

Parameters:
- `name`: name of the sample data set (such as `MRHead`)

Return:
- 200 (text/plain): plain text message for the user
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /volumeSelection

Cycles through loaded volumes in the scene.

Parameters:
- `cmd`: either `next` or `previous` to indicate direction

Return:
- 200 (text/plain): plain text message for the user
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /volumes, GET /gridtransforms

Get a list of mrml volume or grid transform node names and ids.

Parameters:
- `cmd`: either `next` or `previous` to indicate direction

Return:
- 200 (application/json): list of json objects, with `name` and `id` attributes.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

Example of successful output:

    [
        {"name": "Volume1", "id": "vtkMRMLScalarVolumeNode1"},
        {"name": "Volume2", "id": "vtkMRMLScalarVolumeNode2"},
    ]

#### GET /volume, GET /griddtransform

Retrieve the specified volume or grid transform as a .nrrd file.

Parameters:
- `id`: id of the node to get

Return:
- 200 (application/octet-stream): data stream of a nrrd file
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### POST /volume

Create or update a volume from a .nrrd file.
Only uncompressed 3D volumes are accepted, in LPS coordinate system, with little endian short pixel type.

Parameters:
- `id`: id of the volume to create or update.

Return:
- 200 (application/json): data stream of a nrrd file
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### GET /fiducials

Retrieve basic information about all markup point lists (formerly called fiducial lists) in the scene.

Return:
- 200 (application/json): Basic information about all markup point lists.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

Example of successful output:

    {
        "vtkMRMLMarkupsFiducialNode1": {
            "name": "F",
            "color": [1.0, 0.5000076295109483, 0.5000076295109483],
            "scale": 3.0,
            "markups": [
                {"label": "F-1", "position": [-35.422643698898014, 13.121414583492907, -10.214302062988281]},
                {"label": "F-2", "position": [43.217879176918984, 41.565859027937364, -10.214302062988281]},
                {"label": "F-3", "position": [39.8714739481608, -32.05505600474238, -10.214302062988281]}]},
        "vtkMRMLMarkupsFiducialNode2": {
            "name": "F_1",
            "color": [1.0, 0.5000076295109483, 0.5000076295109483],
            "scale": 3.0,
            "markups": [
                {"label": "F_1-1", "position": [82.53814061482748, 13.121414583492907, -23.599922978020956]},
                {"label": "F_1-2", "position": [-4.468395332884938, 13.121414583492907, 65.07981558407056]}]}
    }

#### PUT /fiducial

Set the location of a control point in a markups point list (formerly called fiducial list).

Parameters:
- `id`: id of the node to update.
- `r`: Right coordinate
- `a`: Anterior coordinate
- `s`: Superior coordinate

Return:
- 200 (application/json): JSON object, with `success` property set to true.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

#### POST /accessDICOMwebStudy

Access DICOMweb server to download requested study, add it to Slicer's dicom database, and load it into the scene.

Request body: json string with the following properties
- 'dicomWEBPrefix': is the start of the url
- 'dicomWEBStore': is the middle of the url
- 'studyUID': is the end of the url
- 'accessToken': is the authorization bearer token for the DICOMweb server

Return:
- 200 (application/json): JSON object, with `success` property set to true.
- 500 (application/json): In case of unexpected error. `message` attribute contains error message.

## Related modules

- The [OpenIGTLink](https://github.com/openigtlink/SlicerOpenIGTLink) Extension has some similar functionality customized for image guided therapy applications. It should be preferred for integration with imaging devices and use in a clinical setting or setting up continuous high-throughput image and transform streams.

## Future work

Features have been added to this server based on the needs of demos and proof of concept prototypes.  A more comprehensive mapping of the Slicer API to a web accessible API has not yet been performed.  Similarly, the DICOMweb implementation is bare-bones and has only been implemented to the extent required to support a simple viewer scenario without performance optimizations.  The existing framework could also be improved through the implementation of newer HTTP features and code refactoring.

## History

The development of the first implementation was started by Steve Pieper in 2012 and has been developed over the years to include additional experiments. See [https://github.com/pieper/SlicerWeb](https://github.com/pieper/SlicerWeb)

Then, in November 2021, a stripped down version of the module addressing the most common expected use cases was proposed in pull request [#5999](https://github.com/Slicer/Slicer/pull/5999).

In May 2022, the module was integrated into Slicer.

## Contributors

- Steve Pieper, original author (Isomics)
- Andras Lasso, refactoring and Slicer integration (Queens)
- Jean-Christophe Fillion-Robin (Kitware)

## Acknowledgements

This work was partially funded by NIH grant 3P41RR013218.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
