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
#include "qSlicerSubjectHierarchyGenericFolderPlugin.h"
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
class qSlicerSubjectHierarchyGenericFolderPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyGenericFolderPlugin);
protected:
  qSlicerSubjectHierarchyGenericFolderPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyGenericFolderPluginPrivate(qSlicerSubjectHierarchyGenericFolderPlugin& object);
  ~qSlicerSubjectHierarchyGenericFolderPluginPrivate();
  void init();
public:
  QIcon FolderIcon;

  QAction* CreateSubjectAction;
  QAction* CreateGenericFolderAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyGenericFolderPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyGenericFolderPluginPrivate::qSlicerSubjectHierarchyGenericFolderPluginPrivate(qSlicerSubjectHierarchyGenericFolderPlugin& object)
: q_ptr(&object)
{
  this->FolderIcon = QIcon(":Icons/Folder.png");

  this->CreateSubjectAction = NULL;
  this->CreateGenericFolderAction = NULL;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyGenericFolderPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyGenericFolderPlugin);

  this->CreateSubjectAction = new QAction("Create new subject",q);
  QObject::connect(this->CreateSubjectAction, SIGNAL(triggered()), q, SLOT(createSubjectNode()));

  this->CreateGenericFolderAction = new QAction("Create child generic folder",q);
  QObject::connect(this->CreateGenericFolderAction, SIGNAL(triggered()), q, SLOT(createGenericFolderUnderCurrentNode()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyGenericFolderPluginPrivate::~qSlicerSubjectHierarchyGenericFolderPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyGenericFolderPlugin::qSlicerSubjectHierarchyGenericFolderPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyGenericFolderPluginPrivate(*this) )
{
  this->m_Name = QString("GenericFolder");

  // Scene -> Subject
  //TODO: Top-level nodes cannot be created automatically, remove if confirmed that it works
  //qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap.insert( QString(),
  //  vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT );
  // Subject -> Generic folder
  qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap.insert( vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT,
    vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_GENERIC_FOLDER );
  // Generic folder -> Generic folder
  qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap.insert( vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_GENERIC_FOLDER,
    vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_GENERIC_FOLDER );

  Q_D(qSlicerSubjectHierarchyGenericFolderPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyGenericFolderPlugin::~qSlicerSubjectHierarchyGenericFolderPlugin()
{
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyGenericFolderPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyGenericFolderPlugin::canOwnSubjectHierarchyNode: Input node is NULL!";
    return 0.0;
    }

  // Subject level
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT))
    {
    return 0.5;
    }
  // Generic folder
  if (node->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_GENERIC_FOLDER))
    {
    return 1.0;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyGenericFolderPlugin::roleForPlugin()const
{
  // NOTE: This is a plugin of third type, that defines levels and containers (folders, groups, etc.)
  // Get current node to determine tole
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyGenericFolderPlugin::roleForPlugin: Invalid current node!";
    return "Error!";
    }

  // Subject level
  if (currentNode->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT))
    {
    return "Patient";
    }
  // GenericFolder level
  if (currentNode->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_GENERIC_FOLDER))
    {
    return "Generic folder";
    }

  return QString("Error!");
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyGenericFolderPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyGenericFolderPlugin::icon: NULL node given!";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyGenericFolderPlugin);

  // Subject and GenericFolder icon
  if ( node->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT)
    || node->IsLevel(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_GENERIC_FOLDER) )
    {
    return d->FolderIcon;
    }

  // Node unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyGenericFolderPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyGenericFolderPlugin::nodeContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyGenericFolderPlugin);

  QList<QAction*> actions;
  actions << d->CreateGenericFolderAction;
  return actions;
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyGenericFolderPlugin::sceneContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyGenericFolderPlugin);

  QList<QAction*> actions;
  actions << d->CreateSubjectAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyGenericFolderPlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyGenericFolderPlugin);
  this->hideAllContextMenuActions();

  // Scene
  if (!node)
    {
    d->CreateSubjectAction->setVisible(true);
    return;
    }
  // GenericFolder can be created under any node
  else
    {
    d->CreateGenericFolderAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyGenericFolderPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  Q_UNUSED(node);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyGenericFolderPlugin::createSubjectNode()
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene)
    {
    qCritical() << "qSlicerSubjectHierarchyGenericFolderPlugin::createSubjectNod: Invalid MRML scene!";
    return;
    }

  // Create subject subject hierarchy node
  std::string nodeName = vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_NEW_NODE_NAME_PREFIX + vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT;
  vtkMRMLSubjectHierarchyNode* childSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
    scene, NULL, vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_SUBJECT, nodeName.c_str());
  emit requestExpandNode(childSubjectHierarchyNode);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyGenericFolderPlugin::createGenericFolderUnderCurrentNode()
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();
  if (!currentNode || !scene)
    {
    qCritical() << "qSlicerSubjectHierarchyGenericFolderPlugin::createGenericFolderUnderCurrentNode: Invalid current node or MRML scene!";
    return;
    }

  // Create generic folder subject hierarchy node
  std::string nodeName = vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_NEW_NODE_NAME_PREFIX + vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_GENERIC_FOLDER;
  vtkMRMLSubjectHierarchyNode* childSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
    scene, currentNode, vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_LEVEL_GENERIC_FOLDER, nodeName.c_str());
  emit requestExpandNode(childSubjectHierarchyNode);
}
