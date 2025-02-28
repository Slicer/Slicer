import slicer

# Test reading/writing color table with standard terminology information

colorNode = slicer.vtkMRMLColorTableNode()
colorNode.SetName("CustomColors")
colorNode.SetHideFromEditors(0)
colorNode.SetTypeToUser()  # allow editing ("File" type would not allow editing)
largestLabelValue = 10
colorNode.SetNumberOfColors(largestLabelValue + 1)  # +1 for the background
if colorNode.GetLookupTable() is not None:
    colorNode.GetLookupTable().SetTableRange(0, largestLabelValue)

# Skip color 0 (background) to leave it undefined

colorNode.SetColor(1, "left_kidney", 185/255, 102/255, 83/255, 255/255)
colorNode.SetTerminology(1, "SCT", "123037004", "Anatomical Structure", "SCT", "64033007", "Kidney", "SCT", "7771000", "Left")

colorNode.SetColor(2, "right_kidney", 185/255, 102/255, 83/255, 255/255)
colorNode.SetTerminology(2, "SCT", "123037004", "Anatomical Structure", "SCT", "64033007", "Kidney", "SCT", "24028007", "Right")

colorNode.SetColor(10, "right_kidney_mass", 144/255, 238/255, 144/255, 255/255)
colorNode.SetTerminology(10,
                         "SCT","49755003","Morphologically Altered Structure","SCT","4147007","Mass","","","",
                         "SCT","64033007","Kidney","SCT","24028007","Right")

slicer.mrmlScene.AddNode(colorNode)

# Check if data are correctly set
assert(colorNode.GetColorName(2) == "right_kidney")
assert(colorNode.GetColorName(10) == "right_kidney_mass")

# Test writing

filePath = slicer.app.temporaryPath + "/__tmp__ColorTerminologiesTest1.csv"

assert(colorNode.AddDefaultStorageNode())
colorStorageNode = colorNode.GetStorageNode()
colorStorageNode.SetFileName(filePath)
assert(colorStorageNode.WriteData(colorNode))

# Test reading

readColorNode = slicer.util.loadNodeFromFile(filePath)
assert(readColorNode is not None)
assert(readColorNode.GetClassName() == "vtkMRMLColorTableNode")

# Check if names are correctly set
assert(readColorNode.GetColorName(2) == "right_kidney")
assert(readColorNode.GetColorName(10) == "right_kidney_mass")

# Test getting terminology

category = readColorNode.GetTerminologyCategory(2)
assert(category.GetCodingSchemeDesignator() == "SCT")
assert(category.GetCodeValue() == "123037004")
assert(category.GetCodeMeaning() == "Anatomical Structure")

type = readColorNode.GetTerminologyType(2)
assert(type.GetCodingSchemeDesignator() == "SCT")
assert(type.GetCodeValue() == "64033007")
assert(type.GetCodeMeaning() == "Kidney")

typeModifier = readColorNode.GetTerminologyTypeModifier(2)
assert(typeModifier.GetCodingSchemeDesignator() == "SCT")
assert(typeModifier.GetCodeValue() == "24028007")
assert(typeModifier.GetCodeMeaning() == "Right")

region = readColorNode.GetTerminologyRegion(10)
assert(region.GetCodingSchemeDesignator() == "SCT")
assert(region.GetCodeValue() == "64033007")
assert(region.GetCodeMeaning() == "Kidney")

regionModifier = readColorNode.GetTerminologyRegionModifier(10)
assert(regionModifier.GetCodingSchemeDesignator() == "SCT")
assert(regionModifier.GetCodeValue() == "24028007")
assert(regionModifier.GetCodeMeaning() == "Right")

print("Test passed")
