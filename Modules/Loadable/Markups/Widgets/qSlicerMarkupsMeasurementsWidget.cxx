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

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported by the Center for Intelligent Image-guided Interventions (CI3).

==============================================================================*/

// Markups Widgets includes
#include "qSlicerMarkupsMeasurementsWidget.h"
#include "ui_qSlicerMarkupsMeasurementsWidget.h"

// Markups includes
#include <vtkSlicerMarkupsLogic.h>

// Slicer includes
#include <qSlicerAbstractCoreModule.h>
#include <qSlicerApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerLayoutManager.h>

// MRML includes
#include <vtkMRMLCrosshairNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMeasurement.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceNode.h>
#include <vtkStringArray.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// Qt includes
#include <QAction>
#include <QCheckBox>
#include <QDebug>
#include <QGroupBox>
#include <QInputDialog>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QPushButton>
#include <QSignalMapper>

// qMRML includes
#include <qMRMLListWidget.h>
#include <qMRMLSortFilterSubjectHierarchyProxyModel.h>
#include <qMRMLSubjectHierarchyModel.h>

// CTK includes
#include <ctkCollapsibleGroupBox.h>
#include <ctkDynamicSpacer.h>
#include <ctkExpandableWidget.h>
#include <ctkMessageBox.h>

// VTK includes
#include <vtkCollectionIterator.h>

// Logic includes
#include <vtkSlicerMarkupsLogic.h>
#include <vtkSlicerSubjectHierarchyModuleLogic.h>

static const char* NAME_PROPERTY = "name";

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerMarkupsMeasurementsWidgetPrivate : public Ui_qSlicerMarkupsMeasurementsWidget
{
  Q_DECLARE_PUBLIC(qSlicerMarkupsMeasurementsWidget);

protected:
  qSlicerMarkupsMeasurementsWidget* const q_ptr;

public:
  qSlicerMarkupsMeasurementsWidgetPrivate(qSlicerMarkupsMeasurementsWidget& object);
  ~qSlicerMarkupsMeasurementsWidgetPrivate();
  virtual void setupUi(qSlicerMarkupsMeasurementsWidget*);

  // update the markups creation buttons.
  void createMarkupsPushButtons();

  // update the allowed markups
  void setAllowedMarkups(const QStringList& allowedMarkups);

  vtkMRMLSelectionNode* selectionNode();
  vtkMRMLInteractionNode* interactionNode();
  vtkMRMLNode* selectionNodeActivePlaceNode();
  void setSelectionNodeActivePlaceNode(vtkMRMLNode*);
  void setMRMLMarkupsNodeFromSelectionNode();
  void setPlaceModeEnabled(bool placeEnable, int persistence = 0);
  void jumpAllSlices(vtkMRMLSliceNode* exclude = nullptr);

private:
  bool JumpToSliceEnabled;
  int ViewGroup;
  unsigned int createMarkupsButtonsColumns;
  QStringList allowedMarkups;

  vtkWeakPointer<vtkMRMLMarkupsNode> CurrentMarkupsNode;
};

// --------------------------------------------------------------------------
qSlicerMarkupsMeasurementsWidgetPrivate::qSlicerMarkupsMeasurementsWidgetPrivate(qSlicerMarkupsMeasurementsWidget& object)
  : q_ptr(&object)
  , JumpToSliceEnabled(true)
  , ViewGroup(-1)
  , createMarkupsButtonsColumns(3)
  , allowedMarkups({})
{
}

//-----------------------------------------------------------------------------
qSlicerMarkupsMeasurementsWidgetPrivate::~qSlicerMarkupsMeasurementsWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidgetPrivate::setupUi(qSlicerMarkupsMeasurementsWidget* widget)
{
  Q_Q(qSlicerMarkupsMeasurementsWidget);
  this->Ui_qSlicerMarkupsMeasurementsWidget::setupUi(widget);

  this->measurementsListView->sortFilterProxyModel()->setShowEmptyHierarchyItems(false);
  this->measurementsListView->setColumnWidth(this->measurementsListView->model()->visibilityColumn(), 30);
  this->measurementsListView->setColumnWidth(this->measurementsListView->model()->colorColumn(), 30);

  if (q->markupsLogic())
  {
    // Add event observers for registration/unregistration of markups
    q->qvtkConnect(q->markupsLogic(), vtkSlicerMarkupsLogic::MarkupRegistered, q, SLOT(onCreateMarkupsPushButtons()));
    q->qvtkConnect(q->markupsLogic(), vtkSlicerMarkupsLogic::MarkupUnregistered, q, SLOT(onCreateMarkupsPushButtons()));
  }

  this->removeMeasurementPushButton->setEnabled(false);

  // set up the active markups node selector
  QObject::connect(this->measurementsListView, SIGNAL(clicked(QModelIndex)), q, SLOT(onActiveMarkupItemClicked(QModelIndex)));
  QObject::connect(this->measurementsListView, SIGNAL(currentItemChanged(vtkIdType)), q, SLOT(onActiveMarkupItemChanged(vtkIdType)));

  // listen for a right click
  this->measurementsListView->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(this->measurementsListView, SIGNAL(customContextMenuRequested(QPoint)), q, SLOT(onRightClickActiveMeasurement(QPoint)));

  QObject::connect(this->removeMeasurementPushButton, SIGNAL(clicked()), q, SLOT(onDeleteMeasurement()));

  QObject::connect(this->goToMarkupsModulePushButton, SIGNAL(clicked()), q, SLOT(onGoToMarkupsModule()));

  this->measurementSettingsCollapsibleGroupBox->setCollapsed(true);
  QHeaderView* horizontalHeader = this->measurementSettingsTableWidget->horizontalHeader();
  horizontalHeader->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidgetPrivate::setAllowedMarkups(const QStringList& allowedMarkups)
{
  Q_Q(qSlicerMarkupsMeasurementsWidget);

  this->allowedMarkups = allowedMarkups;
  QStringList registeredMarkups;
  foreach (const std::string& markupName, q->markupsLogic()->GetRegisteredMarkupsTypes())
  {
    vtkMRMLMarkupsNode* markupsNode = q->markupsLogic()->GetNodeByMarkupsType(markupName.c_str());
    if (!markupsNode)
    {
      continue;
    }

    QString className = markupsNode->GetClassName();
    if (this->allowedMarkups.size() > 0 && !this->allowedMarkups.contains(className))
    {
      continue;
    }

    registeredMarkups << className;
  }

  this->measurementsListView->setNodeTypes(registeredMarkups);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidgetPrivate::createMarkupsPushButtons()
{
  Q_Q(qSlicerMarkupsMeasurementsWidget);

  vtkMRMLApplicationLogic* appLogic = q->appLogic();
  if (!appLogic)
  {
    qCritical() << Q_FUNC_INFO << q->tr("createMarkupsPushButtons: invalid application logic.");
    return;
  }

  vtkSlicerMarkupsLogic* markupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(appLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
  {
    qCritical() << Q_FUNC_INFO << q->tr("createMarkupsPushButtons: invalid application logic.");
    return;
  }

  // NOTE: this is a temporary widget to reparent the former layout (so it will get destroyed)
  if (this->createMarkupsGroupBox->layout())
  {
    QWidget tempWidget;
    tempWidget.setLayout(this->createMarkupsGroupBox->layout());
  }

  if (this->createMarkupsButtonsColumns == 0)
  {
    this->createMarkupsGroupBox->hide();
    return;
  }

  QVBoxLayout* vlayout = new QVBoxLayout();
  QGridLayout* layout = new QGridLayout();

  vlayout->addItem(new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Fixed));

  unsigned int markupIndex = 0;
  QStringList nodeTypes;
  foreach (const std::string& markupName, q->markupsLogic()->GetRegisteredMarkupsTypes())
  {
    vtkMRMLMarkupsNode* markupsNode = markupsLogic->GetNodeByMarkupsType(markupName.c_str());
    if (!markupsNode || !q->markupsLogic()->GetCreateMarkupsPushButton(markupName.c_str()))
    {
      continue;
    }

    QString className = markupsNode->GetClassName();
    if (this->allowedMarkups.size() > 0 && !this->allowedMarkups.contains(className))
    {
      continue;
    }

    nodeTypes.append(className);

    QSignalMapper* mapper = new QSignalMapper(q);
    QPushButton* markupCreatePushButton = new QPushButton();
    // NOTE: We assign object name so we can test the dynamic creation of buttons in the tests.
    markupCreatePushButton->setObjectName(QString(q->tr("Create")) + QString(markupsNode->GetMarkupType()) + QString(q->tr("PushButton")));
    markupCreatePushButton->setIcon(QIcon(markupsNode->GetPlaceAddIcon()));
    markupCreatePushButton->setToolTip(QString(q->tr("Create ")) + QString::fromStdString(markupsNode->GetTypeDisplayName()));
    layout->addWidget(markupCreatePushButton, markupIndex / this->createMarkupsButtonsColumns, markupIndex % this->createMarkupsButtonsColumns);

    QObject::connect(markupCreatePushButton, SIGNAL(clicked()), mapper, SLOT(map()));
    mapper->setMapping(markupCreatePushButton, markupsNode->GetClassName());
    QObject::connect(mapper, SIGNAL(mapped(const QString&)), q, SLOT(onCreateMarkupByClass(const QString&)));
    ++markupIndex;
  }

  vlayout->addLayout(layout);
  ctkDynamicSpacer* spacer = new ctkDynamicSpacer();
  vlayout->addWidget(spacer);

  this->createMarkupsGroupBox->setLayout(vlayout);
  this->createMarkupsGroupBox->show();

  this->measurementsListView->setNodeTypes(nodeTypes);
}

//-----------------------------------------------------------------------------
vtkMRMLSelectionNode* qSlicerMarkupsMeasurementsWidgetPrivate::selectionNode()
{
  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
  {
    return nullptr;
  }
  return appLogic->GetSelectionNode();
}

//-----------------------------------------------------------------------------
vtkMRMLInteractionNode* qSlicerMarkupsMeasurementsWidgetPrivate::interactionNode()
{
  Q_Q(qSlicerMarkupsMeasurementsWidget);
  if (!q->mrmlScene())
  {
    return nullptr;
  }

  vtkMRMLInteractionNode* interactionNode = vtkMRMLInteractionNode::SafeDownCast(q->mrmlScene()->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
  return interactionNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerMarkupsMeasurementsWidgetPrivate::selectionNodeActivePlaceNode()
{
  Q_Q(qSlicerMarkupsMeasurementsWidget);
  vtkMRMLSelectionNode* selNode = this->selectionNode();
  if (!selNode)
  {
    return nullptr;
  }

  const char* selectionNodeActivePlaceNodeID = selNode->GetActivePlaceNodeID();
  if (!selectionNodeActivePlaceNodeID)
  {
    return nullptr;
  }
  vtkMRMLNode* activePlaceNode = q->mrmlScene()->GetNodeByID(selectionNodeActivePlaceNodeID);
  return activePlaceNode;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidgetPrivate::setSelectionNodeActivePlaceNode(vtkMRMLNode* node)
{
  Q_Q(qSlicerMarkupsMeasurementsWidget);
  if (!q->markupsLogic())
  {
    return;
  }
  vtkMRMLMarkupsNode* activePlaceNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  q->markupsLogic()->SetActiveList(activePlaceNode);
  this->removeMeasurementPushButton->setEnabled(this->CurrentMarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidgetPrivate::setMRMLMarkupsNodeFromSelectionNode()
{
  Q_Q(qSlicerMarkupsMeasurementsWidget);

  // Select current markups node
  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->selectionNodeActivePlaceNode());

  if (!currentMarkupsNode && q->mrmlScene() && this->measurementsListView->subjectHierarchyNode())
  {
    // Active place node is not a markups node then switch to the last markups node.
    vtkCollection* nodes = q->mrmlScene()->GetNodes();
    vtkMRMLSubjectHierarchyNode* shNode = this->measurementsListView->subjectHierarchyNode();
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
      QModelIndex itemIndex = this->measurementsListView->sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
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
  q->setCurrentNode(currentMarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidgetPrivate::setPlaceModeEnabled(bool placeEnable, int persistence /* = 0 */)
{
  Q_Q(qSlicerMarkupsMeasurementsWidget);
  vtkMRMLInteractionNode* interactionNode = nullptr;
  if (q->mrmlScene())
  {
    interactionNode = this->interactionNode();
  }
  if (interactionNode == nullptr)
  {
    if (placeEnable)
    {
      qCritical() << Q_FUNC_INFO << q->tr(" setPlaceModeEnabled failed: invalid interaction node");
    }
    return;
  }

  if (placeEnable)
  {
    interactionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
    interactionNode->SetPlaceModePersistence(persistence);
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
void qSlicerMarkupsMeasurementsWidgetPrivate::jumpAllSlices(vtkMRMLSliceNode* exclude /* =nullptr */)
{
  Q_Q(qSlicerMarkupsMeasurementsWidget);
  if (!this->JumpToSliceEnabled || !q->mrmlScene() || !this->CurrentMarkupsNode)
  {
    return;
  }

  vtkMRMLInteractionNode* interactionNode = this->interactionNode();

  if (!interactionNode || interactionNode->GetCurrentInteractionMode() != vtkMRMLInteractionNode::ViewTransform)
  {
    return;
  }

  if (this->CurrentMarkupsNode->GetNumberOfControlPoints() < 1)
  {
    return;
  }

  // should we calculate the real centroid? (it is an expensive operation)
  double bounds[6];
  this->CurrentMarkupsNode->GetRASBounds(bounds);
  double boundsCenter[3];
  boundsCenter[0] = bounds[0] + (bounds[1] - bounds[0]) * 0.5;
  boundsCenter[1] = bounds[2] + (bounds[3] - bounds[2]) * 0.5;
  boundsCenter[2] = bounds[4] + (bounds[5] - bounds[4]) * 0.5;

  vtkMRMLSliceNode::JumpAllSlices(q->mrmlScene(), boundsCenter[0], boundsCenter[1], boundsCenter[2], vtkMRMLSliceNode::CenteredJumpSlice, this->ViewGroup, exclude);

  vtkSmartPointer<vtkCollection> crosshairNodes;
  crosshairNodes.TakeReference(q->mrmlScene()->GetNodesByClass("vtkMRMLCrosshairNode"));
  if (!crosshairNodes.GetPointer())
  {
    return;
  }
  vtkMRMLCrosshairNode* crosshairNode = nullptr;
  vtkCollectionSimpleIterator it;
  for (crosshairNodes->InitTraversal(it); (crosshairNode = static_cast<vtkMRMLCrosshairNode*>(crosshairNodes->GetNextItemAsObject(it)));)
  {
    crosshairNode->SetCrosshairBehavior(vtkMRMLCrosshairNode::CenteredJumpSlice);
    crosshairNode->SetCrosshairRAS(boundsCenter);
  }
}

//-----------------------------------------------------------------------------
// qSlicerMarkupsMeasurementsWidget methods

//-----------------------------------------------------------------------------
qSlicerMarkupsMeasurementsWidget::qSlicerMarkupsMeasurementsWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerMarkupsMeasurementsWidgetPrivate(*this))
{
  this->setup();
}

//-----------------------------------------------------------------------------
qSlicerMarkupsMeasurementsWidget::~qSlicerMarkupsMeasurementsWidget()
{
  // remove mrml scene observations, don't need to update the GUI while the
  // module is not showing
  this->qvtkDisconnectAll();

  // remove observations from measurements
  Q_D(qSlicerMarkupsMeasurementsWidget);
  if (d->CurrentMarkupsNode)
  {
    for (int index = 0; index < d->CurrentMarkupsNode->GetNumberOfMeasurements(); ++index)
    {
      vtkMRMLMeasurement* currentMeasurement = d->CurrentMarkupsNode->GetNthMeasurement(index);
      if (currentMeasurement)
      {
        qvtkDisconnect(currentMeasurement, vtkCommand::ModifiedEvent, this, SLOT(onMeasurementModified()));
      }
    }

    qvtkDisconnect(d->CurrentMarkupsNode, vtkMRMLMarkupsNode::MeasurementsCollectionModifiedEvent, this, SLOT(onMeasurementsCollectionModified()));
  }

  this->setCurrentNode(nullptr);
}

//-----------------------------------------------------------------------------
vtkSlicerMarkupsLogic* qSlicerMarkupsMeasurementsWidget::markupsLogic()
{
  qSlicerApplication* app = qSlicerApplication::application();
  if (!app)
  {
    qCritical() << Q_FUNC_INFO << tr(": cannot get application.");
    return nullptr;
  }

  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
  {
    qCritical() << Q_FUNC_INFO << tr(": cannot get application logic.");
    return nullptr;
  }

  vtkSlicerMarkupsLogic* markupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(appLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
  {
    qCritical() << Q_FUNC_INFO << tr(": could not get the Markups module logic.");
    return nullptr;
  }

  return markupsLogic;
}

//-----------------------------------------------------------------------------
ctkCollapsibleGroupBox* qSlicerMarkupsMeasurementsWidget::createMarkupsGroupBox()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  return d->createMarkupsGroupBox;
}

//-----------------------------------------------------------------------------
ctkCollapsibleGroupBox* qSlicerMarkupsMeasurementsWidget::measurementsGroupBox()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  return d->measurementsGroupBox;
}

//-----------------------------------------------------------------------------
ctkCollapsibleGroupBox* qSlicerMarkupsMeasurementsWidget::measurementDisplayGroupBox()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  return d->measurementDisplayGroupBox;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::setup()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::setCurrentNode(vtkMRMLNode* currentNode)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  if (!this->mrmlScene())
  {
    return;
  }

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(currentNode);
  if (currentMarkupsNode == d->CurrentMarkupsNode)
  {
    // not changed
    return;
  }

  if (currentMarkupsNode)
  {
    QString className = currentMarkupsNode->GetClassName();
    if (d->allowedMarkups.size() > 0 && !d->allowedMarkups.contains(className))
    {
      this->setCurrentNode(nullptr);
      return;
    }
  }

  // Reconnect the appropriate nodes
  if (d->CurrentMarkupsNode)
  {
    qvtkDisconnect(d->CurrentMarkupsNode, vtkMRMLMarkupsNode::MeasurementsCollectionModifiedEvent, this, SLOT(onMeasurementsCollectionModified()));
  }
  if (currentMarkupsNode)
  {
    qvtkConnect(currentMarkupsNode, vtkMRMLMarkupsNode::MeasurementsCollectionModifiedEvent, this, SLOT(onMeasurementsCollectionModified()));
  }

  d->CurrentMarkupsNode = currentMarkupsNode;
  bool wasBlocked = d->measurementsListView->blockSignals(true);
  if (!d->CurrentMarkupsNode)
  {
    d->measurementsListView->setCurrentItem(-1);
  }
  else
  {
    d->measurementsListView->setCurrentNode(d->CurrentMarkupsNode);
    vtkMRMLSubjectHierarchyNode* shNode = d->measurementsListView->subjectHierarchyNode();
    if (shNode)
    {
      vtkIdType itemID = shNode->GetItemByDataNode(d->CurrentMarkupsNode);
      QModelIndex itemIndex = d->measurementsListView->sortFilterProxyModel()->indexFromSubjectHierarchyItem(itemID);
      if (itemIndex.row() >= 0)
      {
        d->measurementsListView->scrollTo(itemIndex);
      }
    }
  }
  d->measurementsListView->blockSignals(wasBlocked);

  this->onMeasurementsCollectionModified();
  this->updateMeasurementsDescriptionLabel();
  d->removeMeasurementPushButton->setEnabled(d->CurrentMarkupsNode);
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsMeasurementsWidget::jumpToSliceEnabled() const
{
  Q_D(const qSlicerMarkupsMeasurementsWidget);
  return d->JumpToSliceEnabled;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::setJumpToSliceEnabled(bool enable)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  d->JumpToSliceEnabled = enable;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::setViewGroup(int newViewGroup)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  d->ViewGroup = newViewGroup;
}

//-----------------------------------------------------------------------------
int qSlicerMarkupsMeasurementsWidget::viewGroup() const
{
  Q_D(const qSlicerMarkupsMeasurementsWidget);
  return d->ViewGroup;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::setCreateMarkupsButtonsColumns(unsigned int columns)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  d->createMarkupsButtonsColumns = columns;
  this->onCreateMarkupsPushButtons();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onNodeAddedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);

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
  this->setCurrentNode(markupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onNodeRemovedEvent(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  Q_UNUSED(node);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onActiveMarkupItemClicked(QModelIndex)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  this->onActiveMarkupMRMLNodeChanged(d->measurementsListView->currentNode());
  d->jumpAllSlices();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onActiveMarkupItemChanged(vtkIdType)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  this->onActiveMarkupMRMLNodeChanged(d->measurementsListView->currentNode());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onActiveMarkupMRMLNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode || markupsNode == d->CurrentMarkupsNode)
  {
    // not changed
    return;
  }

  d->setSelectionNodeActivePlaceNode(markupsNode);
  this->setCurrentNode(markupsNode);
}

//-----------------------------------------------------------------------------
int qSlicerMarkupsMeasurementsWidget::createMarkupsButtonsColumns() const
{
  Q_D(const qSlicerMarkupsMeasurementsWidget);
  return d->createMarkupsButtonsColumns;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onSelectionNodeActivePlaceNodeIDChanged()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->selectionNodeActivePlaceNode());
  this->setCurrentNode(currentMarkupsNode);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onEndPlacementEvent()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  if (!this->mrmlScene())
  {
    return;
  }

  vtkMRMLMarkupsNode* currentMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(d->selectionNodeActivePlaceNode());
  if (!currentMarkupsNode)
  {
    return;
  }

  QString className = currentMarkupsNode->GetClassName();
  if (d->allowedMarkups.size() > 0 && !d->allowedMarkups.contains(className))
  {
    return;
  }

  QString isAMeasurement = currentMarkupsNode->GetAttribute("deleteIfMeasurementsNotValid");
  if (isAMeasurement.compare("true") != 0 || currentMarkupsNode->IsA("vtkMRMLMarkupsFiducialNode"))
  {
    return;
  }

  if (currentMarkupsNode->GetNumberOfEnabledAndDefinedMeasurements() == 0)
  {
    this->mrmlScene()->RemoveNode(currentMarkupsNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onMRMLSceneEndImportEvent()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  d->setMRMLMarkupsNodeFromSelectionNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onMRMLSceneEndRestoreEvent()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  d->setMRMLMarkupsNodeFromSelectionNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onMRMLSceneEndBatchProcessEvent()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  if (!this->mrmlScene())
  {
    return;
  }

  d->setMRMLMarkupsNodeFromSelectionNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onMRMLSceneEndCloseEvent()
{
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
  {
    return;
  }

  this->setCurrentNode(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onMeasurementsCollectionModified()
{
  this->observeMeasurementsInCurrentMarkupsNode();
  this->populateMeasurementSettingsTable();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::observeMeasurementsInCurrentMarkupsNode()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  if (!d->CurrentMarkupsNode)
  {
    return;
  }

  for (int index = 0; index < d->CurrentMarkupsNode->GetNumberOfMeasurements(); ++index)
  {
    vtkMRMLMeasurement* currentMeasurement = d->CurrentMarkupsNode->GetNthMeasurement(index);
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
void qSlicerMarkupsMeasurementsWidget::onMeasurementModified(vtkObject* caller)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

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
        QWidget* layoutObject = d->measurementSettingsTableWidget->cellWidget(nameItem->row(), 1);
        if (!layoutObject)
        {
          continue;
        }

        QCheckBox* enabledCheckbox = layoutObject->findChild<QCheckBox*>("enabledCheckbox");
        if (!enabledCheckbox)
        {
          continue;
        }

        enabledCheckbox->setChecked(measurement->GetEnabled());
      }
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onMeasurementEnabledCheckboxToggled(bool on)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  if (!d->CurrentMarkupsNode)
  {
    return;
  }

  // Get measurement name from checkbox
  QCheckBox* checkbox = qobject_cast<QCheckBox*>(this->sender());
  QString measurementName = checkbox->property(NAME_PROPERTY).toString();

  // Enable/disable measurement with name
  for (int index = 0; index < d->CurrentMarkupsNode->GetNumberOfMeasurements(); ++index)
  {
    vtkMRMLMeasurement* currentMeasurement = d->CurrentMarkupsNode->GetNthMeasurement(index);
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
void qSlicerMarkupsMeasurementsWidget::onRightClickActiveMeasurement(QPoint pos)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  Q_UNUSED(pos);
  if (!d->CurrentMarkupsNode)
  {
    return;
  }

  QMenu menu;
  // Rename
  QAction* renameMarkupsAction = new QAction(QString("Rename"), &menu);
  menu.addAction(renameMarkupsAction);
  QObject::connect(renameMarkupsAction, SIGNAL(triggered()), this, SLOT(onRenameMeasurement()));

  // Clone
  QAction* cloneMarkupsAction = new QAction(QString("Clone"), &menu);
  menu.addAction(cloneMarkupsAction);
  QObject::connect(cloneMarkupsAction, SIGNAL(triggered()), this, SLOT(onCloneMeasurement()));

  // Delete
  QAction* deleteMarkupsAction = new QAction(QString("Delete"), &menu);
  menu.addAction(deleteMarkupsAction);
  QObject::connect(deleteMarkupsAction, SIGNAL(triggered()), this, SLOT(onDeleteMeasurement()));

  menu.exec(QCursor::pos());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onDeleteMeasurement()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  if (!d->CurrentMarkupsNode)
  {
    return;
  }

  if (!this->mrmlScene() || !qSlicerApplication::application())
  {
    return;
  }

  QWidget* mainWindow = qSlicerApplication::application()->mainWindow();
  if (!mainWindow)
  {
    return;
  }

  ctkMessageBox warningMessageBox(mainWindow);
  warningMessageBox.setWindowTitle(tr("Deleting Measurement"));
  warningMessageBox.setText(tr("The selected measurement will be permanently deleted. Do you want to proceed? \n"));
  warningMessageBox.setIcon(QMessageBox::Warning);
  warningMessageBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
  int resultCode = warningMessageBox.exec();
  if (resultCode == QMessageBox::Cancel)
  {
    return;
  }

  d->setPlaceModeEnabled(false);
  vtkWeakPointer<vtkMRMLMarkupsNode> markupsToRemove = d->CurrentMarkupsNode;
  this->setCurrentNode(nullptr);
  this->mrmlScene()->RemoveNode(markupsToRemove);
}

//-----------------------------------------------------------------------------
bool qSlicerMarkupsMeasurementsWidget::onGoToMarkupsModule()
{
  qSlicerModuleManager* moduleManager = qSlicerCoreApplication::application()->moduleManager();
  if (!moduleManager)
  {
    return false;
  }
  qSlicerAbstractCoreModule* module = moduleManager->module("Markups");
  if (!module)
  {
    return false;
  }
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
  {
    return false;
  }

  layoutManager->setCurrentModule("Markups");
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::updateMeasurementsDescriptionLabel()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  if (!d->CurrentMarkupsNode || d->CurrentMarkupsNode->GetNumberOfEnabledAndDefinedMeasurements() == 0)
  {
    d->measurementsLabel->setText(this->tr("No measurement"));
    return;
  }

  QString measurementsString;
  if (d->CurrentMarkupsNode->GetNumberOfEnabledAndDefinedMeasurements() == 1)
  {
    vtkMRMLMeasurement* measurement = d->CurrentMarkupsNode->GetNthMeasurement(0);
    if (measurement && measurement->GetEnabled() && measurement->GetValueDefined())
    {
      QString name = QString::fromStdString(measurement->GetName());
      measurementsString = name + ": " + QString::fromStdString(d->CurrentMarkupsNode->GetPropertiesLabelText());
    }
  }
  else
  {
    measurementsString = QString::fromStdString(d->CurrentMarkupsNode->GetPropertiesLabelText());
  }

  d->measurementsLabel->setText(measurementsString.isEmpty() ? this->tr("No measurement") : measurementsString);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::populateMeasurementSettingsTable()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  d->measurementSettingsTableWidget->clear();
  d->measurementSettingsTableWidget->setVisible(d->CurrentMarkupsNode != nullptr && d->CurrentMarkupsNode->GetNumberOfMeasurements() > 0);

  if (!d->CurrentMarkupsNode)
  {
    return;
  }

  d->measurementSettingsTableWidget->setHorizontalHeaderLabels(QStringList() << "Name" << "Enabled");
  d->measurementSettingsTableWidget->setRowCount(d->CurrentMarkupsNode->GetNumberOfMeasurements());
  for (int index = 0; index < d->CurrentMarkupsNode->GetNumberOfMeasurements(); ++index)
  {
    vtkMRMLMeasurement* currentMeasurement = d->CurrentMarkupsNode->GetNthMeasurement(index);
    if (!currentMeasurement)
    {
      continue;
    }

    QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(currentMeasurement->GetName()));
    d->measurementSettingsTableWidget->setItem(index, 0, nameItem);

    QCheckBox* enabledCheckbox = new QCheckBox();
    enabledCheckbox->setObjectName("enabledCheckbox");
    enabledCheckbox->setChecked(currentMeasurement->GetEnabled());
    enabledCheckbox->setProperty(NAME_PROPERTY, QString::fromStdString(currentMeasurement->GetName()));

    QWidget* layoutObject = new QWidget();
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setAlignment(Qt::AlignCenter);
    hLayout->addWidget(enabledCheckbox);
    layoutObject->setLayout(hLayout);

    QObject::connect(enabledCheckbox, SIGNAL(toggled(bool)), this, SLOT(onMeasurementEnabledCheckboxToggled(bool)));
    d->measurementSettingsTableWidget->setCellWidget(index, 1, layoutObject);
    d->measurementSettingsTableWidget->setRowHeight(index, enabledCheckbox->sizeHint().height());
  }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onRenameMeasurement()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  if (!d->CurrentMarkupsNode)
  {
    return;
  }
  // pop up an entry box for the new name, with the old name as default
  QString oldName = d->CurrentMarkupsNode->GetName();

  bool ok = false;
  QString newName = QInputDialog::getText(this, "Rename " + oldName, "New name:", QLineEdit::Normal, oldName, &ok);
  if (!ok)
  {
    return;
  }
  d->CurrentMarkupsNode->SetName(newName.toUtf8());
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onCloneMeasurement()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  if (!d->CurrentMarkupsNode)
  {
    return;
  }

  vtkMRMLSubjectHierarchyNode* shNode = d->measurementsListView->subjectHierarchyNode();
  vtkIdType itemIDToClone = shNode->GetItemByDataNode(d->CurrentMarkupsNode);
  vtkSlicerSubjectHierarchyModuleLogic::CloneSubjectHierarchyItem(shNode, itemIDToClone);
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->setMRMLScene(scene, {});
}

//------------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::setMRMLScene(vtkMRMLScene* scene, const QStringList& allowedMarkups)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  this->Superclass::setMRMLScene(scene);

  d->measurementsListView->setMRMLScene(scene);

  d->setAllowedMarkups(allowedMarkups);
  d->createMarkupsPushButtons();

  this->qvtkConnect(scene, vtkMRMLScene::NodeAddedEvent, this, SLOT(onNodeAddedEvent(vtkObject*, vtkObject*)));
  this->qvtkConnect(scene, vtkMRMLScene::NodeRemovedEvent, this, SLOT(onNodeRemovedEvent(vtkObject*, vtkObject*)));

  vtkMRMLNode* selectionNode = d->selectionNode();
  if (selectionNode)
  {
    this->qvtkConnect(selectionNode, vtkMRMLSelectionNode::ActivePlaceNodeIDChangedEvent, this, SLOT(onSelectionNodeActivePlaceNodeIDChanged()));
  }

  vtkMRMLNode* interactionNode = d->interactionNode();
  if (interactionNode)
  {
    this->qvtkConnect(interactionNode, vtkMRMLInteractionNode::EndPlacementEvent, this, SLOT(onEndPlacementEvent()));
  }

  d->setMRMLMarkupsNodeFromSelectionNode();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onCreateMarkupsPushButtons()
{
  Q_D(qSlicerMarkupsMeasurementsWidget);

  d->createMarkupsPushButtons();
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onCreateMarkupByClass(const QString& className)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  if (!this->markupsLogic())
  {
    qWarning() << Q_FUNC_INFO << " failed: invalid markups logic";
    return;
  }

  d->setPlaceModeEnabled(false);
  vtkMRMLMarkupsNode* markupsNode = this->markupsLogic()->AddNewMarkupsNode(className.toStdString());
  // deleteIfMeasurementsNotValid attribute is necessary to indicate that the markups has been
  // generated from the Measurement panel. Markups added from this widget will be delete if
  // they have no valid measurement at the end of the placement.
  markupsNode->SetAttribute("deleteIfMeasurementsNotValid", "true");

  // Check if the active measurements have been overridden
  vtkMRMLMarkupsNode* defaultMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(this->mrmlScene()->GetDefaultNodeByClass(className.toStdString().c_str()));
  if (defaultMarkupsNode)
  {
    vtkStringArray* defaultMeasurements = defaultMarkupsNode->GetDefaultMeasurements();
    if (defaultMeasurements->GetNumberOfValues() != 0)
    {
      markupsNode->DisableAllMeasurements();
      for (int index = 0; index < defaultMeasurements->GetNumberOfValues(); ++index)
      {
        vtkStdString measurementName = defaultMeasurements->GetValue(index);
        markupsNode->EnableMeasurement(measurementName.c_str());
      }
    }
  }

  // Check if there are no measurement enabled, and if none enable the first one
  if (markupsNode->GetNumberOfEnabledMeasurements() == 0)
  {
    markupsNode->EnableMeasurement(markupsNode->GetMeasurementNameFromIndex(0).c_str());
  }

  if (markupsNode)
  {
    this->onActiveMarkupMRMLNodeAdded(markupsNode);
  }
}

//-----------------------------------------------------------------------------
void qSlicerMarkupsMeasurementsWidget::onActiveMarkupMRMLNodeAdded(vtkMRMLNode* node)
{
  Q_D(qSlicerMarkupsMeasurementsWidget);
  if (this->markupsLogic())
  {
    this->markupsLogic()->AddNewDisplayNodeForMarkupsNode(node);
  }
  // make sure it's set up for the mouse mode tool bar to easily add points to
  // it by making it active in the selection node
  d->setSelectionNodeActivePlaceNode(node);

  vtkMRMLMarkupsNode* activePlaceNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  d->setPlaceModeEnabled(true, activePlaceNode->IsA("vtkMRMLMarkupsFiducialNode") ? 1 : 0);
}
