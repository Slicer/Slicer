# Event Broker

## Overview

Profiling tool for the developers. The event broker manages observations between VTK objects in Slicer. Observed objects are populated in the tree view, their observers are listed under each observed object events. Selecting a VTK object in the list, dumps (PrintSelf()) all its information.

## Panels and their use

- **Actions**: Actions in the view.
  - **Refresh**: Rescan the event broker and populate the tree view with the observers of the MRML scene, nodes, logics.
  - **Reset Times**: Reset the time spent in all callbacks to 0 second. To be used with "Show observations with Elapsed Times
  - **Show observations with Elapsed Times >0**: Filter the tree view to show only objects for which time has been spent in at least one of its callback.

## Contributors

Julien Finet (Kitware)

## Acknowledgements

This work is part of the National Alliance for Medical Image Computing (NAMIC), funded by the National Institutes of Health through the NIH Roadmap for Medical Research, Grant U54 EB005149.
