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
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyParseLocalDataPlugin.h"
#include "qSlicerSubjectHierarchyFolderPlugin.h"

// Qt includes
#include <QDebug>
#include <QAction>
#include <QStandardItem>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyParseLocalDataPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyParseLocalDataPlugin);
protected:
  qSlicerSubjectHierarchyParseLocalDataPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyParseLocalDataPluginPrivate(qSlicerSubjectHierarchyParseLocalDataPlugin& object);
  ~qSlicerSubjectHierarchyParseLocalDataPluginPrivate() override;
  void init();
public:
  QAction* CreateHierarchyFromLoadedLocalDirectoriesAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyParseLocalDataPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyParseLocalDataPluginPrivate::qSlicerSubjectHierarchyParseLocalDataPluginPrivate(qSlicerSubjectHierarchyParseLocalDataPlugin& object)
: q_ptr(&object)
{
  this->CreateHierarchyFromLoadedLocalDirectoriesAction = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyParseLocalDataPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyParseLocalDataPlugin);

  this->CreateHierarchyFromLoadedLocalDirectoriesAction = new QAction("Create hierarchy from loaded directory structure",q);
  QObject::connect(this->CreateHierarchyFromLoadedLocalDirectoriesAction, SIGNAL(triggered()), q, SLOT(createHierarchyFromLoadedDirectoryStructure()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyParseLocalDataPluginPrivate::~qSlicerSubjectHierarchyParseLocalDataPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyParseLocalDataPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyParseLocalDataPlugin::qSlicerSubjectHierarchyParseLocalDataPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyParseLocalDataPluginPrivate(*this) )
{
  this->m_Name = QString("ParseLocalData");

  Q_D(qSlicerSubjectHierarchyParseLocalDataPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyParseLocalDataPlugin::~qSlicerSubjectHierarchyParseLocalDataPlugin() = default;

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyParseLocalDataPlugin::sceneContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyParseLocalDataPlugin);

  QList<QAction*> actions;
  actions << d->CreateHierarchyFromLoadedLocalDirectoriesAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyParseLocalDataPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  Q_D(qSlicerSubjectHierarchyParseLocalDataPlugin);

  // Scene
  if (itemID == shNode->GetSceneItemID())
    {
    // Only show create hierarchy from loaded local directory structure if there are possible nodes to use
    // That is to have nodes in the scene with storage nodes with valid file names outside subject hierarchy
    vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
    vtkSmartPointer<vtkCollection> storableNodes = vtkSmartPointer<vtkCollection>::Take( scene->GetNodesByClass("vtkMRMLStorableNode") );
    vtkObject* nextObject = nullptr;
    for (storableNodes->InitTraversal(); (nextObject = storableNodes->GetNextItemAsObject()); )
      {
      vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(nextObject);
      if ( storableNode && storableNode->GetStorageNode() && !storableNode->GetHideFromEditors() )
        {
        QList<qSlicerSubjectHierarchyAbstractPlugin*> foundPlugins =
          qSlicerSubjectHierarchyPluginHandler::instance()->pluginsForAddingNodeToSubjectHierarchy(storableNode);
        if (!foundPlugins.empty())
          {
          d->CreateHierarchyFromLoadedLocalDirectoriesAction->setVisible(true);
          }
        }
      }
    }
}

//--------------------------------------------------------------------------
void qSlicerSubjectHierarchyParseLocalDataPlugin::createHierarchyFromLoadedDirectoryStructure()
{
  QList<QStringList> loadedFilePaths;
  QList<vtkMRMLStorableNode*> loadedNodes;
  QList<vtkIdType> vtkIdTypes;
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get all file paths from the storable nodes into the list
  vtkSmartPointer<vtkCollection> storableNodes = vtkSmartPointer<vtkCollection>::Take( scene->GetNodesByClass("vtkMRMLStorableNode") );
  vtkObject* nextObject = nullptr;
  for (storableNodes->InitTraversal(); (nextObject = storableNodes->GetNextItemAsObject()); )
    {
    vtkMRMLStorableNode* storableNode = vtkMRMLStorableNode::SafeDownCast(nextObject);
    if ( storableNode && storableNode->GetStorageNode() && !storableNode->GetHideFromEditors() )
      {
      vtkIdType shItemID = shNode->GetItemByDataNode(storableNode);
      if (!shItemID)
        {
        qCritical() << Q_FUNC_INFO << ": Data node " << storableNode->GetName() << " is not in subject hierarchy!";
        continue;
        }
      // Exclude nodes that are already in a hierarchy (have parent). Sanity check for valid subject hierarchy node
      if (shNode->GetItemParent(shItemID) != shNode->GetSceneItemID())
        {
        continue;
        }
      // Add storable node to the list (cannot parse if loaded from multiple files - in which case there is a non-empty file list)
      vtkMRMLStorageNode* storageNode = storableNode->GetStorageNode();
      if ( storageNode->GetNumberOfFileNames() == 0 && storageNode->GetFileName() )
        {
        QString filePath(storageNode->GetFileName());
        loadedFilePaths << filePath.split('/', QString::SkipEmptyParts);
        loadedNodes << storableNode;
        vtkIdTypes << shItemID;
        }
      }
    }
  if (loadedFilePaths.empty())
    {
    return;
    }

  // Remove the leading components of the file paths until they match
  bool firstComponentMatch = true;
  do
    {
    QString firstComponent;
    foreach(QStringList filePath, loadedFilePaths)
      {
      if (filePath.count() == 0)
        {
        qWarning() << Q_FUNC_INFO << ": Too shallow file path found!";
        firstComponentMatch = false;
        break;
        }
      if (firstComponent.isEmpty())
        {
        firstComponent = filePath[0];
        }
      else if (firstComponent.compare(filePath[0]))
        {
        firstComponentMatch = false;
        break;
        }
      }

    // If first component matches through all file paths, remove it
    if (firstComponentMatch)
      {
      qDebug() << Q_FUNC_INFO << ": First component (" << firstComponent << ") matches in all paths, removing";
      for (int i=0; i<loadedFilePaths.count(); ++i)
        {
        QStringList currentFilePath = loadedFilePaths[i];
        currentFilePath.removeFirst();
        loadedFilePaths.replace(i, currentFilePath);
        }
      }
    }
  while (firstComponentMatch);

  // Create hierarchy
  QList<vtkIdType> createdItemIDs;
  for (int nodeIndex=0; nodeIndex<loadedNodes.count(); ++nodeIndex)
    {
    vtkIdType parentItemID = shNode->GetSceneItemID(); // Start from the scene
    for (int componentIndex=0; componentIndex<loadedFilePaths[nodeIndex].count(); ++componentIndex)
      {
      QString currentComponent = loadedFilePaths[nodeIndex][componentIndex];
      vtkIdType itemID = shNode->GetItemChildWithName(parentItemID, currentComponent.toUtf8().constData());
      // If hierarchy node already created
      if (itemID)
        {
        parentItemID = itemID;
        }
      // If hierarchy node not yet created, create it (not the last component -> folder name not file)
      else if (componentIndex < loadedFilePaths[nodeIndex].count()-1)
        {
        // Create parent node if not found for path component
        qSlicerSubjectHierarchyFolderPlugin* folderPlugin = qobject_cast<qSlicerSubjectHierarchyFolderPlugin*>(
          qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Folder") );
        parentItemID = folderPlugin->createFolderUnderItem(parentItemID);
        shNode->SetItemName(parentItemID, currentComponent.toUtf8().constData());
        createdItemIDs << parentItemID;
        }
      // Leaf node (file name) and not top-level
      else if (parentItemID)
        {
        shNode->SetItemParent(vtkIdTypes[nodeIndex], parentItemID, true);
        }
      }
    shNode->ItemModified(parentItemID); // Update subject hierarchy items in the tree
    }

  // Expand generated branches
  foreach(vtkIdType createdItemID, createdItemIDs)
    {
    emit requestExpandItem(createdItemID);
    }
}
