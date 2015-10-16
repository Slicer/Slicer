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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QHeaderView>
#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QMouseEvent>

// SlicerQt includes
#include "qSlicerApplication.h"

// SubjectHierarchy includes
#include "qMRMLSubjectHierarchyTreeView.h"
#include "qMRMLSceneSubjectHierarchyModel.h"
#include "qMRMLSortFilterSubjectHierarchyProxyModel.h"
#include "qMRMLTransformItemDelegate.h"

#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

// MRML includes
#include <vtkMRMLScene.h>

// MRML Widgets includes
#include "qMRMLTreeView_p.h"

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qMRMLSubjectHierarchyTreeViewPrivate : public qMRMLTreeViewPrivate
{
  Q_DECLARE_PUBLIC(qMRMLSubjectHierarchyTreeView);
public:
  typedef qMRMLTreeViewPrivate Superclass;
  qMRMLSubjectHierarchyTreeViewPrivate(qMRMLSubjectHierarchyTreeView& object);

  /// Different initializer method is needed, because when qMRMLTreeView
  /// calls its init(), then the instance has not been constructed as a
  /// qMRMLSubjectHierarchyTreeView, only as a qMRMLTreeView, so the subject
  /// hierarchy related initializations have to be done from within the
  /// constructor of qMRMLSubjectHierarchyTreeView
  virtual void init2();

  /// Setup all actions for tree view
  void setupActions();

  QList<QAction*> SelectPluginActions;
  QMenu* SelectPluginSubMenu;
  QActionGroup* SelectPluginActionGroup;
  QAction* RemoveFromSubjectHierarchyAction;
  QAction* ExpandToDepthAction;
  qMRMLTransformItemDelegate* TransformItemDelegate;

  /// Flag determining whether to highlight nodes referenced by DICOM. Storing DICOM references:
  ///   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  ///   -> SH node instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  bool HighlightReferencedNodes;

  /// Cached list of highlighted nodes to speed up clearing highlight after new selection
  QList<vtkMRMLSubjectHierarchyNode*> HighlightedNodes;
};

//------------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeViewPrivate::qMRMLSubjectHierarchyTreeViewPrivate(qMRMLSubjectHierarchyTreeView& object)
  : qMRMLTreeViewPrivate(object)
{
  this->RemoveFromSubjectHierarchyAction = NULL;
  this->ExpandToDepthAction = NULL;
  this->SelectPluginSubMenu = NULL;
  this->HighlightReferencedNodes = true;
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::init2()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);

  // Set up scene model and sort and proxy model
  qMRMLSceneSubjectHierarchyModel* sceneModel = new qMRMLSceneSubjectHierarchyModel(q);
  QObject::connect( sceneModel, SIGNAL(saveTreeExpandState()), q, SLOT(saveTreeExpandState()) );
  QObject::connect( sceneModel, SIGNAL(loadTreeExpandState()), q, SLOT(loadTreeExpandState()) );
  q->setSceneModel(sceneModel, "SubjectHierarchy");

  q->setSortFilterProxyModel(new qMRMLSortFilterSubjectHierarchyProxyModel(q));

  // Change item visibility
  q->setShowScene(true);
  //TODO: this would be desirable to set, but results in showing the scrollbar, which makes
  //      subject hierarchy much less usable (because there will be two scrollbars)
  //q->setUniformRowHeights(false);

  // Set up headers
  q->header()->setStretchLastSection(false);
  q->header()->setResizeMode(sceneModel->nameColumn(), QHeaderView::Stretch);
  q->header()->setResizeMode(sceneModel->visibilityColumn(), QHeaderView::ResizeToContents);
  q->header()->setResizeMode(sceneModel->transformColumn(), QHeaderView::Interactive);
  q->header()->setResizeMode(sceneModel->idColumn(), QHeaderView::ResizeToContents);

  // Set item delegate (that creates widgets for certain types of data)
  this->TransformItemDelegate = new qMRMLTransformItemDelegate(q);
  this->TransformItemDelegate->setFixedRowHeight(16);
  this->TransformItemDelegate->setMRMLScene(q->mrmlScene());
  q->setItemDelegateForColumn(sceneModel->transformColumn(), this->TransformItemDelegate);
  QObject::connect(this->TransformItemDelegate, SIGNAL(removeTransformsFromBranchOfCurrentNode()),
    sceneModel, SLOT(onRemoveTransformsFromBranchOfCurrentNode()));
  QObject::connect(this->TransformItemDelegate, SIGNAL(hardenTransformOnBranchOfCurrentNode()),
    sceneModel, SLOT(onHardenTransformOnBranchOfCurrentNode()));

  // Connect Edit properties... action to a different slot than in the base class
  QObject::disconnect(this->EditAction, SIGNAL(triggered()), (qMRMLTreeView*)q, SLOT(editCurrentNode()));
  QObject::connect(this->EditAction, SIGNAL(triggered()), q, SLOT(editCurrentSubjectHierarchyNode()));

  // Connect Delete action to a different slot than in the base class
  QObject::disconnect(this->DeleteAction, SIGNAL(triggered()), q, SLOT(deleteCurrentNode()));
  QObject::connect(this->DeleteAction, SIGNAL(triggered()), q, SLOT(deleteSelectedNodes()));

  // Connect invalidate filters
  QObject::connect( q->sceneModel(), SIGNAL(invalidateFilter()), q->model(), SLOT(invalidate()) );

  // Set up scene and node actions for the tree view
  this->setupActions();
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeViewPrivate::setupActions()
{
  Q_Q(qMRMLSubjectHierarchyTreeView);

  // Set up Remove from subject hierarchy action (hidden by default)
  this->RemoveFromSubjectHierarchyAction = new QAction(qMRMLTreeView::tr("Remove from subject hierarchy"), this->NodeMenu);
  this->NodeMenu->addAction(this->RemoveFromSubjectHierarchyAction);
  this->RemoveFromSubjectHierarchyAction->setVisible(false);
  QObject::connect(this->RemoveFromSubjectHierarchyAction, SIGNAL(triggered()), q, SLOT(removeCurrentNodeFromSubjectHierarchy()));

  // Set up expand to level action and its menu
  this->ExpandToDepthAction = new QAction(qMRMLTreeView::tr("Expand tree to level..."), this->NodeMenu);
  this->SceneMenu->addAction(this->ExpandToDepthAction);

  QMenu* expandToDepthSubMenu = new QMenu();
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_1 = new QAction("1",q);
  QObject::connect(expandToDepth_1, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_1);
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_2 = new QAction("2",q);
  QObject::connect(expandToDepth_2, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_2);
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_3 = new QAction("3",q);
  QObject::connect(expandToDepth_3, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_3);
  this->ExpandToDepthAction->setMenu(expandToDepthSubMenu);
  QAction* expandToDepth_4 = new QAction("4",q);
  QObject::connect(expandToDepth_4, SIGNAL(triggered()), q, SLOT(expandToDepthFromContextMenu()));
  expandToDepthSubMenu->addAction(expandToDepth_4);

  // Perform tasks needed for all plugins
  int index = 0; // Index used to insert actions before default tree actions
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    // Add node context menu actions
    foreach (QAction* action, plugin->nodeContextMenuActions())
      {
      this->NodeMenu->insertAction(this->NodeMenu->actions()[index++], action);
      }

    // Add scene context menu actions
    foreach (QAction* action, plugin->sceneContextMenuActions())
      {
      this->SceneMenu->addAction(action);
      }

    // Connect plugin events to be handled by the tree view
    QObject::connect( plugin, SIGNAL(requestExpandNode(vtkMRMLSubjectHierarchyNode*)),
      q, SLOT(expandNode(vtkMRMLSubjectHierarchyNode*)) );
    QObject::connect( plugin, SIGNAL(requestInvalidateFilter()), q->sceneModel(), SIGNAL(invalidateFilter()) );
    }

  // Create a plugin selection action for each plugin in a sub-menu
  this->SelectPluginSubMenu = this->NodeMenu->addMenu("Select role");
  this->SelectPluginActionGroup = new QActionGroup(q);
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    QAction* selectPluginAction = new QAction(plugin->name(),q);
    selectPluginAction->setCheckable(true);
    selectPluginAction->setActionGroup(this->SelectPluginActionGroup);
    selectPluginAction->setData(QVariant(plugin->name()));
    this->SelectPluginSubMenu->addAction(selectPluginAction);
    QObject::connect(selectPluginAction, SIGNAL(triggered()), q, SLOT(selectPluginForCurrentNode()));
    this->SelectPluginActions << selectPluginAction;
    }

  // Update actions in owner plugin sub-menu when opened
  QObject::connect( this->SelectPluginSubMenu, SIGNAL(aboutToShow()), q, SLOT(updateSelectPluginActions()) );
}

//------------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeView::qMRMLSubjectHierarchyTreeView(QWidget *parent)
  : qMRMLTreeView(new qMRMLSubjectHierarchyTreeViewPrivate(*this), parent)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->init2();
}

//------------------------------------------------------------------------------
qMRMLSubjectHierarchyTreeView::~qMRMLSubjectHierarchyTreeView()
{
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  Q_ASSERT(d->SortFilterModel);
  vtkMRMLNode* rootNode = this->rootNode();
  d->SceneModel->setMRMLScene(scene);
  d->TransformItemDelegate->setMRMLScene(scene);
  this->setRootNode(rootNode);
  this->expandToDepth(4);
}

//--------------------------------------------------------------------------
bool qMRMLSubjectHierarchyTreeView::highlightReferencedNodes()const
{
  Q_D(const qMRMLSubjectHierarchyTreeView);
  return d->HighlightReferencedNodes;
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setHighlightReferencedNodes(bool highlightOn)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  d->HighlightReferencedNodes = highlightOn;
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::toggleVisibility(const QModelIndex& index)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  vtkMRMLNode* node = d->SortFilterModel->mrmlNodeFromIndex(index);
  if (!node)
    {
    return;
    }

  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(node);
  if (!subjectHierarchyNode)
    {
    vtkErrorWithObjectMacro(this->mrmlScene(),"toggleVisibility: Invalid node in subject hierarchy tree! Nodes must all be subject hierarchy nodes.");
    return;
    }
  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin =
    qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyNode(subjectHierarchyNode);

  int visible = (ownerPlugin->getDisplayVisibility(subjectHierarchyNode) > 0 ? 0 : 1);
  ownerPlugin->setDisplayVisibility(subjectHierarchyNode, visible);
}

//------------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::mousePressEvent(QMouseEvent* e)
{
  // Perform default mouse press event (make selections etc.)
  this->QTreeView::mousePressEvent(e);

  // Collect selected subject hierarchy nodes
  QList<vtkMRMLSubjectHierarchyNode*> selectedShNodes;
  QList<QModelIndex> selectedIndices = this->selectedIndexes();
  foreach(QModelIndex index, selectedIndices)
    {
    // Only consider the first column to avoid duplicates
    if (index.column() != 0)
      {
      continue;
      }
    vtkMRMLNode* node = this->sortFilterProxyModel()->mrmlNodeFromIndex(index);
    vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(node);
    if (shNode)
      {
      selectedShNodes.append(shNode);
      }
    }
  // Set current node(s) to plugin handler
  qSlicerSubjectHierarchyPluginHandler::instance()->setCurrentNodes(selectedShNodes);

  // Highlight nodes referenced by DICOM in case of single-selection
  //   Referenced SOP instance UIDs (in attribute named vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName())
  //   -> SH node instance UIDs (serialized string lists in subject hierarchy UID vtkMRMLSubjectHierarchyConstants::GetDICOMInstanceUIDName())
  if (this->highlightReferencedNodes())
    {
    this->applyReferenceHighlightForNode(selectedShNodes);
    }

  // Not the right button clicked, handle events the default way
  if (e->button() == Qt::RightButton)
    {
    // Make sure the shown context menu is up-to-date
    this->populateContextMenuForCurrentNode();

    // Show context menu
    this->qMRMLTreeView::mousePressEvent(e);
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::populateContextMenuForCurrentNode()
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  // Get current node(s)
  QList<vtkMRMLSubjectHierarchyNode*> currentNodes = qSlicerSubjectHierarchyPluginHandler::instance()->currentNodes();
  if (currentNodes.size() > 1)
    {
    // Multi-selection: only show delete action
    d->EditAction->setVisible(false);
    d->RenameAction->setVisible(false);
    d->SelectPluginSubMenu->menuAction()->setVisible(false);

    // Hide all plugin context menu items
    foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
      {
      plugin->hideAllContextMenuActions();
      }

    return;
    }

  // Single selection
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    // Don't show certain actions for non-subject hierarchy nodes (i.e. filtering is turned off)
    d->EditAction->setVisible(false);
    d->SelectPluginSubMenu->menuAction()->setVisible(false);
    }
  else
    {
    // Show basic actions for all subject hierarchy nodes
    d->EditAction->setVisible(true);
    d->SelectPluginSubMenu->menuAction()->setVisible(true);
    }

  // Have all plugins show context menu items for current node
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, qSlicerSubjectHierarchyPluginHandler::instance()->allPlugins())
    {
    plugin->showContextMenuActionsForNode(currentNode);
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::expandNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  if (node)
    {
    QModelIndex nodeIndex = d->SortFilterModel->indexFromMRMLNode(node);
    this->expand(nodeIndex);
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::selectPluginForCurrentNode()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qMRMLSubjectHierarchyTreeView::selectPluginForCurrentNode: Invalid current node for manually selecting owner plugin!";
    return;
    }
  QString selectedPluginName = d->SelectPluginActionGroup->checkedAction()->data().toString();
  if (selectedPluginName.isEmpty())
    {
    qCritical() << "qMRMLSubjectHierarchyTreeView::selectPluginForCurrentNode: No owner plugin found for node " << currentNode->GetName();
    return;
    }
  else if (!selectedPluginName.compare(currentNode->GetOwnerPluginName()))
    {
    // Do nothing if the owner plugin stays the same
    return;
    }

  // Check if the user is setting the plugin that would otherwise be chosen automatically
  qSlicerSubjectHierarchyAbstractPlugin* mostSuitablePluginByConfidenceNumbers =
    qSlicerSubjectHierarchyPluginHandler::instance()->findOwnerPluginForSubjectHierarchyNode(currentNode);
  bool mostSuitablePluginByConfidenceNumbersSelected =
    !mostSuitablePluginByConfidenceNumbers->name().compare(selectedPluginName);
  // Set owner plugin auto search flag to false if the user manually selected a plugin other
  // than the most suitable one by confidence numbers
  currentNode->SetOwnerPluginAutoSearch(mostSuitablePluginByConfidenceNumbersSelected);

  // Set new owner plugin
  currentNode->SetOwnerPluginName(selectedPluginName.toLatin1().constData());
  //qDebug() << "qMRMLSubjectHierarchyTreeView::selectPluginForCurrentNode: Owner plugin of subject hierarchy node '"
  //  << currentNode->GetName() << "' has been manually changed to '" << d->SelectPluginActionGroup->checkedAction()->data().toString() << "'";
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::updateSelectPluginActions()
{
  Q_D(qMRMLSubjectHierarchyTreeView);
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qMRMLSubjectHierarchyTreeView::updateSelectPluginActions: Invalid current node!";
    return;
    }
  QString ownerPluginName = QString(currentNode->GetOwnerPluginName());

  foreach (QAction* currentSelectPluginAction, d->SelectPluginActions)
    {
    // Check select plugin action if it's the owner
    bool isOwner = !(currentSelectPluginAction->data().toString().compare(ownerPluginName));

    // Get confidence numbers and show the plugins with non-zero confidence
    qSlicerSubjectHierarchyAbstractPlugin* currentPlugin =
      qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName( currentSelectPluginAction->data().toString() );
    double confidenceNumber = currentPlugin->canOwnSubjectHierarchyNode(currentNode);

    if (confidenceNumber <= 0.0 && !isOwner)
      {
      currentSelectPluginAction->setVisible(false);
      }
    else
      {
      // Set text to display for the role
      QString role = currentPlugin->roleForPlugin();
      QString currentSelectPluginActionText = QString("%1: '%2', (%3%)").arg(
        role).arg(currentPlugin->displayedNodeName(currentNode)).arg(confidenceNumber*100.0, 0, 'f', 0);
      currentSelectPluginAction->setText(currentSelectPluginActionText);
      currentSelectPluginAction->setVisible(true);
      }

    currentSelectPluginAction->setChecked(isOwner);
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::removeCurrentNodeFromSubjectHierarchy()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qMRMLSubjectHierarchyTreeView::removeCurrentNodeFromSubjectHierarchy: Invalid current node!";
    return;
    }

  currentNode->DisableModifiedEventOn();
  qSlicerSubjectHierarchyPluginHandler::instance()->scene()->RemoveNode(currentNode);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::editCurrentSubjectHierarchyNode()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qMRMLSubjectHierarchyTreeView::editCurrentSubjectHierarchyNode: Invalid current node!";
    return;
    }

  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin =
    qSlicerSubjectHierarchyPluginHandler::instance()->getOwnerPluginForSubjectHierarchyNode(currentNode);
  ownerPlugin->editProperties(currentNode);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::deleteSelectedNodes()
{
  QList<vtkMRMLSubjectHierarchyNode*> currentNodes = qSlicerSubjectHierarchyPluginHandler::instance()->currentNodes();
  foreach(vtkMRMLSubjectHierarchyNode* node, currentNodes)
  {
    this->mrmlScene()->RemoveNode(node);
  }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::expandToDepthFromContextMenu()
{
  QAction* senderAction = qobject_cast<QAction*>(this->sender());
  if (!senderAction)
    {
    qCritical() << "qMRMLSubjectHierarchyTreeView::expandToDepthFromContextMenu: Unable to get sender action!";
    return;
    }

  int depth = senderAction->text().toInt();
  this->expandToDepth(depth);
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::applyReferenceHighlightForNode(QList<vtkMRMLSubjectHierarchyNode*> nodes)
{
  Q_D(qMRMLSubjectHierarchyTreeView);

  // Get scene model and column to highlight
  qMRMLSceneSubjectHierarchyModel* sceneModel = qobject_cast<qMRMLSceneSubjectHierarchyModel*>(this->sceneModel());
  int nameColumn = sceneModel->nameColumn();

  // Clear highlight for previously highlighted nodes
  foreach(vtkMRMLSubjectHierarchyNode* highlightedNode, d->HighlightedNodes)
    {
    QStandardItem* item = sceneModel->itemFromNode(highlightedNode, nameColumn);
    if (item)
      {
      item->setBackground(Qt::transparent);
      }
    }
  d->HighlightedNodes.clear();

  // Go through all selected nodes
  foreach(vtkMRMLSubjectHierarchyNode* node, nodes)
    {
    // Get nodes referenced by argument node by DICOM
    std::vector<vtkMRMLSubjectHierarchyNode*> referencedNodes = node->GetSubjectHierarchyNodesReferencedByDICOM();

    // Highlight referenced nodes
    std::vector<vtkMRMLSubjectHierarchyNode*>::iterator nodeIt;
    for (nodeIt = referencedNodes.begin(); nodeIt != referencedNodes.end(); ++nodeIt)
      {
      vtkMRMLSubjectHierarchyNode* referencedNode = (*nodeIt);
      QStandardItem* item = sceneModel->itemFromNode(referencedNode, nameColumn);
      if (item && !d->HighlightedNodes.contains(referencedNode))
        {
        item->setBackground(Qt::yellow);
        d->HighlightedNodes.append(referencedNode);
        }
      }
    }
}

//--------------------------------------------------------------------------
void qMRMLSubjectHierarchyTreeView::setMultiSelection(bool multiSelectionOn)
{
  if (multiSelectionOn)
    {
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }
  else
    {
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    }
}
