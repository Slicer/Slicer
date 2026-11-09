/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// Checks that the widget offers what the source supports (mesh arrays incl. cell data, interpolation only for meshes
// without volumetric cells, and the modes it enables) and that its controls set the display node.

// Qt includes
#include <QApplication>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>

// CTK includes
#include <ctkCheckableComboBox.h>
#include <ctkSliderWidget.h>
#include <ctkCollapsibleGroupBox.h>
#include <ctkColorPickerButton.h>
#include <ctkDoubleSpinBox.h>

// qMRML includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLVectorFieldDisplayWidget.h"
#include "qMRMLWidget.h"

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLMarkupsFiducialNode.h>
#include <vtkMRMLMarkupsROINode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkSegment.h>
#include <vtkSegmentation.h>
#include <vtkSegmentationConverter.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLVectorFieldDisplayNode.h>
#include <vtkMRMLVectorFieldPipeline.h>
#include <vtkMRMLVectorFieldTestingHelpers.h>

// VTK includes
#include <vtkAssignAttribute.h>
#include <vtkDoubleArray.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVector.h>

// STD includes
#include <cmath>
#include <iostream>

using namespace vtkMRMLVectorFieldTesting;

namespace
{

//----------------------------------------------------------------------------
/// Whether a widget is meant to be shown, also while its collapsed section hides it
bool IsShownToParent(QWidget* widget)
{
  QVariant visibilityToParent = widget->property("visibilityToParent");
  return (visibilityToParent.isValid() ? visibilityToParent.toBool() : !(widget->isHidden() && widget->testAttribute(Qt::WA_WState_ExplicitShowHide)));
}

//----------------------------------------------------------------------------
template <class T>
T* GetChild(QWidget& widget, const char* name)
{
  T* child = widget.findChild<T*>(name);
  if (!child)
  {
    std::cerr << "Widget " << name << " not found" << std::endl;
  }
  return child;
}

} // namespace

//----------------------------------------------------------------------------
int qMRMLVectorFieldDisplayWidgetTest1(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  vtkNew<vtkMRMLScene> scene;

  // A surface mesh with a point data field and a cell data field
  vtkSmartPointer<vtkPolyData> surface = CreateSurfaceMesh(50.0, 30);
  AddPointField(surface, "PointField", SmoothField);
  AddCellField(surface, "CellField", SmoothField);
  vtkMRMLVectorFieldDisplayNode* surfaceDisplayNode = AddModelWithFieldDisplay(scene, surface);
  surfaceDisplayNode->SetFieldArrayName("PointField");

  qMRMLVectorFieldDisplayWidget widget;
  widget.setMRMLScene(scene);
  CHECK_BOOL(widget.isEnabled(), false);
  widget.setMRMLVectorFieldDisplayNode(surfaceDisplayNode);
  CHECK_BOOL(widget.isEnabled(), true);

  QComboBox* fieldArrayComboBox = GetChild<QComboBox>(widget, "FieldArrayComboBox");
  QCheckBox* interpolateCheckBox = GetChild<QCheckBox>(widget, "InterpolateBetweenPointsCheckBox");
  ctkDoubleSpinBox* maximumDistanceSpinBox = GetChild<ctkDoubleSpinBox>(widget, "InterpolationMaximumDistanceSpinBox");
  QLabel* residualLabel = GetChild<QLabel>(widget, "InterpolationResidualValueLabel");
  QToolButton* gridModeButton = GetChild<QToolButton>(widget, "GridModeToolButton");
  CHECK_NOT_NULL(fieldArrayComboBox);
  CHECK_NOT_NULL(interpolateCheckBox);
  CHECK_NOT_NULL(maximumDistanceSpinBox);
  CHECK_NOT_NULL(residualLabel);
  CHECK_NOT_NULL(gridModeButton);
  qMRMLNodeComboBox* regionNodeComboBox = GetChild<qMRMLNodeComboBox>(widget, "SamplingRegionNodeComboBox");
  CHECK_NOT_NULL(regionNodeComboBox);
  QComboBox* partComboBox = GetChild<QComboBox>(widget, "SamplingRegionPartComboBox");
  QComboBox* patternComboBox = GetChild<QComboBox>(widget, "SamplingPatternComboBox");
  QWidget* sampleSpacingSpinBox = GetChild<QWidget>(widget, "SampleSpacingSpinBox");
  QWidget* pointsNumberSpinBox = GetChild<QWidget>(widget, "SamplingPointsNumberSpinBox");
  qMRMLNodeComboBox* maskingNodeComboBox = GetChild<qMRMLNodeComboBox>(widget, "MaskingRegionNodeComboBox");
  ctkCheckableComboBox* maskingSegmentsComboBox = GetChild<ctkCheckableComboBox>(widget, "MaskingSegmentsComboBox");
  CHECK_NOT_NULL(partComboBox);
  CHECK_NOT_NULL(patternComboBox);
  CHECK_NOT_NULL(sampleSpacingSpinBox);
  CHECK_NOT_NULL(pointsNumberSpinBox);
  CHECK_NOT_NULL(maskingNodeComboBox);
  CHECK_NOT_NULL(maskingSegmentsComboBox);
  // Without a node the model itself is sampled: a surface mesh field is only known at its points, which can be thinned
  // out at random over its bounds, surface or volume.
  CHECK_STD_STRING(regionNodeComboBox->noneDisplay().toStdString(), "(This model)");
  CHECK_BOOL(regionNodeComboBox->isVisibleTo(&widget), false);
  CHECK_BOOL(partComboBox->isVisibleTo(&widget), true);
  CHECK_BOOL(patternComboBox->isVisibleTo(&widget), true);
  // Every point of the mesh gets a glyph, so there is no spacing
  CHECK_BOOL(sampleSpacingSpinBox->isVisibleTo(&widget), false);
  CHECK_BOOL(pointsNumberSpinBox->isVisibleTo(&widget), false);
  patternComboBox->setCurrentIndex(patternComboBox->findData(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom));
  CHECK_INT(surfaceDisplayNode->GetSamplingPattern(), vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  CHECK_BOOL(pointsNumberSpinBox->isVisibleTo(&widget), true);
  patternComboBox->setCurrentIndex(patternComboBox->findData(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular));

  // The arrays of the mesh, cell data included
  CHECK_BOOL(fieldArrayComboBox->findData(QString("PointField")) >= 0, true);
  CHECK_BOOL(fieldArrayComboBox->findData(QString("CellField")) >= 0, true);
  CHECK_STD_STRING(fieldArrayComboBox->currentData().toString().toStdString(), "PointField");
  fieldArrayComboBox->setCurrentIndex(fieldArrayComboBox->findData(QString("CellField")));
  CHECK_STRING(surfaceDisplayNode->GetFieldArrayName(), "CellField");
  CHECK_BOOL(surfaceDisplayNode->IsFieldArrayCellData(), true);
  fieldArrayComboBox->setCurrentIndex(fieldArrayComboBox->findData(QString("PointField")));
  CHECK_STRING(surfaceDisplayNode->GetFieldArrayName(), "PointField");

  // A model without a vector array has nothing to draw: the widget says so and offers no settings
  QLabel* noVectorArraysLabel = GetChild<QLabel>(widget, "NoVectorArraysLabel");
  CHECK_NOT_NULL(noVectorArraysLabel);
  CHECK_BOOL(noVectorArraysLabel->isVisibleTo(&widget), false);
  CHECK_BOOL(fieldArrayComboBox->isVisibleTo(&widget), true);
  vtkSmartPointer<vtkPolyData> scalarsOnlySurface = CreateSurfaceMesh(50.0, 30);
  vtkNew<vtkDoubleArray> scalarArray;
  scalarArray->SetName("Scalar");
  scalarArray->SetNumberOfTuples(scalarsOnlySurface->GetNumberOfPoints());
  scalarArray->Fill(1.0);
  scalarsOnlySurface->GetPointData()->AddArray(scalarArray);
  vtkMRMLVectorFieldDisplayNode* scalarsOnlyDisplayNode = AddModelWithFieldDisplay(scene, scalarsOnlySurface);
  widget.setMRMLVectorFieldDisplayNode(scalarsOnlyDisplayNode);
  CHECK_BOOL(widget.isEnabled(), false);
  CHECK_BOOL(noVectorArraysLabel->isVisibleTo(&widget), true);
  CHECK_BOOL(fieldArrayComboBox->isVisibleTo(&widget), true);
  CHECK_INT(fieldArrayComboBox->count(), 1);
  // The widget follows the mesh of the model
  vtkMRMLModelNode::SafeDownCast(scalarsOnlyDisplayNode->GetDisplayableNode())->SetAndObserveMesh(surface);
  CHECK_BOOL(widget.isEnabled(), true);
  CHECK_BOOL(noVectorArraysLabel->isVisibleTo(&widget), false);
  CHECK_INT(fieldArrayComboBox->count(), 3);
  vtkMRMLModelNode::SafeDownCast(scalarsOnlyDisplayNode->GetDisplayableNode())->SetAndObserveMesh(scalarsOnlySurface);
  CHECK_BOOL(widget.isEnabled(), false);
  // No node: no note either
  widget.setMRMLVectorFieldDisplayNode(static_cast<vtkMRMLVectorFieldDisplayNode*>(nullptr));
  CHECK_BOOL(widget.isEnabled(), false);
  CHECK_BOOL(noVectorArraysLabel->isVisibleTo(&widget), false);
  widget.setMRMLVectorFieldDisplayNode(surfaceDisplayNode);
  CHECK_BOOL(widget.isEnabled(), true);

  // Interpolation is offered for a surface mesh, and its settings only once it is enabled
  CHECK_BOOL(interpolateCheckBox->isVisibleTo(&widget), true);
  CHECK_BOOL(interpolateCheckBox->isChecked(), false);
  CHECK_BOOL(maximumDistanceSpinBox->isVisibleTo(&widget), false);
  CHECK_BOOL(residualLabel->isVisibleTo(&widget), false);
  // Without interpolation only glyphs can be drawn
  CHECK_BOOL(gridModeButton->isVisibleTo(&widget), false);

  interpolateCheckBox->setChecked(true);
  CHECK_BOOL(surfaceDisplayNode->GetInterpolateBetweenPoints(), true);
  // Interpolated, it can be sampled at other nodes too
  CHECK_BOOL(regionNodeComboBox->isVisibleTo(&widget), true);
  CHECK_BOOL(maximumDistanceSpinBox->isVisibleTo(&widget), true);
  CHECK_BOOL(residualLabel->isVisibleTo(&widget), true);
  CHECK_BOOL(gridModeButton->isVisibleTo(&widget), true);
  // How closely the fit follows the data is shown as a number
  bool isNumber = false;
  double residual = residualLabel->text().toDouble(&isNumber);
  std::cout << "Fit error shown: " << qPrintable(residualLabel->text()) << std::endl;
  CHECK_BOOL(isNumber, true);
  CHECK_DOUBLE_TOLERANCE(residual, vtkMRMLVectorFieldPipeline::GetSharedPipeline(surfaceDisplayNode)->GetInterpolationResidual(), 0.001 + 0.01 * residual);

  maximumDistanceSpinBox->setValue(7.5);
  CHECK_DOUBLE(surfaceDisplayNode->GetInterpolationMaximumDistance(), 7.5);
  // The widget follows changes made to the node
  surfaceDisplayNode->SetInterpolationMaximumDistance(0.0);
  CHECK_DOUBLE(maximumDistanceSpinBox->value(), 0.0);
  CHECK_STD_STRING(maximumDistanceSpinBox->spinBox()->text().toStdString(), "Automatic");
  gridModeButton->click();
  CHECK_INT(surfaceDisplayNode->GetVisualizationMode(), vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  // The scale factor is a slider, whose range grows to a value that is set beyond it
  ctkSliderWidget* scaleFactorSlider = GetChild<ctkSliderWidget>(widget, "ScaleFactorSliderWidget");
  CHECK_NOT_NULL(scaleFactorSlider);
  surfaceDisplayNode->SetGridScaleFactor(250.0);
  CHECK_BOOL(scaleFactorSlider->maximum() >= 250.0, true);
  CHECK_DOUBLE(scaleFactorSlider->value(), 250.0);
  // A negative factor is shown as its magnitude on the slider, and the reverse button pressed
  QToolButton* reverseButton = GetChild<QToolButton>(widget, "ScaleFactorReverseButton");
  CHECK_NOT_NULL(reverseButton);
  CHECK_BOOL(reverseButton->isChecked(), false);
  surfaceDisplayNode->SetGridScaleFactor(-2.0);
  CHECK_DOUBLE(scaleFactorSlider->value(), 2.0);
  CHECK_BOOL(reverseButton->isChecked(), true);
  // The button only flips the sign, the slider keeps it
  reverseButton->click();
  CHECK_DOUBLE(surfaceDisplayNode->GetGridScaleFactor(), 2.0);
  reverseButton->click();
  CHECK_DOUBLE(surfaceDisplayNode->GetGridScaleFactor(), -2.0);
  scaleFactorSlider->setValue(0.5);
  CHECK_DOUBLE(surfaceDisplayNode->GetGridScaleFactor(), -0.5);
  // The range of the slider grows to the magnitude of a negative factor too
  surfaceDisplayNode->SetGridScaleFactor(-2500.0);
  CHECK_BOOL(scaleFactorSlider->maximum() >= 2500.0, true);
  CHECK_DOUBLE(scaleFactorSlider->value(), 2500.0);
  CHECK_BOOL(reverseButton->isChecked(), true);
  surfaceDisplayNode->SetGridScaleFactor(1.0);
  CHECK_BOOL(reverseButton->isChecked(), false);
  // The range suits the field, not the factor that a scene was saved with: it is the same for a factor of 0 as for a
  // small one, so larger factors can be reached, and a factor of 1 is the true deformation of a grid
  auto reshowNode = [&widget, surfaceDisplayNode]()
  {
    widget.setMRMLVectorFieldDisplayNode(static_cast<vtkMRMLVectorFieldDisplayNode*>(nullptr));
    widget.setMRMLVectorFieldDisplayNode(surfaceDisplayNode);
  };
  surfaceDisplayNode->SetGridScaleFactor(0.0);
  reshowNode();
  CHECK_DOUBLE(scaleFactorSlider->value(), 0.0);
  double fittedMaximum = scaleFactorSlider->maximum();
  CHECK_BOOL(fittedMaximum >= 1.0, true);
  surfaceDisplayNode->SetGridScaleFactor(0.001);
  reshowNode();
  CHECK_DOUBLE(scaleFactorSlider->maximum(), fittedMaximum);
  // A larger factor still fits, and a tidy maximum is 1, 2 or 5 times a power of ten
  surfaceDisplayNode->SetGridScaleFactor(30.0 * fittedMaximum);
  reshowNode();
  CHECK_BOOL(scaleFactorSlider->maximum() >= 30.0 * fittedMaximum, true);
  double mantissa = scaleFactorSlider->maximum() / std::pow(10.0, std::floor(std::log10(scaleFactorSlider->maximum())));
  CHECK_BOOL(std::abs(mantissa - 1.0) < 1e-6 || std::abs(mantissa - 2.0) < 1e-6 || std::abs(mantissa - 5.0) < 1e-6, true);
  surfaceDisplayNode->SetGridScaleFactor(1.0);
  // The grid can be filled
  QCheckBox* gridFillCheckBox = GetChild<QCheckBox>(widget, "GridFillCheckBox");
  CHECK_NOT_NULL(gridFillCheckBox);
  surfaceDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_BOOL(gridFillCheckBox->isVisibleTo(&widget), true);
  CHECK_BOOL(gridFillCheckBox->isChecked(), false);
  gridFillCheckBox->setChecked(true); // in the collapsed Advanced section
  CHECK_BOOL(surfaceDisplayNode->GetGridFill(), true);
  surfaceDisplayNode->SetGridFill(false);
  CHECK_BOOL(gridFillCheckBox->isChecked(), false);
  // and its lines hidden, each option in a row of its own below the grid spacing
  QCheckBox* gridLinesCheckBox = GetChild<QCheckBox>(widget, "GridLinesVisibilityCheckBox");
  CHECK_NOT_NULL(gridLinesCheckBox);
  CHECK_BOOL(gridLinesCheckBox->isVisibleTo(&widget), true);
  CHECK_BOOL(gridLinesCheckBox->isChecked(), true);
  gridLinesCheckBox->setChecked(false); // in the collapsed Advanced section
  CHECK_BOOL(surfaceDisplayNode->GetGridLinesVisibility(), false);
  surfaceDisplayNode->SetGridLinesVisibility(true);
  CHECK_BOOL(gridLinesCheckBox->isChecked(), true);
  for (const char* labelName : { "GridShowNonWarpedLabel", "GridLinesVisibilityLabel", "GridFillLabel" })
  {
    CHECK_BOOL(GetChild<QLabel>(widget, labelName)->isVisibleTo(&widget), true);
  }
  // The color of the non-warped grid can be chosen when that grid is shown
  ctkColorPickerButton* nonWarpedColorButton = GetChild<ctkColorPickerButton>(widget, "GridNonWarpedColorPickerButton");
  CHECK_NOT_NULL(nonWarpedColorButton);
  CHECK_BOOL(nonWarpedColorButton->isVisibleTo(&widget), true);
  CHECK_BOOL(GetChild<QCheckBox>(widget, "GridShowNonWarpedCheckBox")->isVisibleTo(&widget), true);
  // Widgets are disabled in a collapsed section
  ctkCollapsibleGroupBox* nonWarpedGroupBox = GetChild<ctkCollapsibleGroupBox>(widget, "AdvancedGroupBox");
  bool nonWarpedGroupBoxCollapsed = nonWarpedGroupBox->collapsed();
  nonWarpedGroupBox->setCollapsed(false);
  surfaceDisplayNode->SetGridShowNonWarped(false);
  CHECK_BOOL(nonWarpedColorButton->isEnabled(), false);
  surfaceDisplayNode->SetGridShowNonWarped(true);
  CHECK_BOOL(nonWarpedColorButton->isEnabled(), true);
  surfaceDisplayNode->SetGridNonWarpedColor(1.0, 0.0, 0.0);
  CHECK_BOOL(nonWarpedColorButton->color() == QColor(255, 0, 0), true);
  nonWarpedColorButton->setColor(QColor(0, 255, 0));
  CHECK_DOUBLE(surfaceDisplayNode->GetGridNonWarpedColor()[0], 0.0);
  CHECK_DOUBLE(surfaceDisplayNode->GetGridNonWarpedColor()[1], 1.0);
  surfaceDisplayNode->SetGridShowNonWarped(false);
  nonWarpedGroupBox->setCollapsed(nonWarpedGroupBoxCollapsed);
  // Choosing a field without a color table sets the default one, and does not replace one that is set
  std::string previousColorNodeID = surfaceDisplayNode->GetColorNodeID() ? surfaceDisplayNode->GetColorNodeID() : "";
  int fieldArrayIndex = fieldArrayComboBox->currentIndex();
  fieldArrayComboBox->setCurrentIndex(0);
  surfaceDisplayNode->SetAndObserveColorNodeID(static_cast<const char*>(nullptr));
  fieldArrayComboBox->setCurrentIndex(fieldArrayIndex);
  CHECK_STRING(surfaceDisplayNode->GetColorNodeID(), "vtkMRMLColorTableNodeFilePlasma.txt");
  fieldArrayComboBox->setCurrentIndex(0);
  surfaceDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeRainbow");
  fieldArrayComboBox->setCurrentIndex(fieldArrayIndex);
  CHECK_STRING(surfaceDisplayNode->GetColorNodeID(), "vtkMRMLColorTableNodeRainbow");
  surfaceDisplayNode->SetAndObserveColorNodeID(previousColorNodeID.empty() ? nullptr : previousColorNodeID.c_str());
  // The opacity in slice views
  ctkSliderWidget* sliceOpacitySlider = GetChild<ctkSliderWidget>(widget, "SliceOpacitySliderWidget");
  CHECK_NOT_NULL(sliceOpacitySlider);
  sliceOpacitySlider->setValue(0.4);
  CHECK_DOUBLE(surfaceDisplayNode->GetSliceIntersectionOpacity(), 0.4);
  surfaceDisplayNode->SetSliceIntersectionOpacity(0.7);
  CHECK_DOUBLE(sliceOpacitySlider->value(), 0.7);
  // Colored by the vectors: the component to color by is chosen next to the array, the
  // magnitude by default
  QSpinBox* componentSpinBox = GetChild<QSpinBox>(widget, "ActiveScalarComponentSpinBox");
  CHECK_NOT_NULL(componentSpinBox);
  surfaceDisplayNode->SetActiveScalar("PointField", vtkAssignAttribute::POINT_DATA);
  CHECK_BOOL(componentSpinBox->isVisibleTo(&widget), true);
  CHECK_INT(componentSpinBox->value(), -1);
  CHECK_STD_STRING(componentSpinBox->text().toStdString(), "Magnitude");
  CHECK_INT(componentSpinBox->maximum(), 2);
  componentSpinBox->setValue(1);
  CHECK_INT(surfaceDisplayNode->GetActiveScalarComponent(), 1);
  surfaceDisplayNode->SetActiveScalarComponent(2);
  CHECK_INT(componentSpinBox->value(), 2);
  surfaceDisplayNode->SetActiveScalarComponent(-1);
  CHECK_INT(componentSpinBox->value(), -1);
  // Without contour levels of its own, the ones that are drawn are shown greyed
  surfaceDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  QLineEdit* contourLevelsLineEdit = GetChild<QLineEdit>(widget, "ContourLevelsLineEdit");
  CHECK_NOT_NULL(contourLevelsLineEdit);
  CHECK_BOOL(contourLevelsLineEdit->text().isEmpty(), true);
  std::cout << "Automatic contour levels: " << qPrintable(contourLevelsLineEdit->placeholderText()) << std::endl;
  CHECK_BOOL(contourLevelsLineEdit->placeholderText().isEmpty(), false);
  surfaceDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  // The segments of a mask are indented below it
  CHECK_INT(GetChild<QLabel>(widget, "MaskingSegmentsLabel")->indent(), 10);
  surfaceDisplayNode->SetInterpolateBetweenPoints(false);
  CHECK_BOOL(interpolateCheckBox->isChecked(), false);
  CHECK_BOOL(gridModeButton->isVisibleTo(&widget), false);

  // A mesh can be colored by any of its arrays
  QComboBox* colorByComboBox = GetChild<QComboBox>(widget, "ColorByComboBox");
  QCheckBox* sameColoringCheckBox = GetChild<QCheckBox>(widget, "SameColoringAsSurfaceCheckBox");
  QWidget* scalarsDisplayWidget = GetChild<QWidget>(widget, "ScalarsDisplayWidget");
  CHECK_NOT_NULL(colorByComboBox);
  CHECK_NOT_NULL(sameColoringCheckBox);
  CHECK_NOT_NULL(scalarsDisplayWidget);
  CHECK_STD_STRING(colorByComboBox->itemText(1).toStdString(), "Array");
  // The coloring settings are in a section that is collapsed at first
  ctkCollapsibleGroupBox* coloringGroupBox = GetChild<ctkCollapsibleGroupBox>(widget, "ColoringGroupBox");
  CHECK_NOT_NULL(coloringGroupBox);
  coloringGroupBox->setCollapsed(false);
  // Following the coloring of the surface is offered once the model has a surface display
  CHECK_BOOL(sameColoringCheckBox->isVisibleTo(&widget), false);
  vtkMRMLModelNode* surfaceModelNode = vtkMRMLModelNode::SafeDownCast(surfaceDisplayNode->GetDisplayableNode());
  vtkMRMLModelDisplayNode* surfaceModelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelDisplayNode"));
  surfaceModelNode->AddAndObserveDisplayNodeID(surfaceModelDisplayNode->GetID());
  surfaceModelDisplayNode->SetActiveScalar("CellField", vtkAssignAttribute::CELL_DATA);
  surfaceDisplayNode->Modified();
  CHECK_BOOL(sameColoringCheckBox->isVisibleTo(&widget), true);
  CHECK_BOOL(sameColoringCheckBox->isChecked(), false);
  CHECK_BOOL(scalarsDisplayWidget->isEnabledTo(&widget), true);
  // Checking it links the coloring, which is then edited on the surface
  sameColoringCheckBox->setChecked(true);
  CHECK_POINTER(surfaceDisplayNode->GetColoringSourceDisplayNode(), surfaceModelDisplayNode);
  CHECK_STRING(surfaceDisplayNode->GetActiveScalarName(), "CellField");
  CHECK_BOOL(scalarsDisplayWidget->isEnabledTo(&widget), false);
  CHECK_BOOL(colorByComboBox->isEnabled(), false);
  sameColoringCheckBox->setChecked(false);
  CHECK_NULL(surfaceDisplayNode->GetColoringSourceDisplayNode());
  CHECK_BOOL(scalarsDisplayWidget->isEnabledTo(&widget), true);

  // A volumetric mesh is interpolated exactly in its cells, but only there: interpolation is
  // offered to extend its field beyond them
  vtkSmartPointer<vtkUnstructuredGrid> grid = CreateVolumetricMesh(4, 20.0);
  AddPointField(grid, "PointField", SmoothField);
  vtkMRMLVectorFieldDisplayNode* gridDisplayNode = AddModelWithFieldDisplay(scene, grid);
  gridDisplayNode->SetFieldArrayName("PointField");
  widget.setMRMLVectorFieldDisplayNode(gridDisplayNode);
  CHECK_BOOL(interpolateCheckBox->isVisibleTo(&widget), true);
  CHECK_BOOL(maximumDistanceSpinBox->isVisibleTo(&widget), false);
  CHECK_BOOL(gridModeButton->isVisibleTo(&widget), true);
  // Its isosurfaces are built on its cells, everywhere: there is nothing to sample
  QLabel* samplingLabel = GetChild<QLabel>(widget, "SamplingLabel");
  gridDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeContour);
  CHECK_BOOL(regionNodeComboBox->isVisibleTo(&widget), false);
  CHECK_BOOL(samplingLabel->isVisibleTo(&widget), false);
  CHECK_BOOL(partComboBox->isVisibleTo(&widget), false);
  CHECK_BOOL(sampleSpacingSpinBox->isVisibleTo(&widget), false);
  // Its streamlines are traced on its cells too, but from seeds sampled in the region
  gridDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  CHECK_BOOL(regionNodeComboBox->isVisibleTo(&widget), true);
  CHECK_BOOL(samplingLabel->isVisibleTo(&widget), true);
  CHECK_BOOL(partComboBox->isVisibleTo(&widget), true);
  // Its grids are sampled on a lattice in the region
  gridDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_BOOL(regionNodeComboBox->isVisibleTo(&widget), true);
  CHECK_BOOL(sampleSpacingSpinBox->isVisibleTo(&widget), true);

  // A transform is not shown in 3D views until a region is chosen
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLLinearTransformNode"));
  vtkMRMLVectorFieldDisplayNode* transformDisplayNode = AddFieldDisplayNode(scene, transformNode);
  widget.setMRMLVectorFieldDisplayNode(transformDisplayNode);
  CHECK_STD_STRING(regionNodeComboBox->noneDisplay().toStdString(), "Select a node");
  CHECK_BOOL(interpolateCheckBox->isVisibleTo(&widget), false);

  // Sampled in a region of interest: its inside, on a lattice or at random
  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLMarkupsROINode"));
  vtkMRMLMarkupsFiducialNode* pointsNode = AddPointList(scene, { vtkVector3d(0.0, 0.0, 0.0) });
  // A transform is the field: there is no array to choose
  CHECK_BOOL(widget.isEnabled(), true);
  CHECK_BOOL(noVectorArraysLabel->isVisibleTo(&widget), false);
  CHECK_BOOL(fieldArrayComboBox->isVisibleTo(&widget), false);
  transformDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  regionNodeComboBox->setCurrentNode(roiNode);
  CHECK_POINTER(transformDisplayNode->GetSamplingRegionNode(), roiNode);
  // A region of interest only has an inside, so there is no part to choose
  CHECK_BOOL(partComboBox->isVisibleTo(&widget), false);
  CHECK_BOOL(sampleSpacingSpinBox->isVisibleTo(&widget), true);
  CHECK_BOOL(pointsNumberSpinBox->isVisibleTo(&widget), false);
  transformDisplayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRandom);
  CHECK_BOOL(sampleSpacingSpinBox->isVisibleTo(&widget), false);
  CHECK_BOOL(pointsNumberSpinBox->isVisibleTo(&widget), true);
  transformDisplayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  // At the points of a point list, which are taken as they are
  regionNodeComboBox->setCurrentNode(pointsNode);
  CHECK_POINTER(transformDisplayNode->GetSamplingRegionNode(), pointsNode);
  CHECK_BOOL(sampleSpacingSpinBox->isVisibleTo(&widget), false);
  // Grids are built on a lattice, whatever the pattern of the samples
  transformDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_BOOL(patternComboBox->isVisibleTo(&widget), false);
  transformDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);

  // Masked by some of the segments of a segmentation
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLSegmentationNode"));
  segmentationNode->GetSegmentation()->SetSourceRepresentationName(vtkSegmentationConverter::GetClosedSurfaceRepresentationName());
  for (const char* segmentID : { "liver", "spleen" })
  {
    vtkNew<vtkSegment> segment;
    segment->SetName(std::string(segmentID) == "liver" ? "Liver" : "Spleen");
    vtkNew<vtkPolyData> surface;
    segment->AddRepresentation(vtkSegmentationConverter::GetClosedSurfaceRepresentationName(), surface);
    segmentationNode->GetSegmentation()->AddSegment(segment, segmentID);
  }
  CHECK_INT(segmentationNode->GetSegmentation()->GetNumberOfSegments(), 2);
  CHECK_BOOL(maskingSegmentsComboBox->isVisibleTo(&widget), false);
  maskingNodeComboBox->setCurrentNode(segmentationNode);
  CHECK_POINTER(transformDisplayNode->GetMaskingRegionNode(), segmentationNode);
  CHECK_BOOL(maskingSegmentsComboBox->isVisibleTo(&widget), true);
  CHECK_INT(maskingSegmentsComboBox->count(), 2);
  CHECK_STD_STRING(maskingSegmentsComboBox->itemText(1).toStdString(), "Spleen");
  maskingSegmentsComboBox->setCheckState(maskingSegmentsComboBox->model()->index(1, 0), Qt::Checked);
  CHECK_BOOL(transformDisplayNode->GetMaskingSegmentIDs() == std::vector<std::string>({ "spleen" }), true);
  // Shown outside of them
  QComboBox* maskingModeComboBox = GetChild<QComboBox>(widget, "MaskingModeComboBox");
  CHECK_NOT_NULL(maskingModeComboBox);
  maskingModeComboBox->setCurrentIndex(maskingModeComboBox->findData(false));
  CHECK_BOOL(transformDisplayNode->GetMaskingShowInside(), false);
  // The segments that were chosen belong to the segmentation, so another node clears them
  maskingNodeComboBox->setCurrentNode(roiNode);
  CHECK_BOOL(transformDisplayNode->GetMaskingSegmentIDs().empty(), true);
  CHECK_BOOL(maskingSegmentsComboBox->isVisibleTo(&widget), false);

  // A vector volume has one vector array, so there is no array to choose. Its streamlines
  // follow its voxels, so they have no resolution to set either.
  vtkMRMLVectorVolumeNode* volumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLVectorVolumeNode"));
  vtkNew<vtkImageData> vectorImage;
  vectorImage->SetDimensions(5, 5, 5);
  vectorImage->AllocateScalars(VTK_DOUBLE, 3);
  vectorImage->GetPointData()->GetScalars()->Fill(1.0);
  volumeNode->SetAndObserveImageData(vectorImage);
  vtkMRMLVectorFieldDisplayNode* volumeDisplayNode = AddNewFieldDisplayNode(scene);
  volumeNode->AddAndObserveDisplayNodeID(volumeDisplayNode->GetID());
  widget.setMRMLVectorFieldDisplayNode(volumeDisplayNode);
  CHECK_BOOL(widget.isEnabled(), true);
  CHECK_BOOL(noVectorArraysLabel->isVisibleTo(&widget), false);
  CHECK_BOOL(fieldArrayComboBox->isVisibleTo(&widget), false);
  // It can only be colored by the magnitude of its vectors, and has no surface to follow
  CHECK_STD_STRING(colorByComboBox->itemText(1).toStdString(), "Magnitude");
  CHECK_BOOL(sameColoringCheckBox->isVisibleTo(&widget), false);
  volumeDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  CHECK_BOOL(sampleSpacingSpinBox->isVisibleTo(&widget), true);
  // A grid is not cut by the threshold, so it is not offered
  QWidget* thresholdCheckBox = GetChild<QWidget>(widget, "FieldThresholdCheckBox");
  CHECK_NOT_NULL(thresholdCheckBox);
  CHECK_BOOL(thresholdCheckBox->isVisibleTo(&widget), false);
  // The whole row is hidden, so that it leaves no gap: a form layout keeps the spacing of a
  // row whose field is a layout, and of a row with nothing in it
  QWidget* thresholdWidget = GetChild<QWidget>(widget, "FieldThresholdWidget");
  CHECK_NOT_NULL(thresholdWidget);
  CHECK_BOOL(thresholdWidget->isVisibleTo(&widget), false);
  for (QFormLayout* formLayout : widget.findChildren<QFormLayout*>())
  {
    if (scalarsDisplayWidget && scalarsDisplayWidget->isAncestorOf(formLayout->parentWidget()))
    {
      continue; // lays out its own rows
    }
    for (int row = 0; row < formLayout->rowCount(); ++row)
    {
      QLayoutItem* labelItem = formLayout->itemAt(row, QFormLayout::LabelRole);
      QLayoutItem* fieldItem = formLayout->itemAt(row, QFormLayout::FieldRole);
      QLayoutItem* spanningItem = formLayout->itemAt(row, QFormLayout::SpanningRole);
      if (!labelItem && !fieldItem && !spanningItem)
      {
        std::cerr << "Empty row " << row << " in " << qPrintable(formLayout->objectName()) << std::endl;
        return EXIT_FAILURE;
      }
      // Qt 5 keeps the spacing of a hidden row, so a hidden row is taken out of the layout
      QWidget* label = (labelItem ? labelItem->widget() : nullptr);
      QWidget* field = (fieldItem ? fieldItem->widget() : (spanningItem ? spanningItem->widget() : nullptr));
      if (field && !IsShownToParent(field) && (!label || !IsShownToParent(label)))
      {
        std::cerr << "Hidden row " << row << " (" << qPrintable(field->objectName()) << ") left in " << qPrintable(formLayout->objectName()) << std::endl;
        return EXIT_FAILURE;
      }
    }
  }
  // Only glyphs have a diameter
  QWidget* glyphDiameterWidget = GetChild<QWidget>(widget, "GlyphDiameterWidget");
  CHECK_NOT_NULL(glyphDiameterWidget);
  CHECK_BOOL(glyphDiameterWidget->isVisibleTo(&widget), false);
  volumeDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  CHECK_BOOL(thresholdCheckBox->isVisibleTo(&widget), true);
  volumeDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  // Streamlines are traced through the voxels; the spacing is the spacing of their seeds
  // Random seeds are counted, not spaced
  volumeDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeStreamline);
  CHECK_BOOL(sampleSpacingSpinBox->isVisibleTo(&widget), false);
  volumeDisplayNode->SetSamplingPattern(vtkMRMLVectorFieldDisplayNode::SamplingPatternRegular);
  CHECK_BOOL(sampleSpacingSpinBox->isVisibleTo(&widget), true);
  CHECK_STD_STRING(regionNodeComboBox->noneDisplay().toStdString(), "(This volume)");

  // Rows that are updated while their section is collapsed are all there when it is expanded:
  // a collapsed section hides its children, which does not mean that their rows are not used
  widget.show();
  ctkCollapsibleGroupBox* advancedGroupBox = GetChild<ctkCollapsibleGroupBox>(widget, "AdvancedGroupBox");
  CHECK_NOT_NULL(advancedGroupBox);
  QFormLayout* advancedLayout = GetChild<QFormLayout>(widget, "advancedLayout");
  CHECK_NOT_NULL(advancedLayout);
  QWidget* glyphTypeComboBox = GetChild<QWidget>(widget, "GlyphTypeComboBox");
  CHECK_NOT_NULL(glyphTypeComboBox);
  advancedGroupBox->setCollapsed(true);
  QApplication::processEvents();
  volumeDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  volumeDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGlyph);
  advancedGroupBox->setCollapsed(false);
  QApplication::processEvents();
  int glyphTypeRow = -1;
  QFormLayout::ItemRole glyphTypeRole;
  advancedLayout->getWidgetPosition(glyphTypeComboBox, &glyphTypeRow, &glyphTypeRole);
  CHECK_BOOL(glyphTypeRow >= 0, true);
  CHECK_BOOL(glyphTypeComboBox->isVisible(), true);
  // Also a row whose visibility is never set: the opacity, shown in every mode
  ctkCollapsibleGroupBox* threeDDisplayGroupBox = GetChild<ctkCollapsibleGroupBox>(widget, "ThreeDDisplayGroupBox");
  QFormLayout* threeDDisplayLayout = GetChild<QFormLayout>(widget, "threeDDisplayLayout");
  QWidget* opacitySliderWidget = GetChild<QWidget>(widget, "OpacitySliderWidget");
  CHECK_NOT_NULL(threeDDisplayGroupBox);
  CHECK_NOT_NULL(threeDDisplayLayout);
  CHECK_NOT_NULL(opacitySliderWidget);
  threeDDisplayGroupBox->setCollapsed(true);
  QApplication::processEvents();
  volumeDisplayNode->SetVisualizationMode(vtkMRMLVectorFieldDisplayNode::VisualizationModeGrid);
  threeDDisplayGroupBox->setCollapsed(false);
  QApplication::processEvents();
  int opacityRow = -1;
  QFormLayout::ItemRole opacityRole;
  threeDDisplayLayout->getWidgetPosition(opacitySliderWidget, &opacityRow, &opacityRole);
  CHECK_BOOL(opacityRow >= 0, true);
  CHECK_BOOL(opacitySliderWidget->isVisible(), true);
  widget.hide();

  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
