/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QMouseEvent>
#include <QSettings>
#include <QShortcut>
#include <QSignalMapper>
#include <QStringList>
#include <QTableWidgetItem>
#include <QTimer>

// CTK includes
#include "ctkMessageBox.h"

// Slicer includes
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"
#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLUtils.h"
#include "qSlicerApplication.h"

// MRML includes
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// MRMLDM includes
#include "vtkMRMLMarkupsDisplayableManager.h"

// Markups includes
#include "qSlicerMarkupsModule.h"
#include "qSlicerMarkupsModuleWidget.h"
#include "ui_qSlicerMarkupsModule.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsLineNode.h"
#include "vtkMRMLMarkupsAngleNode.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLMarkupsClosedCurveNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkSlicerMarkupsLogic.h"
#include "vtkSlicerDijkstraGraphGeodesicPath.h"

// VTK includes
#include <vtkMath.h>
#include <vtkNew.h>
#include "vtkPoints.h"
#include <math.h>

static const int JUMP_MODE_COMBOBOX_INDEX_OFFSET = 0;
static const int JUMP_MODE_COMBOBOX_INDEX_CENTERED = 1;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Markups
class qSlicerMarkupsModuleWidgetPrivate: public Ui_qSlicerMarkupsModule
{
  Q_DECLARE_PUBLIC(qSlicerMarkupsModuleWidget);
protected:
  qSlicerMarkupsModuleWidget* const q_ptr;

public:
  qSlicerMarkupsModuleWidgetPrivate(qSlicerMarkupsModuleWidget& object);
  ~qSlicerMarkupsModuleWidgetPrivate();

  void setupUi(qSlicerWidget* widget);

  /// the number of columns matches the column labels by using the size of the QStringList
  int numberOfColumns();
  /// return the column index for a given QString, -1 if not a valid header
  int columnIndex(QString label);

  vtkMRMLSelectionNode* selectionNode();

  vtkMRMLNode* selectionNodeActivePlaceNode();
  void setSelectionNodeActivePlaceNode(vtkMRMLNode* activePlaceNode);
  void setMRMLMarkupsNodeFromSelectionNode();

  void setPlaceModeEnabled(bool placeEnable);

  vtkMRMLMarkupsDisplayNode* markupsDisplayNode();

  static const char* getCurveTypeAsHumanReadableString(int curveType);
  static const char* getCostFunctionAsHumanReadableString(int costFunction);

private:
  vtkWeakPointer<vtkMRMLMarkupsNode> MarkupsNode;

  QStringList columnLabels;

  QAction*    newMarkupWithCurrentDisplayPropertiesAction;

  QMenu*      visibilityMenu;
  QAction*    visibilityOnAllMarkupsInListAction;
  QAction*    visibilityOffAllMarkupsInListAction;

  QMenu*      selectedMenu;
  QAction*    selectedOnAllMarkupsInListAction;
  QAction*    selectedOffAllMarkupsInListAction;

  QMenu*      lockMenu;
  QAction*    lockAllMarkupsInListAction;
  QAction*    unlockAllMarkupsInListAction;

  QAction*    cutAction;
  QAction*    copyAction;
  QAction*    pasteAction;

  QTimer*     editScalarFunctionDelay;
};

//-----------------------------------------------------------------------------
// qSlicerMarkupsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidgetPrivate::qSlicerMarkupsModuleWidgetPrivate(qSlicerMarkupsModuleWidget& object)
  : q_ptr(&object)
{
  this->columnLabels << "Selected" << "Locked" << "Visible" << "Name" << "Description" << "R" << "A" << "S";

  this->newMarkupWithCurrentDisplayPropertiesAction = nullptr;
  this->visibilityMenu = nullptr;
  this->visibilityOnAllMarkupsInListAction = nullptr;
  this->visibilityOffAllMarkupsInListAction = nullptr;

  this->selectedMenu = nullptr;
  this->selectedOnAllMarkupsInListAction = nullptr;
  this->selectedOffAllMarkupsInListAction = nullptr;

  this->lockMenu = nullptr;
  this->lockAllMarkupsInListAction = nullptr;
  this->unlockAllMarkupsInListAction = nullptr;

  this->cutAction = nullptr;
  this->copyAction = nullptr;
  this->pasteAction = nullptr;

  this->editScalarFunctionDelay = nullptr;
}

//-----------------------------------------------------------------------------
int qSlicerMarkupsModuleWidgetPrivate::columnIndex(QString label)
{
  return this->columnLabels.indexOf(label);
}

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidgetPrivate::~qSlicerMarkupsModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerMarkupsModuleWidget);
  this->Ui_qSlicerMarkupsModule::setupUi(widget);

  this->activeMarkupTreeView->setNodeTypes(QStringList(QString("vtkMRMLMarkupsNode")));
  this->activeMarkupTreeView->setColumnHidden(this->activeMarkupTreeView->model()->idColumn(), true);
  this->activeMarkupTreeView->setColumnHidden(this->activeMarkupTreeView->model()->transformColumn(), true);
  this->activeMarkupTreeView->setColumnHidden(this->activeMarkupTreeView->model()->descriptionColumn(), false);

  // set up the list buttons
  // visibility
  // first add actions to the menu, then hook them up
  visibilityMenu = new QMenu(qSlicerMarkupsModuleWidget::tr("Visibility"), this->visibilityAllMarkupsInListMenuButton);
  // visibility on
  this->visibilityOnAllMarkupsInListAction =
    new QAction(QIcon(":/Icons/Small/SlicerVisible.png"), "Visibility On", visibilityMenu);
  this->visibilityOnAllMarkupsInListAction->setToolTip("Set visibility flag to on for all markups in active list");
  this->visibilityOnAllMarkupsInListAction->setCheckable(false);
  QObject::connect(this->visibilityOnAllMarkupsInListAction, SIGNAL(triggered()),
                   q, SLOT(onVisibilityOnAllMarkupsInListPushButtonClicked()));

  // visibility off
  this->visibilityOffAllMarkupsInListAction =
    new QAction(QIcon(":/Icons/Small/SlicerInvisible.png"), "Visibility Off", visibilityMenu);
  this->visibilityOffAllMarkupsInListAction->setToolTip("Set visibility flag to off for all markups in active list");
  this->visibilityOffAllMarkupsInListAction->setCheckable(false);
  QObject::connect(this->visibilityOffAllMarkupsInListAction, SIGNAL(triggered()),
                   q, SLOT(onVisibilityOffAllMarkupsInListPushButtonClicked()));

  this->visibilityMenu->addAction(this->visibilityOnAllMarkupsInListAction);
  this->visibilityMenu->addAction(this->visibilityOffAllMarkupsInListAction);
  this->visibilityAllMarkupsInListMenuButton->setMenu(this->visibilityMenu);
  this->visibilityAllMarkupsInListMenuButton->setIcon(QIcon(":/Icons/VisibleOrInvisible.png"));

  // visibility toggle
  QObject::connect(this->visibilityAllMarkupsInListMenuButton, SIGNAL(clicked()),
                   q, SLOT(onVisibilityAllMarkupsInListToggled()));

  // lock
  // first add actions to the menu, then hook them up
  lockMenu = new QMenu(qSlicerMarkupsModuleWidget::tr("Lock"), this->lockAllMarkupsInListMenuButton);
  // lock
  this->lockAllMarkupsInListAction =
    new QAction(QIcon(":/Icons/Small/SlicerLock.png"), "Lock", lockMenu);
  this->lockAllMarkupsInListAction->setToolTip("Set lock flag to on for all markups in active list");
  this->lockAllMarkupsInListAction->setCheckable(false);
  QObject::connect(this->lockAllMarkupsInListAction, SIGNAL(triggered()),
                   q, SLOT(onLockAllMarkupsInListPushButtonClicked()));

  // lock off
  this->unlockAllMarkupsInListAction =
    new QAction(QIcon(":/Icons/Small/SlicerUnlock.png"), "Unlock", lockMenu);
  this->unlockAllMarkupsInListAction->setToolTip("Set lock flag to off for all markups in active list");
  this->unlockAllMarkupsInListAction->setCheckable(false);
  QObject::connect(this->unlockAllMarkupsInListAction, SIGNAL(triggered()),
                   q, SLOT(onUnlockAllMarkupsInListPushButtonClicked()));

  this->lockMenu->addAction(this->lockAllMarkupsInListAction);
  this->lockMenu->addAction(this->unlockAllMarkupsInListAction);
  this->lockAllMarkupsInListMenuButton->setMenu(this->lockMenu);
  this->lockAllMarkupsInListMenuButton->setIcon(QIcon(":/Icons/Small/SlicerLockUnlock.png"));

  // lock toggle
  QObject::connect(this->lockAllMarkupsInListMenuButton, SIGNAL(clicked()),
                   q, SLOT(onLockAllMarkupsInListToggled()));

  // selected
  // first add actions to the menu, then hook them up
  selectedMenu = new QMenu(qSlicerMarkupsModuleWidget::tr("Selected"), this->selectedAllMarkupsInListMenuButton);
  // selected on
  this->selectedOnAllMarkupsInListAction =
    new QAction(QIcon(":/Icons/MarkupsSelected.png"), "Selected On", selectedMenu);
  this->selectedOnAllMarkupsInListAction->setToolTip("Set selected flag to on for all markups in active list");
  this->selectedOnAllMarkupsInListAction->setCheckable(false);
  QObject::connect(this->selectedOnAllMarkupsInListAction, SIGNAL(triggered()),
                   q, SLOT(onSelectAllMarkupsInListPushButtonClicked()));

  // selected off
  this->selectedOffAllMarkupsInListAction =
    new QAction(QIcon(":/Icons/MarkupsUnselected.png"), "Selected Off", selectedMenu);
  this->selectedOffAllMarkupsInListAction->setToolTip("Set selected flag to off for all markups in active list");
  this->selectedOffAllMarkupsInListAction->setCheckable(false);
  QObject::connect(this->selectedOffAllMarkupsInListAction, SIGNAL(triggered()),
                   q, SLOT(onDeselectAllMarkupsInListPushButtonClicked()));

  this->selectedMenu->addAction(this->selectedOnAllMarkupsInListAction);
  this->selectedMenu->addAction(this->selectedOffAllMarkupsInListAction);
  this->selectedAllMarkupsInListMenuButton->setMenu(this->selectedMenu);
  this->selectedAllMarkupsInListMenuButton->setIcon(QIcon(":/Icons/MarkupsSelectedOrUnselected.png"));

  // selected toggle
  QObject::connect(this->selectedAllMarkupsInListMenuButton, SIGNAL(clicked()),
                   q, SLOT(onSelectedAllMarkupsInListToggled()));

  // add
  QObject::connect(this->addMarkupPushButton, SIGNAL(clicked()),
                   q, SLOT(onAddMarkupPushButtonClicked()));
  // move
  QObject::connect(this->moveMarkupUpPushButton, SIGNAL(clicked()),
                   q, SLOT(onMoveMarkupUpPushButtonClicked()));
  QObject::connect(this->moveMarkupDownPushButton, SIGNAL(clicked()),
                   q, SLOT(onMoveMarkupDownPushButtonClicked()));
  // delete
  QObject::connect(this->deleteMarkupPushButton, SIGNAL(clicked()),
                   q, SLOT(onDeleteMarkupPushButtonClicked()));
  QObject::connect(this->deleteAllMarkupsInListPushButton, SIGNAL(clicked()),
                   q, SLOT(onDeleteAllMarkupsInListPushButtonClicked()));

  this->cutAction = new QAction(q);
  this->cutAction->setText(qSlicerMarkupsModuleWidget::tr("Cut"));
  this->cutAction->setIcon(QIcon(":Icons/Medium/SlicerEditCut.png"));
  this->cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->cutAction->setShortcuts(QKeySequence::Cut);
  this->cutAction->setToolTip(qSlicerMarkupsModuleWidget::tr("Cut"));
  q->addAction(this->cutAction);
  this->CutMarkupsToolButton->setDefaultAction(this->cutAction);
  QObject::connect(this->cutAction, SIGNAL(triggered()), q, SLOT(cutSelectedToClipboard()));

  this->copyAction = new QAction(q);
  this->copyAction->setText(qSlicerMarkupsModuleWidget::tr("Copy"));
  this->copyAction->setIcon(QIcon(":Icons/Medium/SlicerEditCopy.png"));
  this->copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->copyAction->setShortcuts(QKeySequence::Copy);
  this->copyAction->setToolTip(qSlicerMarkupsModuleWidget::tr("Copy"));
  q->addAction(this->copyAction);
  this->CopyMarkupsToolButton->setDefaultAction(this->copyAction);
  QObject::connect(this->copyAction, SIGNAL(triggered()), q, SLOT(copySelectedToClipboard()));

  this->pasteAction = new QAction(q);
  this->pasteAction->setText(qSlicerMarkupsModuleWidget::tr("Paste"));
  this->pasteAction->setIcon(QIcon(":Icons/Medium/SlicerEditPaste.png"));
  this->pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->pasteAction->setShortcuts(QKeySequence::Paste);
  this->pasteAction->setToolTip(qSlicerMarkupsModuleWidget::tr("Paste"));
  q->addAction(this->pasteAction);
  this->PasteMarkupsToolButton->setDefaultAction(this->pasteAction);
  QObject::connect(this->pasteAction, SIGNAL(triggered()), q, SLOT(pasteSelectedFromClipboard()));

  // set up the active markups node selector
  QObject::connect(this->activeMarkupTreeView, SIGNAL(currentItemChanged(vtkIdType)),
    q, SLOT(onActiveMarkupItemChanged(vtkIdType)));
  QObject::connect(this->createFiducialPushButton, SIGNAL(clicked()),
    q, SLOT(onCreateMarkupsFiducial()));
  QObject::connect(this->createLinePushButton, SIGNAL(clicked()),
    q, SLOT(onCreateMarkupsLine()));
  QObject::connect(this->createAnglePushButton, SIGNAL(clicked()),
    q, SLOT(onCreateMarkupsAngle()));
  QObject::connect(this->createOpenCurvePushButton, SIGNAL(clicked()),
    q, SLOT(onCreateMarkupsOpenCurve()));
  QObject::connect(this->createClosedCurvePushButton, SIGNAL(clicked()),
    q, SLOT(onCreateMarkupsClosedCurve()));
  QObject::connect(this->createPlanePushButton, SIGNAL(clicked()),
    q, SLOT(onCreateMarkupsPlane()));

  // Make sure that the Jump to Slices radio buttons match the default of the
  // MRML slice node
  vtkNew<vtkMRMLSliceNode> sliceNode;
  if (sliceNode->GetJumpMode() == vtkMRMLSliceNode::OffsetJumpSlice)
    {
    this->jumpModeComboBox->setCurrentIndex(JUMP_MODE_COMBOBOX_INDEX_OFFSET);
    }
  else if (sliceNode->GetJumpMode() == vtkMRMLSliceNode::CenteredJumpSlice)
    {
    this->jumpModeComboBox->setCurrentIndex(JUMP_MODE_COMBOBOX_INDEX_CENTERED);
    }
  // update the checked state of showing the slice intersections
  // vtkSlicerMarkupsLogic::GetSliceIntersectionsVisibility() cannot be called, as the scene
  // is not yet set, so just set to the default value (slice intersections not visible).
  this->sliceIntersectionsVisibilityCheckBox->setChecked(false);
  QObject::connect(this->sliceIntersectionsVisibilityCheckBox,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onSliceIntersectionsVisibilityToggled(bool)));

  //
  // set up the list visibility/locked buttons
  //
  QObject::connect(this->listLockedUnlockedPushButton, SIGNAL(clicked()),
                   q, SLOT(onListLockedUnlockedPushButtonClicked()));
  //
  // set up the name format line edit
  //
  QObject::connect(this->nameFormatLineEdit, SIGNAL(textEdited(const QString &)),
                   q, SLOT(onNameFormatLineEditTextEdited(const QString &)));
  //
  // set up the reset format button
  //
  QObject::connect(this->resetNameFormatToDefaultPushButton, SIGNAL(clicked()),
                   q, SLOT(onResetNameFormatToDefaultPushButtonClicked()));
  //
  // set up the rename all button
  //
  QObject::connect(this->renameAllWithCurrentNameFormatPushButton, SIGNAL(clicked()),
                   q, SLOT(onRenameAllWithCurrentNameFormatPushButtonClicked()));
  //
  // set up the convert annotations button
  //
  QObject::connect(this->convertAnnotationFiducialsPushButton, SIGNAL(clicked()),
                   q, SLOT(convertAnnotationFiducialsToMarkups()));

  //
  // set up the table
  //

  // only select rows rather than cells
  this->activeMarkupTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
  // allow multi select
  this->activeMarkupTableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // number of columns with headers
  this->activeMarkupTableWidget->setColumnCount(this->numberOfColumns());
  this->activeMarkupTableWidget->setHorizontalHeaderLabels(this->columnLabels);
  this->activeMarkupTableWidget->horizontalHeader()->setFixedHeight(32);

  // adjust the column widths
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("Name"), 60);
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("Description"), 120);
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("R"), 65);
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("A"), 65);
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("S"), 65);

  // show/hide the coordinate columns
  QObject::connect(this->hideCoordinateColumnsCheckBox,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onHideCoordinateColumnsToggled(bool)));
  // show transformed/untransformed coordinates
  QObject::connect(this->transformedCoordinatesCheckBox,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onTransformedCoordinatesToggled(bool)));

  // use an icon for some column headers
  // selected is a check box
  QTableWidgetItem *selectedHeader = this->activeMarkupTableWidget->horizontalHeaderItem(this->columnIndex("Selected"));
  selectedHeader->setText("");
  selectedHeader->setIcon(QIcon(":/Icons/MarkupsSelected.png"));
  selectedHeader->setToolTip(QString("Click in this column to select/deselect markups for passing to CLI modules"));
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("Selected"), 30);
  // locked is an open and closed lock
  QTableWidgetItem *lockedHeader = this->activeMarkupTableWidget->horizontalHeaderItem(this->columnIndex("Locked"));
  lockedHeader->setText("");
  lockedHeader->setIcon(QIcon(":/Icons/Small/SlicerLockUnlock.png"));
  lockedHeader->setToolTip(QString("Click in this column to lock/unlock markups to prevent them from being moved by mistake"));
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("Locked"), 30);
  // visible is an open and closed eye
  QTableWidgetItem *visibleHeader = this->activeMarkupTableWidget->horizontalHeaderItem(this->columnIndex("Visible"));
  visibleHeader->setText("");
  visibleHeader->setIcon(QIcon(":/Icons/Small/SlicerVisibleInvisible.png"));
  visibleHeader->setToolTip(QString("Click in this column to show/hide markups in 2D and 3D"));
  this->activeMarkupTableWidget->setColumnWidth(this->columnIndex("Visible"), 30);

  // listen for changes so can update mrml node
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(cellChanged(int, int)),
                   q, SLOT(onActiveMarkupTableCellChanged(int, int)));

  // listen for click on a markup
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)),
                   q, SLOT(onActiveMarkupTableCellClicked(QTableWidgetItem*)));
  // listen for the current cell changing (happens when arrows are used to navigate)
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(currentCellChanged(int, int, int, int)),
                   q, SLOT(onActiveMarkupTableCurrentCellChanged(int, int, int, int)));
  // listen for a right click
  this->activeMarkupTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(this->activeMarkupTableWidget, SIGNAL(customContextMenuRequested(QPoint)),
                   q, SLOT(onRightClickActiveMarkupTableWidget(QPoint)));

  // set up the display properties buttons
  QObject::connect(this->resetToDefaultDisplayPropertiesPushButton, SIGNAL(clicked()),
    q, SLOT(onResetToDefaultDisplayPropertiesPushButtonClicked()));
  QObject::connect(this->saveToDefaultDisplayPropertiesPushButton, SIGNAL(clicked()),
    q, SLOT(onSaveToDefaultDisplayPropertiesPushButtonClicked()));

  this->resampleCurveCollapsibleButton->setVisible(false);
  QObject::connect(this->resampleCurveButton, SIGNAL(clicked()),
    q, SLOT(onApplyCurveResamplingPushButtonClicked()));


  this->curveTypeComboBox->clear();
  for (int curveType = 0; curveType < vtkCurveGenerator::CURVE_TYPE_LAST; ++curveType)
    {
    this->curveTypeComboBox->addItem(qSlicerMarkupsModuleWidgetPrivate::getCurveTypeAsHumanReadableString(curveType), curveType);
    }

  this->costFunctionComboBox->clear();
  for (int costFunction = 0; costFunction < vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_LAST; ++costFunction)
    {
    this->costFunctionComboBox->addItem(qSlicerMarkupsModuleWidgetPrivate::getCostFunctionAsHumanReadableString(costFunction), costFunction);
    }

  this->editScalarFunctionDelay = new QTimer(q);
  this->editScalarFunctionDelay->setInterval(500);
  this->editScalarFunctionDelay->setSingleShot(true);
  QObject::connect(this->editScalarFunctionDelay, SIGNAL(timeout()),
    q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->curveTypeComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->modelNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->costFunctionComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->scalarFunctionLineEdit, SIGNAL(textChanged(QString)),
    this->editScalarFunctionDelay, SLOT(start()));
}

//-----------------------------------------------------------------------------
int qSlicerMarkupsModuleWidgetPrivate::numberOfColumns()
{
  return this->columnLabels.size();
}

//-----------------------------------------------------------------------------
vtkMRMLSelectionNode* qSlicerMarkupsModuleWidgetPrivate::selectionNode()
{
  Q_Q(qSlicerMarkupsModuleWidget);
  if (!q->mrmlScene() || !q->markupsLogic())
    {
    return nullptr;
    }
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
    q->mrmlScene()->GetNodeByID(q->markupsLogic()->GetSelectionNodeID().c_str()));
  return selectionNode;
}

//-----------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode* qSlicerMarkupsModuleWidgetPrivate::markupsDisplayNode()
{
  Q_Q(qSlicerMarkupsModuleWidget);
  if (!this->MarkupsNode)
    {
    return nullptr;
    }
  return vtkMRMLMarkupsDisplayNode::SafeDownCast(this->MarkupsNode->GetDisplayNode());
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerMarkupsModuleWidgetPrivate::selectionNodeActivePlaceNode()
{
  Q_Q(qSlicerMarkupsModuleWidget);
  vtkMRMLSelectionNode *selNode = this->selectionNode();
  if (!selNode)
    {
    return nullptr;
    }

  const char *selectionNodeActivePlaceNodeID = selNode->GetActivePlaceNodeID();
  if (!selectionNodeActivePlaceNodeID)
    {
    return nullptr;
    }
  vtkMRMLNode* activePlaceNode = q->mrmlScene()->GetNodeByID(selectionNodeActivePlaceNodeID);
  return activePlaceNode;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidgetPrivate::setSelectionNodeActivePlaceNode(vtkMRMLNode* activePlaceNode)
{
  Q_Q(qSlicerMarkupsModuleWidget);
  if (!q->markupsLogic())
    {
    return;
    }
  q->markupsLogic()->SetActiveListID(vtkMRMLMarkupsNode::SafeDownCast(activePlaceNode));
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidgetPrivate::setMRMLMarkupsNodeFromSelectionNode()
{
  Q_Q(qSlicerMarkupsModuleWidget);

  // Select current markups node
  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->selectionNodeActivePlaceNode());
  if (!currentMarkupsNode && q->mrmlScene() && this->activeMarkupTreeView->subjectHierarchyNode())
    {
    // Active place node is not a markups node then switch to the last markups node.
    vtkCollection* nodes = q->mrmlScene()->GetNodes();
    vtkMRMLSubjectHierarchyNode* shNode = this->activeMarkupTreeView->subjectHierarchyNode();
    for (int nodeIndex = nodes->GetNumberOfItems() - 1; nodeIndex >= 0; nodeIndex--)
    {
      vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(nodes->GetItemAsObject(nodeIndex));
      if (!markupsNode)
        {
        continue;
        }
      vtkIdType itemID = shNode->GetItemByDataNode(markupsNode);
      if (!itemID)
        {
        continue;
        }
      QModelIndex itemIndex = this->activeMarkupTreeView->sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
      if (!itemIndex.isValid())
        {
        // not visible in current view
        continue;
        }
      currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(markupsNode);
      }
    if (currentMarkupsNode)
      {
      this->setSelectionNodeActivePlaceNode(currentMarkupsNode);
      }
    }
  q->setMRMLMarkupsNode(currentMarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidgetPrivate::setPlaceModeEnabled(bool placeEnable)
{
  Q_Q(qSlicerMarkupsModuleWidget);
  vtkMRMLInteractionNode* interactionNode = nullptr;
  if (q->mrmlScene())
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(q->mrmlScene()->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
    }
  if (interactionNode == nullptr)
    {
    if (placeEnable)
      {
      qCritical() << Q_FUNC_INFO << " setPlaceModeEnabled failed: invalid interaction node";
      }
    return;
    }

  if (placeEnable)
    {
    interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
    }
  else
    {
    if (interactionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place)
      {
      interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
      }
    }
}

//-----------------------------------------------------------
const char* qSlicerMarkupsModuleWidgetPrivate::getCurveTypeAsHumanReadableString(int curveType)
{
  switch (curveType)
    {
    case vtkCurveGenerator::CURVE_TYPE_LINEAR_SPLINE:
      {
      return "Linear";
      }
    case vtkCurveGenerator::CURVE_TYPE_CARDINAL_SPLINE:
      {
      return "Spline";
      }
    case vtkCurveGenerator::CURVE_TYPE_KOCHANEK_SPLINE:
      {
      return "Kochanek spline";
      }
    case vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL:
      {
      return "Polynomial";
      }
    case vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE:
      {
      return "Shortest distance on surface";
      }
    default:
      {
      vtkGenericWarningMacro("Unknown curve type: " << curveType);
      return "Unknown";
      }
    }
}

//------------------------------------------------------------------------------
const char* qSlicerMarkupsModuleWidgetPrivate::getCostFunctionAsHumanReadableString(int costFunction)
{
  switch (costFunction)
    {
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE:
      {
      return "Distance";
      }
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_ADDITIVE:
      {
      return "Additive";
      }
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_MULTIPLICATIVE:
      {
      return "Multiplicative";
      }
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_INVERSE_SQUARED:
      {
      return "Inverse squared";
      }
    default:
      {
      return "";
      }
    }
}


//-----------------------------------------------------------------------------
// qSlicerMarkupsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidget::qSlicerMarkupsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
    , d_ptr( new qSlicerMarkupsModuleWidgetPrivate(*this) )
{
  this->pToAddShortcut = nullptr;

  this->volumeSpacingScaleFactor = 10.0;
}


//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidget::~qSlicerMarkupsModuleWidget()
{
  this->setMRMLMarkupsNode(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::setup()
{
  Q_D(qSlicerMarkupsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::enter()
{
  Q_D(qSlicerMarkupsModuleWidget);

  this->Superclass::enter();

  this->checkForAnnotationFiducialConversion();

  d->setMRMLMarkupsNodeFromSelectionNode();

  // set up mrml scene observations so that the GUI gets updated
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeAddedEvent,
                    this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::NodeRemovedEvent,
                    this, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndImportEvent,
                    this, SLOT(onMRMLSceneEndImportEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndBatchProcessEvent,
                    this, SLOT(onMRMLSceneEndBatchProcessEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent,
                    this, SLOT(onMRMLSceneEndCloseEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndRestoreEvent,
                    this, SLOT(onMRMLSceneEndRestoreEvent()));

  vtkMRMLNode *selectionNode = d->selectionNode();
  if (selectionNode)
    {
    this->qvtkConnect(selectionNode, vtkMRMLSelectionNode::ActivePlaceNodeIDChangedEvent,
                      this, SLOT(onSelectionNodeActivePlaceNodeIDChanged()));
    }

  // Add event observers to MarkupsNode
  if (d->MarkupsNode)
    {
    vtkMRMLMarkupsNode* markupsNode = d->MarkupsNode;
    d->MarkupsNode = nullptr; // this will force a reset
    this->setMRMLMarkupsNode(markupsNode);
    }

  // install some shortcuts for use while in this module
  this->installShortcuts();

  // check the max scales against volume spacing, they might need to be updated
  this->updateMaximumScaleFromVolumes();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::checkForAnnotationFiducialConversion()
{
  // check to see if there are any annotation fiducial nodes
  // and offer to import them as markups
  int numFids = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLAnnotationFiducialNode");
  int numSceneViews = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLSceneViewNode");
  if (numFids > 0)
    {
    ctkMessageBox convertMsgBox;
    convertMsgBox.setWindowTitle("Convert Annotation hierarchies to Markups list nodes?");
    QString labelText = QString("Convert ")
      + QString::number(numFids)
      + QString(" Annotation fiducials to Markups list nodes?")
      + QString(" Moves all Annotation fiducials out of hierarchies (deletes")
      + QString(" the nodes, but leaves the hierarchies in case rulers or")
      + QString(" ROIs are mixed in) and into Markups fiducial list nodes.");
    if (numSceneViews > 0)
      {
      labelText += QString(" Iterates through ")
        + QString::number(numSceneViews)
        + QString(" Scene Views and converts any fiducials saved in those")
        + QString(" scenes into Markups as well.");
      }
    // don't show again check box conflicts with informative text, so use
    // a long text
    convertMsgBox.setText(labelText);
    QPushButton *convertButton =
      convertMsgBox.addButton(tr("Convert"), QMessageBox::AcceptRole);
    convertMsgBox.addButton(QMessageBox::Cancel);
    convertMsgBox.setDefaultButton(convertButton);
    convertMsgBox.setDontShowAgainVisible(true);
    convertMsgBox.setDontShowAgainSettingsKey("Markups/AlwaysConvertAnnotationFiducials");
    convertMsgBox.exec();
    if (convertMsgBox.clickedButton() == convertButton)
      {
      this->convertAnnotationFiducialsToMarkups();
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::installShortcuts()
{
  // add some shortcut keys
  if (this->pToAddShortcut == nullptr)
    {
    this->pToAddShortcut = new QShortcut(QKeySequence(QString("p")), this);
    }
  QObject::connect(this->pToAddShortcut, SIGNAL(activated()),
                   this, SLOT(onPKeyActivated()));
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::removeShortcuts()
{
  if (this->pToAddShortcut != nullptr)
    {
    //qDebug() << "removeShortcuts";
    this->pToAddShortcut->disconnect(SIGNAL(activated()));
    // TODO: when parent is set to null, using the mouse to place a fid when outside the Markups module is triggering a crash
    //       this->pToAddShortcut->setParent(nullptr);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::convertAnnotationFiducialsToMarkups()
{
  if (this->markupsLogic())
    {
    this->markupsLogic()->ConvertAnnotationFiducialsToMarkups();
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::exit()
{
  this->Superclass::exit();

  // qDebug() << "exit widget";

  this->removeShortcuts();

  // remove mrml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();
}

//-----------------------------------------------------------------------------
vtkSlicerMarkupsLogic *qSlicerMarkupsModuleWidget::markupsLogic()
{
  if (this->logic() == nullptr)
    {
    return nullptr;
    }
  return vtkSlicerMarkupsLogic::SafeDownCast(this->logic());
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerMarkupsModuleWidget);

  bool wasBlocked = d->activeMarkupTreeView->blockSignals(true);
  d->activeMarkupTreeView->setCurrentNode(d->MarkupsNode);
  d->activeMarkupTreeView->blockSignals(wasBlocked);

  d->markupsDisplayWidget->setMRMLMarkupsNode(d->MarkupsNode);

  if (!d->MarkupsNode)
    {
    d->activeMarkupTableWidget->clearContents();
    d->activeMarkupTableWidget->setRowCount(0);

    return;
    }

  if (d->MarkupsNode->GetLocked())
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/SlicerLock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to unlock this markup list so that the markups can be moved by the mouse"));
    }
  else
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/SlicerUnlock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to lock this markup list so that the markups cannot be moved by the mouse"));
    }

  // update slice intersections
  d->sliceIntersectionsVisibilityCheckBox->setChecked(this->sliceIntersectionsVisible());

  // update the list name format
  QString nameFormat = QString(d->MarkupsNode->GetMarkupLabelFormat().c_str());
  d->nameFormatLineEdit->setText(nameFormat);

  // update the transform checkbox label to reflect current transform node name
  const char *transformNodeID = d->MarkupsNode->GetTransformNodeID();
  if (transformNodeID == nullptr)
    {
    d->transformedCoordinatesCheckBox->setText("Transformed");
    }
  else
    {
    const char *xformName = nullptr;
    if (this->mrmlScene() &&
        this->mrmlScene()->GetNodeByID(transformNodeID))
      {
      xformName = this->mrmlScene()->GetNodeByID(transformNodeID)->GetName();
      }
    if (xformName)
      {
      d->transformedCoordinatesCheckBox->setText(QString("Transformed (") +
                                                 QString(xformName) +
                                                 QString(")"));
      }
    else
      {
      d->transformedCoordinatesCheckBox->setText(QString("Transformed (") +
                                                 QString(transformNodeID) +
                                                 QString(")"));
      }
    }

  // update the table
  int numberOfPoints = d->MarkupsNode->GetNumberOfControlPoints();
  if (d->activeMarkupTableWidget->rowCount() != numberOfPoints)
    {
    // clear it out
    d->activeMarkupTableWidget->setRowCount(numberOfPoints);
    }
  for (int m = 0; m < numberOfPoints; m++)
    {
    this->updateRow(m);
    }

  vtkMRMLMarkupsCurveNode *markupsCurveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(d->MarkupsNode);
  d->resampleCurveCollapsibleButton->setVisible(markupsCurveNode != nullptr);
  d->curveSettingsWidget->setEnabled(markupsCurveNode != nullptr);
  if (markupsCurveNode)
    {
    // Update displayed node types.
    // Since updating this list resets the previous node selection,
    // we save and restore previous selection.
    vtkMRMLNode* previousOutputNode = d->resampleCurveOutputNodeSelector->currentNode();
    d->resampleCurveOutputNodeSelector->setNodeTypes(QStringList(QString(markupsCurveNode->GetClassName())));
    if (previousOutputNode && previousOutputNode->IsA(markupsCurveNode->GetClassName()))
      {
      d->resampleCurveOutputNodeSelector->setCurrentNode(previousOutputNode);
      }
    else
      {
      d->resampleCurveOutputNodeSelector->setCurrentNode(nullptr);
      }

    wasBlocked = d->curveTypeComboBox->blockSignals(true);
    d->curveTypeComboBox->setCurrentIndex(d->curveTypeComboBox->findData(markupsCurveNode->GetCurveType()));
    d->curveTypeComboBox->blockSignals(wasBlocked);

    vtkMRMLModelNode* modelNode = markupsCurveNode->GetShortestDistanceSurfaceNode();
    wasBlocked = d->modelNodeSelector->blockSignals(true);
    d->modelNodeSelector->setCurrentNode(modelNode);
    d->modelNodeSelector->blockSignals(wasBlocked);

    wasBlocked = d->costFunctionComboBox->blockSignals(true);
    int costFunction = markupsCurveNode->GetSurfaceCostFunctionType();
    d->costFunctionComboBox->setCurrentIndex(d->costFunctionComboBox->findData(costFunction));
    d->costFunctionComboBox->blockSignals(wasBlocked);

    wasBlocked = d->scalarFunctionLineEdit->blockSignals(true);
    int currentCursorPosition = d->scalarFunctionLineEdit->cursorPosition();
    d->scalarFunctionLineEdit->setText(markupsCurveNode->GetSurfaceDistanceWeightingFunction());
    d->scalarFunctionLineEdit->setCursorPosition(currentCursorPosition);
    d->scalarFunctionLineEdit->blockSignals(wasBlocked);

    if (costFunction == vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE)
      {
      d->scalarFunctionLineEdit->setVisible(false);
      }
    else
      {
      d->scalarFunctionLineEdit->setVisible(true);
      }

    QString prefixString;
    QString suffixString;
    switch (costFunction)
      {
      case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_ADDITIVE:
        prefixString = "distance + ";
        break;
      case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_MULTIPLICATIVE:
        prefixString = "distance * ";
        break;
      case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_INVERSE_SQUARED:
        prefixString = "distance / (";
        suffixString = " ^ 2";
        break;
      default:
      case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE:
        prefixString = "distance";
        break;
      }
    d->scalarFunctionPrefixLabel->setText(prefixString);
    d->scalarFunctionSuffixLabel->setText(suffixString);
    }

  if (markupsCurveNode && markupsCurveNode->GetCurveType() == vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE)
    {
    d->surfaceCurveCollapsibleButton->setEnabled(true);
    }
  else
    {
    d->surfaceCurveCollapsibleButton->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateMaximumScaleFromVolumes()
{
  Q_D(qSlicerMarkupsModuleWidget);

  double maxSliceSpacing = 1.0;

  vtkMRMLSliceLogic *sliceLogic = nullptr;
  vtkMRMLApplicationLogic *mrmlAppLogic = this->logic()->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
    {
    return;
    }

  vtkMRMLNode *mrmlNode = this->mrmlScene()->GetNodeByID("vtkMRMLSliceNodeRed");
  if (!mrmlNode)
    {
    return;
    }
  vtkMRMLSliceNode *redSlice = vtkMRMLSliceNode::SafeDownCast(mrmlNode);
  if (!redSlice)
    {
    return;
    }
  sliceLogic = mrmlAppLogic->GetSliceLogic(redSlice);
  if (!sliceLogic)
    {
    return;
    }

  double *volumeSliceSpacing = sliceLogic->GetBackgroundSliceSpacing();
  if (volumeSliceSpacing != nullptr)
    {
    for (int i = 0; i < 3; ++i)
      {
      if (volumeSliceSpacing[i] > maxSliceSpacing)
        {
        maxSliceSpacing = volumeSliceSpacing[i];
        }
      }
    }
  double maxScale = maxSliceSpacing * this->volumeSpacingScaleFactor;
  // round it up to nearest multiple of 10
  maxScale = ceil(maxScale / 10.0) * 10.0;

  d->markupsDisplayWidget->setMaximumMarkupsScale(maxScale);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateRow(int m)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // this is updating the qt widget from MRML, and should not trigger any updates on the node, so turn off events
  d->activeMarkupTableWidget->blockSignals(true);

  // qDebug() << QString("updateRow: row = ") + QString::number(m) + QString(", number of rows = ") + QString::number(d->activeMarkupTableWidget->rowCount());
  // get active markups node
  vtkMRMLMarkupsNode* markupsNode = nullptr;
   vtkMRMLNode* node = d->activeMarkupTreeView->currentNode();
   if (node)
     {
     // make sure the node is still in the scene and convert to markups
     markupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->mrmlScene()->GetNodeByID(node->GetID()));
     }
  if (!markupsNode
    || m >= markupsNode->GetNumberOfControlPoints()) // markup point is already deleted (possible after batch update)
    {
    //qDebug() << QString("update Row: unable to get markups node with id ") + activeMarkupsNodeID;
    return;
    }

  // selected
  QTableWidgetItem* selectedItem = new QTableWidgetItem();
  if (markupsNode->GetNthControlPointSelected(m))
    {
    selectedItem->setCheckState(Qt::Checked);
    }
  else
    {
    selectedItem->setCheckState(Qt::Unchecked);
    }
  // disable editing so that a double click won't bring up an entry box
  selectedItem->setFlags(selectedItem->flags() & ~Qt::ItemIsEditable);
  int selectedIndex = d->columnIndex("Selected");
  if (d->activeMarkupTableWidget->item(m,selectedIndex) == nullptr ||
      (d->activeMarkupTableWidget->item(m,selectedIndex)->checkState() != selectedItem->checkState()))
    {
    d->activeMarkupTableWidget->setItem(m,selectedIndex,selectedItem);
    }

  // locked
  QTableWidgetItem* lockedItem = new QTableWidgetItem();
  // disable checkable
  lockedItem->setData(Qt::CheckStateRole, QVariant());
  lockedItem->setFlags(lockedItem->flags() & ~Qt::ItemIsUserCheckable);
  // disable editing so that a double click won't bring up an entry box
  lockedItem->setFlags(lockedItem->flags() & ~Qt::ItemIsEditable);
  if (markupsNode->GetNthControlPointLocked(m))
    {
    lockedItem->setData(Qt::UserRole, QVariant(true));
    lockedItem->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerLock.png"));
    }
  else
    {
    lockedItem->setData(Qt::UserRole, QVariant(false));
    lockedItem->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerUnlock.png"));
    }
  int lockedIndex = d->columnIndex("Locked");
  if (d->activeMarkupTableWidget->item(m,lockedIndex) == nullptr ||
      d->activeMarkupTableWidget->item(m,lockedIndex)->data(Qt::UserRole) != lockedItem->data(Qt::UserRole))
    {
    d->activeMarkupTableWidget->setItem(m,lockedIndex,lockedItem);
    }

  // visible
  QTableWidgetItem* visibleItem = new QTableWidgetItem();
  // disable checkable
  visibleItem->setData(Qt::CheckStateRole, QVariant());
  visibleItem->setFlags(visibleItem->flags() & ~Qt::ItemIsUserCheckable);
  // disable editing so that a double click won't bring up an entry box
  visibleItem->setFlags(visibleItem->flags() & ~Qt::ItemIsEditable);
  if (markupsNode->GetNthControlPointVisibility(m))
    {
    visibleItem->setData(Qt::UserRole, QVariant(true));
    visibleItem->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerVisible.png"));
    }
  else
    {
    visibleItem->setData(Qt::UserRole, QVariant(false));
    visibleItem->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerInvisible.png"));
    }
    int visibleIndex = d->columnIndex("Visible");
   if (d->activeMarkupTableWidget->item(m,visibleIndex) == nullptr ||
       d->activeMarkupTableWidget->item(m,visibleIndex)->data(Qt::UserRole) != visibleItem->data(Qt::UserRole))
     {
     d->activeMarkupTableWidget->setItem(m,visibleIndex,visibleItem);
     }

   // name
   int nameIndex = d->columnIndex("Name");
   QString markupLabel = QString(markupsNode->GetNthControlPointLabel(m).c_str());
   if (d->activeMarkupTableWidget->item(m,nameIndex) == nullptr ||
       d->activeMarkupTableWidget->item(m,nameIndex)->text() != markupLabel)
     {
     d->activeMarkupTableWidget->setItem(m,nameIndex,new QTableWidgetItem(markupLabel));
     }

   // description
   int descriptionIndex = d->columnIndex("Description");
   QString markupDescription = QString(markupsNode->GetNthControlPointDescription(m).c_str());
   if (d->activeMarkupTableWidget->item(m,descriptionIndex) == nullptr ||
       d->activeMarkupTableWidget->item(m,descriptionIndex)->text() != markupLabel)
     {
     d->activeMarkupTableWidget->setItem(m,descriptionIndex,new QTableWidgetItem(markupDescription));
     }

   // point
   double point[3] = {0.0, 0.0, 0.0};
   if (d->transformedCoordinatesCheckBox->isChecked())
     {
     double worldPoint[4] = {0.0, 0.0, 0.0, 1.0};
     markupsNode->GetNthControlPointPositionWorld(m, worldPoint);
     for (int p = 0; p < 3; ++p)
       {
       point[p] = worldPoint[p];
       }
     }
   else
     {
     markupsNode->GetNthControlPointPosition(m, point);
     }
  int rColumnIndex = d->columnIndex("R");
  for (int p = 0; p < 3; p++)
    {
    // last argument to number sets the precision
    QString coordinate = QString::number(point[p], 'f', 3);
    if (d->activeMarkupTableWidget->item(m,rColumnIndex + p) == nullptr ||
        d->activeMarkupTableWidget->item(m,rColumnIndex + p)->text() != coordinate)
      {
      d->activeMarkupTableWidget->setItem(m,rColumnIndex + p,new QTableWidgetItem(coordinate));
      }
    }

  // unblock so that changes to the table will propagate to MRML
  d->activeMarkupTableWidget->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNodeAddedEvent(vtkObject*, vtkObject* node)
{
  Q_D(qSlicerMarkupsModuleWidget);

  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (markupsNode)
    {
    // make it active
    d->activeMarkupTreeView->setCurrentNode(markupsNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMRMLSceneEndImportEvent()
{
  this->checkForAnnotationFiducialConversion();
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMRMLSceneEndRestoreEvent()
{
  this->checkForAnnotationFiducialConversion();
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMRMLSceneEndBatchProcessEvent()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->mrmlScene())
    {
    return;
    }
  this->checkForAnnotationFiducialConversion();
  d->setMRMLMarkupsNodeFromSelectionNode();
  // force update (clear GUI if no node is selected anymore)
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMRMLSceneEndCloseEvent()
{
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  this->setMRMLMarkupsNode(nullptr);
  // force update (clear GUI if no node is selected anymore)
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onPKeyActivated()
{
  QPoint pos = QCursor::pos();

  // find out which widget it was over
  QWidget *widget = qSlicerApplication::application()->widgetAt(pos);

  // simulate a mouse press inside the widget
  QPoint widgetPos = widget->mapFromGlobal(pos);
  QMouseEvent click(QEvent::MouseButtonRelease, widgetPos, Qt::LeftButton, nullptr, nullptr);
  click.setAccepted(true);

  // and send it to the widget
  //qDebug() << "onPKeyActivated: sending event with pos " << widgetPos;
  QCoreApplication::sendEvent(widget, &click);
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityOnAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllMarkupsVisibility(d->MarkupsNode, true);
  d->MarkupsNode->SetDisplayVisibility(true);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityOffAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllMarkupsVisibility(d->MarkupsNode, false);
  d->MarkupsNode->SetDisplayVisibility(false);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityAllMarkupsInListToggled()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->ToggleAllMarkupsVisibility(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onLockAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllMarkupsLocked(d->MarkupsNode, true);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onUnlockAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllMarkupsLocked(d->MarkupsNode, false);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onLockAllMarkupsInListToggled()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->ToggleAllMarkupsLocked(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllMarkupsSelected(d->MarkupsNode, true);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeselectAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllMarkupsSelected(d->MarkupsNode, false);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectedAllMarkupsInListToggled()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->ToggleAllMarkupsSelected(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onAddMarkupPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  // get the active node
  if (d->MarkupsNode->GetNumberOfControlPoints() >= d->MarkupsNode->GetMaximumNumberOfControlPoints())
    {
    return;
    }
  d->MarkupsNode->AddControlPoint(vtkVector3d(0,0,0));
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMoveMarkupUpPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, that only one is selected
  if ((selectedItems.size() / d->numberOfColumns()) != 1)
    {
    qDebug() << "Move up: only select one markup to move, current selected: " << selectedItems.size() << ", number of columns = " << d->numberOfColumns();
    return;
    }
  int thisIndex = selectedItems.at(0)->row();
  //qDebug() << "Swapping " << thisIndex << " and " << thisIndex - 1;
  d->MarkupsNode->SwapControlPoints(thisIndex, thisIndex - 1);
  // now make sure the new row is selected so a user can keep moving it up
  d->activeMarkupTableWidget->selectRow(thisIndex - 1);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMoveMarkupDownPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, that only one is selected
  if ((selectedItems.size() / d->numberOfColumns()) != 1)
    {
    return;
    }
  int thisIndex = selectedItems.at(0)->row();
  //qDebug() << "Swapping " << thisIndex << " and " << thisIndex + 1;
  d->MarkupsNode->SwapControlPoints(thisIndex, thisIndex + 1);
  // now make sure the new row is selected so a user can keep moving it down
  d->activeMarkupTableWidget->selectRow(thisIndex + 1);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeleteMarkupPushButtonClicked(bool confirm /*=true*/)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  for (int i = 0; i < selectedItems.size(); i += d->numberOfColumns())
    {
    // get the row
    int row = selectedItems.at(i)->row();
    // qDebug() << "Saving: i = " << i << ", row = " << row;
    rows << row;
    }
  // sort the list
  qSort(rows);

  if (confirm)
    {
    ctkMessageBox deleteAllMsgBox;
    deleteAllMsgBox.setWindowTitle("Delete Markups in this list?");
    QString labelText = QString("Delete ")
      + QString::number(rows.size())
      + QString(" Markups from this list?");
    // don't show again check box conflicts with informative text, so use
    // a long text
    deleteAllMsgBox.setText(labelText);

    QPushButton *deleteButton =
      deleteAllMsgBox.addButton(tr("Delete"), QMessageBox::AcceptRole);
    deleteAllMsgBox.addButton(QMessageBox::Cancel);
    deleteAllMsgBox.setDefaultButton(deleteButton);
    deleteAllMsgBox.setIcon(QMessageBox::Question);
    deleteAllMsgBox.setDontShowAgainVisible(true);
    deleteAllMsgBox.setDontShowAgainSettingsKey("Markups/AlwaysDeleteMarkups");
    deleteAllMsgBox.exec();
    if (deleteAllMsgBox.clickedButton() != deleteButton)
      {
      return;
      }
    }

  // delete from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    // qDebug() << "Deleting: i = " << i << ", index = " << index;
    d->MarkupsNode->RemoveNthControlPoint(index);
    }

  // clear the selection on the table
  d->activeMarkupTableWidget->clearSelection();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeleteAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  ctkMessageBox deleteAllMsgBox;
  deleteAllMsgBox.setWindowTitle("Delete All Markups in this list?");
  QString labelText = QString("Delete all ")
    + QString::number(d->MarkupsNode->GetNumberOfControlPoints())
    + QString(" Markups in this list?");
  // don't show again check box conflicts with informative text, so use
  // a long text
  deleteAllMsgBox.setText(labelText);

  QPushButton *deleteButton =
    deleteAllMsgBox.addButton(tr("Delete All"), QMessageBox::AcceptRole);
  deleteAllMsgBox.addButton(QMessageBox::Cancel);
  deleteAllMsgBox.setDefaultButton(deleteButton);
  deleteAllMsgBox.setIcon(QMessageBox::Question);
  deleteAllMsgBox.setDontShowAgainVisible(true);
  deleteAllMsgBox.setDontShowAgainSettingsKey("Markups/AlwaysDeleteAllMarkups");
  deleteAllMsgBox.exec();
  if (deleteAllMsgBox.clickedButton() == deleteButton)
    {
    d->MarkupsNode->RemoveAllControlPoints();
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupItemChanged(vtkIdType)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->isEntered())
    {
    // ignore any changes if the GUI is not shown
    return;
    }
  this->onActiveMarkupMRMLNodeChanged(d->activeMarkupTreeView->currentNode());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupMRMLNodeChanged(vtkMRMLNode *node)
{
  Q_D(qSlicerMarkupsModuleWidget);

  if (!this->isEntered())
    {
    // ignore any changes if the GUI is not shown
    return;
    }

  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);

  // User changed the selected markup node.
  // We now make it the active place node in the scene.
  if (markupsNode)
    {
    d->setSelectionNodeActivePlaceNode(markupsNode);
    }
  this->setMRMLMarkupsNode(markupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onCreateMarkupsFiducial()
{
  if (this->mrmlScene())
    {
    this->onActiveMarkupMRMLNodeAdded(this->mrmlScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialNode"));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onCreateMarkupsLine()
{
  if (this->mrmlScene())
    {
    this->onActiveMarkupMRMLNodeAdded(this->mrmlScene()->AddNewNodeByClass("vtkMRMLMarkupsLineNode"));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onCreateMarkupsAngle()
{
  if (this->mrmlScene())
    {
    this->onActiveMarkupMRMLNodeAdded(this->mrmlScene()->AddNewNodeByClass("vtkMRMLMarkupsAngleNode"));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onCreateMarkupsOpenCurve()
{
  if (this->mrmlScene())
    {
    this->onActiveMarkupMRMLNodeAdded(this->mrmlScene()->AddNewNodeByClass("vtkMRMLMarkupsCurveNode"));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onCreateMarkupsClosedCurve()
{
  if (this->mrmlScene())
    {
    this->onActiveMarkupMRMLNodeAdded(this->mrmlScene()->AddNewNodeByClass("vtkMRMLMarkupsClosedCurveNode"));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onCreateMarkupsPlane()
{
  if (this->mrmlScene())
    {
    this->onActiveMarkupMRMLNodeAdded(this->mrmlScene()->AddNewNodeByClass("vtkMRMLMarkupsPlaneNode"));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupMRMLNodeAdded(vtkMRMLNode *markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (this->markupsLogic())
    {
    this->markupsLogic()->AddNewDisplayNodeForMarkupsNode(markupsNode);
    }
  // make sure it's set up for the mouse mode tool bar to easily add points to
  // it by making it active in the selection node
  d->setSelectionNodeActivePlaceNode(markupsNode);
  d->setPlaceModeEnabled(true);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectionNodeActivePlaceNodeIDChanged()
{
  Q_D(qSlicerMarkupsModuleWidget);
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->selectionNodeActivePlaceNode());
  this->setMRMLMarkupsNode(markupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onListVisibileInvisiblePushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // toggle the visibility
  int visibleFlag = d->MarkupsNode->GetDisplayVisibility();
  visibleFlag = !visibleFlag;
  d->MarkupsNode->SetDisplayVisibility(visibleFlag);

  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsVisibility(d->MarkupsNode, visibleFlag);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onListLockedUnlockedPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  d->MarkupsNode->SetLocked(!d->MarkupsNode->GetLocked());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNameFormatLineEditTextEdited(const QString text)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  d->MarkupsNode->SetMarkupLabelFormat(std::string(text.toUtf8()));
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onResetNameFormatToDefaultPushButtonClicked()
{
   Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode || !this->mrmlScene())
    {
    return;
    }
   // make a new default markups node and use its value for the name format
  vtkSmartPointer<vtkMRMLMarkupsNode> defaultNode = vtkMRMLMarkupsNode::SafeDownCast(
    this->mrmlScene()->GetDefaultNodeByClass(d->MarkupsNode->GetClassName()));
  if (!defaultNode)
    {
    defaultNode = vtkSmartPointer<vtkMRMLMarkupsNode>::New();
    }
  d->MarkupsNode->SetMarkupLabelFormat(defaultNode->GetMarkupLabelFormat());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onRenameAllWithCurrentNameFormatPushButtonClicked()
{
   Q_D(qSlicerMarkupsModuleWidget);
   if (!d->MarkupsNode || !this->markupsLogic())
     {
     return;
     }
   this->markupsLogic()->RenameAllMarkupsFromCurrentFormat(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupTableCellChanged(int row, int column)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // row corresponds to the index in the list
  int n = row;

  // now switch on the property
  QTableWidgetItem *item = d->activeMarkupTableWidget->item(row, column);
  if (!item)
    {
    qDebug() << QString("Unable to find item in table at ") + QString::number(row) + QString(", ") + QString::number(column);
    return;
    }
  if (column == d->columnIndex("Selected"))
    {
    bool flag = (item->checkState() == Qt::Unchecked ? false : true);
    d->MarkupsNode->SetNthControlPointSelected(n, flag);
    }
  else if (column == d->columnIndex("Locked"))
    {
    bool flag = item->data(Qt::UserRole) == QVariant(true) ? true : false;
    // update the icon
    if (flag)
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerLock.png"));
      }
    else
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerUnlock.png"));
      }
    d->MarkupsNode->SetNthControlPointLocked(n, flag);
    }
  else if (column == d->columnIndex("Visible"))
    {
    bool flag = item->data(Qt::UserRole) == QVariant(true) ? true : false;
    // update the eye icon
    if (flag)
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerVisible.png"));
      }
    else
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/Small/SlicerInvisible.png"));
      }
    d->MarkupsNode->SetNthControlPointVisibility(n, flag);
    }
  else if (column ==  d->columnIndex("Name"))
    {
    std::string name = std::string(item->text().toUtf8());
    d->MarkupsNode->SetNthControlPointLabel(n, name);
    }
  else if (column ==  d->columnIndex("Description"))
    {
    std::string description = std::string(item->text().toUtf8());
    d->MarkupsNode->SetNthControlPointDescription(n, description);
    }
  else if (column == d->columnIndex("R") ||
           column == d->columnIndex("A") ||
           column == d->columnIndex("S"))
    {
    // get the new value
    double newPoint[3] = {0.0, 0.0, 0.0};
    if (d->activeMarkupTableWidget->item(row, d->columnIndex("R")) == nullptr ||
        d->activeMarkupTableWidget->item(row, d->columnIndex("A")) == nullptr ||
        d->activeMarkupTableWidget->item(row, d->columnIndex("S")) == nullptr)
      {
      // init state, return
      return;
      }
    newPoint[0] = d->activeMarkupTableWidget->item(row, d->columnIndex("R"))->text().toDouble();
    newPoint[1] = d->activeMarkupTableWidget->item(row, d->columnIndex("A"))->text().toDouble();
    newPoint[2] = d->activeMarkupTableWidget->item(row, d->columnIndex("S"))->text().toDouble();

    // get the old value
    double point[3] = {0.0, 0.0, 0.0};
    if (d->transformedCoordinatesCheckBox->isChecked())
      {
      double worldPoint[4] = {0.0, 0.0, 0.0, 1.0};
      d->MarkupsNode->GetNthControlPointPositionWorld(n, worldPoint);
      for (int p = 0; p < 3; ++p)
       {
       point[p] = worldPoint[p];
       }
      }
    else
      {
      d->MarkupsNode->GetNthControlPointPosition(n, point);
      }

    // changed?
    double minChange = 0.001;
    if (fabs(newPoint[0] - point[0]) > minChange ||
        fabs(newPoint[1] - point[1]) > minChange ||
        fabs(newPoint[2] - point[2]) > minChange)
      {
      if (d->transformedCoordinatesCheckBox->isChecked())
        {
        d->MarkupsNode->SetNthControlPointPositionWorld(n, newPoint[0], newPoint[1], newPoint[2]);
        }
      else
        {
        d->MarkupsNode->SetNthControlPointPositionFromArray(n, newPoint);
        }
      }
    else
      {
      //qDebug() << QString("Cell changed: no change in location bigger than ") + QString::number(minChange);
      }
    }
  else
    {
    qDebug() << QString("Cell Changed: unknown column: ") + QString::number(column);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupTableCellClicked(QTableWidgetItem* item)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (item == nullptr)
    {
    return;
    }
  int column = item->column();
  if (column == d->columnIndex(QString("Visible")) ||
           column == d->columnIndex(QString("Locked")))
    {
    // toggle the user role, the icon update is triggered by this change
    if (item->data(Qt::UserRole) == QVariant(false))
      {
      item->setData(Qt::UserRole, QVariant(true));
      }
    else
      {
      item->setData(Qt::UserRole, QVariant(false));
      }
    }
}
//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupTableCurrentCellChanged(
     int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_D(qSlicerMarkupsModuleWidget);
  Q_UNUSED(currentColumn);
  Q_UNUSED(previousRow);
  Q_UNUSED(previousColumn);

  // get the active list
  if (!d->MarkupsNode)
    {
    return;
    }

  // is jumping disabled?
  if (!d->jumpSlicesCheckBox->isChecked())
    {
    return;
    }
  // otherwise jump to that slice

  // offset or center?
  bool jumpCentered = false;
  if (d->jumpModeComboBox->currentIndex() == JUMP_MODE_COMBOBOX_INDEX_CENTERED)
    {
    jumpCentered = true;
    }
  // jump to it
  if (this->markupsLogic())
    {
    this->markupsLogic()->JumpSlicesToNthPointInMarkup(d->MarkupsNode->GetID(), currentRow, jumpCentered);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onRightClickActiveMarkupTableWidget(QPoint pos)
{
  Q_D(qSlicerMarkupsModuleWidget);
  Q_UNUSED(pos);

  // qDebug() << "onRightClickActiveMarkupTableWidget: pos = " << pos;

  QMenu menu;

  // Delete
  QAction *deleteFiducialAction =
    new QAction(QString("Delete highlighted fiducial(s)"), &menu);
  menu.addAction(deleteFiducialAction);
  QObject::connect(deleteFiducialAction, SIGNAL(triggered()),
                   this, SLOT(onDeleteMarkupPushButtonClicked()));

  // Jump slices
  QAction *jumpSlicesAction =
    new QAction(QString("Jump slices"), &menu);
  menu.addAction(jumpSlicesAction);
  QObject::connect(jumpSlicesAction, SIGNAL(triggered()),
                   this, SLOT(onJumpSlicesActionTriggered()));

  // Refocus 3D cameras
  QAction *refocusCamerasAction =
    new QAction(QString("Refocus all cameras"), &menu);
  menu.addAction(refocusCamerasAction);
  QObject::connect(refocusCamerasAction, SIGNAL(triggered()),
                   this, SLOT(onRefocusCamerasActionTriggered()));

  menu.addSeparator();
  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();
  if (!selectedItems.isEmpty())
    {
    menu.addAction(d->cutAction);
    menu.addAction(d->copyAction);
    }
  menu.addAction(d->pasteAction);

  this->addSelectedCoordinatesToMenu(&menu);

  menu.exec(QCursor::pos());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::addSelectedCoordinatesToMenu(QMenu *menu)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // get the list of selected rows to sort them in index order
  QList<int> rows;
  // The selected items list contains an item for each column in each row that
  // has been selected. Don't make any assumptions about the order of the
  // selected items, iterate through all of them and collect unique rows
  for (int i = 0; i < selectedItems.size(); ++i)
    {
    // get the row
    int row = selectedItems.at(i)->row();
    if (!rows.contains(row))
      {
      rows << row;
      }
    }
  // sort the list
  qSort(rows);

  // keep track of point to point distance
  double distance = 0.0;
  double lastPoint[3] = {0.0, 0.0, 0.0};

  menu->addSeparator();

  // loop over the selected rows
  for (int i = 0; i < rows.size() ; i++)
    {
    int row = rows.at(i);
    // label this selected markup if more than one
    QString indexString;
    if (rows.size() > 1)
      {
      // if there's a label use it
      if (!(d->MarkupsNode->GetNthControlPointLabel(row).empty()))
        {
        indexString =  QString(d->MarkupsNode->GetNthControlPointLabel(row).c_str());
        }
      else
        {
        // use the row number as an index (row starts at 0, but GUI starts at 1)
        indexString = QString::number(row+1);
        }
      indexString +=  QString(" : ");
      }

    double point[3] = {0.0, 0.0, 0.0};
    if (d->transformedCoordinatesCheckBox->isChecked())
      {
      double worldPoint[4] = {0.0, 0.0, 0.0, 1.0};
      d->MarkupsNode->GetNthControlPointPositionWorld(row, worldPoint);
      for (int p = 0; p < 3; ++p)
        {
        point[p] = worldPoint[p];
        }
      }
    else
      {
      d->MarkupsNode->GetNthControlPointPosition(row, point);
      }
    // format the coordinates
    QString coordinate =
      QString::number(point[0]) + QString(",") +
      QString::number(point[1]) + QString(",") +
      QString::number(point[2]);
    QString menuString = indexString + coordinate;
    menu->addAction(menuString);

    // calculate the point to point accumulated distance for fiducials
    if (rows.size() > 1)
      {
      if (i > 0)
        {
        double distanceToLastPoint = vtkMath::Distance2BetweenPoints(lastPoint, point);
        if (distanceToLastPoint != 0.0)
          {
          distanceToLastPoint = sqrt(distanceToLastPoint);
          }
        distance += distanceToLastPoint;
        }
      lastPoint[0] = point[0];
      lastPoint[1] = point[1];
      lastPoint[2] = point[2];
      }
    }
  if (distance != 0.0)
    {
    menu->addAction(QString("Summed linear distance: %1").arg(distance));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onJumpSlicesActionTriggered()
{
 Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // offset or center?
  bool jumpCentered = false;
  if (d->jumpModeComboBox->currentIndex() == JUMP_MODE_COMBOBOX_INDEX_CENTERED)
    {
    jumpCentered = true;
    }

  // jump to it
  if (this->markupsLogic())
    {
    // use the first selected
    this->markupsLogic()->JumpSlicesToNthPointInMarkup(d->MarkupsNode->GetID(), selectedItems.at(0)->row(), jumpCentered);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onRefocusCamerasActionTriggered()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();
  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }
  // refocus on this point
  if (this->markupsLogic())
    {
    // use the first selected
    this->markupsLogic()->FocusCamerasOnNthPointInMarkup(d->MarkupsNode->GetID(), selectedItems.at(0)->row());
    }
}

//-----------------------------------------------------------------------------
QStringList qSlicerMarkupsModuleWidget::getOtherMarkupNames(vtkMRMLNode *thisMarkup)
{
  QStringList otherMarkups;

  // check for other markups nodes in the scene
  if (!this->mrmlScene())
    {
    return otherMarkups;
    }

  vtkCollection *col = this->mrmlScene()->GetNodesByClass(thisMarkup->GetClassName());
  int numNodes = col->GetNumberOfItems();
  if (numNodes < 2)
    {
    col->RemoveAllItems();
    col->Delete();
    return otherMarkups;
    }

  for (int n = 0; n < numNodes; n++)
    {
    vtkMRMLNode *markupsNodeN = vtkMRMLNode::SafeDownCast(col->GetItemAsObject(n));
    if (strcmp(markupsNodeN->GetID(), thisMarkup->GetID()) != 0)
      {
      otherMarkups.append(QString(markupsNodeN->GetName()));
      }
    }
  col->RemoveAllItems();
  col->Delete();

  return otherMarkups;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::cutSelectedToClipboard()
{
  this->copySelectedToClipboard();
  this->onDeleteMarkupPushButtonClicked(false); // no confirmation message
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::copySelectedToClipboard()
{
  Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  if (!d->MarkupsNode)
    {
    qDebug() << Q_FUNC_INFO << ": no active list from which to cut";
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  for (int i = 0; i < selectedItems.size(); i += d->numberOfColumns())
    {
    // get the row
    int row = selectedItems.at(i)->row();
    // qDebug() << "Saving: i = " << i << ", row = " << row;
    rows << row;
    }
  // sort the list
  qSort(rows);

  vtkNew<vtkMRMLMarkupsFiducialStorageNode> storageNode;
  // Excel recognizes tab character as field separator,
  // therefore use that instead of comma.
  storageNode->SetFieldDelimiterCharacters("\t");

  QString markupsAsString;
  for (int i = 0; i < rows.size(); ++i)
    {
    int markupIndex = rows.at(i);
    markupsAsString += (storageNode->GetPointAsString(d->MarkupsNode, markupIndex).c_str() + QString("\n"));
    }

  QApplication::clipboard()->setText(markupsAsString);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::pasteSelectedFromClipboard()
{
  Q_D(qSlicerMarkupsModuleWidget);

  QString clipboardText = QApplication::clipboard()->text();
  QStringList lines = clipboardText.split("\n");
  if (lines.empty())
    {
    return;
    }

  if (!d->MarkupsNode)
    {
    // No fiducial list is selected - create a new one
    // Assume a fiducial markups
    this->onCreateMarkupsFiducial();
    if (!d->MarkupsNode)
      {
      return;
      }
    }

  vtkNew<vtkMRMLMarkupsFiducialStorageNode> storageNode;
  if (clipboardText.contains("\t"))
    {
    storageNode->SetFieldDelimiterCharacters("\t");
    }

  // SetPointFromString calls various events reporting the id of the point modified.
  // However, already for > 200 points, it gets bad perfomance. Therefore, we call a simply modified call at the end.
  d->MarkupsNode->DisableModifiedEventOn();
  foreach(QString line, lines)
    {
    line = line.trimmed();
    if (line.isEmpty() || line.startsWith('#'))
      {
      // empty line or comment line
      continue;
      }

    storageNode->SetPointFromString(d->MarkupsNode, d->MarkupsNode->GetNumberOfControlPoints(), line.toUtf8());
    }
  d->MarkupsNode->DisableModifiedEventOff();
  d->MarkupsNode->Modified();
  int n = d->MarkupsNode->GetNumberOfControlPoints() - 1;
  d->MarkupsNode->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeModifiedEvent()
{
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->mrmlScene())
    {
    markupsNode = nullptr;
    }
  if (markupsNode == d->MarkupsNode)
    {
    // no change
    return;
    }

  qvtkReconnect(d->MarkupsNode, markupsNode, vtkCommand::ModifiedEvent,
    this, SLOT(onActiveMarkupsNodeModifiedEvent()));

  // points
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkMRMLMarkupsNode::PointModifiedEvent,
    this, SLOT(onActiveMarkupsNodePointModifiedEvent(vtkObject*, void*)));
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkMRMLMarkupsNode::PointAddedEvent,
    this, SLOT(onActiveMarkupsNodePointAddedEvent()));
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkMRMLMarkupsNode::PointRemovedEvent,
    this, SLOT(onActiveMarkupsNodePointRemovedEvent(vtkObject*, void*)));

  // display
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkMRMLDisplayableNode::DisplayModifiedEvent,
    this, SLOT(onActiveMarkupsNodeDisplayModifiedEvent()));

  // transforms
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkMRMLTransformableNode::TransformModifiedEvent,
    this, SLOT(onActiveMarkupsNodeTransformModifiedEvent()));

  d->MarkupsNode = markupsNode;

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodePointModifiedEvent(vtkObject *caller, void *callData)
{
  // the call data should be the index n
  if (caller == nullptr)
    {
    return;
    }

  int* nPtr = reinterpret_cast<int*>(callData);
  int n = (nPtr ? *nPtr : -1);
  if (n>=0)
    {
    this->updateRow(n);
    }
  else
    {
    // batch update finished
    this->updateWidgetFromMRML();
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodePointAddedEvent()
{
  Q_D(qSlicerMarkupsModuleWidget);

  int newRow = d->activeMarkupTableWidget->rowCount();
  d->activeMarkupTableWidget->insertRow(newRow);

  this->updateRow(newRow);

  // scroll to the new row only if jump slices is not selected
  // (if jump slices on click in table is selected, selecting the new
  // row before the point coordinates are updated will cause the slices
  // to jump to 0,0,0)
  if (!d->jumpSlicesCheckBox->isChecked())
    {
    d->activeMarkupTableWidget->setCurrentCell(newRow, 0);
    }

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodePointRemovedEvent(vtkObject *caller, void *callData)
{
  Q_D(qSlicerMarkupsModuleWidget);

  if (caller == nullptr)
    {
    return;
    }

  // the call data should be the index n
  int *nPtr = reinterpret_cast<int*>(callData);
  int n = (nPtr ? *nPtr : -1);
  if (n >= 0)
    {
    d->activeMarkupTableWidget->removeRow(n);
    }
  else
    {
    // batch update finished
    this->updateWidgetFromMRML();
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeDisplayModifiedEvent()
{
  // update the display properties
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeTransformModifiedEvent()
{
  // update the transform check box label
  // update the coordinates in the table
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSliceIntersectionsVisibilityToggled(bool flag)
{
  if (!this->markupsLogic())
    {
    qWarning() << "Unable to get markups logic";
    return;
    }
  this->markupsLogic()->SetSliceIntersectionsVisibility(flag);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNewMarkupWithCurrentDisplayPropertiesTriggered()
{
  Q_D(qSlicerMarkupsModuleWidget);

//  qDebug() << "onNewMarkupWithCurrentDisplayPropertiesTriggered";

  // get the active list
  if (!d->MarkupsNode)
    {
    // if there's no currently active markups list, trigger the default add
    // node
    this->onCreateMarkupsFiducial();
    return;
    }

  // get the display node
  vtkMRMLDisplayNode *displayNode = d->MarkupsNode->GetDisplayNode();
  if (!displayNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: Unable to get the display node on the markups node";
    return;
    }

  // create a new one
  vtkSmartPointer<vtkMRMLNode> newDisplayNode = vtkSmartPointer<vtkMRMLNode>::Take(
    this->mrmlScene()->CreateNodeByClass(displayNode->GetClassName()));
  // copy the old one
  if (!newDisplayNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: error creating display node";
    return;
    }
  newDisplayNode->Copy(displayNode);

  // now create the new markups node
  const char *className = d->MarkupsNode->GetClassName();
  vtkSmartPointer<vtkMRMLMarkupsNode> newMRMLNode = vtkSmartPointer<vtkMRMLMarkupsNode>::Take(
    vtkMRMLMarkupsNode::SafeDownCast(this->mrmlScene()->CreateNodeByClass(className)));
  if (!newMRMLNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: error creating markups node";
    return;
    }
  // copy the name and let them rename it
  newMRMLNode->SetName(d->MarkupsNode->GetName());

  /// add to the scene
  this->mrmlScene()->AddNode(newDisplayNode);
  this->mrmlScene()->AddNode(newMRMLNode);
  newMRMLNode->SetAndObserveDisplayNodeID(newDisplayNode->GetID());

  // set it active
  d->setSelectionNodeActivePlaceNode(newMRMLNode);
  this->setMRMLMarkupsNode(newMRMLNode);
  // let the user rename it
  d->activeMarkupTreeView->renameCurrentItem();
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsModuleWidget::sliceIntersectionsVisible()
{
  if (!this->markupsLogic())
    {
    qWarning() << "Unable to get markups logic";
    return false;
    }
  int flag = this->markupsLogic()->GetSliceIntersectionsVisibility();
  if (flag == 0 || flag == -1)
    {
    return false;
    }
  else
    {
    // if all or some are visible, return true
    return true;
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onHideCoordinateColumnsToggled(bool checked)
{
  Q_D(qSlicerMarkupsModuleWidget);

  d->activeMarkupTableWidget->setColumnHidden(d->columnIndex("R"), checked);
  d->activeMarkupTableWidget->setColumnHidden(d->columnIndex("A"), checked);
  d->activeMarkupTableWidget->setColumnHidden(d->columnIndex("S"), checked);

  if (!checked)
    {
    // back to default column widths
    d->activeMarkupTableWidget->setColumnWidth(d->columnIndex("Name"), 60);
    d->activeMarkupTableWidget->setColumnWidth(d->columnIndex("Description"), 120);
    }
  else
    {
    // expand the name and description columns
    d->activeMarkupTableWidget->setColumnWidth(d->columnIndex("Name"), 120);
    d->activeMarkupTableWidget->setColumnWidth(d->columnIndex("Description"), 240);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onTransformedCoordinatesToggled(bool checked)
{
  Q_UNUSED(checked);

  // update the GUI
  // tbd: only update the coordinates
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------
bool qSlicerMarkupsModuleWidget::setEditedNode(vtkMRMLNode* node,
                                               QString role /* = QString()*/,
                                               QString context /* = QString()*/)
{
  Q_D(qSlicerMarkupsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkMRMLMarkupsNode::SafeDownCast(node))
    {
    d->setSelectionNodeActivePlaceNode(node);
    return true;
    }

  if (vtkMRMLMarkupsDisplayNode::SafeDownCast(node))
    {
    vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(node);
    vtkMRMLMarkupsNode* displayableNode = vtkMRMLMarkupsNode::SafeDownCast(displayNode->GetDisplayableNode());
     if (!displayableNode)
      {
      return false;
      }
    d->setSelectionNodeActivePlaceNode(displayableNode);
    return true;
    }

  return false;
}

//-----------------------------------------------------------
double qSlicerMarkupsModuleWidget::nodeEditable(vtkMRMLNode* node)
{
  if (vtkMRMLMarkupsNode::SafeDownCast(node)
    || vtkMRMLMarkupsDisplayNode::SafeDownCast(node))
    {
    return 0.5;
    }
  else if (node->IsA("vtkMRMLAnnotationFiducialNode"))
    {
    // The module cannot directly edit this type of node but can convert it
    return 0.1;
    }
  else
    {
    return 0.0;
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onResetToDefaultDisplayPropertiesPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  vtkMRMLMarkupsDisplayNode *displayNode = d->markupsDisplayNode();
  if (!displayNode)
    {
    return;
    }
  // set the display node from the logic defaults
  if (!this->markupsLogic())
    {
    return;
    }
  this->markupsLogic()->SetDisplayNodeToDefaults(displayNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onApplyCurveResamplingPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  double resampleNumberOfPoints = d->resampleCurveNumerOfOutputPointsSpinBox->value();
  if (resampleNumberOfPoints <= 1)
    {
    return;
    }

  vtkMRMLMarkupsCurveNode* inputNode = vtkMRMLMarkupsCurveNode::SafeDownCast(d->MarkupsNode);
  if (!inputNode)
    {
    return;
    }
  vtkMRMLMarkupsCurveNode* outputNode = vtkMRMLMarkupsCurveNode::SafeDownCast(d->resampleCurveOutputNodeSelector->currentNode());
  if (!outputNode)
    {
    outputNode = inputNode;
    }
  if(outputNode != inputNode)
    {
    vtkNew<vtkPoints> originalControlPoints;
    inputNode->GetControlPointPositionsWorld(originalControlPoints);
    outputNode->SetControlPointPositionsWorld(originalControlPoints);
    vtkNew<vtkStringArray> originalLabels;
    inputNode->GetControlPointLabels(originalLabels);
    outputNode->SetControlPointLabels(originalLabels, originalControlPoints);
    }
  double sampleDist = outputNode->GetCurveLengthWorld() / (resampleNumberOfPoints - 1);
  vtkMRMLModelNode* constraintNode = vtkMRMLModelNode::SafeDownCast(d->resampleCurveConstraintNodeSelector->currentNode());
  if (constraintNode)
    {
    double maximumSearchRadius = 0.01*d->resampleCurveMaxSearchRadiusSliderWidget->value();
    bool success = outputNode->ResampleCurveSurface(sampleDist, constraintNode, maximumSearchRadius);
    if (!success)
      {
      qWarning("vtkMRMLMarkupsCurveNode::ResampleCurveSurface failed");
      }
    }
  else
    {
    outputNode->ResampleCurveWorld(sampleDist);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSaveToDefaultDisplayPropertiesPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  vtkMRMLMarkupsDisplayNode *displayNode = d->markupsDisplayNode();
  if (!displayNode)
    {
    return;
    }
  // set the display node from the logic defaults
  if (!this->markupsLogic())
    {
    return;
    }
  this->markupsLogic()->SetDisplayDefaultsFromNode(displayNode);

  // also save the settings permanently
  qSlicerMarkupsModule::writeDefaultMarkupsDisplaySettings(this->markupsLogic()->GetDefaultMarkupsDisplayNode());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onCurveTypeParameterChanged()
{
  Q_D(qSlicerMarkupsModuleWidget);
  vtkMRMLMarkupsCurveNode* curveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(d->MarkupsNode);
  if (!curveNode)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(curveNode);
  curveNode->SetCurveType(d->curveTypeComboBox->currentData().toInt());
  curveNode->SetAndObserveShortestDistanceSurfaceNode(vtkMRMLModelNode::SafeDownCast(d->modelNodeSelector->currentNode()));
  std::string functionString = d->scalarFunctionLineEdit->text().toStdString();
  curveNode->SetSurfaceCostFunctionType(d->costFunctionComboBox->currentData().toInt());
  curveNode->SetSurfaceDistanceWeightingFunction(functionString.c_str());
}
