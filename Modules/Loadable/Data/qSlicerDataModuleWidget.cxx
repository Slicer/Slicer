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

// Slicer includes
#include "qSlicerDataModuleWidget.h"
#include "ui_qSlicerDataModuleWidget.h"
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"

// Data Logic includes
#include "vtkSlicerDataModuleLogic.h"

// Subject Hierarchy includes
#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"

// MRMLWidgets includes
#include <qMRMLSceneModel.h>

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>

// Qt includes
#include <QAction>
#include <QDebug>
#include <QTimer>

//-----------------------------------------------------------------------------
class qSlicerDataModuleWidgetPrivate: public Ui_qSlicerDataModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerDataModuleWidget);
protected:
  qSlicerDataModuleWidget* const q_ptr;
public:
  qSlicerDataModuleWidgetPrivate(qSlicerDataModuleWidget& object);
  ~qSlicerDataModuleWidgetPrivate();
  vtkSlicerDataModuleLogic* logic() const;
public:
  QAction* HardenTransformAction;
  int ContextMenusHintShown;

  /// Callback object to get notified about item modified events
  vtkSmartPointer<vtkCallbackCommand> CallBack;

  /// Observer tag for subject hierarchy observation
  unsigned long SubjectHierarchyObservationTag;
};

//-----------------------------------------------------------------------------
// qSlicerDataModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerDataModuleWidgetPrivate::qSlicerDataModuleWidgetPrivate(qSlicerDataModuleWidget& object)
  : q_ptr(&object)
  , HardenTransformAction(nullptr)
  , ContextMenusHintShown(0)
  , SubjectHierarchyObservationTag(0)
{
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->CallBack->SetClientData(q_ptr);
  this->CallBack->SetCallback(qSlicerDataModuleWidget::onSubjectHierarchyItemEvent);
}

//-----------------------------------------------------------------------------
qSlicerDataModuleWidgetPrivate::~qSlicerDataModuleWidgetPrivate()
{
  vtkMRMLSubjectHierarchyNode* shNode = this->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (shNode)
    {
    shNode->RemoveObserver(this->SubjectHierarchyObservationTag);
    }
}

//-----------------------------------------------------------------------------
vtkSlicerDataModuleLogic*
qSlicerDataModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerDataModuleWidget);
  return vtkSlicerDataModuleLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
// qSlicerDataModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerDataModuleWidget::qSlicerDataModuleWidget(QWidget* parentWidget)
  :qSlicerAbstractModuleWidget(parentWidget)
  , d_ptr( new qSlicerDataModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerDataModuleWidget::~qSlicerDataModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::enter()
{
  Q_D(qSlicerDataModuleWidget);

  // Trigger showing the subject hierarchy context menu hint
  this->onCurrentTabChanged(d->ViewTabWidget->currentIndex());

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::setup()
{
  Q_D(qSlicerDataModuleWidget);

  d->setupUi(this);

  // Tab widget
  d->ViewTabWidget->widget(TabIndexSubjectHierarchy)->layout()->setContentsMargins(2,2,2,2);
  d->ViewTabWidget->widget(TabIndexSubjectHierarchy)->layout()->setSpacing(4);

  d->ViewTabWidget->widget(TabIndexTransformHierarchy)->layout()->setContentsMargins(2,2,2,2);
  d->ViewTabWidget->widget(TabIndexTransformHierarchy)->layout()->setSpacing(4);

  d->ViewTabWidget->widget(TabIndexAllNodes)->layout()->setContentsMargins(2,2,2,2);
  d->ViewTabWidget->widget(TabIndexAllNodes)->layout()->setSpacing(4);

  connect( d->ViewTabWidget, SIGNAL(currentChanged(int)),
          this, SLOT(onCurrentTabChanged(int)) );

  //
  // Subject hierarchy tab

  // Make connections for the checkboxes and buttons
  connect( d->SubjectHierarchyDisplayDataNodeIDsCheckBox, SIGNAL(toggled(bool)),
           this, SLOT(setMRMLIDsVisible(bool)) );
  connect( d->SubjectHierarchyDisplayTransformsCheckBox, SIGNAL(toggled(bool)),
           this, SLOT(setTransformsVisible(bool)) );

  // Set up tree view
  d->SubjectHierarchyTreeView->expandToDepth(4);
  d->SubjectHierarchyTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
  // Make subject hierarchy item info label text selectable
  d->SubjectHierarchyItemInfoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

  connect(d->SubjectHierarchyTreeView, SIGNAL(currentItemChanged(vtkIdType)),
           this, SLOT(setDataNodeFromSubjectHierarchyItem(vtkIdType)) );
  connect(d->SubjectHierarchyTreeView, SIGNAL(currentItemChanged(vtkIdType)),
           this, SLOT(setInfoLabelFromSubjectHierarchyItem(vtkIdType)) );

  // Connect name filter
  connect( d->FilterLineEdit, SIGNAL(textChanged(QString)),
           d->SubjectHierarchyTreeView->sortFilterProxyModel(), SLOT(setNameFilter(QString)) );

  // Help button
  connect( d->HelpButton, SIGNAL(clicked()),
           this, SLOT(onHelpButtonClicked()) );
  // Assemble help text for help button tooltip
  QString aggregatedHelpText(
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
    "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\"> p, li { white-space: pre-wrap; }"
    "</style></head><body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">");
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    // Add help text from each plugin
    QString pluginHelpText = plugin->helpText();
    if (!pluginHelpText.isEmpty())
      {
      aggregatedHelpText.append(QString("\n") + pluginHelpText);
      }
    }
  aggregatedHelpText.append(QString("</body></html>"));
  d->HelpButton->setToolTip(aggregatedHelpText);

  //
  // Transform hierarchy tab

  d->TransformMRMLTreeView->sceneModel()->setIDColumn(1);
  d->TransformMRMLTreeView->sceneModel()->setHorizontalHeaderLabels(QStringList() << tr("Nodes") << tr("IDs"));
  d->TransformMRMLTreeView->header()->setStretchLastSection(false);
  d->TransformMRMLTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  d->TransformMRMLTreeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

  connect( d->TransformMRMLTreeView, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
           this, SLOT(onCurrentNodeChanged(vtkMRMLNode*)) );

  // Edit properties
  connect( d->TransformMRMLTreeView, SIGNAL(editNodeRequested(vtkMRMLNode*)),
           qSlicerApplication::application(), SLOT(openNodeModule(vtkMRMLNode*)) );
  // Insert transform
  QAction* insertTransformAction = new QAction(tr("Insert transform"),this);
  d->TransformMRMLTreeView->prependNodeMenuAction(insertTransformAction);
  d->TransformMRMLTreeView->prependSceneMenuAction(insertTransformAction);
  connect( insertTransformAction, SIGNAL(triggered()),
           this, SLOT(insertTransformNode()) );
  // Harden transform
  d->HardenTransformAction = new QAction(tr("Harden transform"), this);
  connect( d->HardenTransformAction, SIGNAL(triggered()),
           this, SLOT(hardenTransformOnCurrentNode()) );
  // Checkboxes
  connect( d->TransformDisplayMRMLIDsCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(setMRMLIDsVisible(bool)) );
  connect( d->TransformShowHiddenCheckBox, SIGNAL(toggled(bool)),
          d->TransformMRMLTreeView->sortFilterProxyModel(), SLOT(setShowHidden(bool)) );
  connect( d->TransformShowHiddenCheckBox, SIGNAL(toggled(bool)),
          d->AllNodesTransformShowHiddenCheckBox, SLOT(setChecked(bool)) );
  // Filter on all the columns
  d->TransformMRMLTreeView->sortFilterProxyModel()->setFilterKeyColumn(-1);
  connect( d->FilterLineEdit, SIGNAL(textChanged(QString)),
          d->TransformMRMLTreeView->sortFilterProxyModel(), SLOT(setFilterWildcard(QString)) );
  // Make connections for the attribute table widget
  connect( d->TransformMRMLTreeView, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          d->MRMLNodeAttributeTableWidget, SLOT(setMRMLNode(vtkMRMLNode*)) );

  //
  // All nodes tab

  d->AllNodesMRMLTreeView->sceneModel()->setIDColumn(1);
  d->AllNodesMRMLTreeView->sceneModel()->setHorizontalHeaderLabels(QStringList() << tr("Nodes") << tr("IDs"));
  d->AllNodesMRMLTreeView->header()->setStretchLastSection(false);
  d->AllNodesMRMLTreeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  d->AllNodesMRMLTreeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

  // Edit properties
  connect( d->AllNodesMRMLTreeView, SIGNAL(editNodeRequested(vtkMRMLNode*)),
           qSlicerApplication::application(), SLOT(openNodeModule(vtkMRMLNode*)) );
  // Checkboxes
  connect( d->AllNodesDisplayMRMLIDsCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(setMRMLIDsVisible(bool)) );
  connect( d->AllNodesTransformShowHiddenCheckBox, SIGNAL(toggled(bool)),
          d->AllNodesMRMLTreeView->sortFilterProxyModel(), SLOT(setShowHidden(bool)) );
  connect( d->AllNodesTransformShowHiddenCheckBox, SIGNAL(toggled(bool)),
          d->TransformShowHiddenCheckBox, SLOT(setChecked(bool)) );
  // Filter on all the columns
  d->AllNodesMRMLTreeView->sortFilterProxyModel()->setFilterKeyColumn(-1);
  connect( d->FilterLineEdit, SIGNAL(textChanged(QString)),
          d->AllNodesMRMLTreeView->sortFilterProxyModel(), SLOT(setFilterWildcard(QString)) );
  // Make connections for the attribute table widget
  connect( d->AllNodesMRMLTreeView, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          d->MRMLNodeAttributeTableWidget, SLOT(setMRMLNode(vtkMRMLNode*)) );
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerDataModuleWidget);

  Superclass::setMRMLScene(scene);

  this->setMRMLIDsVisible(d->SubjectHierarchyDisplayDataNodeIDsCheckBox->isChecked());
  this->setTransformsVisible(d->SubjectHierarchyDisplayTransformsCheckBox->isChecked());
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::setMRMLIDsVisible(bool visible)
{
  Q_D(qSlicerDataModuleWidget);

  // Subject hierarchy view
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->idColumn(), !visible);

  // Transform hierarchy view
  d->TransformMRMLTreeView->setColumnHidden(d->TransformMRMLTreeView->sceneModel()->idColumn(), !visible);
  int columnCount = d->TransformMRMLTreeView->header()->count();
  for (int i=0; i<columnCount; ++i)
    {
    d->TransformMRMLTreeView->resizeColumnToContents(i);
    }

  // All nodes view
  d->AllNodesMRMLTreeView->setColumnHidden(d->AllNodesMRMLTreeView->sceneModel()->idColumn(), !visible);
  columnCount = d->AllNodesMRMLTreeView->header()->count();
  for (int i=0; i<columnCount; ++i)
    {
    d->AllNodesMRMLTreeView->resizeColumnToContents(i);
    }

  // Update each checkbox that represent the same thing
  bool wereSignalsBlocked = d->SubjectHierarchyDisplayDataNodeIDsCheckBox->blockSignals(true);
  d->SubjectHierarchyDisplayDataNodeIDsCheckBox->setChecked(visible);
  d->SubjectHierarchyDisplayDataNodeIDsCheckBox->blockSignals(wereSignalsBlocked);

  wereSignalsBlocked = d->TransformDisplayMRMLIDsCheckBox->blockSignals(true);
  d->TransformDisplayMRMLIDsCheckBox->setChecked(visible);
  d->TransformDisplayMRMLIDsCheckBox->blockSignals(wereSignalsBlocked);

  wereSignalsBlocked = d->AllNodesDisplayMRMLIDsCheckBox->blockSignals(true);
  d->AllNodesDisplayMRMLIDsCheckBox->setChecked(visible);
  d->AllNodesDisplayMRMLIDsCheckBox->blockSignals(wereSignalsBlocked);
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::onCurrentTabChanged(int tabIndex)
{
  Q_D(qSlicerDataModuleWidget);

  if (tabIndex == TabIndexSubjectHierarchy)
    {
    // Prevent the taller widget affect the size of the other
    d->TransformMRMLTreeView->setVisible(false);
    d->SubjectHierarchyTreeView->setVisible(true);
    d->AllNodesMRMLTreeView->setVisible(false);

    // Make sure MRML node attribute widget is updated
    this->setDataNodeFromSubjectHierarchyItem(d->SubjectHierarchyTreeView->currentItem());
    }
  else if (tabIndex == TabIndexTransformHierarchy)
    {
    // Prevent the taller widget affect the size of the other
    d->TransformMRMLTreeView->setVisible(true);
    d->SubjectHierarchyTreeView->setVisible(false);
    d->AllNodesMRMLTreeView->setVisible(false);

    // MRML node attribute widget always enabled in transform mode
    d->MRMLNodeAttributeTableWidget->setEnabled(true);
    // Make sure MRML node attribute widget is updated
    d->MRMLNodeAttributeTableWidget->setMRMLNode(d->TransformMRMLTreeView->currentNode());
    }
  else if (tabIndex == TabIndexAllNodes)
    {
    // Prevent the taller widget affect the size of the other
    d->TransformMRMLTreeView->setVisible(false);
    d->SubjectHierarchyTreeView->setVisible(false);
    d->AllNodesMRMLTreeView->setVisible(true);

    // MRML node attribute widget always enabled in all nodes mode
    d->MRMLNodeAttributeTableWidget->setEnabled(true);
    // Make sure MRML node attribute widget is updated
    d->MRMLNodeAttributeTableWidget->setMRMLNode(d->AllNodesMRMLTreeView->currentNode());
    }
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::onCurrentNodeChanged(vtkMRMLNode* newCurrentNode)
{
  Q_D(qSlicerDataModuleWidget);
  vtkMRMLTransformableNode* transformableNode =
    vtkMRMLTransformableNode::SafeDownCast(newCurrentNode);
  vtkMRMLTransformNode* transformNode =
    transformableNode ? transformableNode->GetParentTransformNode() : nullptr;
  if (transformNode &&
      (transformNode->CanApplyNonLinearTransforms() ||
      transformNode->IsTransformToWorldLinear()))
    {
    d->TransformMRMLTreeView->prependNodeMenuAction(d->HardenTransformAction);
    }
  else
    {
    d->TransformMRMLTreeView->removeNodeMenuAction(d->HardenTransformAction);
    }
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::insertTransformNode()
{
  Q_D(qSlicerDataModuleWidget);
  vtkNew<vtkMRMLLinearTransformNode> linearTransform;
  this->mrmlScene()->AddNode(linearTransform.GetPointer());

  vtkMRMLNode* parent = vtkMRMLTransformNode::SafeDownCast(
    d->TransformMRMLTreeView->currentNode());
  if (parent)
    {
    linearTransform->SetAndObserveTransformNodeID( parent->GetID() );
    }
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::hardenTransformOnCurrentNode()
{
  Q_D(qSlicerDataModuleWidget);
  vtkMRMLNode* node = d->TransformMRMLTreeView->currentNode();
  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(node);
  if (transformableNode)
    {
    transformableNode->HardenTransform();
    }
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::setTransformsVisible(bool visible)
{
  Q_D(qSlicerDataModuleWidget);

  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(d->SubjectHierarchyTreeView->model());
  d->SubjectHierarchyTreeView->setColumnHidden(model->transformColumn(), !visible);
  d->SubjectHierarchyTreeView->header()->resizeSection(model->transformColumn(), 60);

  bool wereSignalsBlocked = d->SubjectHierarchyDisplayTransformsCheckBox->blockSignals(true);
  d->SubjectHierarchyDisplayTransformsCheckBox->setChecked(visible);
  d->SubjectHierarchyDisplayTransformsCheckBox->blockSignals(wereSignalsBlocked);
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::setDataNodeFromSubjectHierarchyItem(vtkIdType itemID)
{
  Q_D(qSlicerDataModuleWidget);

  vtkMRMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  vtkMRMLNode* dataNode = nullptr;
  if (itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    dataNode = shNode->GetItemDataNode(itemID);
    }
  d->MRMLNodeAttributeTableWidget->setEnabled(dataNode);
  d->MRMLNodeAttributeTableWidget->setMRMLNode(dataNode);
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::setInfoLabelFromSubjectHierarchyItem(vtkIdType itemID)
{
  Q_D(qSlicerDataModuleWidget);

  vtkMRMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }

  if (itemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    std::stringstream infoStream;
    shNode->PrintItem(itemID, infoStream, vtkIndent(0));
    d->SubjectHierarchyItemInfoLabel->setText(infoStream.str().c_str());

    // Connect node for updating info label
    if (!shNode->HasObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, d->CallBack))
      {
      d->SubjectHierarchyObservationTag = shNode->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, d->CallBack, -10.0);
      }
    }
  else
    {
    d->SubjectHierarchyItemInfoLabel->setText(tr("No item selected"));
    }

  // Store item ID in the label object
  d->SubjectHierarchyItemInfoLabel->setProperty("itemID", itemID);
}

//-----------------------------------------------------------------------------
void qSlicerDataModuleWidget::onSubjectHierarchyItemEvent(
  vtkObject* caller, unsigned long event, void* clientData, void* callData )
{
  vtkMRMLSubjectHierarchyNode* shNode = reinterpret_cast<vtkMRMLSubjectHierarchyNode*>(caller);
  qSlicerDataModuleWidget* widget = reinterpret_cast<qSlicerDataModuleWidget*>(clientData);
  if (!widget || !shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid event parameters";
    return;
    }

  // Get item ID
  vtkIdType itemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  if (callData)
    {
    vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
    if (itemIdPtr)
      {
      itemID = *itemIdPtr;
      }
    }

  switch (event)
    {
    case vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent:
      widget->onSubjectHierarchyItemModified(itemID);
      break;
    }
}

//------------------------------------------------------------------------------
void qSlicerDataModuleWidget::onSubjectHierarchyItemModified(vtkIdType itemID)
{
  Q_D(qSlicerDataModuleWidget);

  // Get displayed item's ID from label object
  vtkIdType displayedItemID = d->SubjectHierarchyItemInfoLabel->property("itemID").toLongLong();

  // Update label if the displayed item is the one that changed
  if (displayedItemID == itemID && displayedItemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    vtkMRMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
    if (!shNode)
      {
      qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
      return;
      }

    std::stringstream infoStream;
    shNode->PrintItem(itemID, infoStream, vtkIndent(0));
    d->SubjectHierarchyItemInfoLabel->setText(infoStream.str().c_str());
    }
}

//-----------------------------------------------------------------------------
qMRMLSubjectHierarchyModel* qSlicerDataModuleWidget::subjectHierarchySceneModel()const
{
  Q_D(const qSlicerDataModuleWidget);

  qMRMLSubjectHierarchyModel* model = qobject_cast<qMRMLSubjectHierarchyModel*>(d->SubjectHierarchyTreeView->model());
  return model;
}

//------------------------------------------------------------------------------
void qSlicerDataModuleWidget::onHelpButtonClicked()
{
  Q_D(qSlicerDataModuleWidget);

  // Reset counter so that it is shown
  if (d->ContextMenusHintShown >= 2)
    {
    d->ContextMenusHintShown = 0;
    }

  // Show first or second tooltip
  if (d->SubjectHierarchyTreeView->showContextMenuHint(d->ContextMenusHintShown > 0))
    {
    d->ContextMenusHintShown++;
    }
}
