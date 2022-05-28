# Segment editor

See examples of using Segment editor effects from Python scripts in the [script repository](../script_repository.md#how-to-run-segment-editor-effects-from-a-script).

## Effect parameters

Common parameters must be set using `setCommonParameter` method (others can be set using `setParameter` method). Both common and effect-specific parameters can be retrieved using

### Fill between slices

| Parameter  | Type | Common | Default | Values |
|------------|------|--------|---------|--------|
| AutoUpdate | int  | no     | 1       | 0 or 1 |

### Grow from seeds

| Parameter          | Type  | Common | Default | Values |
|--------------------|-------|--------|---------|--------|
| SeedLocalityFactor | float | no     | 0.0     | >=0.0  |
| AutoUpdate         | int   | no     | 1       | 0 or 1 |

### Hollow

| Parameter                 | Type  | Common | Default        | Values                     |
|---------------------------|-------|--------|----------------|----------------------------|
| ApplyToAllVisibleSegments | int   | no     | 0              | 0 or 1                     |
| ShellMode                 | enum  | no     | INSIDE_SURFACE | INSIDE_SURFACE, MEDIAL_SURFACE, OUTSIDE_SURFACE |
| ShellThicknessMm          | float | no     | 3.0            | >0.0                       |

### Islands

| Parameter     | Type | Common | Default             | Values |
|---------------|------|--------|---------------------|--------|
| Operation     | enum | no     | KEEP_LARGEST_ISLAND | KEEP_LARGEST_ISLAND, KEEP_SELECTED_ISLAND, REMOVE_SMALL_ISLANDS, REMOVE_SELECTED_ISLAND, ADD_SELECTED_ISLAND, SPLIT_ISLANDS_TO_SEGMENTS |
| MinimumSize   | int  | no     | 1000                | >0     |

### Logical operators

| Parameter                 | Type   | Common | Default | Values                     |
|---------------------------|--------|--------|---------|----------------------------|
| Operation                 | enum   | no     | COPY    | COPY, UNION, INTERSECT, SUBTRACT, INVERT, CLEAR, FILL |
| ModifierSegmentID         | string | no     |         | segment ID                 |
| BypassMasking             | int    | no     | 0       | 0 or 1                     |

### Margin

| Parameter                 | Type  | Common | Default | Values                     |
|---------------------------|-------|--------|---------|----------------------------|
| ApplyToAllVisibleSegments | int   | no     | 0       | 0 or 1                     |
| MarginSizeMm              | float | no     | 3.0     | <0.0 (shrink), >0.0 (grow) |

### Paint effect and Erase effect

| Parameter                    | Type  | Common | Default | Values |
|------------------------------|-------|--------|---------|--------|
| BrushMinimumAbsoluteDiameter | float | yes    | 0.01    | >=0.0  |
| BrushMaximumAbsoluteDiameter | float | yes    | 100.0   | >0.0   |
| BrushAbsoluteDiameter        | float | yes    | 5.0     | >=0.0  |
| BrushRelativeDiameter        | float | yes    | 3.0     | >=0.0  |
| BrushRelativeDiameter        | int   | yes    | 1       | 0 or 1 |
| BrushSphere                  | int   | yes    | 0       | 0 or 1 |
| EditIn3DViews                | int   | yes    | 0       | 0 or 1 |
| ColorSmudge                  | int   | no     | 0       | 0 or 1 |
| EraseAllSegments             | int   | no     | 0       | 0 or 1 |
| BrushPixelMode               | int   | yes    | 0       | 0 or 1 |

### Scissors

| Parameter                 | Type  | Common | Default     | Values                                             |
|---------------------------|-------|--------|-------------|----------------------------------------------------|
| ApplyToAllVisibleSegments | int   | no     | 0           | 0 or 1                                             |
| Operation                 | enum  | no     | EraseInside | EraseInside, EraseOutside, FillInside, FillOutside |
| Shape                     | enum  | no     | FreeForm    | FreeForm, Circle, Rectangle                        |
| ShapeDrawCentered         | int   | no     | 0           | 0 (first click defines rectangle or circle boundary), 1 (click defines shape center) |
| SliceCutMode              | enum  | no     | Unlimited   | Unlimited, Positive, Negative, Symmetric           |
| SliceCutDepthMm           | float | no     | 0.0         | >=0.0 (single slice = 0.0)                         |

### Smoothing

| Parameter                   | Type  | Common | Default | Values                     |
|-----------------------------|-------|--------|---------|----------------------------|
| ApplyToAllVisibleSegments   | int   | no     | 0       | 0 or 1                     |
| SmoothingMethod             | enum  | no     | MEDIAN  | MEDIAN, GAUSSIAN, MORPHOLOGICAL_OPENING, MORPHOLOGICAL_CLOSING, JOINT_TAUBIN |
| KernelSizeMm                | float | no     | 3.0     | >0.0                       |
| GaussianStandardDeviationMm | float | no     | 3.0     | >0.0                       |
| JointTaubinSmoothingFactor  | float | no     | 0.5     | >0.0                       |

### Threshold

| Parameter           | Type  | Common | Default                     | Values                  |
|---------------------|-------|--------|-----------------------------|-------------------------|
| AutoThresholdMethod | enum  | no     | OTSU                        | HUANG, INTERMODES, ISO_DATA, KITTLER_ILLINGWORTH, LI, MAXIMUM_ENTROPY, MOMENTS, OTSU, RENYI_ENTROPY, SHANBHAG, TRIANGLE, YEN |
| MinimumThreshold    | float | no     | 25th percentile voxel value |                         |
| MaximumThreshold    | float | no     | maximum voxel value         |                         |
| AutoThresholdMode   | enum  | no     | SET_LOWER_MAX               | SET_LOWER_MAX, SET_UPPER, SET_LOWER, SET_MIN_UPPER |
| BrushType           | enum  | no     | CIRCLE                      | CIRCLE, BOX, DRAW, LINE |
| HistogramSetLower   | enum  | no     | LOWER                       | MINIMUM, LOWER, AVERAGE |
| HistogramSetUpper   | enum  | no     | UPPER                       | AVERAGE, UPPER, MAXIMUM |

## Examples

Examples for common operations on segmentations are provided in the [script repository](../script_repository.md#segmentations).
