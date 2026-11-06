/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>

// CTK includes
#include "ctkTest.h"
#include <ctkCollapsibleButton.h>
#include <ctkCoordinatesWidget.h>
#include <ctkDoubleSpinBox.h>
#include <ctkSliderWidget.h>

// MRML includes
#include <qMRMLNodeComboBox.h>
#include <qMRMLSubjectHierarchyTreeView.h>
#include <qMRMLWidget.h>
#include "qSlicerTransformsModule.h"
#include "qSlicerTransformsModuleWidget.h"
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMeshDisplacementTransform.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVectorVolumeNode.h>

// SubjectHierarchy includes
#include <qSlicerSubjectHierarchyPluginHandler.h>
#include <qSlicerSubjectHierarchyPluginLogic.h>
#include <qSlicerSubjectHierarchyTransformsPlugin.h>
#include <vtkSlicerSubjectHierarchyModuleLogic.h>

// Slicer includes
#include <qSlicerApplication.h>

// VTK includes
#include <vtkBSplineTransform.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkOrientedBSplineTransform.h>
#include <vtkOrientedGridTransform.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkScatteredBSplineTransform.h>

// ----------------------------------------------------------------------------
class qSlicerTransformsModuleWidgetTester : public QObject
{
  Q_OBJECT

public:
  QSharedPointer<qSlicerTransformsModule> transformsModule;
  QSharedPointer<qSlicerSubjectHierarchyPluginLogic> pluginLogic;
  vtkSmartPointer<vtkSlicerSubjectHierarchyModuleLogic> shModuleLogic;
  vtkSmartPointer<vtkMRMLScene> scene;

  bool interactive{ false };

private slots:

  void initTestCase();
  void cleanupTestCase();

  void testIdentity();
  void testInvert();
  void testExportImport();

private:
  /// Qt 5 keeps the spacing of a hidden form row, so no hidden row may be left in the form
  static void verifyNoHiddenRows(QFormLayout* formLayout);
};

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::verifyNoHiddenRows(QFormLayout* formLayout)
{
  QVERIFY(formLayout != nullptr);
  for (int row = 0; row < formLayout->rowCount(); ++row)
  {
    QLayoutItem* labelItem = formLayout->itemAt(row, QFormLayout::LabelRole);
    QLayoutItem* fieldItem = formLayout->itemAt(row, QFormLayout::FieldRole);
    if (!fieldItem)
    {
      fieldItem = formLayout->itemAt(row, QFormLayout::SpanningRole);
    }
    QWidget* label = (labelItem ? labelItem->widget() : nullptr);
    QWidget* field = (fieldItem ? fieldItem->widget() : nullptr);
    QVERIFY2(field != nullptr, qPrintable(QString("Row %1 has no field").arg(row)));
    // A collapsed section hides its children, and keeps whether they are meant to be shown
    auto isShown = [](QWidget* widget)
    {
      QVariant visibilityToParent = widget->property("visibilityToParent");
      return (visibilityToParent.isValid() ? visibilityToParent.toBool() : !(widget->isHidden() && widget->testAttribute(Qt::WA_WState_ExplicitShowHide)));
    };
    QVERIFY2(isShown(field) || (label && isShown(label)), qPrintable(QString("Hidden row %1 (%2) is left in the form").arg(row).arg(field->objectName())));
  }
}

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::initTestCase()
{
  // Transforms module must be initialized once and only once and we do it here
  // (multiple initializations would result in re-registering plugins,
  // which would lead to warnings that would make the test fail).

  scene = vtkSmartPointer<vtkMRMLScene>::New();

  // Set up Subject Hierarchy logic (needed for the subject hierarchy tree view)
  shModuleLogic = vtkSmartPointer<vtkSlicerSubjectHierarchyModuleLogic>::New();
  shModuleLogic->SetMRMLScene(scene);
  pluginLogic = QSharedPointer<qSlicerSubjectHierarchyPluginLogic>(new qSlicerSubjectHierarchyPluginLogic());
  pluginLogic->setMRMLScene(scene);
  qSlicerSubjectHierarchyPluginHandler::instance()->setPluginLogic(pluginLogic.get());
  qSlicerSubjectHierarchyPluginHandler::instance()->setMRMLScene(scene);

  transformsModule = QSharedPointer<qSlicerTransformsModule>(new qSlicerTransformsModule);
  // register qSlicerSubjectHierarchyTransformsPlugin
  transformsModule->initialize(qSlicerApplication::application()->applicationLogic());
  transformsModule->setMRMLScene(scene);
  transformsModule->logic();
}

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::cleanupTestCase()
{
  transformsModule.clear();
  pluginLogic.clear();
  shModuleLogic = nullptr;
  scene = nullptr;
}

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::testIdentity()
{
  qSlicerTransformsModuleWidget* transformsWidget = dynamic_cast<qSlicerTransformsModuleWidget*>(transformsModule->widgetRepresentation());

  scene->Clear();
  vtkNew<vtkMRMLTransformNode> transformNode;
  scene->AddNode(transformNode);

  transformsWidget->setEditedNode(transformNode);

  vtkNew<vtkMatrix4x4> matrix;
  transformNode->GetMatrixTransformToParent(matrix);
  matrix->SetElement(0, 0, 10.);
  matrix->SetElement(1, 0, 2.);
  transformNode->SetMatrixTransformToParent(matrix);
  transformsWidget->identity();
  transformNode->GetMatrixTransformToParent(matrix);

  if (interactive)
  {
    transformsWidget->show();
    qApp->exec();
  }

  QCOMPARE(matrix->GetElement(0, 0), 1.);
  QCOMPARE(matrix->GetElement(1, 0), 0.);
}

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::testInvert()
{
  qSlicerTransformsModuleWidget* transformsWidget = dynamic_cast<qSlicerTransformsModuleWidget*>(transformsModule->widgetRepresentation());

  scene->Clear();
  vtkNew<vtkMRMLTransformNode> transformNode;
  scene->AddNode(transformNode);

  transformsWidget->setEditedNode(transformNode);

  vtkNew<vtkMatrix4x4> matrix;
  transformNode->GetMatrixTransformToParent(matrix);
  matrix->SetElement(0, 0, 10.);
  matrix->SetElement(1, 0, 2.);
  transformNode->SetMatrixTransformToParent(matrix);
  transformsWidget->invert();
  transformNode->GetMatrixTransformToParent(matrix);

  if (interactive)
  {
    transformsWidget->show();
    qApp->exec();
  }

  QCOMPARE(matrix->GetElement(0, 0), 0.1);
  QCOMPARE(matrix->GetElement(1, 0), -0.2);
}

// ----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetTester::testExportImport()
{
  qSlicerTransformsModuleWidget* transformsWidget = dynamic_cast<qSlicerTransformsModuleWidget*>(transformsModule->widgetRepresentation());

  scene->Clear();
  vtkNew<vtkMRMLTransformNode> transformNode;
  scene->AddNode(transformNode);
  vtkNew<vtkMatrix4x4> matrix;
  matrix->SetElement(0, 3, 5.0);
  transformNode->SetMatrixTransformToParent(matrix);
  transformsWidget->setEditedNode(transformNode);

  QRadioButton* exportRadioButton = transformsWidget->findChild<QRadioButton*>("ExportRadioButton");
  QRadioButton* importRadioButton = transformsWidget->findChild<QRadioButton*>("ImportRadioButton");
  QRadioButton* vectorVolumeOutputRadioButton = transformsWidget->findChild<QRadioButton*>("ExportVectorVolumeRadioButton");
  QRadioButton* magnitudeVolumeOutputRadioButton = transformsWidget->findChild<QRadioButton*>("ExportMagnitudeVolumeRadioButton");
  QRadioButton* vectorVolumeInputRadioButton = transformsWidget->findChild<QRadioButton*>("ImportVectorVolumeRadioButton");
  QRadioButton* modelInputRadioButton = transformsWidget->findChild<QRadioButton*>("ImportModelRadioButton");
  qMRMLNodeComboBox* referenceVolumeComboBox = transformsWidget->findChild<qMRMLNodeComboBox*>("ExportReferenceVolumeNodeComboBox");
  qMRMLNodeComboBox* outputNodeComboBox = transformsWidget->findChild<qMRMLNodeComboBox*>("ExportOutputNodeComboBox");
  qMRMLNodeComboBox* inputNodeComboBox = transformsWidget->findChild<qMRMLNodeComboBox*>("ImportInputNodeComboBox");
  QComboBox* arrayComboBox = transformsWidget->findChild<QComboBox*>("ImportDisplacementArrayComboBox");
  QPushButton* applyButton = transformsWidget->findChild<QPushButton*>("ExportImportPushButton");
  QVERIFY(exportRadioButton && importRadioButton && vectorVolumeOutputRadioButton && magnitudeVolumeOutputRadioButton);
  QVERIFY(vectorVolumeInputRadioButton && modelInputRadioButton && referenceVolumeComboBox && outputNodeComboBox);
  QVERIFY(inputNodeComboBox && arrayComboBox && applyButton);

  // The radio buttons are at the left: the rows end with a stretch
  for (const char* layoutName : { "exportImportOperationLayout", "exportOutputTypeLayout", "importInputTypeLayout" })
  {
    QHBoxLayout* radioLayout = transformsWidget->findChild<QHBoxLayout*>(layoutName);
    QVERIFY(radioLayout != nullptr);
    QVERIFY(radioLayout->itemAt(radioLayout->count() - 1)->spacerItem() != nullptr);
  }
  QFormLayout* exportImportLayout = transformsWidget->findChild<QFormLayout*>("exportImportLayout");
  verifyNoHiddenRows(exportImportLayout);

  // Export: the output node is of the chosen type, and is created if none is selected
  QVERIFY(exportRadioButton->isChecked());
  QVERIFY(referenceVolumeComboBox->isVisibleTo(transformsWidget));
  QVERIFY(!inputNodeComboBox->isVisibleTo(transformsWidget));
  QCOMPARE(outputNodeComboBox->nodeTypes(), QStringList("vtkMRMLTransformNode"));
  magnitudeVolumeOutputRadioButton->setChecked(true);
  QCOMPARE(outputNodeComboBox->nodeTypes(), QStringList("vtkMRMLScalarVolumeNode"));
  vectorVolumeOutputRadioButton->setChecked(true);
  QCOMPARE(outputNodeComboBox->nodeTypes(), QStringList("vtkMRMLVectorVolumeNode"));
  QVERIFY(!applyButton->isEnabled()); // no reference volume yet

  vtkMRMLScalarVolumeNode* referenceVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLScalarVolumeNode"));
  vtkNew<vtkImageData> referenceImage;
  referenceImage->SetDimensions(5, 5, 5);
  referenceImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
  referenceVolumeNode->SetAndObserveImageData(referenceImage);
  referenceVolumeComboBox->setCurrentNode(referenceVolumeNode);
  outputNodeComboBox->setCurrentNode(nullptr);
  QVERIFY(applyButton->isEnabled());
  applyButton->click();
  vtkMRMLVectorVolumeNode* exportedVolumeNode = vtkMRMLVectorVolumeNode::SafeDownCast(outputNodeComboBox->currentNode());
  QVERIFY(exportedVolumeNode != nullptr);
  QCOMPARE(exportedVolumeNode->GetImageData()->GetScalarComponentAsDouble(2, 2, 2, 0), 5.0);

  // Import from the exported volume: the transform becomes a grid transform
  importRadioButton->setChecked(true);
  verifyNoHiddenRows(exportImportLayout);
  QVERIFY(!referenceVolumeComboBox->isVisibleTo(transformsWidget));
  QVERIFY(inputNodeComboBox->isVisibleTo(transformsWidget));
  QVERIFY(vectorVolumeInputRadioButton->isChecked());
  QCOMPARE(inputNodeComboBox->nodeTypes(), QStringList("vtkMRMLVectorVolumeNode"));
  // The voxels hold the displacements: nothing to choose, but the row stays
  QVERIFY(arrayComboBox->isVisibleTo(transformsWidget));
  QVERIFY(!arrayComboBox->isEnabled());
  QCOMPARE(arrayComboBox->currentText(), QString("voxels"));

  // Without a transform, the section is still available, and the import goes into a new
  // transform, which is then selected
  qMRMLSubjectHierarchyTreeView* transformSelector = transformsWidget->findChild<qMRMLSubjectHierarchyTreeView*>("TransformNodeSelector");
  QVERIFY(transformSelector != nullptr);
  transformSelector->setCurrentItem(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);
  QWidget* exportImportSectionWidget = transformsWidget->findChild<QWidget*>("ExportImportCollapsibleButton");
  QVERIFY(exportImportSectionWidget && exportImportSectionWidget->isEnabled());
  inputNodeComboBox->setCurrentNode(exportedVolumeNode);
  QVERIFY(applyButton->isEnabled());
  int numberOfTransformNodes = scene->GetNumberOfNodesByClass("vtkMRMLTransformNode");
  applyButton->click();
  QCOMPARE(scene->GetNumberOfNodesByClass("vtkMRMLTransformNode"), numberOfTransformNodes + 1);
  vtkMRMLTransformNode* newTransformNode = vtkMRMLTransformNode::SafeDownCast(transformSelector->currentNode());
  QVERIFY(newTransformNode != nullptr);
  QVERIFY(vtkOrientedGridTransform::SafeDownCast(newTransformNode->GetTransformToParent()) != nullptr);
  // Export still needs a transform
  transformSelector->setCurrentItem(vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID);
  exportRadioButton->setChecked(true);
  QVERIFY(!applyButton->isEnabled());
  importRadioButton->setChecked(true);

  vtkNew<vtkMRMLTransformNode> importedTransformNode;
  scene->AddNode(importedTransformNode);
  transformsWidget->setEditedNode(importedTransformNode);
  inputNodeComboBox->setCurrentNode(exportedVolumeNode);
  QVERIFY(applyButton->isEnabled());
  applyButton->click();
  QVERIFY(vtkOrientedGridTransform::SafeDownCast(importedTransformNode->GetTransformToParent()) != nullptr);
  double origin[3] = { 2.0, 2.0, 2.0 };
  double moved[3] = { 0.0, 0.0, 0.0 };
  importedTransformNode->GetTransformToParent()->TransformPoint(origin, moved);
  QVERIFY(std::abs(moved[0] - 7.0) < 1e-6);

  // Import from a model: only its 3-component point arrays are offered
  modelInputRadioButton->setChecked(true);
  verifyNoHiddenRows(exportImportLayout);
  QCOMPARE(inputNodeComboBox->nodeTypes(), QStringList("vtkMRMLModelNode"));
  QVERIFY(arrayComboBox->isVisibleTo(transformsWidget));
  QVERIFY(arrayComboBox->isEnabled());
  vtkNew<vtkSphereSource> sphere;
  sphere->SetRadius(10.0);
  sphere->Update();
  vtkNew<vtkPolyData> mesh;
  mesh->DeepCopy(sphere->GetOutput());
  vtkNew<vtkDoubleArray> displacements;
  displacements->SetName("Displacement");
  displacements->SetNumberOfComponents(3);
  displacements->SetNumberOfTuples(mesh->GetNumberOfPoints());
  displacements->Fill(1.0);
  mesh->GetPointData()->AddArray(displacements);
  vtkNew<vtkDoubleArray> velocities;
  velocities->SetName("Velocity");
  velocities->SetNumberOfComponents(3);
  velocities->SetNumberOfTuples(mesh->GetNumberOfPoints());
  velocities->Fill(2.0);
  mesh->GetPointData()->AddArray(velocities);
  vtkNew<vtkDoubleArray> scalars;
  scalars->SetName("Pressure");
  scalars->SetNumberOfTuples(mesh->GetNumberOfPoints());
  scalars->Fill(0.0);
  mesh->GetPointData()->AddArray(scalars);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLModelNode"));
  modelNode->SetAndObserveMesh(mesh);
  inputNodeComboBox->setCurrentNode(modelNode);
  QStringList arrayNames;
  for (int itemIndex = 0; itemIndex < arrayComboBox->count(); ++itemIndex)
  {
    arrayNames << arrayComboBox->itemText(itemIndex);
  }
  QVERIFY(arrayNames.contains("Displacement"));
  QVERIFY(!arrayNames.contains("Pressure"));
  arrayComboBox->setCurrentText("Displacement");
  QVERIFY(applyButton->isEnabled());
  applyButton->click();
  // An ordinary transform node, with a scattered transform that is linked to the model
  vtkScatteredBSplineTransform* scatteredTransform = vtkScatteredBSplineTransform::SafeDownCast(importedTransformNode->GetScatteredTransform());
  QVERIFY(scatteredTransform != nullptr);
  QCOMPARE(importedTransformNode->GetDisplacementModelNode(), modelNode);
  QCOMPARE(QString(scatteredTransform->GetDisplacementArrayName()), QString("Displacement"));
  importedTransformNode->GetTransformToParent()->TransformPoint(origin, moved);
  QVERIFY(std::abs(moved[0] - 3.0) < 0.01);

  // Its model and array can be changed in the Edit section, and are shown there
  QWidget* scatteredSourceWidget = transformsWidget->findChild<QWidget*>("ScatteredTransformSourceWidget");
  qMRMLNodeComboBox* scatteredModelComboBox = transformsWidget->findChild<qMRMLNodeComboBox*>("ScatteredSourceModelNodeComboBox");
  QComboBox* scatteredArrayComboBox = transformsWidget->findChild<QComboBox*>("ScatteredDisplacementArrayComboBox");
  QVERIFY(scatteredSourceWidget && scatteredModelComboBox && scatteredArrayComboBox);
  QVERIFY(scatteredSourceWidget->isVisibleTo(transformsWidget));
  QCOMPARE(scatteredModelComboBox->currentNode(), modelNode);
  QCOMPARE(scatteredArrayComboBox->currentText(), QString("Displacement"));
  scatteredArrayComboBox->setCurrentIndex(scatteredArrayComboBox->findText("Velocity"));
  QCOMPARE(QString(scatteredTransform->GetDisplacementArrayName()), QString("Velocity"));
  importedTransformNode->GetTransformToParent()->TransformPoint(origin, moved);
  QVERIFY(std::abs(moved[0] - 4.0) < 0.01);
  // Set on the transform, shown in the section
  scatteredTransform->SetDisplacementArrayName("Displacement");
  QCOMPARE(scatteredArrayComboBox->currentText(), QString("Displacement"));
  scatteredModelComboBox->setCurrentNode(nullptr);
  QVERIFY(importedTransformNode->GetDisplacementModelNode() == nullptr);
  QVERIFY(!scatteredArrayComboBox->isEnabled());
  scatteredModelComboBox->setCurrentNode(modelNode);
  QCOMPARE(importedTransformNode->GetDisplacementModelNode(), modelNode);

  // How it is fitted, and how closely
  QLabel* fitResultLabel = transformsWidget->findChild<QLabel*>("ScatteredFitResultValueLabel");
  QVERIFY(fitResultLabel != nullptr);
  QVERIFY(fitResultLabel->text().contains("levels"));
  ctkSliderWidget* displacementScaleSlider = transformsWidget->findChild<ctkSliderWidget*>("ScatteredDisplacementScaleSliderWidget");
  QToolButton* displacementScaleReverseButton = transformsWidget->findChild<QToolButton*>("ScatteredDisplacementScaleReverseButton");
  ctkDoubleSpinBox* extrapolationSpinBox = transformsWidget->findChild<ctkDoubleSpinBox*>("ScatteredExtrapolationDistanceSpinBox");
  ctkCoordinatesWidget* controlPointSpacingWidget = transformsWidget->findChild<ctkCoordinatesWidget*>("ScatteredControlPointSpacingWidget");
  ctkDoubleSpinBox* minimumSpacingSpinBox = transformsWidget->findChild<ctkDoubleSpinBox*>("ScatteredMinimumControlPointSpacingSpinBox");
  QSpinBox* maximumLevelsSpinBox = transformsWidget->findChild<QSpinBox*>("ScatteredMaximumNumberOfLevelsSpinBox");
  ctkDoubleSpinBox* toleranceSpinBox = transformsWidget->findChild<ctkDoubleSpinBox*>("ScatteredToleranceSpinBox");
  QCheckBox* linearApproximationCheckBox = transformsWidget->findChild<QCheckBox*>("ScatteredUseLinearApproximationCheckBox");
  QComboBox* borderModeComboBox = transformsWidget->findChild<QComboBox*>("ScatteredBorderModeComboBox");
  QVERIFY(displacementScaleSlider && displacementScaleReverseButton && extrapolationSpinBox && controlPointSpacingWidget && minimumSpacingSpinBox);
  QVERIFY(maximumLevelsSpinBox && toleranceSpinBox && linearApproximationCheckBox && borderModeComboBox);
  // The values that were set from the points when the transform was created
  QCOMPARE(extrapolationSpinBox->value(), scatteredTransform->GetGridBoundsMargin());
  QVERIFY(extrapolationSpinBox->value() > 0.0);
  displacementScaleSlider->setValue(2.0);
  QCOMPARE(scatteredTransform->GetDisplacementScale(), 2.0);
  importedTransformNode->GetTransformToParent()->TransformPoint(origin, moved);
  QVERIFY(std::abs(moved[0] - 4.0) < 0.01);
  // The reverse button makes the displacement scale negative. The scale applies to the spline part of the fit and the
  // moved position is linear in it, so the position at scale 0 is halfway between those at 2 and -2.
  QVERIFY(!displacementScaleReverseButton->isChecked());
  displacementScaleReverseButton->click();
  QCOMPARE(scatteredTransform->GetDisplacementScale(), -2.0);
  double movedReversed[3] = { 0.0, 0.0, 0.0 };
  importedTransformNode->GetTransformToParent()->TransformPoint(origin, movedReversed);
  QVERIFY(movedReversed[0] < moved[0] - 1.0);
  scatteredTransform->SetDisplacementScale(0.0);
  double movedUnscaled[3] = { 0.0, 0.0, 0.0 };
  importedTransformNode->GetTransformToParent()->TransformPoint(origin, movedUnscaled);
  QVERIFY(std::abs(movedReversed[0] + moved[0] - 2.0 * movedUnscaled[0]) < 0.01);
  QVERIFY(displacementScaleReverseButton->isChecked() == false); // 0 is shown without the sign
  scatteredTransform->SetDisplacementScale(-2.0);
  QVERIFY(displacementScaleReverseButton->isChecked());
  displacementScaleSlider->setValue(0.5);
  QCOMPARE(scatteredTransform->GetDisplacementScale(), -0.5);
  // Set on the transform: the slider shows its magnitude, the button its sign, and the range
  // of the slider grows to a value beyond it
  scatteredTransform->SetDisplacementScale(1.5);
  QCOMPARE(displacementScaleSlider->value(), 1.5);
  QVERIFY(!displacementScaleReverseButton->isChecked());
  scatteredTransform->SetDisplacementScale(-25.0);
  QCOMPARE(displacementScaleSlider->value(), 25.0);
  QVERIFY(displacementScaleSlider->maximum() >= 25.0);
  QVERIFY(displacementScaleReverseButton->isChecked());
  displacementScaleReverseButton->click();
  QCOMPARE(scatteredTransform->GetDisplacementScale(), 25.0);
  displacementScaleSlider->setValue(2.0);
  QCOMPARE(scatteredTransform->GetDisplacementScale(), 2.0);
  extrapolationSpinBox->setValue(3.0);
  QCOMPARE(scatteredTransform->GetGridBoundsMargin(), 3.0);
  controlPointSpacingWidget->setCoordinates(4.0, 5.0, 6.0);
  QCOMPARE(scatteredTransform->GetControlPointSpacing()[1], 5.0);
  minimumSpacingSpinBox->setValue(0.5);
  QCOMPARE(scatteredTransform->GetMinimumControlPointSpacing(), 0.5);
  maximumLevelsSpinBox->setValue(3);
  QCOMPARE(scatteredTransform->GetMaximumNumberOfLevels(), 3);
  toleranceSpinBox->setValue(0.02);
  QCOMPARE(scatteredTransform->GetTolerance(), 0.02);
  linearApproximationCheckBox->setChecked(false);
  QCOMPARE(scatteredTransform->GetUseLinearApproximation(), false);
  borderModeComboBox->setCurrentIndex(borderModeComboBox->findData(VTK_BSPLINE_ZERO_AT_BORDER));
  QCOMPARE(scatteredTransform->GetBorderMode(), VTK_BSPLINE_ZERO_AT_BORDER);
  // Set on the transform, shown in the section
  scatteredTransform->SetDisplacementScale(0.5);
  QCOMPARE(displacementScaleSlider->value(), 0.5);
  scatteredTransform->SetBorderMode(VTK_BSPLINE_ZERO);
  QCOMPARE(borderModeComboBox->currentData().toInt(), VTK_BSPLINE_ZERO);
  scatteredTransform->SetMaximumNumberOfLevels(5);
  QCOMPARE(maximumLevelsSpinBox->value(), 5);

  // The displacements can be used as they are, without fitting: another transform, with the
  // same displacements
  QComboBox* interpolationComboBox = transformsWidget->findChild<QComboBox*>("ScatteredInterpolationComboBox");
  QWidget* fittingGroupBox = transformsWidget->findChild<QWidget*>("ScatteredFittingGroupBox");
  QVERIFY(interpolationComboBox && fittingGroupBox);
  QVERIFY(fittingGroupBox->isEnabled());
  interpolationComboBox->setCurrentIndex(1);
  vtkMeshDisplacementTransform* meshTransform = vtkMeshDisplacementTransform::SafeDownCast(importedTransformNode->GetScatteredTransform());
  QVERIFY(meshTransform != nullptr);
  QCOMPARE(QString(meshTransform->GetDisplacementArrayName()), QString("Displacement"));
  QCOMPARE(meshTransform->GetDisplacementScale(), 0.5);
  QCOMPARE(importedTransformNode->GetDisplacementModelNode(), modelNode);
  QVERIFY(!fittingGroupBox->isEnabled());
  QVERIFY(fitResultLabel->text().contains("not fitted"));
  interpolationComboBox->setCurrentIndex(0);
  QVERIFY(vtkScatteredBSplineTransform::SafeDownCast(importedTransformNode->GetScatteredTransform()) != nullptr);
  QVERIFY(fittingGroupBox->isEnabled());

  // Inverted, the transform to parent is computed from it: still edited here, with a note
  QVERIFY(!fitResultLabel->text().contains("Computed from inverse"));
  vtkAbstractTransform* fittedTransform = importedTransformNode->GetScatteredTransform();
  importedTransformNode->Inverse();
  QCOMPARE(importedTransformNode->GetScatteredTransform(), fittedTransform);
  QVERIFY(importedTransformNode->IsScatteredTransformFromParent());
  QVERIFY(scatteredSourceWidget->isVisibleTo(transformsWidget));
  QVERIFY(fitResultLabel->text().contains("Computed from inverse"));
  maximumLevelsSpinBox->setValue(4);
  QCOMPARE(vtkScatteredBSplineTransform::SafeDownCast(fittedTransform)->GetMaximumNumberOfLevels(), 4);
  // Another interpolation keeps the direction
  interpolationComboBox->setCurrentIndex(1);
  QVERIFY(vtkMeshDisplacementTransform::SafeDownCast(importedTransformNode->GetScatteredTransform()) != nullptr);
  QVERIFY(importedTransformNode->IsScatteredTransformFromParent());
  interpolationComboBox->setCurrentIndex(0);
  QVERIFY(importedTransformNode->IsScatteredTransformFromParent());
  importedTransformNode->Inverse();
  QVERIFY(!importedTransformNode->IsScatteredTransformFromParent());
  QVERIFY(!fitResultLabel->text().contains("Computed from inverse"));
  // Other transforms are not edited here
  transformsWidget->setEditedNode(transformNode);
  QVERIFY(!scatteredSourceWidget->isVisibleTo(transformsWidget));

  exportRadioButton->setChecked(true);
  verifyNoHiddenRows(exportImportLayout);

  // Rows that are updated while the section is collapsed are all there when it is expanded: a
  // collapsed section hides its children, which does not mean that their rows are not used
  transformsWidget->show();
  ctkCollapsibleButton* exportImportSection = transformsWidget->findChild<ctkCollapsibleButton*>("ExportImportCollapsibleButton");
  QVERIFY(exportImportSection != nullptr);
  exportImportSection->setCollapsed(true);
  QApplication::processEvents();
  importRadioButton->setChecked(true);
  exportRadioButton->setChecked(true);
  exportImportSection->setCollapsed(false);
  QApplication::processEvents();
  // Shown, and laid out: a widget that is taken out of the form is shown by the expanded
  // section anyway, but not where it belongs
  QWidget* operationWidget = transformsWidget->findChild<QWidget*>("ExportImportOperationWidget");
  QVERIFY(operationWidget != nullptr);
  for (QWidget* rowWidget : { operationWidget, static_cast<QWidget*>(referenceVolumeComboBox), static_cast<QWidget*>(outputNodeComboBox), static_cast<QWidget*>(applyButton) })
  {
    QVERIFY2(rowWidget->isVisible(), qPrintable(rowWidget->objectName()));
    int row = -1;
    QFormLayout::ItemRole role;
    exportImportLayout->getWidgetPosition(rowWidget, &row, &role);
    QVERIFY2(row >= 0, qPrintable(rowWidget->objectName() + QString(" is not in the form")));
  }
  QVERIFY(!inputNodeComboBox->isVisible());
  verifyNoHiddenRows(exportImportLayout);
  transformsWidget->hide();
}

// ----------------------------------------------------------------------------
// CTK_TEST_MAIN(qSlicerTransformsModuleWidgetTest)
int qSlicerTransformsModuleWidgetTest(int argc, char* argv[])
{
  qMRMLWidget::preInitializeApplication();
  qSlicerApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qSlicerTransformsModuleWidgetTester tc;
  tc.interactive = false; // set it to true for interactive testing

  return QTest::qExec(&tc, argc, argv);
}

#include "qSlicerTransformsModuleWidgetTest.moc"
