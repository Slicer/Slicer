# Colors

## Color table CSV file format (.csv)

The color file format can store a [color table node](https://apidocs.slicer.org/main/classvtkMRMLColorTableNode.html) in a standard comma-separated value file with the `.csv` extension. Columns are separated by comma (`,`) character. Values that contain comma must be put between quotation marks (`"`). Due to a limitation in VTK library's delimited text reader/writer classes, values must not contain quotation mark characters.

Columns:
- `LabelValue`: label value that is associated with the color. Typically it is used for assigning color and meaning to a voxel value in a label image. It must be a positive integer. 0 value always corresponds to the background value and generally should not be included in the file (only in the rare case when the background needs to be colored). Label values do not have to be sorted. Label values may be skipped (left undefined), but since label values may be stored in a contiguous table from 0 to the largest label value, it is generally recommended to keep the range of values small (e.g., do not use label values such as 1000, 1001, 1002, 15000, 15001 but instead use the values 10, 11, 12, 20, 21).
- `Name`: internal (project-specific) name. It is only intended for convenience during development and testing. Content of a color table entry is intended to be described by standard terminology columns. Optional.
- `Color_R`, `Color_G`, `Color_B`, `Color_A`: red, green, blue, and opacity (alpha) component of the color. Values are integer, between 0 and 255. `Color_A` column is optional, if missing then the default value 255 (fully opaque) is used.
- Standard terminology columns for specifying content of the segment. The columns are optional and values in them can be empty. However, if there are non-empty values then `Category` and `Type` must be specified. For clinical imaging, it is recommended to use [coding schemes defined for use in DICOM](https://dicom.nema.org/medical/dicom/current/output/html/part16.html#chapter_8).
    - `Category_CodingScheme`, `Category_CodeValue`, `Category_CodeMeaning`: category, such as "anatomical structure" or "physical object"
    - `Type_CodingScheme`, `Type_CodeValue`, `Type_CodeMeaning`: type, such as "Kidney" or "Mass"
    - `TypeModifier_CodingScheme`, `TypeModifier_CodeValue`, `TypeModifier_CodeMeaning`: type modifier, such as "Left" or "Right"
    - `Region_CodingScheme`, `Region_CodeValue`, `Region_CodeMeaning`: region where the item is located in, such as "Kidney"
    - `RegionModifier_CodingScheme`, `RegionModifier_CodeValue`, `RegionModifier_CodeMeaning`: region modifier, such as "Left" or "Right"

Example:

```
LabelValue,Name,Color_R,Color_G,Color_B,Color_A,Category_CodingScheme,Category_CodeValue,Category_CodeMeaning,Type_CodingScheme,Type_CodeValue,Type_CodeMeaning,TypeModifier_CodingScheme,TypeModifier_CodeValue,TypeModifier_CodeMeaning,Region_CodingScheme,Region_CodeValue,Region_CodeMeaning,RegionModifier_CodingScheme,RegionModifier_CodeValue,RegionModifier_CodeMeaning
1,left_kidney,185,102,83,255,SCT,123037004,Anatomical Structure,SCT,64033007,Kidney,SCT,7771000,Left,,,,,,
5,right_kidney,185,102,83,255,SCT,123037004,Anatomical Structure,SCT,64033007,Kidney,SCT,24028007,Right,,,,,,
6,right_kidney_mass,144,238,144,255,SCT,49755003,Morphologically Altered Structure,SCT,4147007,Mass,,,,SCT,64033007,Kidney,SCT,24028007,Right
10,catheter_renal_artery,127,127,127,255,SCT,260787004,Physical object,SCT,19923001,Catheter,,,,SCT,2841007,Renal artery,,,
```

## Color table text file format (.txt, .ctbl)

The color file format can store a [color node](https://apidocs.slicer.org/main/classvtkMRMLColorNode.html) in a plain text file with the `.txt` or `.ctbl` extension. It is a text file with values separated by space, with a custom header to specify lookup table type. Header lines are prefixed with `#`.

### Discrete scale color lookup table

Header starts with `# Color table file`. Each data line contains `color index` (integer), `color name` (string, if the name contains spaces then the spaces must be replaced by underscore), `red` (0-255), `green` (0-255), `blue` (0-255), and `opacity` (0-255).

Example:

```
# Color table file C:/Users/andra/OneDrive/Projects/SlicerTesting2022/20220109-ColorLegend/Segmentation-label_ColorTable.ctbl
# 4 values
0 Background 0 0 0 0
1 artery 216 101 79 255
2 bone 241 214 145 255
3 connective_tissue 111 184 210 255
```

### Continuous scale color lookup table

Header starts with `# Color procedural file`. Each data line contains `position` (mapped value, a floating-point number), `red` (0.0-1.0), `green` (0.0-1.0), `blue` (0.0-1.0).

Example:

```
# Color procedural file /path/to/file.txt
# 5 points
# position R G B
0 0 0 0
63 0 0.501961 0.490196
128 0.501961 0 1
192 1 0.501961 0
255 1 1 1
```

## Debugging

### Access scalar bar actor

Access to the scalar bar VTK actor may can be useful for debugging and for experimenting with new features. This code snippet shows how to access the actor in the Red slice view using Python:

```python
displayableNode = getNode('Model')
colorLegendDisplayNode = slicer.modules.colors.logic().GetColorLegendDisplayNode(displayableNode)
sliceView = slicer.app.layoutManager().sliceWidget('Red').sliceView()
displayableManager = sliceView.displayableManagerByClassName("vtkMRMLColorLegendDisplayableManager")
colorLegendActor = displayableManager.GetColorLegendActor(colorLegendDisplayNode)

# Experimental adjustment of a parameter that is not exposed via the colorLegendDisplayNode
colorLegendActor.SetBarRatio(0.2)
sliceView.forceRender()
```
