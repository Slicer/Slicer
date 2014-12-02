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
#include "qSlicerSubjectHierarchyFolderPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Qt includes
#include <QDebug>
#include <QAction>
#include <QStandardItem>

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyFolderPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyFolderPlugin);
protected:
  qSlicerSubjectHierarchyFolderPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyFolderPluginPrivate(qSlicerSubjectHierarchyFolderPlugin& object);
  ~qSlicerSubjectHierarchyFolderPluginPrivate();
  void init();
public:
  QIcon FolderIcon;

  QAction* CreateFolderUnderSceneAction;
  QAction* CreateFolderUnderNodeAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyFolderPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyFolderPluginPrivate::qSlicerSubjectHierarchyFolderPluginPrivate(qSlicerSubjectHierarchyFolderPlugin& object)
: q_ptr(&object)
{
  this->FolderIcon = QIcon(":Icons/Folder.png");

  this->CreateFolderUnderSceneAction = NULL;
  this->CreateFolderUnderNodeAction = NULL;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyFolderPlugin);

  this->CreateFolderUnderSceneAction = new QAction("Create new folder",q);
  QObject::connect(this->CreateFolderUnderSceneAction, SIGNAL(triggered()), q, SLOT(createFolderUnderScene()));

  this->CreateFolderUnderNodeAction = new QAction("Create child folder",q);
  QObject::connect(this->CreateFolderUnderNodeAction, SIGNAL(triggered()), q, SLOT(createFolderUnderCurrentNode()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyFolderPluginPrivate::~qSlicerSubjectHierarchyFolderPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyFolderPlugin::qSlicerSubjectHierarchyFolderPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyFolderPluginPrivate(*this) )
{
  this->m_Name = QString("Folder");

  Q_D(qSlicerSubjectHierarchyFolderPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyFolderPlugin::~qSlicerSubjectHierarchyFolderPlugin()
{
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyFolderPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyFolderPlugin::canOwnSubjectHierarchyNode: Input node is NULL!";
    return 0.0;
    }

  // Folder
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()))
    {
    return 1.0;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyFolderPlugin::roleForPlugin()const
{
  // Get current node to determine role
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyFolderPlugin::roleForPlugin: Invalid current node!";
    return "Error!";
    }

  // Folder level
  if (currentNode->IsLevel(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()))
    {
    return "Folder";
    }

  return QString("Error!");
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyFolderPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyFolderPlugin::icon: NULL node given!";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyFolderPlugin);

  // Subject and Folder icon
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()))
    {
    return d->FolderIcon;
    }

  // Node unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyFolderPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyFolderPlugin::nodeContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyFolderPlugin);

  QList<QAction*> actions;
  actions << d->CreateFolderUnderNodeAction;
  return actions;
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyFolderPlugin::sceneContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyFolderPlugin);

  QList<QAction*> actions;
  actions << d->CreateFolderUnderSceneAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyFolderPlugin);
  this->hideAllContextMenuActions();

  // Scene
  if (!node)
    {
    d->CreateFolderUnderSceneAction->setVisible(true);
    return;
    }

  // Folder can be created under any node
  if (node)
    {
    d->CreateFolderUnderNodeAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  Q_UNUSED(node);
}

//---------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qSlicerSubjectHierarchyFolderPlugin::createFolderUnderNode(vtkMRMLSubjectHierarchyNode* parentNode)
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene)
    {
    qCritical() << "qSlicerSubjectHierarchyFolderPlugin::createFolderUnderNode: Invalid MRML scene!";
    return NULL;
    }

  // Create folder subject hierarchy node
  std::string nodeName = vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyNewNodeNamePrefix() + vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder();
  nodeName = scene->GenerateUniqueName(nodeName);
  vtkMRMLSubjectHierarchyNode* childSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
    scene, parentNode, vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder(), nodeName.c_str());
  emit requestExpandNode(childSubjectHierarchyNode);

  return childSubjectHierarchyNode;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPlugin::createFolderUnderScene()
{
  this->createFolderUnderNode(NULL);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyFolderPlugin::createFolderUnderCurrentNode()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyFolderPlugin::createFolderUnderCurrentNode: Invalid current node!";
    return;
    }

  this->createFolderUnderNode(currentNode);
}
