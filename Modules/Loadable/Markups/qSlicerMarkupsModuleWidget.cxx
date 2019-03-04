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

// CTK includes
#include "ctkMessageBox.h"

// SlicerQt includes
#include "qMRMLSceneModel.h"
#include "qMRMLUtils.h"
#include "qSlicerApplication.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"

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

// VTK includes
#include <vtkMath.h>
#include <vtkNew.h>

#include <math.h>


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

private:
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
}

//-----------------------------------------------------------------------------
int qSlicerMarkupsModuleWidgetPrivate::columnIndex(QString label)
{
  return this->columnLabels.indexOf(label);
}

//-----------------------------------------------------------------------------
qSlicerMarkupsModuleWidgetPrivate::~qSlicerMarkupsModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerMarkupsModuleWidget);
  this->Ui_qSlicerMarkupsModule::setupUi(widget);

  //std::cout << "setupUI\n";

  // use the ctk color dialog on the color picker buttons
  this->selectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);
  this->unselectedColorPickerButton->setDialogOptions(ctkColorPickerButton::UseCTKColorDialog);

  // set up the display properties
  QObject::connect(this->selectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                   q, SLOT(onSelectedColorPickerButtonChanged(QColor)));
  QObject::connect(this->unselectedColorPickerButton, SIGNAL(colorChanged(QColor)),
                   q, SLOT(onUnselectedColorPickerButtonChanged(QColor)));
  QObject::connect(this->glyphTypeComboBox, SIGNAL(currentIndexChanged(QString)),
                   q, SLOT(onGlyphTypeComboBoxChanged(QString)));
  QObject::connect(this->glyphScaleSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onGlyphScaleSliderWidgetChanged(double)));
  QObject::connect(this->textScaleSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onTextScaleSliderWidgetChanged(double)));
  QObject::connect(this->opacitySliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onOpacitySliderWidgetChanged(double)));

  // populate the glyph type combo box
  if (this->glyphTypeComboBox->count() == 0)
    {
    vtkNew<vtkMRMLMarkupsDisplayNode> displayNode;
    int min = displayNode->GetMinimumGlyphType();
    int max = displayNode->GetMaximumGlyphType();
    this->glyphTypeComboBox->setEnabled(false);
    for (int i = min; i <= max; i++)
      {
      displayNode->SetGlyphType(i);
      this->glyphTypeComboBox->addItem(displayNode->GetGlyphTypeAsString());
      }
    this->glyphTypeComboBox->setEnabled(true);
    }
  // set the default value if not set
  if (this->glyphTypeComboBox->currentIndex() == 0)
    {
    vtkNew<vtkMRMLMarkupsDisplayNode> displayNode;
    QString glyphType = QString(displayNode->GetGlyphTypeAsString());
    this->glyphTypeComboBox->setEnabled(false);
    int index =  this->glyphTypeComboBox->findData(glyphType);
    if (index != -1)
      {
      this->glyphTypeComboBox->setCurrentIndex(index);
      }
    else
      {
      // glyph types start at 1, combo box is 0 indexed
      this->glyphTypeComboBox->setCurrentIndex(displayNode->GetGlyphType() - 1);
      }
    this->glyphTypeComboBox->setEnabled(true);
    }
  // set up the display properties buttons
  QObject::connect(this->resetToDefaultDisplayPropertiesPushButton,  SIGNAL(clicked()),
                   q, SLOT(onResetToDefaultDisplayPropertiesPushButtonClicked()));
  QObject::connect(this->saveToDefaultDisplayPropertiesPushButton,  SIGNAL(clicked()),
                   q, SLOT(onSaveToDefaultDisplayPropertiesPushButtonClicked()));

  // set up the list buttons
  // visibility
  // first add actions to the menu, then hook them up
  visibilityMenu = new QMenu(q->tr("Visibility"), this->visibilityAllMarkupsInListMenuButton);
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
  lockMenu = new QMenu(q->tr("Lock"), this->lockAllMarkupsInListMenuButton);
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
  selectedMenu = new QMenu(q->tr("Selected"), this->selectedAllMarkupsInListMenuButton);
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
  this->cutAction->setText(q->tr("Cut"));
  this->cutAction->setIcon(QIcon(":Icons/Medium/SlicerEditCut.png"));
  this->cutAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->cutAction->setShortcuts(QKeySequence::Cut);
  this->cutAction->setToolTip(q->tr("Cut"));
  q->addAction(this->cutAction);
  this->CutMarkupsToolButton->setDefaultAction(this->cutAction);
  QObject::connect(this->cutAction, SIGNAL(triggered()), q, SLOT(cutSelectedToClipboard()));

  this->copyAction = new QAction(q);
  this->copyAction->setText(q->tr("Copy"));
  this->copyAction->setIcon(QIcon(":Icons/Medium/SlicerEditCopy.png"));
  this->copyAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->copyAction->setShortcuts(QKeySequence::Copy);
  this->copyAction->setToolTip(q->tr("Copy"));
  q->addAction(this->copyAction);
  this->CopyMarkupsToolButton->setDefaultAction(this->copyAction);
  QObject::connect(this->copyAction, SIGNAL(triggered()), q, SLOT(copySelectedToClipboard()));

  this->pasteAction = new QAction(q);
  this->pasteAction->setText(q->tr("Paste"));
  this->pasteAction->setIcon(QIcon(":Icons/Medium/SlicerEditPaste.png"));
  this->pasteAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  this->pasteAction->setShortcuts(QKeySequence::Paste);
  this->pasteAction->setToolTip(q->tr("Paste"));
  q->addAction(this->pasteAction);
  this->PasteMarkupsToolButton->setDefaultAction(this->pasteAction);
  QObject::connect(this->pasteAction, SIGNAL(triggered()), q, SLOT(pasteSelectedFromClipboard()));

  // markup scale
  QObject::connect(this->markupScaleSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onMarkupScaleSliderWidgetValueChanged(double)));

  // set up the active markups node selector
  QObject::connect(this->activeMarkupMRMLNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onActiveMarkupMRMLNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->activeMarkupMRMLNodeComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
                   q, SLOT(onActiveMarkupMRMLNodeAdded(vtkMRMLNode*)));

  // Make sure that the Jump to Slices radio buttons match the default of the
  // MRML slice node
  vtkNew<vtkMRMLSliceNode> sliceNode;
  if (sliceNode->GetJumpMode() == vtkMRMLSliceNode::OffsetJumpSlice)
    {
    if (this->jumpOffsetRadioButton->isChecked() != true)
      {
      this->jumpOffsetRadioButton->setChecked(true);
      }
    }
  else if (sliceNode->GetJumpMode() == vtkMRMLSliceNode::CenteredJumpSlice)
    {
    if (this->jumpCenteredRadioButton->isChecked() != true)
      {
      this->jumpCenteredRadioButton->setChecked(true);
      }
    }
  // update the checked state of showing the slice intersections
  // vtkSlicerMarkupsLogic::GetSliceIntersectionsVisibility() cannot be called, as the scene
  // is not yet set, so just set to the default value (slice intersections not visible).
  this->sliceIntersectionsVisibilityCheckBox->setChecked(false);
  QObject::connect(this->sliceIntersectionsVisibilityCheckBox,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onSliceIntersectionsVisibilityToggled(bool)));

  //
  // add an action to create a new markups list using the display node
  // settings from the currently active list
  //

  this->newMarkupWithCurrentDisplayPropertiesAction =
    new QAction("New markups with current display properties",
                this->activeMarkupMRMLNodeComboBox);
  this->activeMarkupMRMLNodeComboBox->addMenuAction(this->newMarkupWithCurrentDisplayPropertiesAction);
  this->activeMarkupMRMLNodeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  QObject::connect(this->newMarkupWithCurrentDisplayPropertiesAction, SIGNAL(triggered()),
                    q, SLOT(onNewMarkupWithCurrentDisplayPropertiesTriggered()));

  //
  // set up the list visibility/locked buttons
  //
  QObject::connect(this->listVisibileInvisiblePushButton, SIGNAL(clicked()),
                   q, SLOT(onListVisibileInvisiblePushButtonClicked()));
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
}

//-----------------------------------------------------------------------------
int qSlicerMarkupsModuleWidgetPrivate::numberOfColumns()
{
  return this->columnLabels.size();
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
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::setup()
{
  Q_D(qSlicerMarkupsModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  // for now disable the combo box: when the combo box has it's mrml scene
  // set, it sets the first markups node as the current node, which can end
  // up over riding the selection node's active place node id
  // d->activeMarkupMRMLNodeComboBox->setEnabled(false);
  d->activeMarkupMRMLNodeComboBox->blockSignals(true);

  // this->updateLogicFromSettings();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::enter()
{
  Q_D(qSlicerMarkupsModuleWidget);

  this->Superclass::enter();

  // qDebug() << "enter widget";

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

  if (this->mrmlScene() && this->markupsLogic())
    {
    vtkMRMLNode *selectionNode = this->mrmlScene()->GetNodeByID(this->markupsLogic()->GetSelectionNodeID());
    if (selectionNode)
      {
      this->qvtkConnect(selectionNode, vtkMRMLSelectionNode::ActivePlaceNodeIDChangedEvent,
                        this, SLOT(onSelectionNodeActivePlaceNodeIDChanged()));
      }
    }

  // now enable the combo box and update
  //d->activeMarkupMRMLNodeComboBox->setEnabled(true);
  d->activeMarkupMRMLNodeComboBox->blockSignals(false);

  // check to see if we need to make a previously loaded list active
  int nodeCount = d->activeMarkupMRMLNodeComboBox->nodeCount();
  if (nodeCount > 0 &&
      d->activeMarkupMRMLNodeComboBox->currentNodeID() == "")
    {
    // select the last list
    d->activeMarkupMRMLNodeComboBox->setCurrentNodeIndex(nodeCount - 1);
    }

  // install some shortcuts for use while in this module
  this->installShortcuts();

  this->checkForAnnotationFiducialConversion();

  // check the max scales against volume spacing, they might need to be updated
  this->updateMaximumScaleFromVolumes();

  this->updateWidgetFromMRML();
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
    //       this->pToAddShortcut->setParent(NULL);
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

  // std::cout << "updateWidgetFromMRML" << std::endl;

  if (!this->mrmlScene())
    {
    this->clearGUI();
    return;
    }

  // get the active markup
  std::string listID = (this->markupsLogic() ?
                        this->markupsLogic()->GetActiveListID() :
                        std::string(""));

  // if there's no active list in the logic, check if the node selector has one
  // selected. This can happen after restoring an older scene view that
  // didn't have the active place node set on the selection node
  if (listID.compare("") == 0)
    {
    QString currentNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
    if (currentNodeID.compare("") != 0)
      {
      listID = std::string(currentNodeID.toLatin1());
      qDebug() << "Active list id was null, using GUI combo box setting";
      // and set it active
      vtkMRMLNode *mrmlNode = nullptr;
      vtkMRMLSelectionNode *selectionNode = nullptr;
      if (this->markupsLogic())
        {
         mrmlNode = this->mrmlScene()->GetNodeByID(this->markupsLogic()->GetSelectionNodeID());
        }
      if (mrmlNode)
        {
        selectionNode = vtkMRMLSelectionNode::SafeDownCast(mrmlNode);
        }
      if (selectionNode)
        {
        selectionNode->SetActivePlaceNodeID(listID.c_str());
        }
      }
    }

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->mrmlScene()->GetNodeByID(listID.c_str()));

  // make sure that the GUI updates on changes to the current mark up node,
  // remove observations if no current node
  this->observeMarkupsNode(markupsNode);

  if (!markupsNode)
    {
    // qDebug() << "updateWidgetFromMRML: Unable to get active markups node,
    // clearing out the table";
    this->clearGUI();

    return;
    }

  // update the combo box
  QString activePlaceNodeID = QString(listID.c_str());
  QString currentNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
  if (currentNodeID == "" ||
      (currentNodeID != activePlaceNodeID &&
       activePlaceNodeID.contains("vtkMRMLMarkups")))
    {
    d->activeMarkupMRMLNodeComboBox->setCurrentNodeID(activePlaceNodeID);
    }

  // update display widgets
  this->updateWidgetFromDisplayNode();

  // update the visibility and locked buttons
  this->updateListVisibileInvisiblePushButton(markupsNode->GetDisplayVisibility());

  if (markupsNode->GetLocked())
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

  // update the point projections
  // To DO: update to use also other markups nodes
  if (markupsNode->IsA("vtkMRMLMarkupsFiducialNode"))
    {
    d->pointFiducialProjectionWidget->setMRMLFiducialNode(
        vtkMRMLMarkupsFiducialNode::SafeDownCast(markupsNode));
    }
  else
    {
    d->pointFiducialProjectionWidget->setMRMLFiducialNode(nullptr);
    }

  // update the list name format
  QString nameFormat = QString(markupsNode->GetMarkupLabelFormat().c_str());
  d->nameFormatLineEdit->setText(nameFormat);

  // update the transform checkbox label to reflect current transform node name
  const char *transformNodeID = markupsNode->GetTransformNodeID();
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
  int numberOfPoints = markupsNode->GetNumberOfControlPoints();
  if (d->activeMarkupTableWidget->rowCount() != numberOfPoints)
    {
    // clear it out
    d->activeMarkupTableWidget->setRowCount(numberOfPoints);
    }
  for (int m = 0; m < numberOfPoints; m++)
    {
    this->updateRow(m);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateWidgetFromDisplayNode()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active markup
  std::string listID = (this->markupsLogic() ? this->markupsLogic()->GetActiveListID() : std::string(""));
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->mrmlScene()->GetNodeByID(listID.c_str()));

  // update the display properties from the markups display node
  vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
  double *color;
  QColor qColor;

  if (displayNode)
    {
    // views
    d->listDisplayNodeViewComboBox->setEnabled(true);
    d->listDisplayNodeViewComboBox->setMRMLDisplayNode(displayNode);

    // selected color
    color = displayNode->GetSelectedColor();
    qMRMLUtils::colorToQColor(color, qColor);
    d->selectedColorPickerButton->setColor(qColor);

    // unselected color
    color = displayNode->GetColor();
    qMRMLUtils::colorToQColor(color, qColor);
    d->unselectedColorPickerButton->setColor(qColor);

    // opacity
    double opacity = displayNode->GetOpacity();
    d->opacitySliderWidget->setValue(opacity);

    // now for some markups specific display properties
    vtkMRMLMarkupsDisplayNode *markupsDisplayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(displayNode);
    if (markupsDisplayNode)
      {
      // does the glyph type combo box have entries?
      if (d->glyphTypeComboBox->count() == 0)
        {
        qDebug() << "No entries in the glyph type combo box";
        }
      else
        {
        // glyph type
        const char *glyphType = markupsDisplayNode->GetGlyphTypeAsString();
        int index =  d->glyphTypeComboBox->findData(QString(glyphType));
        if (index != -1)
          {
          d->glyphTypeComboBox->setCurrentIndex(index);
          }
        else
          {
          // glyph types start at 1, combo box is 0 indexed
          d->glyphTypeComboBox->setCurrentIndex(markupsDisplayNode->GetGlyphType() - 1);
          }
        }

      // glyph scale
      double glyphScale = markupsDisplayNode->GetGlyphScale();

      // make sure that the slider can accommodate this scale
      if (glyphScale > d->glyphScaleSliderWidget->maximum())
        {
        d->glyphScaleSliderWidget->setMaximum(glyphScale);
        }
      d->glyphScaleSliderWidget->setValue(glyphScale);

      // text scale
      double textScale = markupsDisplayNode->GetTextScale();
      if (textScale > d->textScaleSliderWidget->maximum())
        {
        d->textScaleSliderWidget->setMaximum(textScale);
        }
      d->textScaleSliderWidget->setValue(textScale);
      }
    }
  else
    {
    // disable the views combo box for now
    d->listDisplayNodeViewComboBox->setEnabled(false);

    // reset to defaults from logic
    if (this->markupsLogic())
      {
      color = this->markupsLogic()->GetDefaultMarkupsDisplayNodeSelectedColor();
      qMRMLUtils::colorToQColor(color, qColor);
      d->selectedColorPickerButton->setColor(qColor);
      color = this->markupsLogic()->GetDefaultMarkupsDisplayNodeColor();
      qMRMLUtils::colorToQColor(color, qColor);
      d->unselectedColorPickerButton->setColor(qColor);
      d->opacitySliderWidget->setValue(this->markupsLogic()->GetDefaultMarkupsDisplayNodeOpacity());
      QString glyphTypeString = QString(this->markupsLogic()->GetDefaultMarkupsDisplayNodeGlyphTypeAsString().c_str());
      int glyphTypeInt = this->markupsLogic()->GetDefaultMarkupsDisplayNodeGlyphType();
      // glyph types start at 1, combo box is 0 indexed
      int glyphTypeIndex = glyphTypeInt - 1;
      if (glyphTypeIndex != -1)
        {
        d->glyphTypeComboBox->setCurrentIndex(glyphTypeIndex);
        }

      // make sure that the slider max values can accommodate the default settings
      if (d->glyphScaleSliderWidget->maximum() <
          this->markupsLogic()->GetDefaultMarkupsDisplayNodeGlyphScale())
        {
        d->glyphScaleSliderWidget->setMaximum(
          this->markupsLogic()->GetDefaultMarkupsDisplayNodeGlyphScale());
        }
      // glyph scale
      d->glyphScaleSliderWidget->setValue(this->markupsLogic()->GetDefaultMarkupsDisplayNodeGlyphScale());

      // check slider max
      if (d->textScaleSliderWidget->maximum() <
          this->markupsLogic()->GetDefaultMarkupsDisplayNodeTextScale())
        {
        d->textScaleSliderWidget->setMaximum(
          this->markupsLogic()->GetDefaultMarkupsDisplayNodeTextScale());
        }
      // text scale
      d->textScaleSliderWidget->setValue(this->markupsLogic()->GetDefaultMarkupsDisplayNodeTextScale());
      }
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

  if (maxScale > d->glyphScaleSliderWidget->maximum())
    {
    d->glyphScaleSliderWidget->setMaximum(maxScale);
    }
  if (maxScale > d->textScaleSliderWidget->maximum())
    {
    d->textScaleSliderWidget->setMaximum(maxScale);
    }
  if (maxScale > d->markupScaleSliderWidget->maximum())
    {
    d->markupScaleSliderWidget->setMaximum(maxScale);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateRow(int m)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // this is updating the qt widget from MRML, and should not trigger any updates on the node, so turn off events
  d->activeMarkupTableWidget->blockSignals(true);

  // qDebug() << QString("updateRow: row = ") + QString::number(m) + QString(", number of rows = ") + QString::number(d->activeMarkupTableWidget->rowCount());
  // get active markups node
  QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
  vtkMRMLNode *mrmlNode = this->mrmlScene()->GetNodeByID(activeMarkupsNodeID.toLatin1());
  vtkMRMLMarkupsNode *markupsNode = nullptr;
  if (mrmlNode)
    {
    markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
    }
  if (!markupsNode)
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
    d->activeMarkupMRMLNodeComboBox->setCurrentNodeID(markupsNode->GetID());
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

  // only respond if it was the last node that was removed
  int numNodes = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLMarkupsNode");
  if (numNodes == 0)
    {
    this->clearGUI();
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
  if (!this->mrmlScene())
    {
    return;
    }
  this->checkForAnnotationFiducialConversion();
  // qDebug() << "qSlicerMarkupsModuleWidget::onMRMLSceneEndBatchProcessEvent";
  std::string selectionNodeID = (this->markupsLogic() ? this->markupsLogic()->GetSelectionNodeID() : std::string(""));
  vtkMRMLNode *node = this->mrmlScene()->GetNodeByID(selectionNodeID.c_str());
  vtkMRMLSelectionNode *selectionNode = nullptr;
  if (node)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(node);
    }

  QString activePlaceNodeID;
  if (selectionNode)
    {
    activePlaceNodeID = selectionNode->GetActivePlaceNodeID();
    }
  if (activePlaceNodeID.isEmpty())
    {
    // this might have been triggered after a file load, set the last markups node active
    int numNodes = this->mrmlScene()->GetNumberOfNodesByClass("vtkMRMLMarkupsNode");
    if (numNodes > 0)
      {
      vtkMRMLNode *lastNode = this->mrmlScene()->GetNthNodeByClass(numNodes - 1, "vtkMRMLMarkupsNode");
      if (lastNode)
        {
        // qDebug() << "onMRMLSceneEndBatchProcessEvent: setting active place node id to " << lastNode->GetID();
        selectionNode->SetActivePlaceNodeID(lastNode->GetID());
        }
      }
    }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMRMLSceneEndCloseEvent()
{
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  // qDebug() << "qSlicerMarkupsModuleWidget::onMRMLSceneEndCloseEvent";
  this->clearGUI();
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
void qSlicerMarkupsModuleWidget::onSelectedColorPickerButtonChanged(QColor qcolor)
{
  Q_D(qSlicerMarkupsModuleWidget);

  double color[3];
  qMRMLUtils::qColorToColor(qcolor, color);

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = markupsNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  displayNode->SetSelectedColor(color);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onUnselectedColorPickerButtonChanged(QColor qcolor)
{
  Q_D(qSlicerMarkupsModuleWidget);

  double color[3];
  qMRMLUtils::qColorToColor(qcolor, color);

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = markupsNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  displayNode->SetColor(color);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onGlyphTypeComboBoxChanged(QString value)
{
  Q_D(qSlicerMarkupsModuleWidget);

  if (value.isEmpty())
    {
    return;
    }

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = markupsNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  displayNode->SetGlyphTypeFromString(value.toLatin1());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onGlyphScaleSliderWidgetChanged(double value)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = markupsNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  displayNode->SetGlyphScale(value);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onTextScaleSliderWidgetChanged(double value)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = markupsNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  displayNode->SetTextScale(value);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onOpacitySliderWidgetChanged(double value)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = markupsNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  displayNode->SetOpacity(value);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onResetToDefaultDisplayPropertiesPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = markupsNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  // set the display node from the logic defaults
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetDisplayNodeToDefaults(displayNode);
    }

  // push an update on the GUI
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSaveToDefaultDisplayPropertiesPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = markupsNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  // save the settings
  QSettings *settings = qSlicerApplication::application()->settingsDialog()->settings();
  settings->setValue("Markups/GlyphType", displayNode->GetGlyphTypeAsString());
  QColor qcolor;
  double  *selectedColor = displayNode->GetSelectedColor();
  if (selectedColor)
    {
    qcolor = QColor::fromRgbF(selectedColor[0], selectedColor[1], selectedColor[2]);
    }
  settings->setValue("Markups/SelectedColor", qcolor);
  double *unselectedColor = displayNode->GetColor();
  if (unselectedColor)
    {
    qcolor = QColor::fromRgbF(unselectedColor[0], unselectedColor[1], unselectedColor[2]);
    }
  settings->setValue("Markups/UnselectedColor", qcolor);
  settings->setValue("Markups/GlyphScale", displayNode->GetGlyphScale());
  settings->setValue("Markups/TextScale", displayNode->GetTextScale());
  settings->setValue("Markups/Opacity", displayNode->GetOpacity());

  // projection
  settings->setValue("Markups/SliceProjection", displayNode->GetSliceProjection());
  double *projectionColor = displayNode->GetSliceProjectionColor();
  if (projectionColor)
    {
    qcolor = QColor::fromRgbF(projectionColor[0], projectionColor[1], projectionColor[2]);
    }
  settings->setValue("Markups/SliceProjectionColor", qcolor);
  settings->setValue("Markups/SliceProjectionOpacity", displayNode->GetSliceProjectionOpacity());

  // set the logic defaults from the settings
  this->updateLogicFromSettings();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityOnAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsVisibility(markupsNode, true);
    markupsNode->SetDisplayVisibility(true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityOffAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsVisibility(markupsNode, false);
    markupsNode->SetDisplayVisibility(false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onVisibilityAllMarkupsInListToggled()
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (this->markupsLogic())
    {
    this->markupsLogic()->ToggleAllMarkupsVisibility(markupsNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onLockAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsLocked(markupsNode, true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onUnlockAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsLocked(markupsNode, false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onLockAllMarkupsInListToggled()
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (this->markupsLogic())
    {
    this->markupsLogic()->ToggleAllMarkupsLocked(markupsNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsSelected(markupsNode, true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeselectAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsSelected(markupsNode, false);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectedAllMarkupsInListToggled()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (this->markupsLogic())
    {
    this->markupsLogic()->ToggleAllMarkupsSelected(markupsNode);
    }
}
//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMarkupScaleSliderWidgetValueChanged(double value)
{
   Q_D(qSlicerMarkupsModuleWidget);

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // get the display node
  vtkMRMLMarkupsDisplayNode *displayNode = markupsNode->GetMarkupsDisplayNode();
  if (!displayNode)
    {
    return;
    }

  // apply the value for both glyph and text scale
  int disabledModify = displayNode->StartModify();
  displayNode->SetGlyphScale(value);
  displayNode->SetTextScale(value);
  displayNode->EndModify(disabledModify);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onAddMarkupPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

   // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (markupsNode &&
    markupsNode->GetNumberOfControlPoints() < markupsNode->GetMaximumNumberOfControlPoints())
    {
    markupsNode->AddControlPoint(vtkVector3d(0,0,0));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMoveMarkupUpPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, that only one is selected
  if ((selectedItems.size() / d->numberOfColumns()) != 1)
    {
    qDebug() << "Move up: only select one markup to move, current selected: " << selectedItems.size() << ", number of columns = " << d->numberOfColumns();
    return;
    }
  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (markupsNode)
    {
    int thisIndex = selectedItems.at(0)->row();
    //qDebug() << "Swapping " << thisIndex << " and " << thisIndex - 1;
    markupsNode->SwapControlPoints(thisIndex, thisIndex - 1);
    // now make sure the new row is selected so a user can keep moving it up
    d->activeMarkupTableWidget->selectRow(thisIndex - 1);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onMoveMarkupDownPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, that only one is selected
  if ((selectedItems.size() / d->numberOfColumns()) != 1)
    {
    return;
    }
  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (markupsNode)
    {
    int thisIndex = selectedItems.at(0)->row();
    //qDebug() << "Swapping " << thisIndex << " and " << thisIndex + 1;
    markupsNode->SwapControlPoints(thisIndex, thisIndex + 1);
    // now make sure the new row is selected so a user can keep moving it down
    d->activeMarkupTableWidget->selectRow(thisIndex + 1);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeleteMarkupPushButtonClicked(bool confirm /*=true*/)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    qDebug() << "Delete markup: no active list from which to delete";
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
    markupsNode->RemoveNthControlPoint(index);
    }

  // clear the selection on the table
  d->activeMarkupTableWidget->clearSelection();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onDeleteAllMarkupsInListPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);
  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (markupsNode)
    {
    // qDebug() << "Removing markups from list " << markupsNode->GetName();
    ctkMessageBox deleteAllMsgBox;
    deleteAllMsgBox.setWindowTitle("Delete All Markups in this list?");
    QString labelText = QString("Delete all ")
      + QString::number(markupsNode->GetNumberOfControlPoints())
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
      markupsNode->RemoveAllControlPoints();
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupMRMLNodeChanged(vtkMRMLNode *markupsNode)
{
  Q_D(qSlicerMarkupsModuleWidget);

  //qDebug() << "onActiveMarkupMRMLNodeChanged, markupsNode is " << (markupsNode ? markupsNode->GetID() : "null");

  // update the selection node
  vtkMRMLSelectionNode *selectionNode = nullptr;
  if (this->mrmlScene() && this->markupsLogic())
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(
          this->mrmlScene()->GetNodeByID(this->markupsLogic()->GetSelectionNodeID().c_str()));
    }
  if (selectionNode)
    {
    // check if changed
    const char *selectionNodeActivePlaceNodeID = selectionNode->GetActivePlaceNodeID();

    const char *activeID = nullptr;
    if (markupsNode)
      {
      activeID = markupsNode->GetID();
      }

    // get the current node from the combo box
    //QString activeMarkupsNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
    //qDebug() << "setActiveMarkupsNode: combo box says: " << qPrintable(activeMarkupsNodeID) << ", input node says " << (activeID ? activeID : "null");

    // don't update the selection node if the active ID is nullptr (can happen
    // when entering the module)
    if (activeID != nullptr)
      {
      if (!selectionNodeActivePlaceNodeID || !activeID ||
          strcmp(selectionNodeActivePlaceNodeID, activeID) != 0)
        {
        selectionNode->SetReferenceActivePlaceNodeID(activeID);
        }
      }
    else
      {
      if (selectionNodeActivePlaceNodeID != nullptr)
        {
        //std::cout << "Setting combo box from selection node " << selectionNodeActivePlaceNodeID << std::endl;
        d->activeMarkupMRMLNodeComboBox->setCurrentNodeID(selectionNodeActivePlaceNodeID);
        }
      }
    }

  // update the GUI
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupMRMLNodeAdded(vtkMRMLNode *markupsNode)
{
  // qDebug() << "onActiveMarkupMRMLNodeAdded, markupsNode is " << (markupsNode ? markupsNode->GetID() : "null");

  if (this->markupsLogic())
    {
    this->markupsLogic()->AddNewDisplayNodeForMarkupsNode(markupsNode);
    }
  // update the visibility button
  vtkMRMLMarkupsNode *displayableNode = nullptr;
  if (markupsNode)
    {
    displayableNode = vtkMRMLMarkupsNode::SafeDownCast(markupsNode);
    }
  if (displayableNode)
    {
    int visibleFlag = displayableNode->GetDisplayVisibility();
    this->updateListVisibileInvisiblePushButton(visibleFlag);
    }

  // make sure it's set up for the mouse mode tool bar to easily add points to
  // it by making it active in the selection node
  if (this->markupsLogic())
    {
    this->markupsLogic()->SetActiveListID(displayableNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onSelectionNodeActivePlaceNodeIDChanged()
{
  Q_D(qSlicerMarkupsModuleWidget);

  //qDebug() << "onSelectionNodeActivePlaceNodeIDChanged";
  std::string listID = (this->markupsLogic() ? this->markupsLogic()->GetActiveListID() : std::string(""));
  QString activePlaceNodeID = QString(listID.c_str());
  if (activePlaceNodeID.size() == 0)
    {
    qDebug() << "onSelectionNodeActivePlaceNodeIDChanged: No current active place node id";
    d->activeMarkupMRMLNodeComboBox->setCurrentNodeID("");
    }
  else
    {
    QString currentNodeID = d->activeMarkupMRMLNodeComboBox->currentNodeID();
    //std::cout << "\t\tcombo box current node id = " << qPrintable(currentNodeID) << std::endl;
    if (currentNodeID == "" ||
        (currentNodeID != activePlaceNodeID &&
         activePlaceNodeID.contains("vtkMRMLMarkups")))
      {
      d->activeMarkupMRMLNodeComboBox->setCurrentNodeID(activePlaceNodeID);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onListVisibileInvisiblePushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // toggle the visibility
  int visibleFlag = markupsNode->GetDisplayVisibility();
  visibleFlag = !visibleFlag;
  markupsNode->SetDisplayVisibility(visibleFlag);
  this->updateListVisibileInvisiblePushButton(visibleFlag);

  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsVisibility(markupsNode, visibleFlag);
    markupsNode->SetDisplayVisibility(visibleFlag);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateListVisibileInvisiblePushButton(int visibleFlag)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // update the list visibility button icon and tool tip
  if (visibleFlag)
    {
    d->listVisibileInvisiblePushButton->setIcon(QIcon(":Icons/Medium/SlicerVisible.png"));
    d->listVisibileInvisiblePushButton->setToolTip(QString("Click to hide this markup list"));
    }
  else
    {
    d->listVisibileInvisiblePushButton->setIcon(QIcon(":Icons/Medium/SlicerInvisible.png"));
    d->listVisibileInvisiblePushButton->setToolTip(QString("Click to show this markup list"));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onListLockedUnlockedPushButtonClicked()
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    return;
    }
  // toggle the lock
  int locked = markupsNode->GetLocked();
  locked = !locked;
  markupsNode->SetLocked(locked);

  if (this->markupsLogic())
    {
    this->markupsLogic()->SetAllMarkupsLocked(markupsNode, locked);
    }

  // update the button
  if (locked)
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/SlicerLock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to unlock this markup list so that the markups can be moved by the mouse"));
    }
  else
    {
    d->listLockedUnlockedPushButton->setIcon(QIcon(":Icons/Medium/SlicerUnlock.png"));
    d->listLockedUnlockedPushButton->setToolTip(QString("Click to lock this markup list so that the markups cannot be moved by the mouse"));
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onNameFormatLineEditTextEdited(const QString text)
{
  Q_D(qSlicerMarkupsModuleWidget);

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    qDebug() << QString("Name format edited: unable to get current list");
    return;
    }
  markupsNode->SetMarkupLabelFormat(std::string(text.toLatin1()));
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onResetNameFormatToDefaultPushButtonClicked()
{
   Q_D(qSlicerMarkupsModuleWidget);

   vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
   if (!markupsNode)
     {
     qDebug() << QString("Reset name format: unable to get current list");
     return;
     }
   // make a new default markups node and use its value for the name format
   vtkNew<vtkMRMLMarkupsNode> defaultNode;
   markupsNode->SetMarkupLabelFormat(defaultNode->GetMarkupLabelFormat());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onRenameAllWithCurrentNameFormatPushButtonClicked()
{
   Q_D(qSlicerMarkupsModuleWidget);

   if (!this->markupsLogic())
    {
    qDebug() << "Cannot rename without a logic class!";
    return;
    }

   vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
   this->markupsLogic()->RenameAllMarkupsFromCurrentFormat(markupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupTableCellChanged(int row, int column)
{
  Q_D(qSlicerMarkupsModuleWidget);

//  qDebug() << QString("cell changed: row = ") + QString::number(row) + QString(", col = ") + QString::number(column);
  // get the active list
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    qDebug() << QString("Cell Changed: unable to get current list");
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
    markupsNode->SetNthControlPointSelected(n, flag);
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
    markupsNode->SetNthControlPointLocked(n, flag);
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
    markupsNode->SetNthControlPointVisibility(n, flag);
    }
  else if (column ==  d->columnIndex("Name"))
    {
    std::string name = std::string(item->text().toLatin1());
    markupsNode->SetNthControlPointLabel(n, name);
    }
  else if (column ==  d->columnIndex("Description"))
    {
    std::string description = std::string(item->text().toLatin1());
    markupsNode->SetNthControlPointDescription(n, description);
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
      markupsNode->GetNthControlPointPositionWorld(n, worldPoint);
      for (int p = 0; p < 3; ++p)
       {
       point[p] = worldPoint[p];
       }
      }
    else
      {
      markupsNode->GetNthControlPointPosition(n, point);
      }

    // changed?
    double minChange = 0.001;
    if (fabs(newPoint[0] - point[0]) > minChange ||
        fabs(newPoint[1] - point[1]) > minChange ||
        fabs(newPoint[2] - point[2]) > minChange)
      {
      vtkMRMLMarkupsNode *fidList = vtkMRMLMarkupsNode::SafeDownCast(markupsNode);
      if (fidList)
        {
        if (d->transformedCoordinatesCheckBox->isChecked())
          {
          fidList->SetNthControlPointPositionWorld(n, newPoint[0], newPoint[1], newPoint[2]);
          }
        else
          {
          fidList->SetNthControlPointPositionFromArray(n, newPoint);
          }
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

  //int row = item->row();
  int column = item->column();
  //qDebug() << "onActiveMarkupTableCellClicked: row = " << row << ", col = " << column;

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

  // is jumping disabled?
  if (!d->jumpSlicesGroupBox->isChecked())
    {
    return;
    }
  // otherwise jump to that slice

  // get the active list
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  if (!mrmlNode)
    {
    return;
    }
  // offset or center?
  bool jumpCentered = false;
  if (d->jumpCenteredRadioButton->isChecked())
    {
    jumpCentered = true;
    }
  // jump to it
  if (this->markupsLogic())
    {
    this->markupsLogic()->JumpSlicesToNthPointInMarkup(mrmlNode->GetID(), currentRow, jumpCentered);
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

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  if (!mrmlNode)
    {
    return;
    }
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (!markupsNode)
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
      if (!(markupsNode->GetNthControlPointLabel(row).empty()))
        {
        indexString =  QString(markupsNode->GetNthControlPointLabel(row).c_str());
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
      markupsNode->GetNthControlPointPositionWorld(row, worldPoint);
      for (int p = 0; p < 3; ++p)
        {
        point[p] = worldPoint[p];
        }
      }
    else
      {
      markupsNode->GetNthControlPointPosition(row, point);
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

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  if (!mrmlNode)
    {
    return;
    }

  // offset or center?
  bool jumpCentered = false;
  if (d->jumpCenteredRadioButton->isChecked())
    {
    jumpCentered = true;
    }

  // jump to it
  if (this->markupsLogic())
    {
    // use the first selected
    this->markupsLogic()->JumpSlicesToNthPointInMarkup(mrmlNode->GetID(), selectedItems.at(0)->row(), jumpCentered);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onRefocusCamerasActionTriggered()
{
 Q_D(qSlicerMarkupsModuleWidget);

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // get the active node
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  if (!mrmlNode)
    {
    return;
    }

  // refocus on this point
  if (this->markupsLogic())
    {
    // use the first selected
    this->markupsLogic()->FocusCamerasOnNthPointInMarkup(mrmlNode->GetID(), selectedItems.at(0)->row());
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

  // get the selected rows
  QList<QTableWidgetItem *> selectedItems = d->activeMarkupTableWidget->selectedItems();

  // first, check if nothing is selected
  if (selectedItems.isEmpty())
    {
    return;
    }

  // get the active node
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    qDebug() << Q_FUNC_INFO << ": no active list from which to cut";
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
    markupsAsString += (storageNode->GetPointAsString(markupsNode, markupIndex).c_str() + QString("\n"));
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

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->currentNode());
  if (!markupsNode)
    {
    // No fiducial list is selected - create a new one
    // Assume a fiducial markups
    markupsNode = vtkMRMLMarkupsFiducialNode::SafeDownCast(d->activeMarkupMRMLNodeComboBox->addNode("vtkMRMLMarkupsFiducialNode"));
    if (!markupsNode)
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
  markupsNode->DisableModifiedEventOn();
  foreach(QString line, lines)
    {
    line = line.trimmed();
    if (line.isEmpty() || line.startsWith('#'))
      {
      // empty line or comment line
      continue;
      }

    storageNode->SetPointFromString(markupsNode, markupsNode->GetNumberOfControlPoints(), line.toLatin1());
    }
  markupsNode->DisableModifiedEventOff();
  markupsNode->Modified();
  int n = markupsNode->GetNumberOfControlPoints() - 1;
  markupsNode->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeModifiedEvent()
{
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::observeMarkupsNode(vtkMRMLNode *markupsNode)
{
  if (this->mrmlScene())
    {
    // remove all connections
    vtkCollection *col = this->mrmlScene()->GetNodesByClass("vtkMRMLMarkupsNode");
    int numNodes = col->GetNumberOfItems();
    // qDebug() << "observeMarkupsNode: have " << numNodes << " markups nodes";
    for (int n = 0; n < numNodes; n++)
      {
      vtkMRMLNode *node = vtkMRMLNode::SafeDownCast(col->GetItemAsObject(n));
      if (node)
        {
        if (markupsNode)
          {
          // is this the markups node?
          if (node->GetID() && markupsNode->GetID() && strcmp(node->GetID(), markupsNode->GetID()) == 0)
            {
            // don't disconnect
            // qDebug() << "\tskipping disconnecting " << node->GetID();
            continue;
            }
          }
        // qDebug() << "\tdisconnecting " << node->GetID();
        this->qvtkDisconnect(markupsNode, vtkCommand::ModifiedEvent,
                             this, SLOT(onActiveMarkupsNodeModifiedEvent()));
        this->qvtkDisconnect(node, vtkMRMLMarkupsNode::LockModifiedEvent,
                             this, SLOT(onActiveMarkupsNodeLockModifiedEvent()));
        this->qvtkDisconnect(node, vtkMRMLMarkupsNode::LabelFormatModifiedEvent,
                             this, SLOT(onActiveMarkupsNodeLabelFormatModifiedEvent()));
        this->qvtkDisconnect(node, vtkMRMLMarkupsNode::PointModifiedEvent,
                             this, SLOT(onActiveMarkupsNodePointModifiedEvent(vtkObject*,vtkObject*)));
        this->qvtkDisconnect(node, vtkMRMLMarkupsNode::PointAddedEvent,
                             this, SLOT(onActiveMarkupsNodePointAddedEvent()));
        this->qvtkDisconnect(node, vtkMRMLMarkupsNode::PointRemovedEvent,
                             this, SLOT(onActiveMarkupsNodePointRemovedEvent(vtkObject*,vtkObject*)));

        // display node
        vtkMRMLMarkupsNode *displayableNode = vtkMRMLMarkupsNode::SafeDownCast(node);
        if (displayableNode)
          {
          this->qvtkDisconnect(displayableNode,
                               vtkMRMLDisplayableNode::DisplayModifiedEvent,
                               this,
                               SLOT(onActiveMarkupsNodeDisplayModifiedEvent()));
          }
        // transforms
        vtkMRMLTransformableNode *transformableNode =
          vtkMRMLTransformableNode::SafeDownCast(node);
        if (transformableNode)
          {
          this->qvtkDisconnect(transformableNode,
                               vtkMRMLTransformableNode::TransformModifiedEvent,
                               this,
                               SLOT(onActiveMarkupsNodeTransformModifiedEvent()));
          }
        }
      }
    col->RemoveAllItems();
    col->Delete();
    }
  else
    {
    qWarning() << "observeMarkupsNode: no scene";
    }
  if (markupsNode)
    {
    // is the node already connected?
    if (this->qvtkIsConnected(markupsNode, vtkMRMLMarkupsNode::LockModifiedEvent,
                                     this, SLOT(onActiveMarkupsNodeLockModifiedEvent())))
      {
      // qDebug() << "\tmarkups node is already connected: " << markupsNode->GetID();
      }
    else
      {
      // add connections for this node
      this->qvtkConnect(markupsNode, vtkCommand::ModifiedEvent,
                        this, SLOT(onActiveMarkupsNodeModifiedEvent()));
      this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::LockModifiedEvent,
                        this, SLOT(onActiveMarkupsNodeLockModifiedEvent()));
      this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::LabelFormatModifiedEvent,
                        this, SLOT(onActiveMarkupsNodeLabelFormatModifiedEvent()));
      this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::PointModifiedEvent,
                        this, SLOT(onActiveMarkupsNodePointModifiedEvent(vtkObject*,vtkObject*)));
      this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::PointAddedEvent,
                        this, SLOT(onActiveMarkupsNodePointAddedEvent()));
      this->qvtkConnect(markupsNode, vtkMRMLMarkupsNode::PointRemovedEvent,
                       this, SLOT(onActiveMarkupsNodePointRemovedEvent(vtkObject*,vtkObject*)));
      // qDebug() << "\tconnected markups node " << markupsNode->GetID();
      // display node
      vtkMRMLMarkupsNode *displayableNode = vtkMRMLMarkupsNode::SafeDownCast(markupsNode);
      if (displayableNode)
        {
        this->qvtkConnect(displayableNode,
                          vtkMRMLDisplayableNode::DisplayModifiedEvent,
                          this,
                          SLOT(onActiveMarkupsNodeDisplayModifiedEvent()));
        }
      // transforms
      vtkMRMLTransformableNode *transformableNode =
        vtkMRMLTransformableNode::SafeDownCast(markupsNode);
      if (transformableNode)
        {
        this->qvtkConnect(transformableNode,
                          vtkMRMLTransformableNode::TransformModifiedEvent,
                          this,
                          SLOT(onActiveMarkupsNodeTransformModifiedEvent()));
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::clearGUI()
{
  Q_D(qSlicerMarkupsModuleWidget);

  d->activeMarkupTableWidget->clearContents();
  d->activeMarkupTableWidget->setRowCount(0);

  // setting a nullptr node requires casting (and triggers a memory leak),
  // so disable it instead
  d->listDisplayNodeViewComboBox->setEnabled(false);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeLockModifiedEvent()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active list
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  if (!mrmlNode)
    {
    return;
    }
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (!markupsNode)
    {
    return;
    }
  if (markupsNode->GetLocked())
    {
    // disable the table
    d->activeMarkupTableWidget->setEnabled(false);
    }
  else
    {
    // enable it
    d->activeMarkupTableWidget->setEnabled(true);
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeLabelFormatModifiedEvent()
{
  Q_D(qSlicerMarkupsModuleWidget);

  // get the active list
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  if (!mrmlNode)
    {
    return;
    }
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (markupsNode)
    {
    d->nameFormatLineEdit->setText(markupsNode->GetMarkupLabelFormat().c_str());
    }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodePointModifiedEvent(vtkObject *caller, vtkObject *callData)
{
  // the call data should be the index n
  if (caller == nullptr || callData == nullptr)
    {
    return;
    }

  int *nPtr = nullptr;
  int n = -1;
  nPtr = reinterpret_cast<int *>(callData);
  if (nPtr)
    {
    n = *nPtr;
    }

  this->updateRow(n);
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
  if (!d->jumpSlicesGroupBox->isChecked())
    {
    d->activeMarkupTableWidget->setCurrentCell(newRow, 0);
    }

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodePointRemovedEvent(vtkObject *caller, vtkObject *callData)
{
  Q_D(qSlicerMarkupsModuleWidget);

  // the call data should be the index n
  if (caller == nullptr || callData == nullptr)
    {
    return;
    }

  int *nPtr = nullptr;
  int n = -1;
  nPtr = reinterpret_cast<int *>(callData);
  if (nPtr)
    {
    n = *nPtr;
    }

  d->activeMarkupTableWidget->removeRow(n);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::onActiveMarkupsNodeDisplayModifiedEvent()
{
  // update the display properties
  this->updateWidgetFromDisplayNode();
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
  vtkMRMLNode *mrmlNode = d->activeMarkupMRMLNodeComboBox->currentNode();
  if (!mrmlNode)
    {
    // if there's no currently active markups list, trigger the default add
    // node
    d->activeMarkupMRMLNodeComboBox->addNode();
    return;
    }

  // otherwise make a new one of the same class
  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(mrmlNode);
  if (!markupsNode)
    {
    qDebug() << "Unable to get the currently active markups node";
    return;
    }

  // get the display node
  vtkMRMLDisplayNode *displayNode = markupsNode->GetDisplayNode();
  if (!displayNode)
    {
    qDebug() << "Unable to get the display node on the markups node";
    }

  // create a new one
  vtkMRMLNode *newDisplayNode = this->mrmlScene()->CreateNodeByClass(displayNode->GetClassName());
  // copy the old one
  newDisplayNode->Copy(displayNode);
  /// add to the scene
  this->mrmlScene()->AddNode(newDisplayNode);

  // now create the new markups node
  const char *className = markupsNode->GetClassName();
  vtkMRMLNode *newMRMLNode = this->mrmlScene()->CreateNodeByClass(className);
  // copy the name and let them rename it
  newMRMLNode->SetName(markupsNode->GetName());
  this->mrmlScene()->AddNode(newMRMLNode);
  // and observe the copied display node
  vtkMRMLDisplayableNode *newDisplayableNode = vtkMRMLDisplayableNode::SafeDownCast(newMRMLNode);
  newDisplayableNode->SetAndObserveDisplayNodeID(newDisplayNode->GetID());

  // set it active
  d->activeMarkupMRMLNodeComboBox->setCurrentNodeID(newMRMLNode->GetID());
  // let the user rename it
  d->activeMarkupMRMLNodeComboBox->renameCurrentNode();

  // update the display properties manually since the display node wasn't
  // observed when it was added
  this->updateWidgetFromMRML();

  // clean up
  newDisplayNode->Delete();
  newMRMLNode->Delete();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsModuleWidget::updateLogicFromSettings()
{
  // update logic from settings
  if (this->logic() == nullptr)
    {
    qWarning() << "updateLogicFromSettings: no logic to set";
    return;
    }

  if (!qSlicerApplication::application() ||
      !qSlicerApplication::application()->settingsDialog())
    {
    qWarning() << "updateLogicFromSettings: nullptr application or settings dialog";
    return;
    }

  QSettings *settings = qSlicerApplication::application()->settingsDialog()->settings();
  if (!settings)
    {
    qWarning() << "updateLogicFromSettings: nullptr settings";
    return;
    }

  // have settings been saved before?
  if (!settings->contains("Markups/GlyphType") ||
      !settings->contains("Markups/SelectedColor") ||
      !settings->contains("Markups/UnselectedColor") ||
      !settings->contains("Markups/GlyphScale") ||
      !settings->contains("Markups/TextScale") ||
      !settings->contains("Markups/Opacity") ||
      !settings->contains("Markups/SliceProjection") ||
      !settings->contains("Markups/SliceProjectionColor") ||
      !settings->contains("Markups/SliceProjectionOpacity"))
    {
    // display settings not saved yet, use defaults
    return;
    }

  QString glyphType = settings->value("Markups/GlyphType").toString();
  QColor qcolor;
  QVariant variant = settings->value("Markups/SelectedColor");
  qcolor = variant.value<QColor>();
  double selectedColor[3];
  qMRMLUtils::qColorToColor(qcolor, selectedColor);
  variant = settings->value("Markups/UnselectedColor");
  QColor qcolorUnsel = variant.value<QColor>();
  double unselectedColor[3];
  qMRMLUtils::qColorToColor(qcolorUnsel, unselectedColor);
  double glyphScale = settings->value("Markups/GlyphScale").toDouble();
  double textScale = settings->value("Markups/TextScale").toDouble();
  double opacity = settings->value("Markups/Opacity").toDouble();

  int sliceProjection = settings->value("Markups/SliceProjection").toInt();
  variant = settings->value("Markups/SliceProjectionColor");
  QColor qcolorProjection = variant.value<QColor>();
  double projectionColor[3];
  qMRMLUtils::qColorToColor(qcolorProjection, projectionColor);
  double projectionOpacity = settings->value("Markups/SliceProjectionOpacity").toDouble();

  if (!this->markupsLogic())
    {
    qWarning() << "Unable to get markups logic";
    return;
    }
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeGlyphTypeFromString(glyphType.toLatin1());
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeGlyphScale(glyphScale);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeTextScale(textScale);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeSelectedColor(selectedColor);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeColor(unselectedColor);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeOpacity(opacity);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeSliceProjection(sliceProjection);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeSliceProjectionColor(projectionColor);
  this->markupsLogic()->SetDefaultMarkupsDisplayNodeSliceProjectionOpacity(projectionOpacity);
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
    d->activeMarkupMRMLNodeComboBox->setCurrentNode(node);
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
    d->activeMarkupMRMLNodeComboBox->setCurrentNode(displayableNode);
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
