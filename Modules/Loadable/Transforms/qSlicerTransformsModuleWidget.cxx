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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QAction>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QMessageBox>
#include <QRadioButton>
#include <QStringBuilder>
#include <QTableWidgetItem>

// C++ includes
#include <cmath>

// Slicer includes
#include "qSlicerTransformsModuleWidget.h"
#include "ui_qSlicerTransformsModuleWidget.h"

// vtkSlicerLogic includes
#include "vtkSlicerTransformLogic.h"

// Colors includes
#include <vtkMRMLColorLegendDisplayNode.h>
#include <vtkSlicerColorLogic.h>

// MRMLWidgets includes
#include <qMRMLUtils.h>

// vtkAddon includes
#include <vtkScatteredBSplineTransform.h>

// CTK includes
#include <ctkCoordinatesWidget.h>
#include <ctkDoubleSpinBox.h>
#include <ctkSliderWidget.h>

// MRML includes
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMeshDisplacementTransform.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformDisplayNode.h"
#include "vtkMRMLVectorVolumeNode.h"

// VTK includes
#include <vtkAddonMathUtilities.h>
#include <vtkBSplineTransform.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkGeneralTransform.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>

static const int COORDINATE_COMBOBOX_INDEX_WORLD = 0;
static const int COORDINATE_COMBOBOX_INDEX_LOCAL = 1;

//-----------------------------------------------------------------------------
class qSlicerTransformsModuleWidgetPrivate : public Ui_qSlicerTransformsModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerTransformsModuleWidget);

protected:
  qSlicerTransformsModuleWidget* const q_ptr;

public:
  qSlicerTransformsModuleWidgetPrivate(qSlicerTransformsModuleWidget& object);
  static QList<vtkSmartPointer<vtkMRMLTransformableNode>> getSelectedNodes(qMRMLTreeView* tree);
  vtkSlicerTransformLogic* logic() const;
  vtkMRMLTransformNode* MRMLTransformNode;
  QAction* CopyAction;
  QAction* PasteAction;

  /// Export/import form rows in order. Qt 5 keeps spacing for hidden rows, so unused rows are taken out of the
  /// form and put back in place when shown again.
  QList<QPair<QWidget*, QWidget*>> ExportImportRows;
  void compactExportImportRows();

  /// How a scattered transform interpolates the displacements: fitted or as they are. Added in
  /// code, below the displacement array.
  QLabel* ScatteredInterpolationLabel{ nullptr };
  QComboBox* ScatteredInterpolationComboBox{ nullptr };
};

//-----------------------------------------------------------------------------
namespace
{
/// How the displacements of a scattered transform are interpolated, as data of the combo box
enum
{
  InterpolationBSpline = 0, ///< vtkScatteredBSplineTransform
  InterpolationMesh = 1     ///< vtkMeshDisplacementTransform
};

/// Grow the range of a slider that starts at 0 so that it can show a value: to a round maximum
/// that leaves room to make it a few times larger. A range that already holds the value is kept.
void SetSliderRangeToFit(ctkSliderWidget* slider, double value)
{
  if (value <= slider->maximum())
  {
    return;
  }
  double maximum = std::pow(10.0, std::ceil(std::log10(3.0 * value)));
  bool wasBlocked = slider->blockSignals(true);
  slider->setRange(0.0, maximum);
  slider->setSingleStep(maximum / 1000.0);
  slider->blockSignals(wasBlocked);
}

/// Shown as the displacement array of a vector volume, whose voxels hold the displacements
QString VoxelsArrayText()
{
  return qSlicerTransformsModuleWidget::tr("voxels");
}

/// Whether a widget is meant to be shown. Collapsed CTK sections hide children but store the intended state in the
/// "visibilityToParent" property; without that property, only an explicit hide counts.
bool IsShownToParent(QWidget* widget)
{
  QVariant visibilityToParent = widget->property("visibilityToParent");
  return (visibilityToParent.isValid() ? visibilityToParent.toBool() : !(widget->isHidden() && widget->testAttribute(Qt::WA_WState_ExplicitShowHide)));
}

/// The 3-component point data arrays of the mesh of a model, the active vectors first: the
/// arrays that can hold displacements
QStringList GetDisplacementArrayNames(vtkMRMLModelNode* modelNode)
{
  QStringList arrayNames;
  vtkPointSet* mesh = (modelNode ? modelNode->GetMesh() : nullptr);
  if (!mesh)
  {
    return arrayNames;
  }
  vtkPointData* pointData = mesh->GetPointData();
  vtkDataArray* activeVectors = pointData->GetVectors();
  for (int arrayIndex = 0; arrayIndex < pointData->GetNumberOfArrays(); ++arrayIndex)
  {
    vtkDataArray* array = pointData->GetArray(arrayIndex);
    if (array && array->GetName() && array->GetNumberOfComponents() == 3)
    {
      if (array == activeVectors)
      {
        arrayNames.prepend(QString::fromUtf8(array->GetName()));
      }
      else
      {
        arrayNames.append(QString::fromUtf8(array->GetName()));
      }
    }
  }
  return arrayNames;
}

/// Fill a combo box with names, keeping the selected one if it is still there
void SetComboBoxItems(QComboBox* comboBox, const QStringList& names, const QString& selectedName)
{
  QStringList currentNames;
  for (int itemIndex = 0; itemIndex < comboBox->count(); ++itemIndex)
  {
    currentNames << comboBox->itemText(itemIndex);
  }
  bool wasBlocked = comboBox->blockSignals(true);
  if (currentNames != names)
  {
    comboBox->clear();
    comboBox->addItems(names);
  }
  int selectedIndex = comboBox->findText(selectedName);
  comboBox->setCurrentIndex(selectedIndex >= 0 ? selectedIndex : (names.isEmpty() ? -1 : 0));
  comboBox->blockSignals(wasBlocked);
}
} // namespace

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidgetPrivate::compactExportImportRows()
{
  int rowIndex = 0; // where the next shown row goes
  for (const QPair<QWidget*, QWidget*>& row : this->ExportImportRows)
  {
    QWidget* label = row.first;
    QWidget* field = row.second;
    bool shown = (IsShownToParent(field) || (label && IsShownToParent(label)));
    int currentRow = -1;
    QFormLayout::ItemRole role;
    this->exportImportLayout->getWidgetPosition(field, &currentRow, &role);
    bool inLayout = (currentRow >= 0);
    if (shown && !inLayout)
    {
      if (label)
      {
        this->exportImportLayout->insertRow(rowIndex, label, field);
      }
      else
      {
        this->exportImportLayout->insertRow(rowIndex, field);
      }
    }
    else if (!shown && inLayout)
    {
      // The widgets stay, hidden, as children of the section
      QFormLayout::TakeRowResult taken = this->exportImportLayout->takeRow(field);
      delete taken.labelItem;
      delete taken.fieldItem;
    }
    if (shown)
    {
      ++rowIndex;
    }
  }
}

//-----------------------------------------------------------------------------
qSlicerTransformsModuleWidgetPrivate::qSlicerTransformsModuleWidgetPrivate(qSlicerTransformsModuleWidget& object)
  : q_ptr(&object)
{
  this->MRMLTransformNode = nullptr;
  this->CopyAction = nullptr;
  this->PasteAction = nullptr;
}
//-----------------------------------------------------------------------------
vtkSlicerTransformLogic* qSlicerTransformsModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerTransformsModuleWidget);
  return vtkSlicerTransformLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
QList<vtkSmartPointer<vtkMRMLTransformableNode>> qSlicerTransformsModuleWidgetPrivate::getSelectedNodes(qMRMLTreeView* tree)
{
  QModelIndexList selectedIndexes = tree->selectionModel()->selectedRows();
  selectedIndexes = qMRMLTreeView::removeChildren(selectedIndexes);

  // Return the list of nodes
  QList<vtkSmartPointer<vtkMRMLTransformableNode>> selectedNodes;
  for (const QModelIndex& selectedIndex : selectedIndexes)
  {
    vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(tree->sortFilterProxyModel()->mrmlNodeFromIndex(selectedIndex));
    Q_ASSERT(node);
    selectedNodes << node;
  }
  return selectedNodes;
}

//-----------------------------------------------------------------------------
qSlicerTransformsModuleWidget::qSlicerTransformsModuleWidget(QWidget* _parentWidget)
  : Superclass(_parentWidget)
  , d_ptr(new qSlicerTransformsModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerTransformsModuleWidget::~qSlicerTransformsModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::setup()
{
  Q_D(qSlicerTransformsModuleWidget);
  this->Superclass::setup();
  d->setupUi(this);

  // Add coordinate reference button to a button group
  d->CopyAction = new QAction(this);
  d->CopyAction->setIcon(QIcon(":Icons/Medium/SlicerEditCopy.png"));
  d->CopyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  d->CopyAction->setShortcuts(QKeySequence::Copy);
  d->CopyAction->setToolTip(tr("Copy"));
  this->addAction(d->CopyAction);
  d->PasteAction = new QAction(this);
  d->PasteAction->setIcon(QIcon(":Icons/Medium/SlicerEditPaste.png"));
  d->PasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  d->PasteAction->setShortcuts(QKeySequence::Paste);
  d->PasteAction->setToolTip(tr("Paste"));
  this->addAction(d->PasteAction);

  // Connect button group
  this->connect(d->TranslateFirstToolButton, SIGNAL(toggled(bool)), SLOT(onTranslateFirstButtonPressed(bool)));

  // Connect identity button
  this->connect(d->IdentityPushButton, SIGNAL(clicked()), SLOT(identity()));

  // Connect invert button
  this->connect(d->InvertPushButton, SIGNAL(clicked()), SLOT(invert()));

  // Connect split button
  this->connect(d->SplitPushButton, SIGNAL(clicked()), SLOT(split()));

  // Connect the center of transformation combobox
  this->connect(d->CenterOfTransformationCoordinatesComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateCenterOfTransformationWidgets()));

  // Connect the center of transformation spinboxes
  this->connect(d->CenterOfTransformationCoordinatesWidget, SIGNAL(coordinatesChanged(double*)), SLOT(onCenterOfTransformationChanged()));

  // Connect the reset center of transformation button
  this->connect(d->ResetCenterOfTransformationButton, SIGNAL(clicked()), SLOT(resetCenterOfTransformation()));

  // Connect node selector with module itself
  this->connect(d->TransformNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), SLOT(onNodeSelected(vtkMRMLNode*)));
  // A collapsible group box reports being opened as being checked
  this->connect(d->ColorLegendCollapsibleGroupBox, SIGNAL(toggled(bool)), SLOT(colorLegendCollapsibleButtonCollapsed(bool)));
  // The legend explains what the colors of the displacement field mean, so it belongs with
  // the rest of the coloring
  d->TransformDisplayNodeWidget->addColoringWidget(d->ColorLegendCollapsibleGroupBox);

  // Set a static min/max range to let users freely enter values
  d->MatrixWidget->setRange(-1e10, 1e10);

  // Homogeneous transformation matrix is expected to have (0,0,0,1)
  // in its last row, so do not allow users to edit the last row.
  for (int col = 0; col < 4; col++)
  {
    QTableWidgetItem* item = d->MatrixWidget->widgetItem(3, col);
    if (!item)
    {
      continue;
    }
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
  }

  d->RotationSliders->setSingleStep(0.1);
  d->RotationSliders->setDecimals(1);

  // Transform nodes connection
  this->connect(d->TransformToolButton, SIGNAL(clicked()), SLOT(transformSelectedNodes()));
  this->connect(d->UntransformToolButton, SIGNAL(clicked()), SLOT(untransformSelectedNodes()));
  this->connect(d->HardenToolButton, SIGNAL(clicked()), SLOT(hardenSelectedNodes()));

  // Observe display section, if opened, then add display node
  this->connect(d->DisplayCollapsibleButton, SIGNAL(clicked(bool)), SLOT(onDisplaySectionClicked(bool)));

  // Observe Apply transform section to maintain a nice layout
  // even when the section is closed.
  this->connect(d->TransformedCollapsibleButton, SIGNAL(clicked(bool)), SLOT(onTransformableSectionClicked(bool)));

  // Connect copy and paste actions
  d->CopyTransformToolButton->setDefaultAction(d->CopyAction);
  this->connect(d->CopyAction, SIGNAL(triggered()), SLOT(copyTransform()));

  d->PasteTransformToolButton->setDefaultAction(d->PasteAction);
  this->connect(d->PasteAction, SIGNAL(triggered()), SLOT(pasteTransform()));

  // Icons
  QIcon rightIcon = QApplication::style()->standardIcon(QStyle::SP_ArrowRight);
  d->TransformToolButton->setIcon(rightIcon);

  QIcon leftIcon = QApplication::style()->standardIcon(QStyle::SP_ArrowLeft);
  d->UntransformToolButton->setIcon(leftIcon);

  // Export/import
  for (int row = 0; row < d->exportImportLayout->rowCount(); ++row)
  {
    QLayoutItem* labelItem = d->exportImportLayout->itemAt(row, QFormLayout::LabelRole);
    QLayoutItem* fieldItem = d->exportImportLayout->itemAt(row, QFormLayout::FieldRole);
    if (!fieldItem)
    {
      fieldItem = d->exportImportLayout->itemAt(row, QFormLayout::SpanningRole);
    }
    if (fieldItem && fieldItem->widget())
    {
      d->ExportImportRows.append(qMakePair(labelItem ? labelItem->widget() : nullptr, fieldItem->widget()));
    }
  }
  this->connect(d->ExportImportPushButton, SIGNAL(clicked()), SLOT(applyExportImport()));
  for (QRadioButton* radioButton : { d->ExportRadioButton,
                                     d->ImportRadioButton,
                                     d->ExportGridTransformRadioButton,
                                     d->ExportMagnitudeVolumeRadioButton,
                                     d->ExportVectorVolumeRadioButton,
                                     d->ImportVectorVolumeRadioButton,
                                     d->ImportModelRadioButton })
  {
    this->connect(radioButton, SIGNAL(toggled(bool)), SLOT(updateExportImportWidgets()));
  }
  this->connect(d->ExportReferenceVolumeNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), SLOT(updateExportImportWidgets()));
  this->connect(d->ExportOutputNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), SLOT(updateExportImportWidgets()));
  this->connect(d->ImportInputNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), SLOT(updateExportImportWidgets()));

  // Source of a scattered transform
  this->connect(d->ScatteredSourceModelNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), SLOT(onScatteredSourceModelChanged(vtkMRMLNode*)));
  this->connect(d->ScatteredDisplacementArrayComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onScatteredDisplacementArrayChanged(int)));
  // How the displacements are interpolated
  d->ScatteredInterpolationLabel = new QLabel(tr("Interpolation:"), d->ScatteredTransformSourceWidget);
  d->ScatteredInterpolationLabel->setObjectName("ScatteredInterpolationLabel");
  d->ScatteredInterpolationComboBox = new QComboBox(d->ScatteredTransformSourceWidget);
  d->ScatteredInterpolationComboBox->setObjectName("ScatteredInterpolationComboBox");
  d->ScatteredInterpolationComboBox->setToolTip(tr("B-spline fit: a smooth transform is fitted to the displacements, defined everywhere, also beyond the mesh (see Fitting). "
                                                   "Mesh: the displacements are used as they are: the points of the mesh are moved by their own vectors, exactly as the "
                                                   "model is warped by them, and other positions by the vector interpolated in the cell of the mesh that contains them. "
                                                   "Positions outside the cells are not moved."));
  d->ScatteredInterpolationComboBox->addItem(tr("B-spline fit"), InterpolationBSpline);
  d->ScatteredInterpolationComboBox->addItem(tr("Mesh"), InterpolationMesh);
  d->scatteredTransformSourceLayout->insertRow(2, d->ScatteredInterpolationLabel, d->ScatteredInterpolationComboBox);
  this->connect(d->ScatteredInterpolationComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onScatteredFittingParameterChanged()));

  // How it is fitted
  d->ScatteredBorderModeComboBox->addItem(tr("Keep value at border"), VTK_BSPLINE_EDGE);
  d->ScatteredBorderModeComboBox->addItem(tr("Zero outside"), VTK_BSPLINE_ZERO);
  d->ScatteredBorderModeComboBox->addItem(tr("Fade to zero at border"), VTK_BSPLINE_ZERO_AT_BORDER);
  this->connect(d->ScatteredDisplacementScaleSliderWidget, SIGNAL(valueChanged(double)), SLOT(onScatteredFittingParameterChanged()));
  this->connect(d->ScatteredDisplacementScaleReverseButton, SIGNAL(toggled(bool)), SLOT(onScatteredFittingParameterChanged()));
  this->connect(d->ScatteredExtrapolationDistanceSpinBox, SIGNAL(valueChanged(double)), SLOT(onScatteredFittingParameterChanged()));
  this->connect(d->ScatteredControlPointSpacingWidget, SIGNAL(coordinatesChanged(double*)), SLOT(onScatteredFittingParameterChanged()));
  this->connect(d->ScatteredMinimumControlPointSpacingSpinBox, SIGNAL(valueChanged(double)), SLOT(onScatteredFittingParameterChanged()));
  this->connect(d->ScatteredMaximumNumberOfLevelsSpinBox, SIGNAL(valueChanged(int)), SLOT(onScatteredFittingParameterChanged()));
  this->connect(d->ScatteredToleranceSpinBox, SIGNAL(valueChanged(double)), SLOT(onScatteredFittingParameterChanged()));
  this->connect(d->ScatteredUseLinearApproximationCheckBox, SIGNAL(toggled(bool)), SLOT(onScatteredFittingParameterChanged()));
  this->connect(d->ScatteredBorderModeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onScatteredFittingParameterChanged()));

  this->onTransformableSectionClicked(d->TransformedCollapsibleButton->isChecked());
  this->onNodeSelected(nullptr);
  this->updateExportImportWidgets();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::enter()
{
  Q_D(qSlicerTransformsModuleWidget);

  // Connect SH item modified event so that widget state is updated when a display node is created
  // on the currently selected item (when color is set to a folder)
  vtkMRMLSubjectHierarchyNode* shNode = d->TransformNodeSelector->subjectHierarchyNode();
  if (shNode)
  {
    qvtkConnect(shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemDisplayModifiedEvent, this, SLOT(onSubjectHierarchyItemModified(vtkObject*, void*)));
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
  }

  // If no node is selected then select the first displayed node to save the user a click
  if (!d->TransformNodeSelector->currentNode())
  {
    vtkMRMLNode* node = d->TransformNodeSelector->findFirstNodeByClass("vtkMRMLTransformNode");
    if (node)
    {
      d->TransformNodeSelector->setCurrentNode(node);
    }
  }

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::exit()
{
  Q_D(qSlicerTransformsModuleWidget);

  // Disconnect SH node modified when module is not active
  vtkMRMLSubjectHierarchyNode* shNode = d->TransformNodeSelector->subjectHierarchyNode();
  if (shNode)
  {
    qvtkDisconnect(shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemDisplayModifiedEvent, this, SLOT(onSubjectHierarchyItemModified(vtkObject*, void*)));
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
  }

  this->Superclass::exit();
}

//---------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onSubjectHierarchyItemModified(vtkObject* vtkNotUsed(caller), void* vtkNotUsed(callData))
{
  Q_D(qSlicerTransformsModuleWidget);
  // Refresh the display node section. This is necessary because it is possible that the display node did not exist yet
  // when the TransformDisplayNodeWidget was set in the widget.
  d->TransformDisplayNodeWidget->setMRMLTransformNode(d->MRMLTransformNode);
  d->TransformInteractionWidget->setMRMLTransformNode(d->MRMLTransformNode);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onTranslateFirstButtonPressed(bool checked)
{
  Q_D(qSlicerTransformsModuleWidget);

  qMRMLTransformSliders::CoordinateReferenceType ref = checked ? qMRMLTransformSliders::LOCAL : qMRMLTransformSliders::GLOBAL;
  d->TranslationSliders->setCoordinateReference(ref);
  d->RotationSliders->setCoordinateReference(ref);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::updateColorLegendFromMRML()
{
  Q_D(qSlicerTransformsModuleWidget);
  vtkMRMLTransformDisplayNode* displayNode = d->MRMLTransformNode ? vtkMRMLTransformDisplayNode::SafeDownCast(d->MRMLTransformNode->GetDisplayNode()) : nullptr;
  vtkMRMLColorLegendDisplayNode* colorLegendNode = displayNode ? vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode) : nullptr;
  if (!colorLegendNode && displayNode && !d->ColorLegendCollapsibleGroupBox->collapsed())
  {
    // The section is already open, so the legend has to be editable right away
    colorLegendNode = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(displayNode);
    if (colorLegendNode)
    {
      colorLegendNode->SetVisibility(false);
    }
  }
  d->ColorLegendDisplayNodeWidget->setMRMLColorLegendDisplayNode(colorLegendNode);
  // A legend only says something when the colors mean something: it is the color node and
  // the scalar range of the displacement magnitude that it shows.
  d->ColorLegendCollapsibleGroupBox->setEnabled(displayNode != nullptr                //
                                                && displayNode->GetScalarVisibility() //
                                                && displayNode->GetColorNode() != nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::colorLegendCollapsibleButtonCollapsed(bool opened)
{
  Q_D(qSlicerTransformsModuleWidget);
  if (!opened)
  {
    return;
  }
  vtkMRMLTransformDisplayNode* displayNode = d->MRMLTransformNode ? vtkMRMLTransformDisplayNode::SafeDownCast(d->MRMLTransformNode->GetDisplayNode()) : nullptr;
  if (!displayNode)
  {
    return;
  }
  vtkMRMLColorLegendDisplayNode* colorLegendNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode);
  if (!colorLegendNode)
  {
    // Create it the first time the section is opened, hidden until the user shows it
    colorLegendNode = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(displayNode);
    if (colorLegendNode)
    {
      colorLegendNode->SetVisibility(false);
    }
  }
  d->ColorLegendDisplayNodeWidget->setMRMLColorLegendDisplayNode(colorLegendNode);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onNodeSelected(vtkMRMLNode* node)
{
  Q_D(qSlicerTransformsModuleWidget);

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(node);

  this->qvtkReconnect(d->MRMLTransformNode, transformNode, vtkMRMLTransformableNode::TransformModifiedEvent, this, SLOT(onMRMLTransformNodeModified(vtkObject*)));
  // The source of a scattered transform: the model, the array, and the arrays of a new mesh
  this->qvtkReconnect(d->MRMLTransformNode, transformNode, vtkCommand::ModifiedEvent, this, SLOT(updateScatteredTransformWidgets()));
  this->qvtkReconnect(d->MRMLTransformNode, transformNode, vtkMRMLTransformableNode::TransformModifiedEvent, this, SLOT(updateScatteredTransformWidgets()));
  // The legend shows what the coloring means, so it follows the display node: turning the
  // coloring off or changing the color node changes whether a legend says anything.
  this->qvtkReconnect(d->MRMLTransformNode, transformNode, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, SLOT(updateColorLegendFromMRML()));

  if (d->MRMLTransformNode == nullptr && transformNode != nullptr)
  {
    d->TransformedCollapsibleButton->setCollapsed(false);
  }

  d->MRMLTransformNode = transformNode;

  bool isTransform = (transformNode != nullptr);
  bool isLinearTransform = (transformNode != nullptr && transformNode->IsLinear());
  bool isCompositeTransform = (transformNode != nullptr && transformNode->IsComposite());

  // Enable/Disable CoordinateReference, identity, split buttons, MatrixViewGroupBox, and
  // Min/Max translation inputs

  d->InvertPushButton->setEnabled(isTransform);
  d->TransformToolButton->setEnabled(isTransform);
  d->UntransformToolButton->setEnabled(isTransform);
  d->HardenToolButton->setEnabled(isTransform);
  d->TransformableTreeView->setEnabled(isTransform);
  d->TransformedTreeView->setEnabled(isTransform);
  // Export/import stays available without a transform: importing then creates one
  this->updateExportImportWidgets();
  this->updateScatteredTransformWidgets();

  d->TranslateFirstToolButton->setEnabled(isLinearTransform);
  d->IdentityPushButton->setEnabled(isLinearTransform);
  d->MatrixViewGroupBox->setEnabled(isLinearTransform);

  // Do not show/hide the large matrix view group box when clicking on a folder or scene node
  // to reduce the GUI layout jumping around.
  if (isLinearTransform || (isTransform && !isLinearTransform))
  {
    d->MatrixViewGroupBox->setVisible(isLinearTransform);
    d->TranslationSliders->setVisible(isLinearTransform);
    d->RotationSliders->setVisible(isLinearTransform);
    d->TranslateFirstToolButton->setVisible(isLinearTransform);
    d->CopyTransformToolButton->setVisible(isLinearTransform);
    d->PasteTransformToolButton->setVisible(isLinearTransform);
  }

  d->SplitPushButton->setVisible(isCompositeTransform);

  d->TranslationSliders->setMRMLTransformNode(transformNode);
  d->RotationSliders->setMRMLTransformNode(transformNode);
  d->MatrixWidget->setMRMLTransformNode(transformNode);
  d->TransformDisplayNodeWidget->setMRMLTransformNode(transformNode);
  d->TransformInteractionWidget->setMRMLTransformNode(transformNode);
  d->TransformInfoWidget->setMRMLTransformNode(transformNode);
  this->updateColorLegendFromMRML();

  QStringList nodeTypes;
  // If no transform node, it would show the entire scene, lets shown none
  // instead.
  if (transformNode == nullptr)
  {
    nodeTypes << QString("vtkMRMLNotANode");
  }
  d->TransformedTreeView->setNodeTypes(nodeTypes);

  // Filter the current node in the transformed tree view
  d->TransformedTreeView->setRootNode(transformNode);

  // Hide the current node in the transformable tree view
  QStringList hiddenNodeIDs;
  if (transformNode)
  {
    hiddenNodeIDs << QString(transformNode->GetID());
  }
  d->TransformableTreeView->sortFilterProxyModel()->setHiddenNodeIDs(hiddenNodeIDs);

  // If there is no display node then collapse the display section.
  // This allows creation of transform display nodes on request:
  // the display node is created if the user expands the display section.
  vtkMRMLTransformDisplayNode* dispNode = nullptr;
  if (transformNode)
  {
    dispNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
  }
  if (dispNode == nullptr)
  {
    d->DisplayCollapsibleButton->setCollapsed(true);
  }

  this->updateCenterOfTransformationWidgets();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onMRMLTransformNodeModified(vtkObject* caller)
{
  Q_D(qSlicerTransformsModuleWidget);

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(caller);
  if (!transformNode)
  {
    return;
  }
  Q_ASSERT(d->MRMLTransformNode == transformNode);

  bool isLinearTransform = transformNode->IsLinear();
  bool isCompositeTransform = transformNode->IsComposite();

  d->TranslateFirstToolButton->setEnabled(isLinearTransform);
  d->IdentityPushButton->setEnabled(isLinearTransform);
  d->MatrixViewGroupBox->setEnabled(isLinearTransform);

  // This method may be called very frequently (when transform is changing
  // in real time). Due to some reason setVisible calls take time,
  // even if the visibility state does not change.
  // To save time, only call the set function if the visibility has to be changed.
  if (isLinearTransform != d->TranslateFirstToolButton->isVisible())
  {
    d->TranslateFirstToolButton->setVisible(isLinearTransform);
  }
  if (isLinearTransform != d->MatrixViewGroupBox->isVisible())
  {
    d->MatrixViewGroupBox->setVisible(isLinearTransform);
  }
  if (isLinearTransform != d->TranslationSliders->isVisible())
  {
    d->TranslationSliders->setVisible(isLinearTransform);
  }
  if (isLinearTransform != d->RotationSliders->isVisible())
  {
    d->RotationSliders->setVisible(isLinearTransform);
  }
  if (isLinearTransform != d->CopyTransformToolButton->isVisible())
  {
    d->CopyTransformToolButton->setVisible(isLinearTransform);
  }
  if (isLinearTransform != d->PasteTransformToolButton->isVisible())
  {
    d->PasteTransformToolButton->setVisible(isLinearTransform);
  }
  if (isCompositeTransform != d->SplitPushButton->isVisible())
  {
    d->SplitPushButton->setVisible(isCompositeTransform);
  }

  this->updateCenterOfTransformationWidgets();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::updateCenterOfTransformationWidgets()
{
  Q_D(qSlicerTransformsModuleWidget);

  bool isLinearTransform = d->MRMLTransformNode && d->MRMLTransformNode->IsLinear();
  d->CenterOfTransformationGroupBox->setEnabled(isLinearTransform);
  if (isLinearTransform != d->CenterOfTransformationGroupBox->isVisible())
  {
    d->CenterOfTransformationGroupBox->setVisible(isLinearTransform);
  }
  if (!isLinearTransform)
  {
    return;
  }

  double centerOfTransformation[3] = { 0.0, 0.0, 0.0 };
  d->MRMLTransformNode->GetCenterOfTransformation(centerOfTransformation);
  if (d->CenterOfTransformationCoordinatesComboBox->currentIndex() == COORDINATE_COMBOBOX_INDEX_WORLD)
  {
    vtkNew<vtkGeneralTransform> transform;
    d->MRMLTransformNode->GetTransformToWorld(transform);
    transform->TransformPoint(centerOfTransformation, centerOfTransformation);
  }

  bool wasBlocked = d->CenterOfTransformationCoordinatesWidget->blockSignals(true);
  d->CenterOfTransformationCoordinatesWidget->setCoordinates(centerOfTransformation);
  d->CenterOfTransformationCoordinatesWidget->blockSignals(wasBlocked);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::identity()
{
  Q_D(qSlicerTransformsModuleWidget);

  if (d->MRMLTransformNode == nullptr || !d->MRMLTransformNode->IsLinear())
  {
    return;
  }

  d->TranslationSliders->resetUnactiveSliders();
  d->RotationSliders->resetUnactiveSliders();

  vtkNew<vtkMatrix4x4> matrix; // initialized to identity by default
  d->MRMLTransformNode->SetMatrixTransformToParent(matrix.GetPointer());
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::invert()
{
  Q_D(qSlicerTransformsModuleWidget);

  if (!d->MRMLTransformNode)
  {
    return;
  }

  d->TranslationSliders->resetUnactiveSliders();
  d->RotationSliders->resetUnactiveSliders();

  MRMLNodeModifyBlocker blocker(d->MRMLTransformNode);
  d->MRMLTransformNode->Inverse();
  d->MRMLTransformNode->InverseName();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::split()
{
  Q_D(qSlicerTransformsModuleWidget);

  if (d->MRMLTransformNode == nullptr)
  {
    return;
  }

  d->MRMLTransformNode->Split();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::copyTransform()
{
  Q_D(qSlicerTransformsModuleWidget);

  vtkLinearTransform* linearTransform = vtkLinearTransform::SafeDownCast(d->MRMLTransformNode->GetTransformToParent());
  if (!linearTransform)
  {
    // Silent fail, no worries!
    qWarning() << "Unable to cast parent transform as a vtkLinearTransform";
    return;
  }

  vtkMatrix4x4* internalMatrix = linearTransform->GetMatrix();
  std::string delimiter = " ";
  std::string rowDelimiter = "\n";
  std::string output = vtkAddonMathUtilities::ToString(internalMatrix, delimiter, rowDelimiter);
  QApplication::clipboard()->setText(QString::fromStdString(output));
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::pasteTransform()
{
  Q_D(qSlicerTransformsModuleWidget);

  vtkNew<vtkMatrix4x4> tempMatrix;

  std::string text = QApplication::clipboard()->text().toStdString();
  bool success = vtkAddonMathUtilities::FromString(tempMatrix.GetPointer(), text);
  if (!success)
  {
    qWarning() << "Cannot convert pasted string to matrix.";
    return;
  }
  // Homogeneous transformation matrix is expected to have (0,0,0,1)
  // in its last row.
  tempMatrix->SetElement(3, 0, 0.0);
  tempMatrix->SetElement(3, 1, 0.0);
  tempMatrix->SetElement(3, 2, 0.0);
  tempMatrix->SetElement(3, 3, 1.0);
  d->MRMLTransformNode->SetMatrixTransformToParent(tempMatrix.GetPointer());
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onCenterOfTransformationChanged()
{
  Q_D(const qSlicerTransformsModuleWidget);

  if (d->MRMLTransformNode == nullptr)
  {
    return;
  }

  bool isLinearTransform = d->MRMLTransformNode && d->MRMLTransformNode->IsLinear();
  if (!isLinearTransform)
  {
    return;
  }

  const double* coordinates = d->CenterOfTransformationCoordinatesWidget->coordinates();
  if (d->CenterOfTransformationCoordinatesComboBox->currentIndex() == COORDINATE_COMBOBOX_INDEX_LOCAL)
  {
    d->MRMLTransformNode->SetCenterOfTransformation(coordinates);
  }
  else
  {
    // World coordinates
    vtkNew<vtkGeneralTransform> transform;
    d->MRMLTransformNode->GetTransformFromWorld(transform);
    double coordinates_Local[3] = { 0.0, 0.0, 0.0 };
    transform->TransformPoint(coordinates, coordinates_Local);
    d->MRMLTransformNode->SetCenterOfTransformation(coordinates_Local);
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::resetCenterOfTransformation()
{
  Q_D(qSlicerTransformsModuleWidget);

  if (!d->MRMLTransformNode)
  {
    return;
  }

  d->MRMLTransformNode->SetCenterOfTransformation(0.0, 0.0, 0.0);
}

//-----------------------------------------------------------------------------
int qSlicerTransformsModuleWidget::coordinateReference() const
{
  Q_D(const qSlicerTransformsModuleWidget);
  return (d->TranslateFirstToolButton->isChecked() ? qMRMLTransformSliders::LOCAL : qMRMLTransformSliders::GLOBAL);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerTransformsModuleWidget);
  this->Superclass::setMRMLScene(scene);
  // If the root index is set before the scene, it will show the scene as
  // top-level item. Setting the root index to be the scene makes the nodes
  // top-level, and this can only be done once the scene is set.
  d->TransformableTreeView->setRootIndex(d->TransformableTreeView->sortFilterProxyModel()->mrmlSceneIndex());
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::transformSelectedNodes()
{
  Q_D(qSlicerTransformsModuleWidget);
  QList<vtkSmartPointer<vtkMRMLTransformableNode>> nodesToTransform = qSlicerTransformsModuleWidgetPrivate::getSelectedNodes(d->TransformableTreeView);
  for (const vtkSmartPointer<vtkMRMLTransformableNode>& node : nodesToTransform)
  {
    node->SetAndObserveTransformNodeID(d->MRMLTransformNode->GetID());
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::untransformSelectedNodes()
{
  Q_D(qSlicerTransformsModuleWidget);
  QList<vtkSmartPointer<vtkMRMLTransformableNode>> nodesToTransform = qSlicerTransformsModuleWidgetPrivate::getSelectedNodes(d->TransformedTreeView);
  for (const vtkSmartPointer<vtkMRMLTransformableNode>& node : nodesToTransform)
  {
    node->SetAndObserveTransformNodeID(nullptr);
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::hardenSelectedNodes()
{
  Q_D(qSlicerTransformsModuleWidget);
  QList<vtkSmartPointer<vtkMRMLTransformableNode>> nodesToTransform = qSlicerTransformsModuleWidgetPrivate::getSelectedNodes(d->TransformedTreeView);
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  for (const vtkSmartPointer<vtkMRMLTransformableNode>& node : nodesToTransform)
  {
    d->logic()->hardenTransform(vtkMRMLTransformableNode::SafeDownCast(node));
  }
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onDisplaySectionClicked(bool clicked)
{
  Q_D(qSlicerTransformsModuleWidget);
  // If the display section is opened and there is no display node then create one
  if (!clicked)
  {
    return;
  }
  if (d->MRMLTransformNode == nullptr)
  {
    return;
  }
  if (vtkMRMLTransformDisplayNode::SafeDownCast(d->MRMLTransformNode->GetDisplayNode()) == nullptr)
  {
    d->MRMLTransformNode->CreateDefaultDisplayNodes();
    // Refresh the display node section
    d->TransformDisplayNodeWidget->setMRMLTransformNode(d->MRMLTransformNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onTransformableSectionClicked(bool clicked)
{
  Q_D(qSlicerTransformsModuleWidget);
  if (clicked)
  {
    // the transformable section is open, so no need for spacer
    d->BottomSpacer->changeSize(0, 0, QSizePolicy::Fixed, QSizePolicy::Fixed);
  }
  else
  {
    // the transformable section is open, add spacer to prevent stretching of
    // the remaining sections
    d->BottomSpacer->changeSize(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding);
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::updateExportImportWidgets()
{
  Q_D(qSlicerTransformsModuleWidget);
  bool exportOperation = d->ExportRadioButton->isChecked();

  bool importModel = d->ImportModelRadioButton->isChecked();

  for (QWidget* exportWidget : { static_cast<QWidget*>(d->ExportOutputTypeLabel),
                                 static_cast<QWidget*>(d->ExportOutputTypeWidget),
                                 static_cast<QWidget*>(d->ExportReferenceVolumeLabel),
                                 static_cast<QWidget*>(d->ExportReferenceVolumeNodeComboBox),
                                 static_cast<QWidget*>(d->ExportOutputNodeLabel),
                                 static_cast<QWidget*>(d->ExportOutputNodeComboBox) })
  {
    exportWidget->setVisible(exportOperation);
  }
  for (QWidget* importWidget : { static_cast<QWidget*>(d->ImportInputTypeLabel),
                                 static_cast<QWidget*>(d->ImportInputTypeWidget),
                                 static_cast<QWidget*>(d->ImportInputNodeLabel),
                                 static_cast<QWidget*>(d->ImportInputNodeComboBox) })
  {
    importWidget->setVisible(!exportOperation);
  }
  // A vector volume holds a displacement in each voxel, so there is no array to choose, but the
  // row stays so that the form does not change shape when the input type changes
  d->ImportDisplacementArrayLabel->setVisible(!exportOperation);
  d->ImportDisplacementArrayComboBox->setVisible(!exportOperation);
  d->ImportDisplacementArrayLabel->setEnabled(importModel);
  d->ImportDisplacementArrayComboBox->setEnabled(importModel);

  // The output node is of the type that is exported. Setting the types clears the selection,
  // so they are only set when they change.
  QString outputNodeType = QString("vtkMRMLTransformNode");
  QString outputBaseName = tr("Displacement Field");
  if (d->ExportMagnitudeVolumeRadioButton->isChecked())
  {
    outputNodeType = QString("vtkMRMLScalarVolumeNode");
    outputBaseName = tr("Displacement Magnitude");
  }
  else if (d->ExportVectorVolumeRadioButton->isChecked())
  {
    outputNodeType = QString("vtkMRMLVectorVolumeNode");
  }
  if (d->ExportOutputNodeComboBox->nodeTypes() != QStringList(outputNodeType))
  {
    d->ExportOutputNodeComboBox->setNodeTypes(QStringList(outputNodeType));
  }
  d->ExportOutputNodeComboBox->setBaseName(outputBaseName);
  QString inputNodeType = (importModel ? QString("vtkMRMLModelNode") : QString("vtkMRMLVectorVolumeNode"));
  if (d->ImportInputNodeComboBox->nodeTypes() != QStringList(inputNodeType))
  {
    d->ImportInputNodeComboBox->setNodeTypes(QStringList(inputNodeType));
  }

  // The arrays that can hold displacements
  if (importModel)
  {
    vtkMRMLModelNode* inputModelNode = vtkMRMLModelNode::SafeDownCast(d->ImportInputNodeComboBox->currentNode());
    QString selectedArrayName = d->ImportDisplacementArrayComboBox->currentText();
    if (selectedArrayName == VoxelsArrayText())
    {
      selectedArrayName.clear();
    }
    SetComboBoxItems(d->ImportDisplacementArrayComboBox, GetDisplacementArrayNames(inputModelNode), selectedArrayName);
  }
  else
  {
    SetComboBoxItems(d->ImportDisplacementArrayComboBox, QStringList(VoxelsArrayText()), VoxelsArrayText());
  }

  bool canApply = false;
  if (exportOperation)
  {
    // There is nothing to export without a transform. The output node is created if none is
    // selected.
    canApply = (d->MRMLTransformNode != nullptr && d->ExportReferenceVolumeNodeComboBox->currentNode() != nullptr);
  }
  else
  {
    // The transform is created if none is selected
    canApply = (d->ImportInputNodeComboBox->currentNode() != nullptr && (!importModel || d->ImportDisplacementArrayComboBox->count() > 0));
  }
  d->ExportImportPushButton->setEnabled(canApply);
  d->ExportImportPushButton->setToolTip(!exportOperation && !d->MRMLTransformNode ? tr("Import into a new transform") : QString());

  // Rows that are hidden now leave no gap
  d->compactExportImportRows();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::updateScatteredTransformWidgets()
{
  Q_D(qSlicerTransformsModuleWidget);
  vtkAbstractTransform* scatteredTransform = (d->MRMLTransformNode ? d->MRMLTransformNode->GetScatteredTransform() : nullptr);
  d->ScatteredTransformSourceWidget->setVisible(scatteredTransform != nullptr);
  if (!scatteredTransform)
  {
    return;
  }
  vtkScatteredBSplineTransform* bsplineTransform = vtkScatteredBSplineTransform::SafeDownCast(scatteredTransform);
  vtkMeshDisplacementTransform* meshTransform = vtkMeshDisplacementTransform::SafeDownCast(scatteredTransform);
  vtkMRMLModelNode* modelNode = d->MRMLTransformNode->GetDisplacementModelNode();
  bool wasBlocked = d->ScatteredSourceModelNodeComboBox->blockSignals(true);
  d->ScatteredSourceModelNodeComboBox->setCurrentNode(modelNode);
  d->ScatteredSourceModelNodeComboBox->blockSignals(wasBlocked);
  // No name means the active vectors, which come first
  QStringList arrayNames = GetDisplacementArrayNames(modelNode);
  const char* arrayName = (bsplineTransform ? bsplineTransform->GetDisplacementArrayName() : meshTransform->GetDisplacementArrayName());
  QString selectedArrayName = (arrayName && arrayName[0] ? QString::fromUtf8(arrayName) : (arrayNames.isEmpty() ? QString() : arrayNames.first()));
  SetComboBoxItems(d->ScatteredDisplacementArrayComboBox, arrayNames, selectedArrayName);
  d->ScatteredDisplacementArrayComboBox->setEnabled(modelNode != nullptr);

  // How it is fitted
  QList<QWidget*> fittingWidgets = { d->ScatteredInterpolationComboBox,          d->ScatteredDisplacementScaleSliderWidget,
                                     d->ScatteredDisplacementScaleReverseButton, d->ScatteredExtrapolationDistanceSpinBox,
                                     d->ScatteredControlPointSpacingWidget,      d->ScatteredMinimumControlPointSpacingSpinBox,
                                     d->ScatteredMaximumNumberOfLevelsSpinBox,   d->ScatteredToleranceSpinBox,
                                     d->ScatteredUseLinearApproximationCheckBox, d->ScatteredBorderModeComboBox };
  QList<bool> wereBlocked;
  for (QWidget* fittingWidget : fittingWidgets)
  {
    wereBlocked << fittingWidget->blockSignals(true);
  }
  d->ScatteredInterpolationComboBox->setCurrentIndex(d->ScatteredInterpolationComboBox->findData(bsplineTransform ? InterpolationBSpline : InterpolationMesh));
  // The fitting parameters only apply to a fit
  d->ScatteredFittingGroupBox->setEnabled(bsplineTransform != nullptr);
  // The slider shows how much the displacements are scaled, the reverse button which way
  double displacementScale = (bsplineTransform ? bsplineTransform->GetDisplacementScale() : meshTransform->GetDisplacementScale());
  SetSliderRangeToFit(d->ScatteredDisplacementScaleSliderWidget, std::abs(displacementScale));
  d->ScatteredDisplacementScaleSliderWidget->setValue(std::abs(displacementScale));
  d->ScatteredDisplacementScaleReverseButton->setChecked(displacementScale < 0.0);
  if (bsplineTransform)
  {
    d->ScatteredExtrapolationDistanceSpinBox->setValue(bsplineTransform->GetGridBoundsMargin());
    d->ScatteredControlPointSpacingWidget->setCoordinates(bsplineTransform->GetControlPointSpacing());
    d->ScatteredMinimumControlPointSpacingSpinBox->setValue(bsplineTransform->GetMinimumControlPointSpacing());
    d->ScatteredMaximumNumberOfLevelsSpinBox->setValue(bsplineTransform->GetMaximumNumberOfLevels());
    d->ScatteredToleranceSpinBox->setValue(bsplineTransform->GetTolerance());
    d->ScatteredUseLinearApproximationCheckBox->setChecked(bsplineTransform->GetUseLinearApproximation());
    d->ScatteredBorderModeComboBox->setCurrentIndex(d->ScatteredBorderModeComboBox->findData(bsplineTransform->GetBorderMode()));
  }
  for (int widgetIndex = 0; widgetIndex < fittingWidgets.size(); ++widgetIndex)
  {
    fittingWidgets[widgetIndex]->blockSignals(wereBlocked[widgetIndex]);
  }

  // How closely it follows the displacements
  vtkPointSet* input = (bsplineTransform ? bsplineTransform->GetInputPointSet() : meshTransform->GetInputPointSet());
  QString fitResult;
  if (!input)
  {
    fitResult = tr("Nothing to fit: select a model and an array of displacement vectors");
  }
  else if (!bsplineTransform)
  {
    fitResult = tr("not fitted: exact at the points, interpolated in the cells of the mesh");
  }
  else
  {
    bsplineTransform->Update();
    fitResult = tr("largest error %1 mm, %2 levels").arg(bsplineTransform->GetResidual(), 0, 'g', 3).arg(bsplineTransform->GetNumberOfLevels());
  }
  // The displacements are those of the inverse, for example after the transform was inverted
  if (d->MRMLTransformNode->IsScatteredTransformFromParent())
  {
    fitResult += QString("\n") + tr("Computed from inverse: these displacements move positions from the parent to this transform");
  }
  d->ScatteredFitResultValueLabel->setText(fitResult);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onScatteredFittingParameterChanged()
{
  Q_D(qSlicerTransformsModuleWidget);
  vtkAbstractTransform* scatteredTransform = (d->MRMLTransformNode ? d->MRMLTransformNode->GetScatteredTransform() : nullptr);
  if (!scatteredTransform)
  {
    return;
  }
  vtkScatteredBSplineTransform* bsplineTransform = vtkScatteredBSplineTransform::SafeDownCast(scatteredTransform);
  vtkMeshDisplacementTransform* meshTransform = vtkMeshDisplacementTransform::SafeDownCast(scatteredTransform);
  double displacementScale = d->ScatteredDisplacementScaleSliderWidget->value();
  if (d->ScatteredDisplacementScaleReverseButton->isChecked())
  {
    displacementScale = -displacementScale;
  }

  // Another way of interpolating is another transform, with the same displacements
  bool fitWanted = (d->ScatteredInterpolationComboBox->currentData().toInt() == InterpolationBSpline);
  if (fitWanted != (bsplineTransform != nullptr))
  {
    std::string arrayName;
    const char* currentArrayName = (bsplineTransform ? bsplineTransform->GetDisplacementArrayName() : meshTransform->GetDisplacementArrayName());
    if (currentArrayName)
    {
      arrayName = currentArrayName;
    }
    vtkMRMLModelNode* modelNode = d->MRMLTransformNode->GetDisplacementModelNode();
    vtkSmartPointer<vtkAbstractTransform> newTransform;
    if (fitWanted)
    {
      vtkNew<vtkScatteredBSplineTransform> newBSplineTransform;
      vtkSlicerTransformLogic::SetScatteredTransformDefaults(newBSplineTransform, modelNode ? modelNode->GetMesh() : nullptr, arrayName.c_str());
      newBSplineTransform->SetDisplacementScale(displacementScale);
      newTransform = newBSplineTransform.GetPointer();
    }
    else
    {
      vtkNew<vtkMeshDisplacementTransform> newMeshTransform;
      newMeshTransform->SetDisplacementArrayName(arrayName.empty() ? nullptr : arrayName.c_str());
      newMeshTransform->SetDisplacementScale(displacementScale);
      newTransform = newMeshTransform.GetPointer();
    }
    // In the same direction as the transform that it replaces
    if (d->MRMLTransformNode->IsScatteredTransformFromParent())
    {
      d->MRMLTransformNode->SetAndObserveTransformFromParent(newTransform);
    }
    else
    {
      d->MRMLTransformNode->SetAndObserveTransformToParent(newTransform);
    }
    return;
  }

  if (meshTransform)
  {
    meshTransform->SetDisplacementScale(displacementScale);
    return;
  }
  bsplineTransform->SetDisplacementScale(displacementScale);
  bsplineTransform->SetGridBoundsMargin(d->ScatteredExtrapolationDistanceSpinBox->value());
  bsplineTransform->SetControlPointSpacing(d->ScatteredControlPointSpacingWidget->coordinates());
  bsplineTransform->SetMinimumControlPointSpacing(d->ScatteredMinimumControlPointSpacingSpinBox->value());
  bsplineTransform->SetMaximumNumberOfLevels(d->ScatteredMaximumNumberOfLevelsSpinBox->value());
  bsplineTransform->SetTolerance(d->ScatteredToleranceSpinBox->value());
  bsplineTransform->SetUseLinearApproximation(d->ScatteredUseLinearApproximationCheckBox->isChecked());
  QVariant borderMode = d->ScatteredBorderModeComboBox->currentData();
  if (borderMode.isValid())
  {
    bsplineTransform->SetBorderMode(borderMode.toInt());
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onScatteredSourceModelChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerTransformsModuleWidget);
  vtkAbstractTransform* scatteredTransform = (d->MRMLTransformNode ? d->MRMLTransformNode->GetScatteredTransform() : nullptr);
  if (!scatteredTransform)
  {
    return;
  }
  vtkScatteredBSplineTransform* bsplineTransform = vtkScatteredBSplineTransform::SafeDownCast(scatteredTransform);
  vtkMeshDisplacementTransform* meshTransform = vtkMeshDisplacementTransform::SafeDownCast(scatteredTransform);
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(node);
  MRMLNodeModifyBlocker blocker(d->MRMLTransformNode);
  // A model that does not have the array that was chosen is fitted to the first one it has
  QStringList arrayNames = GetDisplacementArrayNames(modelNode);
  const char* arrayName = (bsplineTransform ? bsplineTransform->GetDisplacementArrayName() : meshTransform->GetDisplacementArrayName());
  if (arrayName && arrayName[0] && !arrayNames.contains(QString::fromUtf8(arrayName)) && !arrayNames.isEmpty())
  {
    QByteArray firstArrayName = arrayNames.first().toUtf8();
    if (bsplineTransform)
    {
      bsplineTransform->SetDisplacementArrayName(firstArrayName.constData());
    }
    else
    {
      meshTransform->SetDisplacementArrayName(firstArrayName.constData());
    }
  }
  d->MRMLTransformNode->SetAndObserveDisplacementModelNode(modelNode);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onScatteredDisplacementArrayChanged(int index)
{
  Q_D(qSlicerTransformsModuleWidget);
  vtkAbstractTransform* scatteredTransform = (d->MRMLTransformNode ? d->MRMLTransformNode->GetScatteredTransform() : nullptr);
  if (!scatteredTransform || index < 0)
  {
    return;
  }
  QByteArray arrayName = d->ScatteredDisplacementArrayComboBox->itemText(index).toUtf8();
  if (vtkScatteredBSplineTransform* bsplineTransform = vtkScatteredBSplineTransform::SafeDownCast(scatteredTransform))
  {
    bsplineTransform->SetDisplacementArrayName(arrayName.constData());
  }
  else if (vtkMeshDisplacementTransform* meshTransform = vtkMeshDisplacementTransform::SafeDownCast(scatteredTransform))
  {
    meshTransform->SetDisplacementArrayName(arrayName.constData());
  }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::applyExportImport()
{
  Q_D(qSlicerTransformsModuleWidget);
  bool exportOperation = d->ExportRadioButton->isChecked();
  if (d->MRMLTransformNode == nullptr && exportOperation)
  {
    qWarning("qSlicerTransformsModuleWidget::applyExportImport failed: MRMLTransformNode is invalid");
    return;
  }
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  if (exportOperation)
  {
    vtkMRMLVolumeNode* referenceVolumeNode = vtkMRMLVolumeNode::SafeDownCast(d->ExportReferenceVolumeNodeComboBox->currentNode());
    vtkMRMLNode* outputNode = d->ExportOutputNodeComboBox->currentNode();
    vtkMRMLNode* resultNode = nullptr;
    if (referenceVolumeNode == nullptr)
    {
      qWarning("qSlicerTransformsModuleWidget::applyExportImport failed: reference volume node is invalid");
    }
    else if (d->ExportVectorVolumeRadioButton->isChecked())
    {
      resultNode =
        d->logic()->CreateDisplacementVolumeFromTransform(d->MRMLTransformNode, referenceVolumeNode, false /*magnitudeOnly*/, vtkMRMLVectorVolumeNode::SafeDownCast(outputNode));
    }
    else if (d->ExportMagnitudeVolumeRadioButton->isChecked())
    {
      resultNode =
        d->logic()->CreateDisplacementVolumeFromTransform(d->MRMLTransformNode, referenceVolumeNode, true /*magnitudeOnly*/, vtkMRMLScalarVolumeNode::SafeDownCast(outputNode));
    }
    else
    {
      resultNode = d->logic()->ConvertToGridTransform(d->MRMLTransformNode, referenceVolumeNode, vtkMRMLTransformNode::SafeDownCast(outputNode));
    }
    QApplication::restoreOverrideCursor();
    if (resultNode == nullptr)
    {
      QMessageBox::warning(this, tr("Export failed"), tr("Failed to export transform. See application log for details."));
      return;
    }
    // A node that was created is shown as the output, so that applying again updates it
    d->ExportOutputNodeComboBox->setCurrentNode(resultNode);
    return;
  }

  // Without a transform, the import goes into a new one
  vtkMRMLNode* inputNode = d->ImportInputNodeComboBox->currentNode();
  vtkSmartPointer<vtkMRMLTransformNode> transformNode = d->MRMLTransformNode;
  bool createdTransformNode = false;
  if (!transformNode && inputNode && this->mrmlScene())
  {
    std::string name = std::string(inputNode->GetName() ? inputNode->GetName() : "") + " transform";
    transformNode = vtkMRMLTransformNode::SafeDownCast(this->mrmlScene()->AddNewNodeByClass("vtkMRMLTransformNode", this->mrmlScene()->GenerateUniqueName(name)));
    createdTransformNode = (transformNode != nullptr);
  }
  bool success = false;
  if (transformNode)
  {
    if (d->ImportModelRadioButton->isChecked())
    {
      QByteArray arrayName = d->ImportDisplacementArrayComboBox->currentText().toUtf8();
      success = d->logic()->ImportTransformFromModel(transformNode, vtkMRMLModelNode::SafeDownCast(inputNode), arrayName.isEmpty() ? nullptr : arrayName.constData());
    }
    else
    {
      success = d->logic()->ImportTransformFromDisplacementVolume(transformNode, vtkMRMLVectorVolumeNode::SafeDownCast(inputNode));
    }
  }
  if (createdTransformNode)
  {
    if (success)
    {
      // The new transform is shown, ready to be edited or applied
      this->setEditedNode(transformNode);
    }
    else
    {
      this->mrmlScene()->RemoveNode(transformNode);
    }
  }
  QApplication::restoreOverrideCursor();
  if (!success)
  {
    QMessageBox::warning(this, tr("Import failed"), tr("Failed to import transform. See application log for details."));
  }
}

//-----------------------------------------------------------
bool qSlicerTransformsModuleWidget::setEditedNode(vtkMRMLNode* node, QString role /* = QString()*/, QString context /* = QString()*/)
{
  Q_D(qSlicerTransformsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkMRMLTransformNode::SafeDownCast(node))
  {
    d->TransformNodeSelector->setCurrentNode(node);
    return true;
  }

  if (vtkMRMLTransformDisplayNode::SafeDownCast(node))
  {
    vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(node);
    vtkMRMLTransformNode* displayableNode = vtkMRMLTransformNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
    {
      return false;
    }
    d->TransformNodeSelector->setCurrentNode(displayableNode);
    return true;
  }

  return false;
}
