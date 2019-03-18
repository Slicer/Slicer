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

// Subject hierarchy widgets
#include "qMRMLSubjectHierarchyTreeView.h"
#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"

// SlicerQt includes
#include "qSlicerModelsModuleWidget.h"
#include "ui_qSlicerModelsModuleWidget.h"

// Logic includes
#include "vtkMRMLDisplayableHierarchyLogic.h"
#include "vtkSlicerModelsLogic.h"

// MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkNew.h>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Models
class qSlicerModelsModuleWidgetPrivate: public Ui_qSlicerModelsModuleWidget
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
  : Superclass( _parent )
  , d_ptr( new qSlicerModelsModuleWidgetPrivate )
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

  d->ClipModelsNodeComboBox->setVisible(false);

  d->DisplayClassTabWidget->setVisible(false);

  // Set up tree view
  qMRMLSortFilterSubjectHierarchyProxyModel* sortFilterProxyModel = d->SubjectHierarchyTreeView->sortFilterProxyModel();
  sortFilterProxyModel->setNodeTypes(QStringList() << "vtkMRMLModelNode" << "vtkMRMLModelHierarchyNode" << "vtkMRMLModelDisplayNode");
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->idColumn(), true);
  d->SubjectHierarchyTreeView->setColumnHidden(d->SubjectHierarchyTreeView->model()->transformColumn(), true);
  d->SubjectHierarchyTreeView->setPluginWhitelist(QStringList() << "Models" << "Folder" << "Opacity" << "Visibility");
  d->SubjectHierarchyTreeView->setSelectRoleSubMenuVisible(false);
  d->SubjectHierarchyTreeView->expandToDepth(4);
  d->SubjectHierarchyTreeView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

  connect( d->SubjectHierarchyTreeView, SIGNAL(currentItemChanged(vtkIdType)),
           this, SLOT(setCurrentNodeFromSubjectHierarchyItem(vtkIdType)) );

  connect( d->FilterModelSearchBox, SIGNAL(textChanged(QString)),
           sortFilterProxyModel, SLOT(setNameFilter(QString)) );

  connect(d->IncludeFiberBundleCheckBox, SIGNAL(toggled(bool)),
    this, SLOT(includeFiberBundles(bool)));

  connect(d->DisplayClassTabWidget, SIGNAL(currentChanged(int)),
    this, SLOT(onDisplayClassChanged(int)));

  connect(d->ModelDisplayWidget, SIGNAL(clippingToggled(bool)),
    this, SLOT(onClipSelectedModelToggled(bool)));

  connect(d->ModelDisplayWidget, SIGNAL(clippingConfigurationButtonClicked()),
    this, SLOT(onClippingConfigurationButtonClicked()));

  // add an add hierarchy right click action on the scene and hierarchy nodes
  connect(d->ModelDisplayWidget, SIGNAL(displayNodeChanged()),
    this, SLOT(onDisplayNodeChanged()));

  connect(d->ClipSelectedModelCheckBox, SIGNAL(toggled(bool)),
    this, SLOT(onClipSelectedModelToggled(bool)));

  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::enter()
{
  Q_D(qSlicerModelsModuleWidget);

  // Set minimum models tree height so that it has a reasonable starting size
  int displayedItemCount = d->SubjectHierarchyTreeView->displayedItemCount();
  int headerHeight = d->SubjectHierarchyTreeView->header()->sizeHint().height();
  int treeViewHeight = headerHeight * 3; // Approximately three rows when empty (cannot get row height)
  if (displayedItemCount > 0)
    {
    // Calculate full tree view height based on number of displayed items and item height (and add 2 for the borders).
    treeViewHeight = headerHeight + displayedItemCount * d->SubjectHierarchyTreeView->sizeHintForRow(0) + 2;
    }

  // Get height of the whole Models module widget panel
  int panelHeight = this->sizeHint().height();
  // Set tree view minimum height to be the minimum of the calculated full height and half
  // of the panel height
  d->SubjectHierarchyTreeView->setMinimumHeight(qMin<int>(treeViewHeight, panelHeight / 2.0));

  // Connect SH item modified event so that widget state is updated when a display node is created
  // on the currently selected item (when color is set to a folder)
  vtkMRMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }
  qvtkConnect( shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent,
    this, SLOT( onSubjectHierarchyItemModified(vtkObject*,void*) ) );

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::exit()
{
  Q_D(qSlicerModelsModuleWidget);

  // Disconnect SH node modified when module is not active
  vtkMRMLSubjectHierarchyNode* shNode = d->SubjectHierarchyTreeView->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy";
    return;
    }
  qvtkDisconnect( shNode, vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent,
    this, SLOT( onSubjectHierarchyItemModified(vtkObject*,void*) ) );

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
void qSlicerModelsModuleWidget::onMRMLSceneEvent(vtkObject* vtk_obj, unsigned long event,
                                                void* client_data, void* call_data)
{
  vtkMRMLScene* scene = reinterpret_cast<vtkMRMLScene*>(vtk_obj);
  qSlicerModelsModuleWidget* widget = reinterpret_cast<qSlicerModelsModuleWidget*>(client_data);
  vtkMRMLNode* node = reinterpret_cast<vtkMRMLNode*>(call_data);
  Q_UNUSED(node);
  Q_ASSERT(scene);
  Q_UNUSED(scene);
  Q_ASSERT(widget);
  if (event == vtkMRMLScene::EndImportEvent)
    {
    widget->updateWidgetFromSelectionNode();
    }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::showAllModels()
{
  if (this->logic() == nullptr)
    {
    return;
    }
  vtkSlicerModelsLogic *modelsLogic = vtkSlicerModelsLogic::SafeDownCast(this->logic());
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
  vtkSlicerModelsLogic *modelsLogic = vtkSlicerModelsLogic::SafeDownCast(this->logic());
  if (modelsLogic)
    {
    modelsLogic->SetAllModelsVisibility(0);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::includeFiberBundles(bool include)
{
  Q_D(qSlicerModelsModuleWidget);

  // update selection node
  vtkMRMLSelectionNode* selectionNode = this->getSelectionNode();
  if (selectionNode)
    {
    selectionNode->ClearModelHierarchyDisplayNodeClassNames();
    if (include)
      {
      selectionNode->AddModelHierarchyDisplayNodeClassName("vtkMRMLFiberBundleNode",
                                                          d->FiberDisplayClass.toStdString());
      }
    }

  this->updateWidgetFromSelectionNode();
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::onDisplayClassChanged(int index)
{
  Q_D(qSlicerModelsModuleWidget);

  std::string name;
  if (index == 0)
    {
    name = std::string("vtkMRMLFiberBundleLineDisplayNode");
    }
  else if (index == 1)
    {
    name = std::string("vtkMRMLFiberBundleTubeDisplayNode");
    }
  else if (index == 2)
    {
    name = std::string("vtkMRMLFiberBundleGlyphDisplayNode");
    }

  d->FiberDisplayClass.fromStdString(name);

  vtkMRMLSelectionNode* selectionNode = this->getSelectionNode();
  if (selectionNode)
    {
    selectionNode->ClearModelHierarchyDisplayNodeClassNames();
    selectionNode->AddModelHierarchyDisplayNodeClassName("vtkMRMLFiberBundleNode",
                                                         name);
    }
  this->updateWidgetFromSelectionNode();
}

//-----------------------------------------------------------------------------
vtkMRMLSelectionNode* qSlicerModelsModuleWidget::getSelectionNode()
{
  vtkMRMLSelectionNode* selectionNode = nullptr;
  if (this->mrmlScene())
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(
      this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
    }
  return selectionNode;
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::updateWidgetFromSelectionNode()
{
  Q_D(qSlicerModelsModuleWidget);

  vtkMRMLSelectionNode* selectionNode = this->getSelectionNode();

  bool include = false;
  std::string displayNodeClass("");
  if (selectionNode)
    {
    displayNodeClass = selectionNode->GetModelHierarchyDisplayNodeClassName("vtkMRMLFiberBundleNode");
    include = !displayNodeClass.empty();
    }

  if (include)
    {
    d->HideChildNodeTypes = (QStringList() << "vtkMRMLAnnotationNode");
    d->DisplayClassTabWidget->setVisible(true);
    }
  else
    {
    d->HideChildNodeTypes = (QStringList() << "vtkMRMLFiberBundleNode" << "vtkMRMLAnnotationNode");
    d->DisplayClassTabWidget->setVisible(false);
    }

  if (include)
    {
    d->FiberDisplayClass.fromStdString(displayNodeClass);
    int index = 0;
    if (displayNodeClass == std::string("vtkMRMLFiberBundleTubeDisplayNode"))
      {
      index = 1;
      }
    else if (displayNodeClass == std::string("vtkMRMLFiberBundleGlyphDisplayNode"))
      {
      index = 2;
      }
      d->DisplayClassTabWidget->setCurrentIndex(index);
    }

  qMRMLSortFilterSubjectHierarchyProxyModel* sortFilterProxyModel = d->SubjectHierarchyTreeView->sortFilterProxyModel();
  sortFilterProxyModel->setHideChildNodeTypes(d->HideChildNodeTypes);

  if (include)
    {
    // force update mrml widgets
    std::vector<vtkMRMLNode *> nodes;
    vtkMRMLScene *scene = this->mrmlScene();
    scene->GetNodesByClass(displayNodeClass.c_str(), nodes);
    for (unsigned int i = 0; i < nodes.size(); i++)
      {
      nodes[i]->InvokeEvent(vtkCommand::ModifiedEvent);
      }
    }
  d->ModelDisplayWidget->setMRMLModelOrHierarchyNode(d->ModelDisplayWidget->mrmlDisplayableNode());
}

//-----------------------------------------------------------
bool qSlicerModelsModuleWidget::setEditedNode(vtkMRMLNode* node,
                                              QString role /* = QString()*/,
                                              QString context /* = QString()*/)
{
  Q_D(qSlicerModelsModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);
  if (vtkMRMLModelNode::SafeDownCast(node) || vtkMRMLModelHierarchyNode::SafeDownCast(node))
    {
    d->SubjectHierarchyTreeView->setCurrentNode(node);
    return true;
    }

  if (vtkMRMLModelDisplayNode::SafeDownCast(node))
    {
    vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(node);
    vtkMRMLModelNode* displayableNode = vtkMRMLModelNode::SafeDownCast(displayNode->GetDisplayableNode());
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
  bool wasBlocked = d->ClipSelectedModelCheckBox->blockSignals(true);
  d->ClipSelectedModelLabel->setEnabled(displayNode != nullptr);
  d->ClipSelectedModelCheckBox->setEnabled(displayNode != nullptr);
  d->ClipSelectedModelCheckBox->setChecked(displayNode != nullptr && displayNode->GetClipping());
  d->ClipSelectedModelCheckBox->blockSignals(wasBlocked);
}

//-----------------------------------------------------------
void qSlicerModelsModuleWidget::onClipSelectedModelToggled(bool toggled)
{
  Q_D(qSlicerModelsModuleWidget);
  vtkMRMLModelDisplayNode* displayNode = d->ModelDisplayWidget->mrmlModelDisplayNode();
  if (displayNode)
    {
    int wasModified = displayNode->StartModify();
    displayNode->SetClipping(toggled);
    // By enabling clipping, backfaces may become visible.
    // Therefore, it is better to render them (not cull them).
    if (toggled)
      {
      displayNode->BackfaceCullingOff();
      displayNode->FrontfaceCullingOff();
      if (d->MRMLClipNodeWidget->mrmlClipNode() != nullptr
        && d->MRMLClipNodeWidget->redSliceClipState() == vtkMRMLClipModelsNode::ClipOff
        && d->MRMLClipNodeWidget->greenSliceClipState() == vtkMRMLClipModelsNode::ClipOff
        && d->MRMLClipNodeWidget->yellowSliceClipState() == vtkMRMLClipModelsNode::ClipOff)
        {
        // All clipping planes are disabled.
        // Enable the first clipping plane to show a clipped model to the user.
        d->MRMLClipNodeWidget->setRedSliceClipState(vtkMRMLClipModelsNode::ClipPositiveSpace);
        }
      }
    displayNode->EndModify(wasModified);
    }
}

//-----------------------------------------------------------------------------
void qSlicerModelsModuleWidget::setCurrentNodeFromSubjectHierarchyItem(vtkIdType itemID)
{
  Q_D(qSlicerModelsModuleWidget);

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
  // Only set model node to info widget if it's visible
  d->MRMLModelInfoWidget->setMRMLModelNode(d->InformationButton->collapsed() ? nullptr : dataNode);

  if (dataNode && dataNode->IsA("vtkMRMLModelDisplayNode"))
    {
    d->ModelDisplayWidget->setMRMLModelDisplayNode(dataNode);
    }
  else
    {
    d->ModelDisplayWidget->setMRMLModelOrHierarchyNode(dataNode);
    }
}

//---------------------------------------------------------------------------
void qSlicerModelsModuleWidget::onSubjectHierarchyItemModified(vtkObject* vtkNotUsed(caller), void* vtkNotUsed(callData))
{
  Q_D(qSlicerModelsModuleWidget);

  vtkIdType currentItemID = d->SubjectHierarchyTreeView->currentItem();
  this->setCurrentNodeFromSubjectHierarchyItem(currentItemID);
}
