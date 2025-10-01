# Sequences

## Volume sequence file format (.seq.nrrd)

Whenever it is possible (when origin, spacing, axis directions, and extents are the same in all sequence items), a sequence of volumes is stored in a standard NRRD image file with custom fields to store additional metadata.

### Image data

The image is always stored in 3 spatial dimensions. Even if an image consists of a single slice, the spatial dimension must be still 3 because that is required for specification origin, spacing, and axis directions in 3D space. The file has an additional `list` dimension.

### Metadata

#### Standard fields

- `dimension`: `4`
- `sizes`: image size (I, J, K) followed by the number of time points
- `space directions`: I, J, K axis directions in physical space followed by `none`
- `kinds`: `domain domain domain list`
- `labels`: ` "" "" ""` followed by name of the sequence index (e.g., `"time"`)
- other standard fields (`type`, `space`, `endian`, `encoding`, `space origin`, etc.) can have any value

#### Custom fields

The sequence-specific metadata is stored in NRRD key–value pairs that follow this pattern:

* `axis <A> index type:=<IndexType>`
* `axis <A> index values:=<v0> <v1> ... <vN-1>`
* `axis <A> item <I> <AttrName>:=<AttrValue>`
* `DataNodeClassName:=<MRMLNodeClass>`

Where:

* `<A>` is the **list axis index**. In Slicer-generated `.seq.nrrd` files the list axis is **3**, because sizes are ordered as `I, J, K, list`.
* `<IndexType>` is either `numeric` or `text` and determines how index values are interpreted (e.g., numeric vs. lexicographic sorting).
* `<v0> ... <vN-1>` are the **per-item index values** along the list axis (must have as many values as there are list items). These are stored here rather than using standard NRRD spacing because sequence indices may be **non-uniform** or **non-numeric**.
* `<I>` is the **0-based item index** along the list axis.
* `<AttrName>` / `<AttrValue>` store **per-item node attributes** for the data nodes in the sequence.
* `<MRMLNodeClass>` is the MRML class name of the proxy node to instantiate when reading the sequence (e.g., `vtkMRMLScalarVolumeNode`; for labelmaps use `vtkMRMLLabelMapVolumeNode`).

#### Field definitions

* `axis <A> index type:=numeric|text`
  Declares the index value type for the list axis (affects sorting and parsing).

* `axis <A> index values:=<v0> <v1> ... <vN-1>`
  Provides the explicit index value for each sequence item, separated by spaces. Values can be arbitrary strings, stored using URL encoding.

* `axis <A> item <I> <AttrName>:=<AttrValue>`
  Stores arbitrary per-item attributes from the sequence's proxy nodes (e.g., acquisition time, frame UID).

* `DataNodeClassName:=<MRMLNodeClass>`
  Specifies the MRML node class to create for items in this sequence. Common values include:

  * `vtkMRMLScalarVolumeNode` (default)
  * `vtkMRMLLabelMapVolumeNode` (labelmaps)

:::{note}
* Slicer writes the list axis as **axis 3** and sets `kinds: domain domain domain list`.
* The count of `index values` **must match** the size of the list axis (`sizes[3]` in Slicer files).
* Use `numeric` when index values are intended to be ordered by numeric value (e.g., frame number, time in seconds). Use `text` for non-numeric or mixed tokens (e.g., `"pre" "post" "followup1"`).
:::

#### Example

```text
NRRD0004
# Complete NRRD file format specification at:
# http://teem.sourceforge.net/nrrd/format.html
type: short
dimension: 4
space: left-posterior-superior
sizes: 128 104 72 10
space directions: (1.1499999999999999,0,0) (0,1.1499999999999999,0) (0,0,2) none
kinds: domain domain domain list
labels: "" "" "" "frame"
endian: little
encoding: gzip
space origin: (-62.20000000000001,-68,1692.0989989999998)
DataNodeClassName:=vtkMRMLScalarVolumeNode
axis 3 index type:=numeric
axis 3 index values:=0 1 2 3 4 5 6 7 8 9
# Example per-item attribute (item 5)
axis 3 item 5 AcquisitionTime:=2024-06-21T10:32:45.120Z
```
