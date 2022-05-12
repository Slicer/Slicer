# WebServer

## Overview

Creates a fairly simple but powerful web server that can respond to http(s) requests with data from the current application state or modify the application state.

This module is meant to be the basis for implementing web applications that use Slicer as a remote render / computation engine or for controlling your Slicer instance for interaction with other system code like shell scripts or other applications.

There are three types of endpoints:

| Type | Description | Path | Handler |
|--|--|--|--|
| [Static](#static-endpoints) | Hosts files out of the module's `docroot` like any standard http server. | `/` | [StaticPagesRequestHandler][StaticPagesRequestHandler] |
| [Slicer](#slicer-endpoints) | Give read/write access to features in Slicer's MRML scene and GUI.</br>This interface also exposes the Python interpreter so that arbitrary python code may be executed in the Slicer application. | `/slicer` | [SlicerRequestHandler][SlicerRequestHandler] |
| [DICOMweb](#dicom-endpoints) | Exposes the Slicer dicom database as a DICOMweb services | `/dicom` | [DICOMRequestHandler][DICOMRequestHandler] |

[StaticPagesRequestHandler]: https://github.com/Slicer/Slicer/blob/master/Modules/Scripted/WebServer/WebServerLib/StaticPagesRequestHandler.py
[SlicerRequestHandler]: https://github.com/Slicer/Slicer/blob/master/Modules/Scripted/WebServer/WebServerLib/SlicerRequestHandler.py
[DICOMRequestHandler]: https://github.com/Slicer/Slicer/blob/master/Modules/Scripted/WebServer/WebServerLib/DICOMRequestHandler.py

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

## Static endpoints

Hosts files out of the module's `docroot` like any standard http server.

Currently this is used just for examples, but note that this server can be used to host [web applications](https://en.wikipedia.org/wiki/Single-page_application) of significant complexity with the option of interacting with the Slicer API.

## Slicer endpoints

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

`http` `GET` and `POST` operations can be used to access volume data in nrrd format.

For example, to save a nrrd version of a volume in Slicer, you can use:
```
curl -v http://localhost:2016/slicer/volume\&id='MRHead' -o /tmp/local.nrrd
```

Currently only limited forms are supported (scalar volumes and grid transforms).

Other endpoints allow get/set of transforms and fiducials.

## DICOMweb endpoints

Exposes the Slicer dicom database as a [DICOMweb endpoint](https://www.dicomstandard.org/using/dicomweb).

This version implements a subset of the `QIDO-RS` and `WADO-RS` specifications allowing to host a web app such as the [OHIF Viewer](https://ohif.org/).

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
