# Retired Modules

The modules listed below have been retired and are no longer present in 3D Slicer. This page exists as a convenient marker for historical details about retired modules.

Retiring modules reduces burden on Slicer developers. Most common reason for retiring a module is that a new, improved module is introduced.
A module may also be retired if it no longer works properly (for example due to changes in Slicer core or third-party libraries) and it is not worth investing time into fixing the issue, because the module is not widely used or alternatives exist.

- **Editor**
  - Retired: 2021-11-05, [commit 39283db](https://github.com/Slicer/Slicer/commit/39283db420baf502fa99865c9d5d58d0e5295a6e)
  - Reason: [No longer running properly](https://github.com/Slicer/Slicer/issues/5962) and superseded by the [Segment Editor](segmenteditor.md) module.
- **Label Statistics**
  - Retired: 2021-11-05, [commit cf62bcf](https://github.com/Slicer/Slicer/commit/cf62bcfc89d4fc2606a84ac51f741a93d7037299)
  - Reason: Superseded by the [Segment Statistics](segmentstatistics.md) module.
- **Expert Automated Registration**
  - Retired: 2022-02-19, [commit 87fd349](https://github.com/Slicer/Slicer/commit/87fd349334e6414c28ba373bbc45d03c7345ad0c)
  - Reason: [No longer running properly](https://github.com/Slicer/Slicer/pull/6200#issuecomment-1045962304). BRAINS, Elastix, and ANTs registration toolkits offer superior registration results, see [Automatic Image Registration](../registration.md) section for details.
- **DataStore**
  - Retired: 2022-04-26, [commit fbbac34](https://github.com/Slicer/Slicer/commit/fbbac34e314666d94ad5e70e71736d103e003cc8)
  - Reason: Infrastructure for uploading and organizing datasets was based on the obsolete Slicer server based on Midas. Associated datasets are available as release assets associated with the [Slicer/SlicerDataStore](https://github.com/Slicer/SlicerDataStore) GitHub project.
