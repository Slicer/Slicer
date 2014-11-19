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
#include <QtPlugin>
#include <QDebug>
#include <QMessageBox>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModule.h"
#include "qSlicerSubjectHierarchyModuleWidget.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyDICOMPlugin.h"
#include "qSlicerSubjectHierarchyCloneNodePlugin.h"
#include "qSlicerSubjectHierarchyChartsPlugin.h"
#include "qSlicerSubjectHierarchyParseLocalDataPlugin.h"
#include "qSlicerSubjectHierarchyRegisterPlugin.h"
#include "qSlicerSubjectHierarchySegmentPlugin.h"

// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerSubjectHierarchyModule, qSlicerSubjectHierarchyModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy
class qSlicerSubjectHierarchyModulePrivate
{
public:
  qSlicerSubjectHierarchyModulePrivate();

  bool DeleteBranchInProgress;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModulePrivate::qSlicerSubjectHierarchyModulePrivate()
{
  this->DeleteBranchInProgress = false;
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModule methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModule::qSlicerSubjectHierarchyModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSubjectHierarchyModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModule::~qSlicerSubjectHierarchyModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyModule::helpText()const
{
  QString help =
    "The Subject hierarchy module provides a nice and intuitive tree view of the loaded data. It acts as a convenient central organizing point for many of the operations that 3D Slicer and its extensions perform."
    "For more information see <a href=\"%1/Documentation/%2.%3/Modules/SubjectHierarchy\">%1/Documentation/%2.%3/Modules/SubjectHierarchy</a><br>";
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyModule::acknowledgementText()const
{
  return "This work is part of SparKit project, funded by Cancer Care Ontario (CCO)'s ACRU program and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSubjectHierarchyModule::categories() const
{
  return QStringList() << "" << "Informatics";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSubjectHierarchyModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Csaba Pinter (Queen's)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyModule::icon()const
{
  return QIcon(":/Icons/SubjectHierarchy.png");
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModule::setup()
{
  this->Superclass::setup();

  // Handle scene change event if occurs
  qvtkConnect( this->logic(), vtkCommand::ModifiedEvent, this, SLOT( onLogicModified() ) );

  // Connect scene node added event so that the new subject hierarchy nodes can be claimed by a plugin
  qvtkConnect( this->mrmlScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT( onNodeAdded(vtkObject*,vtkObject*) ) );
  // Connect scene node added event so that the associated subject hierarchy node can be deleted too
  qvtkConnect( this->mrmlScene(), vtkMRMLScene::NodeAboutToBeRemovedEvent, this, SLOT( onNodeAboutToBeRemoved(vtkObject*,vtkObject*) ) );

  // Register Subject Hierarchy core plugins
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyDICOMPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyCloneNodePlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyChartsPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyParseLocalDataPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyRegisterPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchySegmentPlugin());
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSubjectHierarchyModule::createLogic()
{
  return vtkSlicerSubjectHierarchyModuleLogic::New();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerSubjectHierarchyModule::createWidgetRepresentation()
{
  return new qSlicerSubjectHierarchyModuleWidget;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModule::onLogicModified()
{
  vtkMRMLScene* scene = this->mrmlScene();
  vtkMRMLScene* currentScene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();

  if (scene != currentScene)
    {
    // Set the new scene to the plugin handler
    qSlicerSubjectHierarchyPluginHandler::instance()->setScene(scene);

    // Connect scene node added event so that the new subject hierarchy nodes can be claimed by a plugin
    qvtkReconnect( scene, vtkMRMLScene::NodeAddedEvent, this, SLOT( onNodeAdded(vtkObject*,vtkObject*) ) );
    // Connect scene node added event so that the associated subject hierarchy node can be deleted too
    qvtkReconnect( scene, vtkMRMLScene::NodeAboutToBeRemovedEvent, this, SLOT( onNodeAboutToBeRemoved(vtkObject*,vtkObject*) ) );
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModule::onNodeAdded(vtkObject* sceneObject, vtkObject* nodeObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
    {
    return;
    }

  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(nodeObject);
  if (subjectHierarchyNode)
    {
    // Keep 'owner plugin changed' connections up-to date (reconnect to the new plugin)
    qvtkConnect( subjectHierarchyNode, vtkMRMLSubjectHierarchyNode::OwnerPluginChangedEvent,
      qSlicerSubjectHierarchyPluginHandler::instance(), SLOT( reconnectOwnerPluginChanged(vtkObject*,void*) ) );

    // Find plugin for current subject hierarchy node and "claim" it
    if (!scene->IsImporting())
      {
      qSlicerSubjectHierarchyPluginHandler::instance()->findAndSetOwnerPluginForSubjectHierarchyNode(subjectHierarchyNode);
      }

    // See if owner plugin has to be changed when a note is modified
    qvtkConnect( subjectHierarchyNode, vtkCommand::ModifiedEvent, this, SLOT( onSubjectHierarchyNodeModified(vtkObject*) ) );
    }
  // If data node
  else
    {
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(nodeObject);

    // Don't add to subject hierarchy automatically if importing scene, because the SH nodes are stored in the scene and will be loaded
    if (!scene->IsImporting())
      {
      // Abort if invalid or hidden node or if explicitly excluded from subject hierarchy before even adding to the scene
      if ( !node || node->GetHideFromEditors()
        || node->GetAttribute(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_EXCLUDE_FROM_TREE_ATTRIBUTE_NAME.c_str()) )
        {
        return;
        }

      // If there is a plugin that can add the data node to subject hierarchy, then add
      QList<qSlicerSubjectHierarchyAbstractPlugin*> foundPlugins =
        qSlicerSubjectHierarchyPluginHandler::instance()->pluginsForAddingToSubjectHierarchyForNode(node, NULL);
      qSlicerSubjectHierarchyAbstractPlugin* selectedPlugin = NULL;
      if (foundPlugins.size() > 1)
        {
        // Let the user choose a plugin if more than one returned the same non-zero confidence value
        QString textToDisplay = QString("Equal confidence number found for more than one subject hierarchy plugin for adding new node to subject hierarchy.\n\nSelect plugin to add node named\n'%1'\n(type %2)").arg(node->GetName()).arg(node->GetNodeTagName());
        selectedPlugin = qSlicerSubjectHierarchyPluginHandler::instance()->selectPluginFromDialog(textToDisplay, foundPlugins);
        }
       else if (foundPlugins.size() == 1)
        {
        selectedPlugin = foundPlugins[0];
        }
      // Have the selected plugin add the new node to subject hierarchy
      if (selectedPlugin)
        {
        bool successfullyAddedByPlugin = selectedPlugin->addNodeToSubjectHierarchy(node, NULL);
        if (!successfullyAddedByPlugin)
          {
          qWarning() << "qSlicerSubjectHierarchyModule::onNodeAdded: Failed to add node "
            << node->GetName() << " through plugin '" << selectedPlugin->name().toLatin1().constData() << "'";
          }
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModule::onNodeAboutToBeRemoved(vtkObject* sceneObject, vtkObject* nodeObject)
{
  vtkMRMLScene* scene = vtkMRMLScene::SafeDownCast(sceneObject);
  if (!scene)
    {
    return;
    }

  // Do nothing if scene is closing
  if (scene->IsClosing())
    {
    return;
    }

  Q_D(qSlicerSubjectHierarchyModule);

  vtkMRMLNode* dataNode = vtkMRMLNode::SafeDownCast(nodeObject);
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(nodeObject);

  if (subjectHierarchyNode)
    {
    // Remove associated data node if any
    vtkMRMLNode* associatedDataNode = subjectHierarchyNode->GetAssociatedNode();
    if (associatedDataNode && !subjectHierarchyNode->GetDisableModifiedEvent())
      {
      subjectHierarchyNode->DisableModifiedEventOn();
      subjectHierarchyNode->SetAssociatedNodeID(NULL);
      scene->RemoveNode(associatedDataNode);
      }

    // Check if node has children and ask if branch is to be removed
    std::vector<vtkMRMLHierarchyNode*> childrenNodes;
    subjectHierarchyNode->GetAllChildrenNodes(childrenNodes);
    if (!childrenNodes.empty() && !d->DeleteBranchInProgress)
      {
      QMessageBox::StandardButton answer =
        QMessageBox::question(NULL, tr("Delete branch?"),
        tr("The deleted node has children. Do you want to remove those too?\n\nIf you choose yes, the whole branch will be deleted recursively."),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
      // Delete branch if the user chose yes
      if (answer == QMessageBox::Yes)
        {
        d->DeleteBranchInProgress = true;
        for (std::vector<vtkMRMLHierarchyNode*>::iterator childrenIt = childrenNodes.begin();
          childrenIt != childrenNodes.end(); ++childrenIt)
          {
          scene->RemoveNode(*childrenIt);
          }
        d->DeleteBranchInProgress = false;
        }
      }
    }
  else if (dataNode)
    {
    // Remove associated subject hierarchy node if any
    vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(dataNode, scene);
    if (subjectHierarchyNode)
      {
      subjectHierarchyNode->DisableModifiedEventOn();
      subjectHierarchyNode->SetAssociatedNodeID(NULL);
      scene->RemoveNode(subjectHierarchyNode);
      }
    // Remove associated other hierarchy node if any (if there is a nested association)
    vtkMRMLHierarchyNode* hierarchyNode = vtkMRMLHierarchyNode::GetAssociatedHierarchyNode(scene, dataNode->GetID());
    if (hierarchyNode)
      {
      scene->RemoveNode(hierarchyNode);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModule::onSubjectHierarchyNodeModified(vtkObject* nodeObject)
{
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(nodeObject);
  if (subjectHierarchyNode && subjectHierarchyNode->GetOwnerPluginAutoSearch())
    {
    // Find plugin for current subject hierarchy node and "claim" it if the
    // owner plugin is not manually overridden by the user
    QString pluginBefore( subjectHierarchyNode->GetOwnerPluginName() );
    qSlicerSubjectHierarchyPluginHandler::instance()->findAndSetOwnerPluginForSubjectHierarchyNode(subjectHierarchyNode);
    QString pluginAfter( subjectHierarchyNode->GetOwnerPluginName() );
    //if (pluginBefore.compare(pluginAfter))
    //  {
    //  qDebug() << "qSlicerSubjectHierarchyModule::onSubjectHierarchyNodeModified: Subject hierarchy node '" <<
    //    subjectHierarchyNode->GetName() << "' has been modified, plugin search performed, and owner plugin changed from '" <<
    //    pluginBefore << "' to '" << pluginAfter << "'";
    //  }
    }
}
