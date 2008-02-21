PROJECT(Editor)

#
# install the tcl package for the Module
# - make the module packages build directory if needed
# - make this module's package directory
# - copy over the tcl package files to the build directory
#

SET (Package_DIR ${SLICER_LIBRARIES_DIR}/Modules/Packages)
IF (NOT EXISTS ${Package_DIR})
  FILE(MAKE_DIRECTORY ${Package_DIR})
ENDIF (NOT EXISTS ${Package_DIR})

SET(Editor_DIR ${Package_DIR}/Editor)
SET(EditorImageData_DIR ${Package_DIR}/Editor/ImageData)

#
# copy over the tcl files
#
FILE(MAKE_DIRECTORY ${Editor_DIR})
SET(TCLFILES
  pkgIndex.tcl
  Editor.tcl EditorNode.tcl EditorLogic.tcl EditorGUI.tcl
  Box.tcl EditBox.tcl EditColor.tcl ColorBox.tcl
  PaintEffect.tcl DrawEffect.tcl
  MakeModelEffect.tcl
  EffectSWidget.tcl Labeler.tcl
  SaveIslandEffect.tcl ThresholdEffect.tcl RemoveIslandsEffect.tcl
  LevelTracingEffect.tcl
  ImplicitRectangleEffect.tcl
  WandEffect.tcl
  ChangeIslandEffect.tcl ChangeLabelEffect.tcl
)

FOREACH(tclfile ${TCLFILES})
  CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/Modules/Editor/${tclfile}
    ${Editor_DIR}/${tclfile}
    COPYONLY
  )
ENDFOREACH(tclfile)

#
# copy over the ImageData
#
FILE(MAKE_DIRECTORY ${EditorImageData_DIR})
SET(IMAGEFILES
ChangeIsland.png ChangeIslandDisabled.png ChangeIslandSelected.png
ChangeLabel.png ChangeLabelDisabled.png ChangeLabelSelected.png
ChooseColor.png ChooseColorDisabled.png ChooseColorSelected.png
ConnectedComponents.png ConnectedComponentsDisabled.png ConnectedComponentsSelected.png
LevelTracing.png LevelTracingDisabled.png LevelTracingSelected.png
Wand.png WandDisabled.png WandSelected.png
DeleteFiducials.png DeleteFiducialsDisabled.png DeleteFiducialsSelected.png
RemoveIslands.png RemoveIslandsDisabled.png RemoveIslandsSelected.png
DilateLabel.png DilateLabelDisabled.png DilateLabelSelected.png
EraseLabel.png EraseLabelDisabled.png EraseLabelSelected.png
ErodeLabel.png ErodeLabelDisabled.png ErodeLabelSelected.png
FiducialVisibilityOff.png FiducialVisibilityOffDisabled.png FiducialVisibilityOffSelected.png
FiducialVisibilityOn.png FiducialVisibilityOnDisabled.png FiducialVisibilityOnSelected.png
Draw.png DrawDisabled.png DrawSelected.png
GoToEditorModule.png GoToEditorModuleDisabled.png GoToEditorModuleSelected.png
IdentifyIslands.png IdentifyIslandsDisabled.png IdentifyIslandsSelected.png
ImplicitCube.png ImplicitCubeDisabled.png ImplicitCubeSelected.png
ImplicitEllipse.png ImplicitEllipseDisabled.png ImplicitEllipseSelected.png
ImplicitRectangle.png ImplicitRectangleDisabled.png ImplicitRectangleSelected.png
InterpolateLabels.png InterpolateLabelsDisabled.png InterpolateLabelsSelected.png
LabelOpacity.png LabelOpacityDisabled.png LabelOpacitySelected.png
LabelVisibilityOn.png LabelVisibilityOnDisabled.png LabelVisibilityOnSelected.png
LabelVisibilityOff.png LabelVisibilityOffDisabled.png LabelVisibilityOffSelected.png
MakeModel.png MakeModelDisabled.png MakeModelSelected.png
NextFiducial.png NextFiducialDisabled.png NextFiducialSelected.png
Paint.png PaintSelected.png PaintDisabled.png
PinOpen.png PinOpenDisabled.png PinOpenSelected.png
PreviousFiducial.png PreviousFiducialDisabled.png PreviousFiducialSelected.png
SaveIsland.png SaveIslandDisabled.png SaveIslandSelected.png
SlurpColor.png SlurpColorDisabled.png SlurpColorSelected.png
SnapToGridOff.png SnapToGridOffDisabled.png SnapToGridOffSelected.png
SnapToGridOn.png SnapToGridOnDisabled.png SnapToGridOnSelected.png
Threshold.png ThresholdBucket.png ThresholdBucketDisabled.png
ThresholdBucketSelected.png ThresholdDisabled.png ThresholdPaintLabel.png
ThresholdPaintLabelDisabled.png ThresholdPaintLabelSelected.png ThresholdSelected.png
ToggleLabelOutline.png ToggleLabelOutlineDisabled.png ToggleLabelOutlineSelected.png
Watershed.png WatershedDisabled.png WatershedSelected.png
ThresholdPaintLabel.png ThresholdPaintLabelDisabled.png ThresholdPaintLabelSelected.png
ThresholdBucket.png ThresholdBucketDisabled.png ThresholdBucketSelected.png
DefaultTool.png DefaultToolDisabled.png DefaultToolSelected.png
)

FOREACH(imagefile ${IMAGEFILES})
  CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/Modules/Editor/ImageData/${imagefile}
    ${EditorImageData_DIR}/${imagefile}
    COPYONLY
  )
ENDFOREACH(imagefile)
