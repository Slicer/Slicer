/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyTablesPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTableNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLTableViewNode.h>

// Module logic includes
#include "vtkSlicerTablesLogic.h"

// MRML widgets includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLTableWidget.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerLayoutManager.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyTablesPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyTablesPlugin);
protected:
  qSlicerSubjectHierarchyTablesPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyTablesPluginPrivate(qSlicerSubjectHierarchyTablesPlugin& object);
  ~qSlicerSubjectHierarchyTablesPluginPrivate() override;
  void init();
public:
  QIcon TableIcon;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTablesPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTablesPluginPrivate::qSlicerSubjectHierarchyTablesPluginPrivate(qSlicerSubjectHierarchyTablesPlugin& object)
: q_ptr(&object)
{
  this->TableIcon = QIcon(":Icons/Table.png");
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTablesPluginPrivate::init()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTablesPluginPrivate::~qSlicerSubjectHierarchyTablesPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTablesPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTablesPlugin::qSlicerSubjectHierarchyTablesPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyTablesPluginPrivate(*this) )
{
  this->m_Name = QString("Tables");

  Q_D(qSlicerSubjectHierarchyTablesPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTablesPlugin::~qSlicerSubjectHierarchyTablesPlugin() = default;

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyTablesPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is NULL";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLTableNode"))
    {
    // Node is a table
    return 0.5;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyTablesPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
    }

  // Table
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLTableNode"))
    {
    return 0.5; // There may be other plugins that can handle special Tables better
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyTablesPlugin::roleForPlugin()const
{
  return "Table";
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyTablesPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyTablesPlugin);

  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->TableIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyTablesPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTablesPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return;
    }
  if (this->getDisplayVisibility(itemID) == visible)
    {
    return;
    }

  // Get layout node
  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::SafeDownCast(scene->GetFirstNodeByClass("vtkMRMLLayoutNode"));
  if (!layoutNode)
    {
    qCritical("qSlicerSubjectHierarchyTablesPlugin::getTableViewNode: Unable to get layout node");
    return;
    }

  vtkMRMLTableViewNode* tableViewNode = this->getTableViewNode();

  vtkMRMLTableNode* associatedTableNode = vtkMRMLTableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (associatedTableNode && visible)
    {
    // Switch to a layout that contains table
    int currentLayout = qSlicerApplication::application()->layoutManager()->layout();
    int layoutWithTable = vtkSlicerTablesLogic::GetLayoutWithTable(currentLayout);
    layoutNode->SetViewArrangement(layoutWithTable);

    // Make sure we have a valid table view node (if we want to show the table, but there was
    // no table view, then one was just created when we switched to table layout)
    if (!tableViewNode)
      {
      tableViewNode = this->getTableViewNode();
      }
    if (!tableViewNode)
      {
      qCritical("qSlicerSubjectHierarchyTablesPlugin::getTableViewNode: Unable to get table view node");
      return;
      }

    // Hide currently shown table and trigger icon update
    if ( tableViewNode->GetTableNodeID()
      && strcmp(tableViewNode->GetTableNodeID(), associatedTableNode->GetID()) )
      {
      vtkIdType tableItemID = shNode->GetItemByDataNode(scene->GetNodeByID(tableViewNode->GetTableNodeID()));
      if (tableItemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
        {
        tableViewNode->SetTableNodeID(nullptr);
        shNode->ItemModified(tableItemID);
        }
      }

    // Select table to show
    tableViewNode->SetTableNodeID(associatedTableNode->GetID());
    }
  else if (tableViewNode)
    {
    // Hide table
    tableViewNode->SetTableNodeID(nullptr);
    }

  // Trigger icon update
  shNode->ItemModified(itemID);
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyTablesPlugin::getDisplayVisibility(vtkIdType itemID)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return -1;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return -1;
    }

  vtkMRMLTableViewNode* tableViewNode = this->getTableViewNode();
  if (!tableViewNode)
    {
    // No table view has been set yet
    return 0;
    }

  // Return shown if table in table view is the examined node's associated data node
  vtkMRMLTableNode* associatedTableNode = vtkMRMLTableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if ( associatedTableNode && tableViewNode->GetTableNodeID()
    && !strcmp(tableViewNode->GetTableNodeID(), associatedTableNode->GetID()) )
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
vtkMRMLTableViewNode* qSlicerSubjectHierarchyTablesPlugin::getTableViewNode()const
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return nullptr;
    }

  qMRMLLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    return nullptr;
    }

  for (int i = 0; i<layoutManager->tableViewCount(); i++)
    {
    qMRMLTableWidget* tableWidget = layoutManager->tableWidget(i);
    if (!tableWidget)
      {
      // invalid plot widget
      continue;
      }
    vtkMRMLTableViewNode* tableView = tableWidget->mrmlTableViewNode();
    if (tableView)
      {
      return tableView;
      }
    }

  return nullptr;
}
