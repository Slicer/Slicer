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
#include <QButtonGroup>
#include <QClipboard>
#include <QDebug>
#include <QInputDialog>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QMouseEvent>
#include <QSettings>
#include <QShortcut>
#include <QSignalMapper>
#include <QStringList>
#include <QTableWidgetItem>
#include <QSharedPointer>
#include <QSpinBox>

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerAbstractCoreModule.h>

// CTK includes
#include "ctkMessageBox.h"

// Slicer includes
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"
#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLUtils.h"
#include "qSlicerApplication.h"

// MRML includes
#include "vtkMRMLColorLegendDisplayNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLTableNode.h"

// MRMLDM includes
#include "vtkMRMLMarkupsDisplayableManager.h"
#include "vtkMRMLMarkupsDisplayableManagerHelper.h"

// Module logic includes
#include <vtkSlicerColorLogic.h>

// Markups includes
#include "qMRMLMarkupsAbstractOptionsWidget.h"
#include "qMRMLMarkupsOptionsWidgetsFactory.h"
#include "qSlicerMarkupsModule.h"
#include "qSlicerMarkupsModuleWidget.h"
#include "ui_qSlicerMarkupsModule.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLMarkupsNode.h"
#include "vtkSlicerMarkupsLogic.h"
#include "qMRMLMarkupsToolBar.h"

// VTK includes
#include <vtkMath.h>
#include <vtkNew.h>
#include "vtkPoints.h"
#include <math.h>

static const int JUMP_MODE_COMBOBOX_INDEX_IGNORE = 0;
static const int JUMP_MODE_COMBOBOX_INDEX_OFFSET = 1;
static const int JUMP_MODE_COMBOBOX_INDEX_CENTERED = 2;
static const char* NAME_PROPERTY = "name";

static const int COORDINATE_COMBOBOX_INDEX_WORLD = 0;
static const int COORDINATE_COMBOBOX_INDEX_LOCAL = 1;
static const int COORDINATE_COMBOBOX_INDEX_HIDE = 2;


//extern qSlicerMarkupsOptionsWidgetsFactory* qSlicerMarkupsOptionsWidgetsFactory::Instance = nullptr;
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

  qMRMLMarkupsToolBar* toolBar();

  /// the number of columns matches the column labels by using the size of the QStringList
  int numberOfColumns();

  vtkMRMLSelectionNode* selectionNode();

  vtkMRMLNode* selectionNodeActivePlaceNode();
  void setSelectionNodeActivePlaceNode(vtkMRMLNode* activePlaceNode);
  void setMRMLMarkupsNodeFromSelectionNode();

  void setPlaceModeEnabled(bool placeEnable);
  bool getPersistanceModeEnabled();
  vtkMRMLMarkupsDisplayNode* markupsDisplayNode();

  // update the markups creation buttons.
  void createMarkupsPushButtons();

  // update the list of markups widgets from qMRMLMarkupsOptionsWidgetsFactory
  void updateMarkupsOptionsWidgets();

  // place the markups options widgets.
  void placeMarkupsOptionsWidgets();

  enum ControlPointColumnsIds
    {
    SelectedColumn = 0,
    LockedColumn,
    VisibleColumn,
    NameColumn,
    DescriptionColumn,
    RColumn,
    AColumn,
    SColumn,
    PositionColumn
    };

private:
  vtkWeakPointer<vtkMRMLMarkupsNode> MarkupsNode;

  QStringList columnLabels;

  QAction*    newMarkupWithCurrentDisplayPropertiesAction;

  QMenu*      visibilityMenu;
  QAction*    visibilityOnAllControlPointsInListAction;
  QAction*    visibilityOffAllControlPointsInListAction;

  QMenu*      selectedMenu;
  QAction*    selectedOnAllControlPointsInListAction;
  QAction*    selectedOffAllControlPointsInListAction;

  QMenu*      lockMenu;
  QAction*    lockAllControlPointsInListAction;
  QAction*    unlockAllControlPointsInListAction;

  QAction*    cutAction;
  QAction*    copyAction;
  QAction*    pasteAction;

  QPixmap     SlicerLockIcon;
  QPixmap     SlicerUnlockIcon;
  QPixmap     SlicerVisibleIcon;
  QPixmap     SlicerInvisibleIcon;

  // Dynamic list of create markups push buttons
  QList<QPushButton*> ceateMarkupsPushButtons;
  unsigned int createMarkupsButtonsColumns;

  // Export/import section
  QButtonGroup* ImportExportOperationButtonGroup;
  QButtonGroup* ImportExportCoordinateSystemButtonGroup;

  // Markups options widgets
  QList<qMRMLMarkupsAbstractOptionsWidget*> MarkupsOptionsWidgets;
};

//-----------------------------------------------------------------------------
// qSlicerMarkupsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidgetPrivate::qSlicerMarkupsModuleWidgetPrivate(qSlicerMarkupsModuleWidget& object)
  : q_ptr(&object)
{
  Q_Q(qSlicerMarkupsModuleWidget);

  this->columnLabels << qSlicerMarkupsModuleWidget::tr("Selected") << qSlicerMarkupsModuleWidget::tr("Locked") << qSlicerMarkupsModuleWidget::tr("Visible")
    << qSlicerMarkupsModuleWidget::tr("Name") << qSlicerMarkupsModuleWidget::tr("Description") << qSlicerMarkupsModuleWidget::tr("R")
    << qSlicerMarkupsModuleWidget::tr("A") << qSlicerMarkupsModuleWidget::tr("S") << qSlicerMarkupsModuleWidget::tr("Position");

  this->newMarkupWithCurrentDisplayPropertiesAction = nullptr;
  this->visibilityMenu = nullptr;
  this->visibilityOnAllControlPointsInListAction = nullptr;
  this->visibilityOffAllControlPointsInListAction = nullptr;

  this->selectedMenu = nullptr;
  this->selectedOnAllControlPointsInListAction = nullptr;
  this->selectedOffAllControlPointsInListAction = nullptr;

  this->lockMenu = nullptr;
  this->lockAllControlPointsInListAction = nullptr;
  this->unlockAllControlPointsInListAction = nullptr;

  this->cutAction = nullptr;
  this->copyAction = nullptr;
  this->pasteAction = nullptr;

  this->SlicerLockIcon = QPixmap(":/Icons/Small/SlicerLock.png");
  this->SlicerUnlockIcon = QPixmap(":/Icons/Small/SlicerUnlock.png");
  this->SlicerVisibleIcon = QPixmap(":/Icons/Small/SlicerVisible.png");
  this->SlicerInvisibleIcon = QPixmap(":/Icons/Small/SlicerInvisible.png");

  this->createMarkupsButtonsColumns = 0;

  this->ImportExportOperationButtonGroup = nullptr;
  this->ImportExportCoordinateSystemButtonGroup = nullptr;

  this->updateMarkupsOptionsWidgets();
}

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidgetPrivate::~qSlicerMarkupsModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerMarkupsModuleWidget);
  this->Ui_qSlicerMarkupsModule::setupUi(widget);

  QStringList registeredMarkups;
  for(const auto& name: q->markupsLogic()->GetRegisteredMarkupsTypes())
    {
    vtkMRMLMarkupsNode* markupsNode = q->markupsLogic()->GetNodeByMarkupsType(name.c_str());
    if (markupsNode)
      {
      registeredMarkups << markupsNode->GetClassName();
      }
    }

  this->activeMarkupTreeView->setNodeTypes(registeredMarkups);
  this->activeMarkupTreeView->setColumnHidden(this->activeMarkupTreeView->model()->idColumn(), true);
  this->activeMarkupTreeView->setColumnHidden(this->activeMarkupTreeView->model()->transformColumn(), true);
  this->activeMarkupTreeView->setColumnHidden(this->activeMarkupTreeView->model()->descriptionColumn(), false);

  // We need to disable the controlPointsCollapsibleButton here because doing so in the .ui file would lead to
  // "child widget is not accessible" warning in debug mode whenever a point is selected in the control point list.
  this->controlPointsCollapsibleButton->setEnabled(false);

  // set up the list buttons
  // visibility
  // first add actions to the menu, then hook them up
  visibilityMenu = new QMenu(qSlicerMarkupsModuleWidget::tr("Visibility"), this->visibilityAllControlPointsInListMenuButton);
  // visibility on
  this->visibilityOnAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/Small/SlicerVisible.png"), "Visibility On", visibilityMenu);
  this->visibilityOnAllControlPointsInListAction->setToolTip("Set visibility flag to on for all control points in the active markup");
  this->visibilityOnAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->visibilityOnAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onVisibilityOnAllControlPointsInListPushButtonClicked()));

  // visibility off
  this->visibilityOffAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/Small/SlicerInvisible.png"), "Visibility Off", visibilityMenu);
  this->visibilityOffAllControlPointsInListAction->setToolTip("Set visibility flag to off for all control points in the active markup");
  this->visibilityOffAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->visibilityOffAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onVisibilityOffAllControlPointsInListPushButtonClicked()));

  this->visibilityMenu->addAction(this->visibilityOnAllControlPointsInListAction);
  this->visibilityMenu->addAction(this->visibilityOffAllControlPointsInListAction);
  this->visibilityAllControlPointsInListMenuButton->setMenu(this->visibilityMenu);
  this->visibilityAllControlPointsInListMenuButton->setIcon(QIcon(":/Icons/VisibleOrInvisible.png"));

  // visibility toggle
  QObject::connect(this->visibilityAllControlPointsInListMenuButton, SIGNAL(clicked()),
                   q, SLOT(onVisibilityAllControlPointsInListToggled()));

  // lock
  // first add actions to the menu, then hook them up
  lockMenu = new QMenu(qSlicerMarkupsModuleWidget::tr("Lock"), this->lockAllControlPointsInListMenuButton);
  // lock
  this->lockAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/Small/SlicerLock.png"), "Lock", lockMenu);
  this->lockAllControlPointsInListAction->setToolTip("Set lock flag to on for all control points in the active markup");
  this->lockAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->lockAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onLockAllControlPointsInListPushButtonClicked()));

  // lock off
  this->unlockAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/Small/SlicerUnlock.png"), "Unlock", lockMenu);
  this->unlockAllControlPointsInListAction->setToolTip("Set lock flag to off for all control points in the active markup");
  this->unlockAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->unlockAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onUnlockAllControlPointsInListPushButtonClicked()));

  this->lockMenu->addAction(this->lockAllControlPointsInListAction);
  this->lockMenu->addAction(this->unlockAllControlPointsInListAction);
  this->lockAllControlPointsInListMenuButton->setMenu(this->lockMenu);
  this->lockAllControlPointsInListMenuButton->setIcon(QIcon(":/Icons/Small/SlicerLockUnlock.png"));

  // lock toggle
  QObject::connect(this->lockAllControlPointsInListMenuButton, SIGNAL(clicked()),
                   q, SLOT(onLockAllControlPointsInListToggled()));

  // selected
  // first add actions to the menu, then hook them up
  selectedMenu = new QMenu(qSlicerMarkupsModuleWidget::tr("Selected"), this->selectedAllControlPointsInListMenuButton);
  // selected on
  this->selectedOnAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/MarkupsSelected.png"), "Selected On", selectedMenu);
  this->selectedOnAllControlPointsInListAction->setToolTip("Set selected flag to on for all control points in the active markup");
  this->selectedOnAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->selectedOnAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onSelectAllControlPointsInListPushButtonClicked()));

  // selected off
  this->selectedOffAllControlPointsInListAction =
    new QAction(QIcon(":/Icons/MarkupsUnselected.png"), "Selected Off", selectedMenu);
  this->selectedOffAllControlPointsInListAction->setToolTip("Set selected flag to off for all control points in the active markup");
  this->selectedOffAllControlPointsInListAction->setCheckable(false);
  QObject::connect(this->selectedOffAllControlPointsInListAction, SIGNAL(triggered()),
                   q, SLOT(onDeselectAllControlPointsInListPushButtonClicked()));

  this->selectedMenu->addAction(this->selectedOnAllControlPointsInListAction);
  this->selectedMenu->addAction(this->selectedOffAllControlPointsInListAction);
  this->selectedAllControlPointsInListMenuButton->setMenu(this->selectedMenu);
  this->selectedAllControlPointsInListMenuButton->setIcon(QIcon(":/Icons/MarkupsSelectedOrUnselected.png"));

  // selected toggle
  QObject::connect(this->selectedAllControlPointsInListMenuButton, SIGNAL(clicked()),
                   q, SLOT(onSelectedAllControlPointsInListToggled()));

  // add
  QObject::connect(this->addControlPointPushButton, SIGNAL(clicked()),
                   q, SLOT(onAddControlPointPushButtonClicked()));
  // move
  QObject::connect(this->moveControlPointUpPushButton, SIGNAL(clicked()),
                   q, SLOT(onMoveControlPointUpPushButtonClicked()));
  QObject::connect(this->moveControlPointDownPushButton, SIGNAL(clicked()),
                   q, SLOT(onMoveControlPointDownPushButtonClicked()));
  // position status
  QObject::connect(this->missingControlPointPushButton, SIGNAL(clicked()),
      q, SLOT(onMissingControlPointPushButtonClicked()));
  QObject::connect(this->unsetControlPointPushButton, SIGNAL(clicked()),
      q, SLOT(onUnsetControlPointPushButtonClicked()));
  // delete
  QObject::connect(this->deleteControlPointPushButton, SIGNAL(clicked()),
                   q, SLOT(onDeleteControlPointPushButtonClicked()));
  QObject::connect(this->deleteAllControlPointsInListPushButton, SIGNAL(clicked()),
      q, SLOT(onDeleteAllControlPointsInListPushButtonClicked()));

  this->cutAction = new QAction(q);
  this->cutAction->setText(qSlicerMarkupsModuleWidget::tr("Cut"));
  this->cutAction->setIcon(QIcon(":Icons/Medium/SlicerEditCut.png"));
  this->cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->cutAction->setShortcuts(QKeySequence::Cut);
  this->cutAction->setToolTip(qSlicerMarkupsModuleWidget::tr("Cut"));
  q->addAction(this->cutAction);
  this->CutControlPointsToolButton->setDefaultAction(this->cutAction);
  QObject::connect(this->cutAction, SIGNAL(triggered()), q, SLOT(cutSelectedToClipboard()));

  this->copyAction = new QAction(q);
  this->copyAction->setText(qSlicerMarkupsModuleWidget::tr("Copy"));
  this->copyAction->setIcon(QIcon(":Icons/Medium/SlicerEditCopy.png"));
  this->copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->copyAction->setShortcuts(QKeySequence::Copy);
  this->copyAction->setToolTip(qSlicerMarkupsModuleWidget::tr("Copy"));
  q->addAction(this->copyAction);
  this->CopyControlPointsToolButton->setDefaultAction(this->copyAction);
  QObject::connect(this->copyAction, SIGNAL(triggered()), q, SLOT(copySelectedToClipboard()));

  this->pasteAction = new QAction(q);
  this->pasteAction->setText(qSlicerMarkupsModuleWidget::tr("Paste"));
  this->pasteAction->setIcon(QIcon(":Icons/Medium/SlicerEditPaste.png"));
  this->pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->pasteAction->setShortcuts(QKeySequence::Paste);
  this->pasteAction->setToolTip(qSlicerMarkupsModuleWidget::tr("Paste"));
  q->addAction(this->pasteAction);
  this->PasteControlPointsToolButton->setDefaultAction(this->pasteAction);
  QObject::connect(this->pasteAction, SIGNAL(triggered()), q, SLOT(pasteSelectedFromClipboard()));

  // set up the active markups node selector
  QObject::connect(this->activeMarkupTreeView, SIGNAL(currentItemChanged(vtkIdType)),
    q, SLOT(onActiveMarkupItemChanged(vtkIdType)));

  // Create the layout for the create markups group box.
  this->createMarkupsPushButtons();

  // update the checked state of showing the slice intersections
  // vtkMRMLApplicationLogic::GetIntersectingSlicesEnabled cannot be called, as the scene
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
  // set up the control point number locked/unlocked button
  //
  QObject::connect(this->fixedNumberOfControlPointsPushButton, SIGNAL(clicked()),
    q, SLOT(onFixedNumberOfControlPointsPushButtonClicked()));

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
  // We do not use QHeaderView::ResizeToContents, as it slows down table updates when there are many control points
  this->activeMarkupTableWidget->horizontalHeader()->setSectionResizeMode(qSlicerMarkupsModuleWidgetPrivate::NameColumn, QHeaderView::Stretch);
  this->activeMarkupTableWidget->horizontalHeader()->setStretchLastSection(false);

  // adjust the column widths
  this->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::NameColumn, 60);
  this->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::DescriptionColumn, 120);
  this->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::RColumn, 65);
  this->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::AColumn, 65);
  this->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::SColumn, 65);

  // show/hide the coordinate columns
  QObject::connect(this->coordinatesComboBox,
                   SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onHideCoordinateColumnsToggled(int)));

  // use an icon for some column headers
  // selected is a check box
  QTableWidgetItem *selectedHeader = this->activeMarkupTableWidget->horizontalHeaderItem(qSlicerMarkupsModuleWidgetPrivate::SelectedColumn);
  selectedHeader->setText("");
  selectedHeader->setIcon(QIcon(":/Icons/MarkupsSelected.png"));
  selectedHeader->setToolTip(QString("Click in this column to select/deselect control points for passing to CLI modules"));
  this->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::SelectedColumn, 30);
  // locked is an open and closed lock
  QTableWidgetItem *lockedHeader = this->activeMarkupTableWidget->horizontalHeaderItem(qSlicerMarkupsModuleWidgetPrivate::LockedColumn);
  lockedHeader->setText("");
  lockedHeader->setIcon(QIcon(":/Icons/Small/SlicerLockUnlock.png"));
  lockedHeader->setToolTip(QString("Click in this column to lock/unlock control points to prevent them from being moved by mistake"));
  this->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::LockedColumn, 30);
  // visible is an open and closed eye
  QTableWidgetItem *visibleHeader = this->activeMarkupTableWidget->horizontalHeaderItem(qSlicerMarkupsModuleWidgetPrivate::VisibleColumn);
  visibleHeader->setText("");
  visibleHeader->setIcon(QIcon(":/Icons/Small/SlicerVisibleInvisible.png"));
  visibleHeader->setToolTip(QString("Click in this column to show/hide control points in 2D and 3D"));
  this->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::VisibleColumn, 30);
  // position is a location bubble
  QTableWidgetItem *positionHeader = this->activeMarkupTableWidget->horizontalHeaderItem(qSlicerMarkupsModuleWidgetPrivate::PositionColumn);
  positionHeader->setText("");
  positionHeader->setIcon(QIcon(":/Icons/Large/MarkupsPositionStatus.png"));
  positionHeader->setToolTip(QString("Click in this column to modify the control point position state.\n\n"
                                     "- Edit: Enter place mode to modify the control point position in the slice views\n"
                                     "- Skip: 'Place multiple control points' mode skips over the control point entry\n"
                                     "- Restore: Set the control point position to its last known set position\n"
                                     "- Clear: Clear the defined control point position, but do not delete the control point"));
  this->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::PositionColumn, 10);

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

  // Place the options widgets
  this->placeMarkupsOptionsWidgets();
  QObject::connect(qMRMLMarkupsOptionsWidgetsFactory::instance(), SIGNAL(optionsWidgetRegistered()),
                   q, SLOT(onUpdateMarkupsOptionsWidgets()));
  QObject::connect(qMRMLMarkupsOptionsWidgetsFactory::instance(), SIGNAL(optionsWidgetUnregistered()),
                   q, SLOT(onUpdateMarkupsOptionsWidgets()));

  // hide measurement settings table until markups node containing measurement is set
  this->measurementSettingsTableWidget->setVisible(false);

  // Export/import
  this->ImportExportOperationButtonGroup = new QButtonGroup(this->exportImportCollapsibleButton);
  this->ImportExportOperationButtonGroup->addButton(this->tableExportRadioButton);
  this->ImportExportOperationButtonGroup->addButton(this->tableImportRadioButton);

  this->ImportExportCoordinateSystemButtonGroup = new QButtonGroup(this->exportImportCollapsibleButton);
  this->ImportExportCoordinateSystemButtonGroup->addButton(this->lpsExportRadioButton);
  this->ImportExportCoordinateSystemButtonGroup->addButton(this->rasExportRadioButton);

  this->tableExportRadioButton->setChecked(true);
  this->rasExportRadioButton->setChecked(true);

  QObject::connect(this->exportedImportedNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(updateImportExportWidgets()));

  QObject::connect(this->ImportExportOperationButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
    q, SLOT(updateImportExportWidgets()));

  QObject::connect(this->exportImportPushButton, SIGNAL(clicked()),
    q, SLOT(onImportExportApply()));

  q->updateImportExportWidgets();

  QObject::connect(this->ColorLegendCollapsibleGroupBox, SIGNAL(toggled(bool)),
    q, SLOT(onColorLegendCollapsibleGroupBoxToggled(bool)));

  // Add event observers for registration/unregistration of markups
  q->qvtkConnect(q->markupsLogic(), vtkSlicerMarkupsLogic::MarkupRegistered,
    q, SLOT(onCreateMarkupsPushButtons()));
  q->qvtkConnect(q->markupsLogic(), vtkSlicerMarkupsLogic::MarkupUnregistered,
    q, SLOT(onCreateMarkupsPushButtons()));
}

//-----------------------------------------------------------------------------
qMRMLMarkupsToolBar* qSlicerMarkupsModuleWidgetPrivate::toolBar()
{
  Q_Q(const qSlicerMarkupsModuleWidget);
  qSlicerMarkupsModule* module = dynamic_cast<qSlicerMarkupsModule*>(q->module());
  if (!module)
    {
    qWarning("qSlicerMarkupsModuleWidget::toolBar failed: module is not set");
    return nullptr;
    }
  return module->toolBar();
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
void qSlicerMarkupsModuleWidgetPrivate::setSelectionNodeActivePlaceNode(vtkMRMLNode* node)
{
  Q_Q(qSlicerMarkupsModuleWidget);
  if (!q->markupsLogic())
    {
    return;
    }
  vtkMRMLMarkupsNode* activePlaceNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  q->markupsLogic()->SetActiveList(activePlaceNode);
  q->updateToolBar(activePlaceNode);
  q->updateWidgetFromMRML();
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
  q->updateToolBar(currentMarkupsNode);
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
bool qSlicerMarkupsModuleWidgetPrivate::getPersistanceModeEnabled()
{
  Q_Q(qSlicerMarkupsModuleWidget);
  vtkMRMLInteractionNode* interactionNode = nullptr;
  if (q->mrmlScene())
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(q->mrmlScene()->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
    }
  if (interactionNode && interactionNode->GetPlaceModePersistence())
    {
    return true;
    }
  else
    {
    return false;
    }
}

//-----------------------------------------------------------
void qSlicerMarkupsModuleWidgetPrivate::updateMarkupsOptionsWidgets()
{
  foreach(auto widget, this->MarkupsOptionsWidgets)
    {
    widget->setParent(nullptr);
    }

  this->MarkupsOptionsWidgets.clear();

  // Create the markups options widgets registered in qMRMLMarkupsOptionsWidgetsFactory.
  auto factory = qMRMLMarkupsOptionsWidgetsFactory::instance();
  foreach(const auto& widgetClassName, factory->registeredOptionsWidgetsClassNames())
    {
    this->MarkupsOptionsWidgets.append(factory->createWidget(widgetClassName));
    }
}

//-----------------------------------------------------------
void qSlicerMarkupsModuleWidgetPrivate::placeMarkupsOptionsWidgets()
{
  Q_Q(qSlicerMarkupsModuleWidget);

  // Add the options widgets
  foreach(const auto& widget, this->MarkupsOptionsWidgets)
    {
    // If the parent is different from the qSlicerMarkupsModule widget, then add the widget.
    if (widget->parentWidget() != q)
      {
      this->markupsModuleVerticalLayout->addWidget(widget);
      widget->setVisible(false);
      }

      // Forward the mrmlSceneChanged signal
      QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                       widget, SLOT(setMRMLScene(vtkMRMLScene*)));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidgetPrivate::createMarkupsPushButtons()
{
  Q_Q(qSlicerMarkupsModuleWidget);

  QGridLayout *layout= new QGridLayout();

  vtkMRMLApplicationLogic* appLogic = q->appLogic();
  if (!appLogic)
    {
      qCritical() << Q_FUNC_INFO << "createMarkupsPushButtons: invalid application logic.";
      return;
    }

  vtkSlicerMarkupsLogic* markupsLogic =
    vtkSlicerMarkupsLogic::SafeDownCast(appLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    qCritical() << Q_FUNC_INFO << "createMarkupsPushButtons: invalid application logic.";
    return;
    }

  unsigned int i=0;

  for(const auto markupName: q->markupsLogic()->GetRegisteredMarkupsTypes())
    {
    vtkMRMLMarkupsNode* markupsNode =
      markupsLogic->GetNodeByMarkupsType(markupName.c_str());

    // Create markups add buttons.
    if (markupsNode && q->markupsLogic()->GetCreateMarkupsPushButton(markupName.c_str()))
      {
      QSignalMapper* mapper = new QSignalMapper(q);
      QPushButton *markupCreatePushButton = new QPushButton();
      //NOTE: We assign object name so we can test the dynamic creation of buttons in the tests.
      markupCreatePushButton->setObjectName(QString("Create") +
                                            QString(markupsNode->GetMarkupType()) +
                                            QString("PushButton"));
      markupCreatePushButton->setIcon(QIcon(markupsNode->GetPlaceAddIcon()));
      markupCreatePushButton->setToolTip(QString("Create ") +
                                         QString(markupsNode->GetTypeDisplayName()));
      markupCreatePushButton->setText(QString(markupsNode->GetTypeDisplayName()));
      layout->addWidget(markupCreatePushButton,
                        i / this->createMarkupsButtonsColumns,
                        i % this->createMarkupsButtonsColumns);

      QObject::connect(markupCreatePushButton, SIGNAL(clicked()), mapper, SLOT(map()));
      mapper->setMapping(markupCreatePushButton, markupsNode->GetClassName());
      QObject::connect(mapper, SIGNAL(mapped(const QString&)),
                       q, SLOT(onCreateMarkupByClass(const QString&)));

      // NOTE: Alternative connection using lambdas instead of QSignalMapper
      // QObject::connect(markupCreatePushButton, &QPushButton::clicked,
      //                  q, [q, markupsNode] {q->onCreateMarkupByClass(markupsNode->GetClassName());});

      ++i;
      }
    }

  // NOTE: this is a temporary widget to reparent the former layout (so it will get destroyed)
  if (createMarkupsGroupBox->layout())
    {
    QWidget tempWidget;
    tempWidget.setLayout(createMarkupsGroupBox->layout());
    }

  this->createMarkupsGroupBox->setLayout(layout);
}

//-----------------------------------------------------------------------------
// qSlicerMarkupsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidget::qSlicerMarkupsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
    , d_ptr( new qSlicerMarkupsModuleWidgetPrivate(*this) )
{
  this->volumeSpacingScaleFactor = 10.0;
}

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidget::~qSlicerMarkupsModuleWidget()
{
  Q_D(qSlicerMarkupsModuleWidget);
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
  // Toolbar
  qMRMLMarkupsToolBar* toolBar = d->toolBar();
  if (toolBar)
    {
    connect(toolBar, SIGNAL(activeMarkupsNodeChanged(vtkMRMLNode*)), this, SLOT(onActiveMarkupMRMLNodeChanged(vtkMRMLNode*)));
    }

  // Add event observers to MarkupsNode
  if (d->MarkupsNode)
    {
    vtkMRMLMarkupsNode* markupsNode = d->MarkupsNode;
    d->MarkupsNode = nullptr; // this will force a reset
    this->setMRMLMarkupsNode(markupsNode);
    vtkIdType itemID = d->activeMarkupTreeView->subjectHierarchyNode()->GetItemByDataNode(markupsNode);
    QModelIndex itemIndex = d->activeMarkupTreeView->sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
    if(itemIndex.row()>=0)
      {
      d->activeMarkupTreeView->scrollTo(itemIndex);
      d->activeMarkupTreeView->setCurrentNode(markupsNode);
      }
    }

  // check the max scales against volume spacing, they might need to be updated
  this->updateMaximumScaleFromVolumes();
  this->enableMarkupTableButtons(d->MarkupsNode ? 1 : 0);

}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::checkForAnnotationFiducialConversion()
{
  // check to see if there are any annotation fiducial list nodes
  // and offer to import them as markups
  int numFids = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLAnnotationFiducialNode");
  int numSceneViews = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLSceneViewNode");
  if (numFids > 0)
    {
    ctkMessageBox convertMsgBox;
    convertMsgBox.setWindowTitle("Convert Annotation hierarchies to Markups point list nodes?");
    QString labelText = QString("Convert ")
      + QString::number(numFids)
      + QString(" Annotation fiducial lists to Markups point list nodes?")
      + QString(" Moves all Annotation fiducial lists out of hierarchies (deletes")
      + QString(" the nodes, but leaves the hierarchies in case rulers or")
      + QString(" ROIs are mixed in) and into Markups point list nodes.");
    if (numSceneViews > 0)
      {
      labelText += QString(" Iterates through ")
        + QString::number(numSceneViews)
        + QString(" Scene Views and converts any fiducial lists saved in those")
        + QString(" scenes into Markups point list nodes as well.");
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

  // remove mrml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();

  // remove observations from measurements
  Q_D(qSlicerMarkupsModuleWidget);
  if (d->MarkupsNode)
    {
    for (int i=0; i<d->MarkupsNode->Measurements->GetNumberOfItems(); ++i)
      {
      vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(
        d->MarkupsNode->Measurements->GetItemAsObject(i) );
      if (currentMeasurement)
        {
        qvtkDisconnect(currentMeasurement, vtkCommand::ModifiedEvent, this, SLOT(onMeasurementModified()));
        }
      }
    qvtkDisconnect(d->MarkupsNode->Measurements, vtkCommand::ModifiedEvent,
      this, SLOT(onMeasurementsCollectionModified()));
    }
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
  if (d->MarkupsNode)
    {
    vtkIdType itemID = d->activeMarkupTreeView->subjectHierarchyNode()->GetItemByDataNode(d->MarkupsNode);
    QModelIndex itemIndex = d->activeMarkupTreeView->sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
    if (itemIndex.row() >= 0)
      {
      d->activeMarkupTreeView->scrollTo(itemIndex);
      }
    }
  d->activeMarkupTreeView->setCurrentNode(d->MarkupsNode);
  d->activeMarkupTreeView->blockSignals(wasBlocked);
  d->markupsDisplayWidget->setMRMLMarkupsNode(d->MarkupsNode);

  // Color legend
  vtkMRMLColorLegendDisplayNode* colorLegendNode = nullptr;
  vtkMRMLDisplayNode* displayNode = d->markupsDisplayWidget->mrmlMarkupsDisplayNode();
  colorLegendNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode);
  d->ColorLegendDisplayNodeWidget->setMRMLColorLegendDisplayNode(colorLegendNode);

  d->ColorLegendCollapsibleGroupBox->setCollapsed(!colorLegendNode);
  d->ColorLegendCollapsibleGroupBox->setEnabled(displayNode && displayNode->GetColorNode());

  if (!d->MarkupsNode)
    {
    d->activeMarkupTableWidget->clearContents();
    d->activeMarkupTableWidget->setRowCount(0);
    return;
    }

  if (d->MarkupsNode->GetLocked())
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/SlicerLock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to unlock this control point list so points can be moved by the mouse"));
    }
  else
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/SlicerUnlock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to lock this control point list so points cannot be moved by the mouse"));
    }

  if (d->MarkupsNode->GetFixedNumberOfControlPoints())
    {
    d->fixedNumberOfControlPointsPushButton->setIcon(QIcon(":Icons/Medium/SlicerPointNumberLock.png"));
    d->fixedNumberOfControlPointsPushButton->setToolTip(QString("Click to unlock the number of control points so points can be added or deleted"));
    d->deleteControlPointPushButton->setEnabled(false);
    d->deleteAllControlPointsInListPushButton->setEnabled(false);
    }
  else
    {
    d->fixedNumberOfControlPointsPushButton->setIcon(QIcon(":Icons/Medium/SlicerPointNumberUnlock.png"));
    d->fixedNumberOfControlPointsPushButton->setToolTip(QString("Click to lock the number of control points so no points can be added or deleted"));
    d->deleteControlPointPushButton->setEnabled(true);
    d->deleteAllControlPointsInListPushButton->setEnabled(true);
    }
  // update slice intersections
  d->sliceIntersectionsVisibilityCheckBox->setChecked(this->sliceIntersectionsVisible());

  // update the list name format
  QString nameFormat = QString(d->MarkupsNode->GetControlPointLabelFormat().c_str());
  d->nameFormatLineEdit->setText(nameFormat);

   // update the table
  int numberOfPoints = d->MarkupsNode->GetNumberOfControlPoints();
  if (d->activeMarkupTableWidget->rowCount() != numberOfPoints)
    {
    // force full update of the table
    // (after node change or batch update with multiple rows added or deleted)
    this->updateRows();
    }
  // Update options widgets
  foreach(const auto& widget, d->MarkupsOptionsWidgets)
    {
    widget->updateWidgetFromMRML();
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
void qSlicerMarkupsModuleWidget::updateRows()
{
  Q_D(qSlicerMarkupsModuleWidget);
  vtkMRMLMarkupsNode* markupsNode = this->mrmlMarkupsNode();
  if (!markupsNode)
    {
    return;
    }

  int numberOfPoints = d->MarkupsNode->GetNumberOfControlPoints();
  if (d->activeMarkupTableWidget->rowCount() != numberOfPoints)
    {
    d->activeMarkupTableWidget->setRowCount(numberOfPoints);
    }
  for (int m = 0; m < numberOfPoints; m++)
    {
    this->updateRow(m);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateRow(int controlPointIndex)
{
  Q_D(qSlicerMarkupsModuleWidget);
  vtkMRMLMarkupsNode* markupsNode = this->mrmlMarkupsNode();
  if (!markupsNode
    || controlPointIndex >= markupsNode->GetNumberOfControlPoints()) // markup point is already deleted (possible after batch update)
    {
    return;
    }

  if (controlPointIndex < 0)
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid controlPointIndex";
    return;
    }

  // this is updating the qt widget from MRML, and should not trigger any updates on the node, so turn off events
  QSignalBlocker blocker(d->activeMarkupTableWidget);

  // selected
  int column = qSlicerMarkupsModuleWidgetPrivate::SelectedColumn;
  QTableWidgetItem* item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  bool isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    // disable editing so that a double click won't bring up an entry box
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    isNewItem = true;
    }
  Qt::CheckState selectedState = (markupsNode->GetNthControlPointSelected(controlPointIndex) ? Qt::Checked : Qt::Unchecked);
  if (isNewItem || item->checkState() != selectedState)
    {
    item->setCheckState(selectedState);
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }

  // locked
  column = qSlicerMarkupsModuleWidgetPrivate::LockedColumn;
  item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    // disable checkable
    item->setData(Qt::CheckStateRole, QVariant());
    item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
    // disable editing so that a double click won't bring up an entry box
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    isNewItem = true;
    }
  bool locked = markupsNode->GetNthControlPointLocked(controlPointIndex);
  if (isNewItem || item->data(Qt::UserRole).toBool() != locked)
    {
    item->setData(Qt::UserRole, QVariant(locked));
    item->setData(Qt::DecorationRole, locked ? d->SlicerLockIcon : d->SlicerUnlockIcon);
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }

  // visible
  column = qSlicerMarkupsModuleWidgetPrivate::VisibleColumn;
  item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    // disable checkable
    item->setData(Qt::CheckStateRole, QVariant());
    item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
    // disable editing so that a double click won't bring up an entry box
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    isNewItem = true;
    }
  bool visible = markupsNode->GetNthControlPointVisibility(controlPointIndex);
  if (isNewItem || item->data(Qt::UserRole).toBool() != visible)
    {
    item->setData(Qt::UserRole, QVariant(visible));
    item->setData(Qt::DecorationRole, visible ? d->SlicerVisibleIcon : d->SlicerInvisibleIcon);
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }

  // name
  column = qSlicerMarkupsModuleWidgetPrivate::NameColumn;
  item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    isNewItem = true;
    }
  QString label = QString::fromStdString(markupsNode->GetNthControlPointLabel(controlPointIndex));
  if (isNewItem || item->text() != label)
    {
    item->setText(label);
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }

  // description
  column = qSlicerMarkupsModuleWidgetPrivate::DescriptionColumn;
  item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    isNewItem = true;
    }
  QString description = QString::fromStdString(markupsNode->GetNthControlPointDescription(controlPointIndex));
  if (isNewItem || item->text() != description)
    {
    item->setText(description);
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }

   // coordinates
   double point[3] = {0.0, 0.0, 0.0};
   if (d->coordinatesComboBox->currentIndex() == COORDINATE_COMBOBOX_INDEX_WORLD)
     {
     double worldPoint[4] = {0.0, 0.0, 0.0, 1.0};
     markupsNode->GetNthControlPointPositionWorld(controlPointIndex, worldPoint);
     for (int p = 0; p < 3; ++p)
       {
       point[p] = worldPoint[p];
       }
     }
   else
     {
     markupsNode->GetNthControlPointPosition(controlPointIndex, point);
     }
  int rColumnIndex = qSlicerMarkupsModuleWidgetPrivate::RColumn;
  int mPositionStatus = markupsNode->GetNthControlPointPositionStatus(controlPointIndex);
  bool showCoordinates = (mPositionStatus == vtkMRMLMarkupsNode::PositionDefined ||
    mPositionStatus == vtkMRMLMarkupsNode::PositionPreview);
  for (int p = 0; p < 3; p++)
    {
    column = rColumnIndex + p;
    item = d->activeMarkupTableWidget->item(controlPointIndex, column);
    isNewItem = false;
    if (!item)
      {
      item = new QTableWidgetItem();
      isNewItem = true;
      }
    QString coordinate;
    if (showCoordinates)
      {
      // last argument to number sets the precision
      coordinate = QString::number(point[p], 'f', 3);
      }
    if (isNewItem || item->text() != coordinate)
      {
      item->setText(coordinate);
      }
    if (isNewItem)
      {
      d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
      }
    }

  // position status
  column = qSlicerMarkupsModuleWidgetPrivate::PositionColumn;
  item = d->activeMarkupTableWidget->item(controlPointIndex, column);
  isNewItem = false;
  if (!item)
    {
    item = new QTableWidgetItem();
    // disable editing so that a double click won't bring up an entry box
    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    isNewItem = true;
    }
  int positionStatus = markupsNode->GetNthControlPointPositionStatus(controlPointIndex);
  if (isNewItem
    || item->data(Qt::UserRole).toInt() != positionStatus)
    {
    item->setData(Qt::UserRole, positionStatus);
    switch (positionStatus)
      {
      case vtkMRMLMarkupsNode::PositionDefined: item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsDefined.png")); break;
      case vtkMRMLMarkupsNode::PositionPreview: item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsInProgress.png")); break;
      case vtkMRMLMarkupsNode::PositionMissing: item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsMissing.png")); break;
      case vtkMRMLMarkupsNode::PositionUndefined: item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsUndefined.png")); break;
      }
    }
  if (isNewItem)
    {
    d->activeMarkupTableWidget->setItem(controlPointIndex, column, item);
    }
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
  if (!markupsNode)
    {
      return;
    }

  // make it active
  d->activeMarkupTreeView->setCurrentNode(markupsNode);
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
  QMouseEvent click(QEvent::MouseButtonRelease, widgetPos, Qt::LeftButton, Qt::MouseButtons(), Qt::KeyboardModifiers());
  click.setAccepted(true);

  // and send it to the widget
  //qDebug() << "onPKeyActivated: sending event with pos " << widgetPos;
  QCoreApplication::sendEvent(widget, &click);
}


//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityOnAllControlPointsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsVisibility(d->MarkupsNode, true);
  d->MarkupsNode->SetDisplayVisibility(true);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityOffAllControlPointsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsVisibility(d->MarkupsNode, false);
  d->MarkupsNode->SetDisplayVisibility(false);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityAllControlPointsInListToggled()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->ToggleAllControlPointsVisibility(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onLockAllControlPointsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsLocked(d->MarkupsNode, true);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onUnlockAllControlPointsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsLocked(d->MarkupsNode, false);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onLockAllControlPointsInListToggled()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->ToggleAllControlPointsLocked(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectAllControlPointsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsSelected(d->MarkupsNode, true);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeselectAllControlPointsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->SetAllControlPointsSelected(d->MarkupsNode, false);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectedAllControlPointsInListToggled()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic() || !d->MarkupsNode)
    {
    return;
    }
  this->markupsLogic()->ToggleAllControlPointsSelected(d->MarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onAddControlPointPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  if (d->MarkupsNode->GetFixedNumberOfControlPoints())
    {
    return;
    }

  if (d->MarkupsNode->GetMaximumNumberOfControlPoints() >= 0
    && (d->MarkupsNode->GetNumberOfControlPoints() >= d->MarkupsNode->GetMaximumNumberOfControlPoints()) )
    {
    return;
    }

  int index = d->MarkupsNode->AddControlPoint(vtkVector3d(0,0,0));
  d->MarkupsNode->UnsetNthControlPointPosition(index);
  d->setPlaceModeEnabled(false);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMoveControlPointUpPushButtonClicked()
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
void qSlicerMarkupsModuleWidget::onMoveControlPointDownPushButtonClicked()
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
void qSlicerMarkupsModuleWidget::onDeleteControlPointPushButtonClicked(bool confirm /*=true*/)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem*> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  QModelIndexList selectedIndexes = d->activeMarkupTableWidget->selectionModel()->selectedRows();
  for (int i = 0; i < selectedIndexes.size(); i++)
    {
    // get the row
    int row = selectedIndexes[i].row();
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());
  if (confirm)
    {
    ctkMessageBox deleteAllMsgBox;
    deleteAllMsgBox.setWindowTitle("Delete control points in this list?");
    QString labelText = QString("Delete ")
      + QString::number(rows.size())
        + QString(" control points from this list?");
    // don't show again check box conflicts with informative text, so use
    // a long text
    deleteAllMsgBox.setText(labelText);

    QPushButton* deleteButton =
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
void qSlicerMarkupsModuleWidget::onResetControlPointPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem*> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  QModelIndexList selectedIndexes = d->activeMarkupTableWidget->selectionModel()->selectedRows();
  for (int i = 0; i < selectedIndexes.size(); i++)
    {
    // get the row
    int row = selectedIndexes[i].row();
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

  // unplace from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    d->MarkupsNode->ResetNthControlPointPosition(index);
    d->setPlaceModeEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onRestoreControlPointPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem*> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  QModelIndexList selectedIndexes = d->activeMarkupTableWidget->selectionModel()->selectedRows();
  for (int i = 0; i < selectedIndexes.size(); i++)
    {
    // get the row
    int row = selectedIndexes[i].row();
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

  // unplace from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    d->MarkupsNode->RestoreNthControlPointPosition(index);
    }
}
//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onUnsetControlPointPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem*> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  QModelIndexList selectedIndexes = d->activeMarkupTableWidget->selectionModel()->selectedRows();
  for (int i = 0; i < selectedIndexes.size(); i++)
    {
    // get the row
    int row = selectedIndexes[i].row();
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

  // unplace from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    d->MarkupsNode->UnsetNthControlPointPosition(index);
    }
}
//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMissingControlPointPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // get the selected rows
  QList<QTableWidgetItem*> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // iterate over the selected items and save their row numbers (there are
  // selected indices for each column in a row, so jump by the number of
  // columns), so can delete without relying on the table
  QList<int> rows;
  QModelIndexList selectedIndexes = d->activeMarkupTableWidget->selectionModel()->selectedRows();
  for (int i = 0; i < selectedIndexes.size(); i++)
    {
    // get the row
    int row = selectedIndexes[i].row();
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

  // unplace from the end
  for (int i = rows.size() - 1; i >= 0; --i)
    {
    int index = rows.at(i);
    if (d->MarkupsNode->GetNthControlPointPositionStatus(index) == vtkMRMLMarkupsNode::PositionMissing)
      {
      d->MarkupsNode->UnsetNthControlPointPosition(index);
      }
    else
      {
      d->MarkupsNode->SetNthControlPointPositionMissing(index);
      }
  }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeleteAllControlPointsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  ctkMessageBox deleteAllMsgBox;
  deleteAllMsgBox.setWindowTitle("Delete all control points in this list?");
  QString labelText = QString("Delete all ")
    + QString::number(d->MarkupsNode->GetNumberOfControlPoints())
    + QString(" control points in this list?");
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
void qSlicerMarkupsModuleWidget::onActiveMarkupMRMLNodeAdded(vtkMRMLNode * node)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (this->markupsLogic())
    {
    this->markupsLogic()->AddNewDisplayNodeForMarkupsNode(node);
    }
  // make sure it's set up for the mouse mode tool bar to easily add points to
  // it by making it active in the selection node
  d->setSelectionNodeActivePlaceNode(node);
  d->setPlaceModeEnabled(true);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupMRMLNodeChanged(vtkMRMLNode* node)
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
  this->enableMarkupTableButtons(markupsNode ? 1 : 0);
  this->setMRMLMarkupsNode(markupsNode);
}

//------------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onColorLegendCollapsibleGroupBoxToggled(bool toggled)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // Make sure a legend display node exists if the color legend section is opened
  if (!toggled)
    {
    return;
    }

  vtkMRMLDisplayNode* displayNode = d->markupsDisplayWidget->mrmlMarkupsDisplayNode();
  vtkMRMLColorLegendDisplayNode* colorLegendNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode);
  if (!colorLegendNode)
    {
    // color legend node does not exist, we need to create it now

    // Pause render to prevent the new Color legend displayed for a moment before it is hidden.
    vtkMRMLApplicationLogic* mrmlAppLogic = this->logic()->GetMRMLApplicationLogic();
    if (mrmlAppLogic)
      {
      mrmlAppLogic->PauseRender();
      }
    colorLegendNode = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(displayNode);
    colorLegendNode->SetVisibility(false); // just because the groupbox is opened, don't show color legend yet
    if (mrmlAppLogic)
      {
      mrmlAppLogic->ResumeRender();
      }
    }
  d->ColorLegendDisplayNodeWidget->setMRMLColorLegendDisplayNode(colorLegendNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::enableMarkupTableButtons(bool enable)
{
  Q_D(qSlicerMarkupsModuleWidget);

  d->displayCollapsibleButton->setEnabled(enable);
  d->controlPointsCollapsibleButton->setEnabled(enable);
  d->measurementsCollapsibleButton->setEnabled(enable);
  d->exportImportCollapsibleButton->setEnabled(enable);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onCreateMarkupByClass(const QString& className)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic())
    {
    qWarning() << Q_FUNC_INFO << " failed: invalid markups logic";
    return;
    }
  vtkMRMLMarkupsNode* markupsNode = this->markupsLogic()->AddNewMarkupsNode(className.toStdString());
  if (markupsNode)
    {
    this->onActiveMarkupMRMLNodeAdded(markupsNode);
    }
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
  bool visibleFlag = d->MarkupsNode->GetDisplayVisibility();
  visibleFlag = !visibleFlag;
  d->MarkupsNode->SetDisplayVisibility(visibleFlag);

  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllControlPointsVisibility(d->MarkupsNode, visibleFlag);
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
  bool locked = d->MarkupsNode->GetLocked();
  d->MarkupsNode->SetLocked(!locked);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onFixedNumberOfControlPointsPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  d->MarkupsNode->SetFixedNumberOfControlPoints(!d->MarkupsNode->GetFixedNumberOfControlPoints());

// end point placement for locked node
  d->setPlaceModeEnabled(false);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNameFormatLineEditTextEdited(const QString text)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }
  d->MarkupsNode->SetControlPointLabelFormat(std::string(text.toUtf8()));
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
    defaultNode = vtkSmartPointer<vtkMRMLMarkupsNode>::Take(vtkMRMLMarkupsNode::SafeDownCast(
      this->mrmlScene()->CreateNodeByClass(d->MarkupsNode->GetClassName())));
    }
  if (!defaultNode)
    {
    qCritical() << Q_FUNC_INFO << " failed: invalid default markups node";
    }
  d->MarkupsNode->SetControlPointLabelFormat(defaultNode->GetControlPointLabelFormat());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onRenameAllWithCurrentNameFormatPushButtonClicked()
{
   Q_D(qSlicerMarkupsModuleWidget);
   if (!d->MarkupsNode || !this->markupsLogic())
     {
     return;
     }
   this->markupsLogic()->RenameAllControlPointsFromCurrentFormat(d->MarkupsNode);
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
  if (column == qSlicerMarkupsModuleWidgetPrivate::SelectedColumn)
    {
    bool flag = (item->checkState() == Qt::Unchecked ? false : true);
    d->MarkupsNode->SetNthControlPointSelected(n, flag);
    }
  else if (column == qSlicerMarkupsModuleWidgetPrivate::LockedColumn)
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
  else if (column == qSlicerMarkupsModuleWidgetPrivate::VisibleColumn)
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
  else if (column == qSlicerMarkupsModuleWidgetPrivate::NameColumn)
    {
    std::string name = std::string(item->text().toUtf8());
    d->MarkupsNode->SetNthControlPointLabel(n, name);
    }
  else if (column ==  qSlicerMarkupsModuleWidgetPrivate::DescriptionColumn)
    {
    std::string description = std::string(item->text().toUtf8());
    d->MarkupsNode->SetNthControlPointDescription(n, description);
    }
  else if (column == qSlicerMarkupsModuleWidgetPrivate::RColumn ||
           column == qSlicerMarkupsModuleWidgetPrivate::AColumn ||
           column == qSlicerMarkupsModuleWidgetPrivate::SColumn)
    {
    // get the new value
    double newPoint[3] = {0.0, 0.0, 0.0};
    if (d->activeMarkupTableWidget->item(row, qSlicerMarkupsModuleWidgetPrivate::RColumn) == nullptr ||
        d->activeMarkupTableWidget->item(row, qSlicerMarkupsModuleWidgetPrivate::AColumn) == nullptr ||
        d->activeMarkupTableWidget->item(row, qSlicerMarkupsModuleWidgetPrivate::SColumn) == nullptr)
      {
      // init state, return
      return;
      }
    newPoint[0] = d->activeMarkupTableWidget->item(row, qSlicerMarkupsModuleWidgetPrivate::RColumn)->text().toDouble();
    newPoint[1] = d->activeMarkupTableWidget->item(row, qSlicerMarkupsModuleWidgetPrivate::AColumn)->text().toDouble();
    newPoint[2] = d->activeMarkupTableWidget->item(row, qSlicerMarkupsModuleWidgetPrivate::SColumn)->text().toDouble();

    // get the old value
    double point[3] = {0.0, 0.0, 0.0};
    if (d->coordinatesComboBox->currentIndex() == COORDINATE_COMBOBOX_INDEX_WORLD)
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
      if (d->coordinatesComboBox->currentIndex() == COORDINATE_COMBOBOX_INDEX_WORLD)
        {
        d->MarkupsNode->SetNthControlPointPositionWorld(n, newPoint[0], newPoint[1], newPoint[2]);
        }
      else
        {
        d->MarkupsNode->SetNthControlPointPosition(n, newPoint);
        }
      }
    else
      {
      //qDebug() << QString("Cell changed: no change in location bigger than ") + QString::number(minChange);
      }
    }
  else if (column == qSlicerMarkupsModuleWidgetPrivate::PositionColumn)
    {
    bool persistenceModeEnabled = d->getPersistanceModeEnabled();
    if (item->data(Qt::UserRole) == QVariant(vtkMRMLMarkupsNode::PositionUndefined))
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsUndefined.png"));
      d->MarkupsNode->UnsetNthControlPointPosition(row);
      if (!persistenceModeEnabled)
        {
        d->setPlaceModeEnabled(false);
        }
      }
    else if (item->data(Qt::UserRole) == QVariant(vtkMRMLMarkupsNode::PositionPreview))
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsInProgress.png"));
      d->MarkupsNode->ResetNthControlPointPosition(row);
      d->setPlaceModeEnabled(true);
      }
    else if (item->data(Qt::UserRole) == QVariant(vtkMRMLMarkupsNode::PositionMissing))
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsMissing.png"));
      d->MarkupsNode->SetNthControlPointPositionMissing(row);
      if (!persistenceModeEnabled)
        {
        d->setPlaceModeEnabled(false);
        }
      }
    else if (item->data(Qt::UserRole) == QVariant(vtkMRMLMarkupsNode::PositionDefined))
      {
      item->setData(Qt::DecorationRole, QPixmap(":/Icons/XSmall/MarkupsDefined.png"));
      d->MarkupsNode->RestoreNthControlPointPosition(row);
      if (!persistenceModeEnabled)
        {
        d->setPlaceModeEnabled(false);
        }
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
  int row = item->row();
  if (column == qSlicerMarkupsModuleWidgetPrivate::VisibleColumn ||
    column == qSlicerMarkupsModuleWidgetPrivate::LockedColumn)
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
  else if (column == qSlicerMarkupsModuleWidgetPrivate::PositionColumn)
    {
      if (item->data(Qt::UserRole) == QVariant(vtkMRMLMarkupsNode::PositionDefined))
        {
        item->setData(Qt::UserRole, QVariant(vtkMRMLMarkupsNode::PositionUndefined));
        }
      else if (item->data(Qt::UserRole) == QVariant(vtkMRMLMarkupsNode::PositionUndefined))
        {
        item->setData(Qt::UserRole, QVariant(vtkMRMLMarkupsNode::PositionPreview));
        }
      else if (item->data(Qt::UserRole) == QVariant(vtkMRMLMarkupsNode::PositionPreview))
        {
        item->setData(Qt::UserRole, QVariant(vtkMRMLMarkupsNode::PositionMissing));
        }
      else if (item->data(Qt::UserRole) == QVariant(vtkMRMLMarkupsNode::PositionMissing))
        {
        item->setData(Qt::UserRole, QVariant(vtkMRMLMarkupsNode::PositionDefined));
        }
    }
  d->MarkupsNode->SetControlPointPlacementStartIndex(row);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupTableCurrentCellChanged(
     int currentRow, int currentColumn, int previousRow, int previousColumn)
{
  Q_D(qSlicerMarkupsModuleWidget);
  Q_UNUSED(currentColumn);
  Q_UNUSED(previousRow);
  Q_UNUSED(previousColumn);
  if (!d->MarkupsNode || !this->markupsLogic())
    {
    return;
    }
  if ((d->jumpModeComboBox->currentIndex() == JUMP_MODE_COMBOBOX_INDEX_IGNORE))
    {
    // Jump slices is disabled
    return;
    }
  // Jump slices
  bool jumpCentered = (d->jumpModeComboBox->currentIndex() == JUMP_MODE_COMBOBOX_INDEX_CENTERED);
  this->markupsLogic()->JumpSlicesToNthPointInMarkup(d->MarkupsNode->GetID(), currentRow, jumpCentered);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onRightClickActiveMarkupTableWidget(QPoint pos)
{
  Q_D(qSlicerMarkupsModuleWidget);
  Q_UNUSED(pos);

  // qDebug() << "onRightClickActiveMarkupTableWidget: pos = " << pos;

  QMenu menu;

  // Delete
  QAction *deletePointAction =
    new QAction(QString("Delete highlighted control point(s)"), &menu);
  menu.addAction(deletePointAction);
  QObject::connect(deletePointAction, SIGNAL(triggered()),
                   this, SLOT(onDeleteControlPointPushButtonClicked()));

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

  menu.addSeparator();
  // Change position status
  QAction* resetPointAction =
      new QAction(QIcon(":/Icons/XSmall/MarkupsInProgress.png"), QString("Edit position of highlighted control point(s)"), &menu);
  menu.addAction(resetPointAction);
  QObject::connect(resetPointAction, SIGNAL(triggered()),
      this, SLOT(onResetControlPointPushButtonClicked()));

  QAction* missingPointAction =
      new QAction(QIcon(":/Icons/XSmall/MarkupsMissing.png"), QString("Skip placement of highlighted control point(s)"), &menu);
  menu.addAction(missingPointAction);
  QObject::connect(missingPointAction, SIGNAL(triggered()),
      this, SLOT(onMissingControlPointPushButtonClicked()));

  QAction* restorePointAction =
      new QAction(QIcon(":/Icons/XSmall/MarkupsDefined.png"), QString("Restore position of highlighted control point(s)"), &menu);
  menu.addAction(restorePointAction);
  QObject::connect(restorePointAction, SIGNAL(triggered()),
      this, SLOT(onRestoreControlPointPushButtonClicked()));

  QAction* unsetPointAction =
      new QAction(QIcon(":/Icons/XSmall/MarkupsUndefined.png"), QString("Clear position of highlighted control point(s)"), &menu);
  menu.addAction(unsetPointAction);
  QObject::connect(unsetPointAction, SIGNAL(triggered()),
      this, SLOT(onUnsetControlPointPushButtonClicked()));

  menu.addSeparator();
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
  std::sort(rows.begin(), rows.end());

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
    if (d->coordinatesComboBox->currentIndex() == COORDINATE_COMBOBOX_INDEX_WORLD)
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

    // calculate the point to point accumulated distance for control points
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
void qSlicerMarkupsModuleWidget::onCreateMarkupsPushButtons()
{
  Q_D(qSlicerMarkupsModuleWidget);

  d->createMarkupsPushButtons();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onUpdateMarkupsOptionsWidgets()
{
  Q_D(qSlicerMarkupsModuleWidget);

  d->updateMarkupsOptionsWidgets();
  d->placeMarkupsOptionsWidgets();
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
  this->onDeleteControlPointPushButtonClicked(false); // no confirmation message
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
  QModelIndexList selectedIndexes = d->activeMarkupTableWidget->selectionModel()->selectedRows();
  for (int i = 0; i < selectedIndexes.size(); i++)
    {
    // get the row
    int row = selectedIndexes[i].row();
    rows << row;
    }
  // sort the list
  std::sort(rows.begin(), rows.end());

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
    // No point list is selected - create a new one
    // Assume a markups point list
    this->onCreateMarkupByClass("vtkMRMLMarkupsFiducialNode");
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
  // However, already for > 200 points, it gets bad performance. Therefore, we call a simply modified call at the end.
  MRMLNodeModifyBlocker blocker(d->MarkupsNode);
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
    d->MarkupsNode = nullptr;
    }
  if (markupsNode == d->MarkupsNode)
    {
    // no change
    return;
    }

  qvtkReconnect(d->MarkupsNode, markupsNode, vtkCommand::ModifiedEvent,
    this, SLOT(onActiveMarkupsNodeModifiedEvent()));

  // fixed point number
  qvtkReconnect(d->MarkupsNode, markupsNode, vtkMRMLMarkupsNode::FixedNumberOfControlPointsModifiedEvent,
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

  // measurements
  if (d->MarkupsNode)
    {
    qvtkDisconnect(d->MarkupsNode->Measurements, vtkCommand::ModifiedEvent,
      this, SLOT(onMeasurementsCollectionModified()));
    }
  if (markupsNode)
    {
    qvtkConnect(markupsNode->Measurements, vtkCommand::ModifiedEvent,
      this, SLOT(onMeasurementsCollectionModified()));
    }

  // Setting the internal Markups node
  d->MarkupsNode = markupsNode;

  foreach(const auto& widget, d->MarkupsOptionsWidgets)
    {
    widget->setMRMLMarkupsNode(markupsNode);
    widget->setVisible(widget->canManageMRMLMarkupsNode(markupsNode));
    }

  this->observeMeasurementsInCurrentMarkupsNode();
  this->updateMeasurementsDescriptionLabel();
  this->populateMeasurementSettingsTable();

  // Force update of control point table
  d->activeMarkupTableWidget->setRowCount(0);

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
vtkMRMLMarkupsNode* qSlicerMarkupsModuleWidget::mrmlMarkupsNode()
{
  Q_D(qSlicerMarkupsModuleWidget);
  vtkMRMLNode* node = d->activeMarkupTreeView->currentNode();
  if (!node)
    {
    return nullptr;
    }
  if (!this->mrmlScene())
    {
    return nullptr;
    }
 // make sure the node is still in the scene and convert to markups
 vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->mrmlScene()->GetNodeByID(node->GetID()));
 return markupsNode;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodePointModifiedEvent(vtkObject *caller, void *callData)
{
  Q_D(qSlicerMarkupsModuleWidget);

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
    // Only scroll to the point that is currently being placed if jump to slice is enabled
    // (this could be controlled by a separate flag, but there are already too many options on the GUI).
    if (d->jumpModeComboBox->currentIndex() != JUMP_MODE_COMBOBOX_INDEX_IGNORE)
      {
      vtkMRMLMarkupsNode* markupsNode = this->mrmlMarkupsNode();
      if (markupsNode)
        {
        int mPositionStatus = markupsNode->GetNthControlPointPositionStatus(n);
        if (mPositionStatus == vtkMRMLMarkupsNode::PositionPreview
          && d->activeMarkupTableWidget->currentRow() != n)
          {
          d->activeMarkupTableWidget->setCurrentCell(n, 0);
          }
        }
      }
    }
  else
    {
    this->updateRows();
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodePointAddedEvent()
{
  Q_D(qSlicerMarkupsModuleWidget);

  int newRow = d->activeMarkupTableWidget->rowCount();
  d->activeMarkupTableWidget->insertRow(newRow);

  if (newRow >= 0)
    {
    this->updateRow(newRow);
    }
  else
    {
    this->updateRows();
    }

  // scroll to the new row only if jump slices is not selected
  // (if jump slices on click in table is selected, selecting the new
  // row before the point coordinates are updated will cause the slices
  // to jump to 0,0,0)
  if (d->jumpModeComboBox->currentIndex() == JUMP_MODE_COMBOBOX_INDEX_IGNORE)
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
    // If points are only removed then number of rows will not match the number of control points,
    // which will trigger a full update in updateWidgetFromMRML.
    // If points are removed and added then onActiveMarkupsNodePointAddedEvent is called, which
    // performs the full update.
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
  if (!this->appLogic())
    {
    qWarning() << "Unable to get application logic";
    return;
    }
  return this->appLogic()->SetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesVisibility, flag);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNewMarkupWithCurrentDisplayPropertiesTriggered()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active list
  if (!d->MarkupsNode)
    {
    // if there's no currently active markups list, trigger the default add
    // node
    this->onCreateMarkupByClass("vtkMRMLMarkupsFiducialNode");
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
  if (!this->appLogic())
    {
    qWarning() << "Unable to get application logic";
    return false;
    }
  return this->appLogic()->GetIntersectingSlicesEnabled(vtkMRMLApplicationLogic::IntersectingSlicesVisibility);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onHideCoordinateColumnsToggled(int index)
{
  Q_D(qSlicerMarkupsModuleWidget);
  bool hide = bool(index == COORDINATE_COMBOBOX_INDEX_HIDE);

    // back to default column widths
  d->activeMarkupTableWidget->setColumnHidden(qSlicerMarkupsModuleWidgetPrivate::RColumn, hide);
  d->activeMarkupTableWidget->setColumnHidden(qSlicerMarkupsModuleWidgetPrivate::AColumn, hide);
  d->activeMarkupTableWidget->setColumnHidden(qSlicerMarkupsModuleWidgetPrivate::SColumn, hide);

  if(hide)
    {
    d->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::NameColumn, 60);
    d->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::DescriptionColumn, 120);
    }
  else
    {
    // expand the name and description columns
    d->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::NameColumn, 120);
    d->activeMarkupTableWidget->setColumnWidth(qSlicerMarkupsModuleWidgetPrivate::DescriptionColumn, 240);
    }
  this->updateWidgetFromMRML();
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
                                              QString role /*=QString()*/,
                                              QString context /*=QString()*/)
{
  Q_D(qSlicerMarkupsModuleWidget);

  int controlPointIndex = -1; // <0 means control point index is not specified
  if (role == "ControlPointIndex")
    {
    bool ok = false;
    controlPointIndex = context.toInt(&ok);
    if (!ok)
      {
      controlPointIndex = -1;
      }
    }

  if (vtkMRMLMarkupsNode::SafeDownCast(node))
    {
    d->setSelectionNodeActivePlaceNode(node);
    if (controlPointIndex>=0)
      {
      d->activeMarkupTableWidget->setCurrentCell(controlPointIndex, 0);
      }
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
    if (controlPointIndex>=0)
      {
      d->activeMarkupTableWidget->setCurrentCell(controlPointIndex, 0);
      }
    return true;
    }

  return false;
}

//-----------------------------------------------------------
double qSlicerMarkupsModuleWidget::nodeEditable(vtkMRMLNode* node)
{
  if (node != nullptr && node->GetHideFromEditors())
    {
    // we only allow editing of visible nodes in this module
    return 0.0;
    }
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
void qSlicerMarkupsModuleWidget::onMeasurementsCollectionModified()
{
  // Make sure all measurements are observed
  this->observeMeasurementsInCurrentMarkupsNode();

  // Reconstruct measurement settings table
  this->populateMeasurementSettingsTable();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::observeMeasurementsInCurrentMarkupsNode()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  for (int i=0; i<d->MarkupsNode->Measurements->GetNumberOfItems(); ++i)
    {
    vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(
      d->MarkupsNode->Measurements->GetItemAsObject(i) );
    if (!currentMeasurement)
      {
      continue;
      }

    if (!qvtkIsConnected(currentMeasurement, vtkCommand::ModifiedEvent, this, SLOT(onMeasurementModified(vtkObject*))))
      {
      qvtkConnect(currentMeasurement, vtkCommand::ModifiedEvent, this, SLOT(onMeasurementModified(vtkObject*)));
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMeasurementModified(vtkObject* caller)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // Update measurements description label
  this->updateMeasurementsDescriptionLabel();

  // Update settings for modified measurement
  vtkMRMLMeasurement* measurement = vtkMRMLMeasurement::SafeDownCast(caller);
  if (measurement)
    {
    QString measurementName = QString::fromStdString(measurement->GetName());
    if (measurementName.isEmpty())
      {
      qWarning() << Q_FUNC_INFO << ": Cannot update settings UI for modified measurement because it has an empty name";
      }
    else
      {
      QList<QTableWidgetItem*> nameItemsFound = d->measurementSettingsTableWidget->findItems(measurementName, Qt::MatchExactly);
      foreach (QTableWidgetItem* nameItem, nameItemsFound)
        {
        QCheckBox* checkbox = qobject_cast<QCheckBox*>(d->measurementSettingsTableWidget->cellWidget(nameItem->row(), 1));
        checkbox->setChecked(measurement->GetEnabled());
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateMeasurementsDescriptionLabel()
{
  Q_D(qSlicerMarkupsModuleWidget);

  if (!d->MarkupsNode)
    {
    d->measurementsLabel->setText("No measurement");
    return;
    }

  QString measurementsString;
  for (int i=0; i<d->MarkupsNode->Measurements->GetNumberOfItems(); ++i)
    {
    vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(
      d->MarkupsNode->Measurements->GetItemAsObject(i) );
    if (!currentMeasurement || !currentMeasurement->GetEnabled() || !currentMeasurement->GetValueDefined())
      {
      continue;
      }
    measurementsString.append(QString::fromStdString(currentMeasurement->GetName()));
    measurementsString.append(": ");
    if (currentMeasurement->GetLastComputationResult() == vtkMRMLMeasurement::OK)
      {
      measurementsString.append(currentMeasurement->GetValueWithUnitsAsPrintableString().c_str());
      }
    else
      {
      measurementsString.append(currentMeasurement->GetLastComputationResultAsPrintableString());
      }
    if (i != d->MarkupsNode->Measurements->GetNumberOfItems() - 1)
      {
      measurementsString.append("\n");
      }
    }
  d->measurementsLabel->setText(measurementsString.isEmpty() ? tr("No measurement") : measurementsString);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::populateMeasurementSettingsTable()
{
  Q_D(qSlicerMarkupsModuleWidget);

  d->measurementSettingsTableWidget->clear();
  d->measurementSettingsTableWidget->setVisible(
    d->MarkupsNode != nullptr && d->MarkupsNode->Measurements->GetNumberOfItems() > 0 );

  if (!d->MarkupsNode)
    {
    return;
    }

  d->measurementSettingsTableWidget->setHorizontalHeaderLabels(QStringList() << "Name" << "Enabled");
  d->measurementSettingsTableWidget->setRowCount(d->MarkupsNode->Measurements->GetNumberOfItems());
  for (int i=0; i<d->MarkupsNode->Measurements->GetNumberOfItems(); ++i)
    {
    vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(
      d->MarkupsNode->Measurements->GetItemAsObject(i) );
    if (!currentMeasurement)
      {
      continue;
      }

    QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(currentMeasurement->GetName()));
    d->measurementSettingsTableWidget->setItem(i, 0, nameItem);

    QCheckBox* enabledCheckbox = new QCheckBox();
    enabledCheckbox->setChecked(currentMeasurement->GetEnabled());
    enabledCheckbox->setProperty(NAME_PROPERTY, QString::fromStdString(currentMeasurement->GetName()));
    QObject::connect(enabledCheckbox, SIGNAL(toggled(bool)), this, SLOT(onMeasurementEnabledCheckboxToggled(bool)));
    d->measurementSettingsTableWidget->setCellWidget(i, 1, enabledCheckbox);
    d->measurementSettingsTableWidget->setRowHeight(i, enabledCheckbox->sizeHint().height());
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMeasurementEnabledCheckboxToggled(bool on)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!d->MarkupsNode)
    {
    return;
    }

  // Get measurement name from checkbox
  QCheckBox* checkbox = qobject_cast<QCheckBox*>(this->sender());
  QString measurementName = checkbox->property(NAME_PROPERTY).toString();

  // Enable/disable measurement with name
  for (int i=0; i<d->MarkupsNode->Measurements->GetNumberOfItems(); ++i)
    {
    vtkMRMLMeasurement* currentMeasurement = vtkMRMLMeasurement::SafeDownCast(
      d->MarkupsNode->Measurements->GetItemAsObject(i) );
    if (!currentMeasurement)
      {
      continue;
      }

    if (!measurementName.compare(QString::fromStdString(currentMeasurement->GetName())))
      {
      currentMeasurement->SetEnabled(on);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::setCreateMarkupsButtonsColumns(unsigned int columns)
{
  Q_D(qSlicerMarkupsModuleWidget);
  d->createMarkupsButtonsColumns = columns;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateImportExportWidgets()
{
  Q_D(qSlicerMarkupsModuleWidget);
  bool isExport = d->tableExportRadioButton->isChecked();
  if (isExport)
    {
    // Export
    d->exportImportTableLabel->setText("Output table:");
    d->exportImportPushButton->setText("Export");
    d->exportImportPushButton->setToolTip(tr("Export control points coordinates and properties to table."));
    }
  else
    {
    // Import
    d->exportImportTableLabel->setText("Input table:");
    d->exportImportPushButton->setText("Import");
    d->exportImportPushButton->setToolTip(
      tr("Import control points coordinates and properties from table node.\n"
      "Table column names : label, r, a, s, (or l, p, s), defined, selected, visible, locked, description."));
    }
  d->lpsExportRadioButton->setEnabled(isExport);
  d->rasExportRadioButton->setEnabled(isExport);
  d->exportImportPushButton->setEnabled(d->exportedImportedNodeComboBox->currentNode() != nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onImportExportApply()
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->markupsLogic())
    {
    return;
    }
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  if (d->tableExportRadioButton->isChecked())
    {
    // export
    int coordinateSystem = d->rasExportRadioButton->isChecked() ? vtkMRMLStorageNode::CoordinateSystemRAS : vtkMRMLStorageNode::CoordinateSystemLPS;
    this->markupsLogic()->ExportControlPointsToTable(d->MarkupsNode,
      vtkMRMLTableNode::SafeDownCast(d->exportedImportedNodeComboBox->currentNode()), coordinateSystem);
    }
  else
    {
    this->markupsLogic()->ImportControlPointsFromTable(d->MarkupsNode,
      vtkMRMLTableNode::SafeDownCast(d->exportedImportedNodeComboBox->currentNode()));
    }
  QApplication::restoreOverrideCursor();
}

// --------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateToolBar(vtkMRMLMarkupsNode* node)
{
  Q_D(qSlicerMarkupsModuleWidget);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  qMRMLMarkupsToolBar* toolBar = d->toolBar();
  if (toolBar)
    {
    toolBar->setActiveMarkupsNode(node);
    }

}
