# WebServer

## Overview

Creates a fairly simple but powerful web server that can respond to http(s) requests with data from the current application state or modify the application state. This module is meant to be the basis for implementing web applications that use Slicer as a remote render / computation engine or for controlling your Slicer instance for interaction with other system code like shell scripts or other applications.

There are three basic types of endpoints:
- **Static:** Hosts files out of the module's `docroot` like any standard http server. Currently this is used just for examples.
- **Slicer:** Give read/write access to features in Slicer's MRML scene and GUI. This interface also exposes the Python interpreter so that arbitrary python code can be executed in the Slicer application (the user is prompted to approve this endpoint before code is accepted).
- **DICOMweb:** Exposes the Slicer dicom database as a [DICOMweb endpoint](https://www.dicomstandard.org/dicomweb). This is somewhat limited, but enough to host a web app such as the [OHIF Viewer](https://ohif.org/).

The web server is integrated with the Qt event loop so it can be used together with the interactive session.

This code has been developed over a number of years in a separate repository where there are additional experiments demonstrating other potential uses. The version in Slicer core has been stripped down to address the most common expected use cases. See [https://github.com/pieper/SlicerWeb](https://github.com/pieper/SlicerWeb).

Also note that the code should be considered somewhat experimental and a likely security risk.  Do not expose web server endpoints on the public internet without careful consideration.

## Current Features

The server launches on port `2016` by default when you enter the module. Access logs are shown in the GUI by default and can be routed to the console if persistence is needed (the GUI logs are cleared periodically).  If the default port is in use, other ports are checked sequentially until an open port is found, allowing more than one Slicer web server to run concurrently.

Buttons in the module can launch either a desktop browser tab or a `qSlicerWebWidget` pointing to the server's demo page.

The demo page shows most of the features of the Slicer web api and is hosted using the Static api.

## Remote Control

The web server can also be accessed via other commands such as `curl`. A `dict` can be returned as a json object by setting it in the `__execResult` variable.

For example, these command can be used to download data and change the screen layout:

```
curl -X POST localhost:2016/slicer/exec --data "import SampleData; volumeNode = SampleData.SampleDataLogic().downloadMRHead(); slicer.app.layoutManager().setLayout(slicer.vtkMRMLLayoutNode.SlicerLayoutOneUpRedSliceView); __execResult = {'volumeNodeID': volumeNode.GetID()}"
```

See the [Script Repository](../../developer_guide/script_repository.md) for other things you could control remotely.

## Remote Rendering

There are several endpoints to get png images from slice views and 3D views. These endpoints allow control of slice offset or 3D camera view (see method doc strings in the source code for options since there is currently no auto-generated api documentation).  These endpoints can be used as the `src` or `href` for html `img` or `a` tags or as WebGL textures as shown in the demo scripts.

## Data Access

`http` `GET` and `POST` operations can be used to access volume data in nrrd format.

For example, to save a nrrd version of a volume in Slicer, you can use:
```
curl -v http://localhost:2016/slicer/volume\&id='MRHead' -o /tmp/local.nrrd
```

Currently only limited forms are supported (scalar volumes and grid transforms).

Other endpoints allow get/set of transforms and fiducials.

## Panels and their use

Currently there is only the logging console and the demo buttons. Further controls may be added depending on needs.


## Related modules

- The [OpenIGTLink](https://github.com/openigtlink/SlicerOpenIGTLink) Extension has some similar functionality customized for image guided therapy applications. It should be preferred for integration with imaging devices and use in a clinical setting or setting up continuous high-throughput image and transform streams.

## Contributors

- Steve Pieper (Isomics)

## Acknowledgements

This work was partially funded by NIH grant 3P41RR013218.

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/logo_isomics.png)
