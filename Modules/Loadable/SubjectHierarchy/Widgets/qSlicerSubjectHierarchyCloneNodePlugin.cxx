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

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyCloneNodePlugin.h"

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLDisplayableNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>
#include <vtksys/SystemTools.hxx>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyCloneNodePluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyCloneNodePlugin);
protected:
  qSlicerSubjectHierarchyCloneNodePlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyCloneNodePluginPrivate(qSlicerSubjectHierarchyCloneNodePlugin& object);
  ~qSlicerSubjectHierarchyCloneNodePluginPrivate();
  void init();
public:
  QAction* CloneNodeAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyCloneNodePluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePluginPrivate::qSlicerSubjectHierarchyCloneNodePluginPrivate(qSlicerSubjectHierarchyCloneNodePlugin& object)
: q_ptr(&object)
{
  this->CloneNodeAction = NULL;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePluginPrivate::~qSlicerSubjectHierarchyCloneNodePluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePlugin::qSlicerSubjectHierarchyCloneNodePlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyCloneNodePluginPrivate(*this) )
{
  this->m_Name = QString("CloneNode");

  Q_D(qSlicerSubjectHierarchyCloneNodePlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyCloneNodePluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyCloneNodePlugin);

  this->CloneNodeAction = new QAction("Clone node",q);
  QObject::connect(this->CloneNodeAction, SIGNAL(triggered()), q, SLOT(cloneCurrentNode()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyCloneNodePlugin::~qSlicerSubjectHierarchyCloneNodePlugin()
{
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyCloneNodePlugin::nodeContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyCloneNodePlugin);

  QList<QAction*> actions;
  actions << d->CloneNodeAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyCloneNodePlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyCloneNodePlugin);
  this->hideAllContextMenuActions();

  if (!node)
    {
    // There are no scene actions in this plugin
    return;
    }

  // Show clone node for every non-scene nodes
  d->CloneNodeAction->setVisible(true);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyCloneNodePlugin::cloneCurrentNode()
{
  // Get currently selected node and scene
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyCloneNodePlugin::cloneCurrentNode: Invalid current subject hierarchy node!";
    return;
    }

  vtkMRMLSubjectHierarchyNode* clonedSubjectHierarchyNode = qSlicerSubjectHierarchyCloneNodePlugin::cloneSubjectHierarchyNode(currentNode);
  if (!clonedSubjectHierarchyNode)
    {
    qCritical() << "qSlicerSubjectHierarchyCloneNodePlugin::cloneCurrentNode: Failed to clone subject hierarchy node" << currentNode->GetNameWithoutPostfix().c_str();
    }
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qSlicerSubjectHierarchyCloneNodePlugin::cloneSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node, QString name/*=QString()*/)
{
  if (!node)
    {
    return NULL;
    }
  vtkMRMLScene* scene = node->GetScene();
  if (!scene)
    {
    qCritical() << "qSlicerSubjectHierarchyCloneNodePlugin::cloneSubjectHierarchyNode: Invalid MRML scene!";
    return NULL;
    }

  vtkMRMLSubjectHierarchyNode* clonedSubjectHierarchyNode = NULL;
  vtkMRMLNode* associatedDataNode = node->GetAssociatedNode();
  if (associatedDataNode)
    {
    // Create data node clone
    vtkSmartPointer<vtkMRMLNode> clonedDataNode;
    clonedDataNode.TakeReference(scene->CreateNodeByClass(associatedDataNode->GetClassName()));
    std::string clonedDataNodeName = ( name.isEmpty() ? std::string(associatedDataNode->GetName()) + std::string(getCloneNodeNamePostfix().toLatin1().constData()) : std::string(name.toLatin1().constData()) );
    scene->AddNode(clonedDataNode);

    // Clone display node
    vtkSmartPointer<vtkMRMLDisplayNode> clonedDisplayNode;
    vtkMRMLDisplayableNode* displayableDataNode = vtkMRMLDisplayableNode::SafeDownCast(associatedDataNode);
    if (displayableDataNode && displayableDataNode->GetDisplayNode())
      {
      // If display node was automatically created by the specific module logic when the data node was added to the scene, then do not create it
      vtkMRMLDisplayableNode* clonedDisplayableDataNode = vtkMRMLDisplayableNode::SafeDownCast(clonedDataNode);
      if (clonedDisplayableDataNode->GetDisplayNode())
        {
        clonedDisplayNode = clonedDisplayableDataNode->GetDisplayNode();
        }
      else
        {
        clonedDisplayNode = vtkSmartPointer<vtkMRMLDisplayNode>::Take( vtkMRMLDisplayNode::SafeDownCast(
          scene->CreateNodeByClass(displayableDataNode->GetDisplayNode()->GetClassName()) ) );
        clonedDisplayNode->Copy(displayableDataNode->GetDisplayNode());
        std::string clonedDisplayNodeName = clonedDataNodeName + "_Display";
        clonedDisplayNode->SetName(clonedDisplayNodeName.c_str());
        scene->AddNode(clonedDisplayNode);
        clonedDisplayableDataNode->SetAndObserveDisplayNodeID(clonedDisplayNode->GetID());
        }
      }

    // Clone storage node
    vtkSmartPointer<vtkMRMLStorageNode> clonedStorageNode;
    vtkMRMLStorableNode* storableDataNode = vtkMRMLStorableNode::SafeDownCast(associatedDataNode);
    if (storableDataNode && storableDataNode->GetStorageNode())
      {
      // If storage node was automatically created by the specific module logic when the data node was added to the scene, then do not create it
      vtkMRMLStorableNode* clonedStorableDataNode = vtkMRMLStorableNode::SafeDownCast(clonedDataNode);
      if (clonedStorableDataNode->GetStorageNode())
        {
        clonedStorageNode = clonedStorableDataNode->GetStorageNode();
        }
      else
        {
        clonedStorageNode = vtkSmartPointer<vtkMRMLStorageNode>::Take( vtkMRMLStorageNode::SafeDownCast(
          scene->CreateNodeByClass(storableDataNode->GetStorageNode()->GetClassName()) ) );
        clonedStorageNode->Copy(storableDataNode->GetStorageNode());
        if (storableDataNode->GetStorageNode()->GetFileName())
          {
          std::string clonedStorageNodeFileName = std::string(storableDataNode->GetStorageNode()->GetFileName()) + std::string(getCloneNodeNamePostfix().toLatin1().constData());
          clonedStorageNode->SetFileName(clonedStorageNodeFileName.c_str());
          }
        scene->AddNode(clonedStorageNode);
        clonedStorableDataNode->SetAndObserveStorageNodeID(clonedStorageNode->GetID());
        }
      }

    // Copy data node
    // Display and storage nodes might be involved in the copy process, so they are needed to be set up before the copy operation
    clonedDataNode->Copy(associatedDataNode);
    clonedDataNode->SetName(clonedDataNodeName.c_str());
    // Copy overwrites display and storage node references too, need to restore
    if (clonedDisplayNode.GetPointer())
      {
      vtkMRMLDisplayableNode::SafeDownCast(clonedDataNode)->SetAndObserveDisplayNodeID(clonedDisplayNode->GetID());
      }
    if (clonedStorageNode.GetPointer())
      {
      vtkMRMLStorableNode::SafeDownCast(clonedDataNode)->SetAndObserveStorageNodeID(clonedStorageNode->GetID());
      }

    // Get hierarchy nodes
    vtkMRMLHierarchyNode* genericHierarchyNode =
      vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene, associatedDataNode->GetID());

    // Put data node in the same non-subject hierarchy if any
    if (genericHierarchyNode != node)
      {
      vtkSmartPointer<vtkMRMLHierarchyNode> clonedHierarchyNode;
      clonedHierarchyNode.TakeReference( vtkMRMLHierarchyNode::SafeDownCast(
        scene->CreateNodeByClass(genericHierarchyNode->GetClassName()) ) );
      clonedHierarchyNode->Copy(genericHierarchyNode);
      std::string clonedHierarchyNodeName = std::string(genericHierarchyNode->GetName()) + std::string(getCloneNodeNamePostfix().toLatin1().constData());
      clonedHierarchyNode->SetName(clonedHierarchyNodeName.c_str());
      scene->AddNode(clonedHierarchyNode);
      clonedHierarchyNode->SetAssociatedNodeID(clonedDataNode->GetID());
      }

    // Put data node in the same subject hierarchy branch as current node
    clonedSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(scene,
      vtkMRMLSubjectHierarchyNode::SafeDownCast(node->GetParentNode()),
      node->GetLevel(), clonedDataNodeName.c_str(), clonedDataNode);

    // Trigger update
    clonedSubjectHierarchyNode->Modified();
    emit requestInvalidateFilter();
    }
  else // No associated node
    {
    std::string clonedSubjectHierarchyNodeName = node->GetName();
    vtksys::SystemTools::ReplaceString(clonedSubjectHierarchyNodeName,
      vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNodeNamePostfix().c_str(), "");
    clonedSubjectHierarchyNodeName.append(std::string(getCloneNodeNamePostfix().toLatin1().constData()));
    if (!name.isEmpty())
    {
      clonedSubjectHierarchyNodeName = std::string(name.toLatin1().constData());
    }

    clonedSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(scene,
      vtkMRMLSubjectHierarchyNode::SafeDownCast(node->GetParentNode()),
      node->GetLevel(), clonedSubjectHierarchyNodeName.c_str());
    }

  return clonedSubjectHierarchyNode;
}
