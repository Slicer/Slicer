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
#include <QDebug>
#include <QScrollArea>

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <qSlicerAbstractCoreModule.h>

// Subject hierarchy widgets
#include "qMRMLSubjectHierarchyTreeView.h"
#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"

// Slicer includes
#include "qSlicerModelsModuleWidget.h"
#include "ui_qSlicerModelsModuleWidget.h"

// Logic includes
#include "vtkMRMLDisplayableHierarchyLogic.h"
#include "vtkSlicerModelsLogic.h"

// Colors MRML and Logic includes
#include <vtkSlicerColorLogic.h>
#include <vtkMRMLColorLegendDisplayNode.h>

// MRML includes
#include "vtkMRMLFolderDisplayNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>

//-----------------------------------------------------------------------------
class qSlicerModelsModuleWidgetPrivate : public Ui_qSlicerModelsModuleWidget
{
public:
  qSlicerModelsModuleWidgetPrivate();

  QStringList HideChildNodeTypes;
  QString FiberDisplayClass;
  vtkSmartPointer<vtkCallbackCommand> CallBack;
};

//-----------------------------------------------------------------------------
// qSlicerModelsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerModelsModuleWidgetPrivate::qSlicerModelsModuleWidgetPrivate()
{
  this->HideChildNodeTypes = (QStringList() << "vtkMRMLFiberBundleNode" << "vtkMRMLAnnotationNode");
  this->FiberDisplayClass = "vtkMRMLFiberBundleLineDisplayNode";
  this->CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
}

//-----------------------------------------------------------------------------
// qSlicerModelsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerModelsModuleWidget::qSlicerModelsModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerModelsModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerModelsModuleWidget::~qSlicerModelsModuleWidget()
{
  Q_D(qSlicerModelsModuleWidget);

  // set the mrml scene to null so that stop observing it for events
  if (this->mrmlScene())
  {
    this->mrmlScene()->RemoveObserver(d->CallBack);
  }

  this->setMRMLScene(nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::setup()
{
  Q_D(qSlicerModelsModuleWidget);

  d->setupUi(this);

  d->ModelDisplayWidget->setEnabled(false);

  // Set up tree view
  qMRMLSortFilterSubjectHierarchyProxyModel* sortFilterProxyModel = d->SubjectHierarchyTreeView->sortFilterProxyModel();
  sortFilterProxyModel->setNodeTypes(QStringList() << "vtkMRMLModelNode" << "vtkMRMLFolderDisplayNode");
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->idColumn(), true);
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->transformColumn(), true);
  d->SubjectHierarchyTreeView->setPluginAllowlist(QStringList() << "Models" << "Folder" << "Opacity" << "Visibility");
  d->SubjectHierarchyTreeView->setSelectRoleSubMenuVisible(false);
  d->SubjectHierarchyTreeView->expandToDepth(4);
  d->SubjectHierarchyTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

  connect(d->SubjectHierarchyTreeView, SIGNAL(currentItemsChanged(QList<vtkIdType>)), this, SLOT(setDisplaySelectionFromSubjectHierarchyItems(QList<vtkIdType>)));

  connect(d->FilterModelSearchBox, SIGNAL(textChanged(QString)), sortFilterProxyModel, SLOT(setNameFilter(QString)));

  connect(d->InformationButton, SIGNAL(contentsCollapsed(bool)), this, SLOT(onInformationSectionCollapsed(bool)));

  connect(d->ClipModelsNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(onClipModelsNodeChanged(vtkMRMLNode*)));

  connect(d->ModelDisplayWidget, SIGNAL(clippingToggled(bool)), this, SLOT(onClipSelectedModelToggled(bool)));

  connect(d->ModelDisplayWidget, SIGNAL(clippingConfigurationButtonClicked()), this, SLOT(onClippingConfigurationButtonClicked()));

  // add an add hierarchy right click action on the scene and hierarchy nodes
  connect(d->ModelDisplayWidget, SIGNAL(displayNodeChanged()), this, SLOT(onDisplayNodeChanged()));

  connect(d->ColorLegendCollapsibleGroupBox, SIGNAL(toggled(bool)), this, SLOT(onColorLegendCollapsibleGroupBoxToggled(bool)));

  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::enter()
{
  Q_D(qSlicerModelsModuleWidget);

  // Set minimum models tree height so that it has a reasonable starting size
  const int displayedItemCount = d->SubjectHierarchyTreeView->displayedItemCount();
  const int headerHeight = d->SubjectHierarchyTreeView->header()->sizeHint().height();
  int treeViewHeight = headerHeight * 3; // Approximately three rows when empty (cannot get row height)
  if (displayedItemCount > 0)
  {
    // Calculate full tree view height based on number of displayed items and item height (and add 2 for the borders).
    treeViewHeight = headerHeight + displayedItemCount * d->SubjectHierarchyTreeView->sizeHintForRow(0) + 2;
  }

  // Get height of the whole Models module widget panel
  const int panelHeight = this->sizeHint().height();
  // Set tree view minimum height to be the minimum of the calculated full height and half
  // of the panel height
  d->SubjectHierarchyTreeView->setMinimumHeight(qMin<int>(treeViewHeight, panelHeight / 2.0));

  // Connect SH item modified event so that widget state is updated when a display node is created
  // on the currently selected item (when color is set to a folder)
  vtkMRMLSubjectHierarchyNode* const shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (shNode)
  {
    qvtkConnect(shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, this, SLOT(onSubjectHierarchyItemModified(vtkObject*, void*)));
    qvtkConnect(shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemDisplayModifiedEvent, this, SLOT(onSubjectHierarchyItemModified(vtkObject*, void*)));
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
  }

  this->Superclass::enter();

  // If no node is selected then select the first displayed node to save the user a click
  if (!d->SubjectHierarchyTreeView->currentNode())
  {
    vtkMRMLNode* const node = d->SubjectHierarchyTreeView->findFirstNodeByClass("vtkMRMLModelNode");
    if (node)
    {
      d->SubjectHierarchyTreeView->setCurrentNode(node);
    }
  }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::exit()
{
  Q_D(qSlicerModelsModuleWidget);

  // Disconnect SH node modified when module is not active
  vtkMRMLSubjectHierarchyNode* const shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (shNode)
  {
    qvtkDisconnect(shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, this, SLOT(onSubjectHierarchyItemModified(vtkObject*, void*)));
    qvtkDisconnect(shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemDisplayModifiedEvent, this, SLOT(onSubjectHierarchyItemModified(vtkObject*, void*)));
  }
  else
  {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
  }

  this->Superclass::exit();
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qSlicerModelsModuleWidget);

  if (scene == this->mrmlScene())
  {
    return;
  }
  this->Superclass::setMRMLScene(scene);

  if (scene)
  {
    scene->RemoveObserver(d->CallBack);
  }
  if (scene)
  {
    d->CallBack->SetClientData(this);

    d->CallBack->SetCallback(qSlicerModelsModuleWidget::onMRMLSceneEvent);

    scene->AddObserver(vtkMRMLScene::EndImportEvent, d->CallBack);
    this->onMRMLSceneEvent(scene, vtkMRMLScene::EndImportEvent, this, nullptr);
  }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::onMRMLSceneEvent(vtkObject* vtk_obj, unsigned long event, void* client_data, void* call_data)
{
  vtkMRMLScene* const scene = reinterpret_cast<vtkMRMLScene*>(vtk_obj);
  qSlicerModelsModuleWidget* const widget = reinterpret_cast<qSlicerModelsModuleWidget*>(client_data);
  vtkMRMLNode* const node = reinterpret_cast<vtkMRMLNode*>(call_data);
  Q_UNUSED(node);
  Q_ASSERT(scene);
  Q_UNUSED(scene);
  Q_ASSERT(widget);
  if (event == vtkMRMLScene::EndImportEvent)
  {
    // widget->updateWidgetFromSelectionNode();
  }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::showAllModels()
{
  if (this->logic() == nullptr)
  {
    return;
  }
  vtkSlicerModelsLogic* modelsLogic = vtkSlicerModelsLogic::SafeDownCast(this->logic());
  if (modelsLogic)
  {
    modelsLogic->SetAllModelsVisibility(1);
  }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::hideAllModels()
{
  if (this->logic() == nullptr)
  {
    return;
  }
  vtkSlicerModelsLogic* modelsLogic = vtkSlicerModelsLogic::SafeDownCast(this->logic());
  if (modelsLogic)
  {
    modelsLogic->SetAllModelsVisibility(0);
  }
}

//-----------------------------------------------------------
bool qSlicerModelsModuleWidget::setEditedNode(vtkMRMLNode* node, QString role /* = QString()*/, QString context /* = QString()*/)
{
  Q_D(qSlicerModelsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkMRMLModelNode::SafeDownCast(node) || vtkMRMLFolderDisplayNode::SafeDownCast(node))
  {
    d->SubjectHierarchyTreeView->setCurrentNode(node);
    return true;
  }

  if (vtkMRMLModelDisplayNode::SafeDownCast(node))
  {
    vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(node);
    vtkMRMLModelNode* const displayableNode = vtkMRMLModelNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
    {
      return false;
    }
    d->SubjectHierarchyTreeView->setCurrentNode(displayableNode);
    return true;
  }

  return false;
}

//-----------------------------------------------------------
void qSlicerModelsModuleWidget::onClippingConfigurationButtonClicked()
{
  Q_D(qSlicerModelsModuleWidget);
  d->ClippingButton->setCollapsed(false);
  // Make sure import/export is visible
  if (this->parent() && this->parent()->parent() && this->parent()->parent()->parent())
  {
    QScrollArea* scrollArea = qobject_cast<QScrollArea*>(this->parent()->parent()->parent());
    if (scrollArea)
    {
      scrollArea->ensureWidgetVisible(d->ClippingButton);
    }
  }
}

//-----------------------------------------------------------
void qSlicerModelsModuleWidget::onDisplayNodeChanged()
{
  Q_D(qSlicerModelsModuleWidget);
  vtkMRMLModelDisplayNode* displayNode = d->ModelDisplayWidget->mrmlModelDisplayNode();
  vtkMRMLClipNode* const clipNode = displayNode ? displayNode->GetClipNode() : nullptr;

  d->ClippingButton->setEnabled(displayNode != nullptr);

  bool wasBlocked = false;

  wasBlocked = d->MRMLClipNodeDisplayWidget->blockSignals(true);
  d->MRMLClipNodeDisplayWidget->setMRMLDisplayNode(displayNode);
  d->MRMLClipNodeDisplayWidget->blockSignals(wasBlocked);

  wasBlocked = d->ClipModelsNodeComboBox->blockSignals(true);
  d->ClipModelsNodeComboBox->setEnabled(displayNode != nullptr);
  d->ClipModelsNodeComboBox->setCurrentNode(clipNode);
  d->ClipModelsNodeComboBox->blockSignals(wasBlocked);

  wasBlocked = d->MRMLClipNodeWidget->blockSignals(true);
  d->MRMLClipNodeWidget->setEnabled(clipNode != nullptr);
  d->MRMLClipNodeWidget->setMRMLClipNode(clipNode);
  d->MRMLClipNodeWidget->blockSignals(wasBlocked);

  // Color legend
  vtkMRMLColorLegendDisplayNode* colorLegendNode = nullptr;
  colorLegendNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode);
  d->ColorLegendDisplayNodeWidget->setMRMLColorLegendDisplayNode(colorLegendNode);

  if (!colorLegendNode)
  {
    d->ColorLegendCollapsibleGroupBox->setCollapsed(true);
  }
  d->ColorLegendCollapsibleGroupBox->setEnabled(displayNode && displayNode->GetColorNode());
}

//-----------------------------------------------------------
void qSlicerModelsModuleWidget::onClipModelsNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerModelsModuleWidget);

  vtkMRMLClipNode* clipNode = vtkMRMLClipNode::SafeDownCast(node);
  vtkMRMLModelDisplayNode* displayNode = d->ModelDisplayWidget->mrmlModelDisplayNode();
  if (displayNode)
  {
    displayNode->SetAndObserveClipNodeID(clipNode ? clipNode->GetID() : nullptr);
  }
  d->MRMLClipNodeWidget->setMRMLClipNode(clipNode);
}

//-----------------------------------------------------------
void qSlicerModelsModuleWidget::onClipSelectedModelToggled(bool toggled)
{
  Q_D(qSlicerModelsModuleWidget);
  vtkMRMLModelDisplayNode* displayNode = d->ModelDisplayWidget->mrmlModelDisplayNode();
  if (displayNode)
  {
    const MRMLNodeModifyBlocker blocker(displayNode);
    displayNode->SetClipping(toggled);
  }
}

//-----------------------------------------------------------
void qSlicerModelsModuleWidget::onClippingCapVisibilityToggled(bool toggled)
{
  Q_D(qSlicerModelsModuleWidget);
  vtkMRMLModelDisplayNode* displayNode = d->ModelDisplayWidget->mrmlModelDisplayNode();
  if (displayNode)
  {
    const MRMLNodeModifyBlocker blocker(displayNode);
    displayNode->SetClippingCapSurface(toggled);
  }
}

//-----------------------------------------------------------
void qSlicerModelsModuleWidget::onClippingCapOpacityChanged(double value)
{
  Q_D(qSlicerModelsModuleWidget);
  vtkMRMLModelDisplayNode* displayNode = d->ModelDisplayWidget->mrmlModelDisplayNode();
  if (displayNode)
  {
    const MRMLNodeModifyBlocker blocker(displayNode);
    displayNode->SetClippingCapOpacity(value);
  }
}

//-----------------------------------------------------------
void qSlicerModelsModuleWidget::onClippingOutlineVisibilityToggled(bool value)
{
  Q_D(qSlicerModelsModuleWidget);
  vtkMRMLModelDisplayNode* displayNode = d->ModelDisplayWidget->mrmlModelDisplayNode();
  if (displayNode)
  {
    const MRMLNodeModifyBlocker blocker(displayNode);
    displayNode->SetClippingOutline(value);
  }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::setDisplaySelectionFromSubjectHierarchyItems(QList<vtkIdType> itemIDs)
{
  Q_D(qSlicerModelsModuleWidget);

  vtkMRMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
  }

  vtkMRMLNode* firstDataNode = nullptr;
  if (itemIDs.size() > 0
      // In case of empty selection the only item in the list is the scene
      && !(itemIDs.size() == 1 && itemIDs[0] == shNode->GetSceneItemID()))
  {
    firstDataNode = shNode->GetItemDataNode(itemIDs[0]);
  }
  // Only set model node to info widget if it's visible
  d->MRMLModelInfoWidget->setMRMLModelNode(d->InformationButton->collapsed() ? nullptr : firstDataNode);

  d->ModelDisplayWidget->setCurrentSubjectHierarchyItemIDs(itemIDs);
}

//---------------------------------------------------------------------------
void qSlicerModelsModuleWidget::onSubjectHierarchyItemModified(vtkObject* vtkNotUsed(caller), void* vtkNotUsed(callData))
{
  Q_D(qSlicerModelsModuleWidget);

  const QList<vtkIdType> currentItemIDs = d->SubjectHierarchyTreeView->currentItems();
  this->setDisplaySelectionFromSubjectHierarchyItems(currentItemIDs);
}

//---------------------------------------------------------------------------
void qSlicerModelsModuleWidget::onInformationSectionCollapsed(bool collapsed)
{
  Q_D(qSlicerModelsModuleWidget);

  if (!collapsed)
  {
    const QList<vtkIdType> currentItemIDs = d->SubjectHierarchyTreeView->currentItems();
    this->setDisplaySelectionFromSubjectHierarchyItems(currentItemIDs);
  }
}

//------------------------------------------------------------------------------
void qSlicerModelsModuleWidget::onColorLegendCollapsibleGroupBoxToggled(bool toggled)
{
  Q_D(qSlicerModelsModuleWidget);

  // Make sure a legend display node exists if the color legend section is opened
  if (!toggled)
  {
    return;
  }

  vtkMRMLModelDisplayNode* const displayNode = d->ModelDisplayWidget->mrmlModelDisplayNode();
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
