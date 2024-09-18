# Sequences

## Volume sequence file format (.seq.nrrd)

Whenever it is possible (when origin, spacing, axis directions, and extents are the same in all sequence items), a sequence of volumes is stored in a standard NRRD image file with custom fields to store additional metadata.

### Image data

The image is always stored in 3 spatial dimensions. Even if an image consists of a single slice, the spatial dimension must be still 3 because that is required for specification origin, spacing, and axis directions in 3D space. The file has an additional `list` dimension.

### Metadata

#### Standard fields

- `dimension`: `4`
- `sizes`: number of time points, followed by image size (I, J, K)
- `space directions`: `none`, followed by I, J, K axis directions in physical space
- `kinds`: `list domain domain domain`
- `labels`: name of the sequence index (e.g., `"time"`), followed by ` "" "" ""`
- other standard fields (`type`, `space`, `endian`, `encoding`, `space origin`, etc.) can have any value

#### Custom fields

- `axis 0 index type`: `numeric` or `text` (used for example for sorting).
- `axis 0 index values`: index value for each sequence item (cannot use spacing value in the standard NRRD fields, because the values may not be equally spaced and the index values are not always numeric).
- `DataNodeClassName`: class name of the proxy node that needs to be created when the sequence is read. Default (and most common) is `vtkMRMLScalarVolumeNode`.
  For labelmap volumes: `vtkMRMLLabelMapVolumeNode`.

### Example

```text
NRRD0005
# Complete NRRD file format specification at:
# http://teem.sourceforge.net/nrrd/format.html
type: int
dimension: 4
space: right-anterior-superior
sizes: 26 102 102 61
space directions: none (1.9531249999999991,0,0) (0,1.9531249999999991,0) (0,0,1.9531249999999991)
kinds: list domain domain domain
labels: "frame" "" "" ""
endian: little
encoding: gzip
space origin: (-137.16099548339844,-36.806499481201172,-309.71899414062506)
measurement frame: (1,0,0) (0,1,0) (0,0,1)
DataNodeClassName:=vtkMRMLScalarVolumeNode
axis 0 index type:=numeric
axis 0 index values:=0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
```
