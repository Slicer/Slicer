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
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTableNode.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLTableViewNode.h>

// MRML widgets includes
#include "qMRMLNodeComboBox.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// SlicerQt includes
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
  ~qSlicerSubjectHierarchyTablesPluginPrivate();
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

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTablesPluginPrivate::~qSlicerSubjectHierarchyTablesPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTablesPlugin::qSlicerSubjectHierarchyTablesPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyTablesPluginPrivate(*this) )
{
  this->m_Name = QString("Tables");

  Q_D(qSlicerSubjectHierarchyTablesPlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTablesPluginPrivate::init()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTablesPlugin::~qSlicerSubjectHierarchyTablesPlugin()
{
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyTablesPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkMRMLSubjectHierarchyNode* parent/*=NULL*/)const
{
  Q_UNUSED(parent);
  if (!node)
    {
    qCritical("qSlicerSubjectHierarchyTablesPlugin::canAddNodeToSubjectHierarchy: Input node is NULL");
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
double qSlicerSubjectHierarchyTablesPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical("qSlicerSubjectHierarchyTablesPlugin::canOwnSubjectHierarchyNode: Input node is NULL");
    return 0.0;
    }

  // Table
  vtkMRMLNode* associatedNode = node->GetAssociatedNode();
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
QIcon qSlicerSubjectHierarchyTablesPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  if (!node)
    {
    qCritical("qSlicerSubjectHierarchyTablesPlugin::icon: NULL node given");
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyTablesPlugin);

  if (this->canOwnSubjectHierarchyNode(node))
    {
    return d->TableIcon;
    }

  // Node unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyTablesPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTablesPlugin::setDisplayVisibility(vtkMRMLSubjectHierarchyNode* node, int visible)
{
  if (!node)
    {
    qCritical("qSlicerSubjectHierarchyTablesPlugin::setDisplayVisibility: NULL node");
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene)
    {
    qCritical("qSlicerSubjectHierarchyTablesPlugin::setDisplayVisibility: Invalid MRML scene");
    return;
    }
  if (this->getDisplayVisibility(node) == visible)
    {
    return;
    }

  // Get layout node
  vtkSmartPointer<vtkCollection> layoutNodes =
    vtkSmartPointer<vtkCollection>::Take( scene->GetNodesByClass("vtkMRMLLayoutNode") );
  layoutNodes->InitTraversal();
  vtkObject* layoutNodeVtkObject = layoutNodes->GetNextItemAsObject();
  vtkMRMLLayoutNode* layoutNode = vtkMRMLLayoutNode::SafeDownCast(layoutNodeVtkObject);
  if (!layoutNode)
    {
    qCritical("qSlicerSubjectHierarchyTablesPlugin::getTableViewNode: Unable to get layout node");
    return;
    }

  vtkMRMLTableViewNode* tableViewNode = this->getTableViewNode();

  vtkMRMLTableNode* associatedTableNode = vtkMRMLTableNode::SafeDownCast(node->GetAssociatedNode());
  if (associatedTableNode && visible)
    {
    // Switch to four-up table layout
    layoutNode->SetViewArrangement( vtkMRMLLayoutNode::SlicerLayoutFourUpTableView );

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
      vtkMRMLSubjectHierarchyNode* currentTableShNode =
        vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(
        scene->GetNodeByID(tableViewNode->GetTableNodeID()) );
      if (currentTableShNode)
        {
        tableViewNode->SetTableNodeID(NULL);
        currentTableShNode->Modified();
        }
      }

    // Select table to show
    tableViewNode->SetTableNodeID(associatedTableNode->GetID());
    }
  else if (tableViewNode)
    {
    // Hide table
    tableViewNode->SetTableNodeID(NULL);
    }

  // Trigger icon update
  node->Modified();
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyTablesPlugin::getDisplayVisibility(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical("qSlicerSubjectHierarchyTablesPlugin::getDisplayVisibility: NULL node");
    return -1;
    }

  vtkMRMLTableViewNode* tableViewNode = this->getTableViewNode();
  if (!tableViewNode)
    {
    // No table view has been set yet
    return 0;
    }

  // Return hidden if current layout is not one of the table ones
  if ( qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutFourUpTableView
    /* All layouts should be added here where a table can be shown:
    && qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutOneUpTableView
    && qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutConventionalTableView
    && qSlicerApplication::application()->layoutManager()->layout() != vtkMRMLLayoutNode::SlicerLayoutThreeOverThreeTableView
    */
    )
    {
    return 0;
    }

  // Return shown if table in table view is the examined node's associated data node
  vtkMRMLTableNode* associatedTableNode = vtkMRMLTableNode::SafeDownCast(node->GetAssociatedNode());
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
void qSlicerSubjectHierarchyTablesPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  // Switch to tables module and select transform
  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("Tables");
  if (moduleWidget)
    {
    // Get node selector combobox
    qMRMLNodeComboBox* nodeSelector = moduleWidget->findChild<qMRMLNodeComboBox*>("TableNodeSelector");

    // Choose current data node
    if (nodeSelector)
      {
      nodeSelector->setCurrentNode(node->GetAssociatedNode());
      }
    }
}

//---------------------------------------------------------------------------
vtkMRMLTableViewNode* qSlicerSubjectHierarchyTablesPlugin::getTableViewNode()const
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene)
    {
    qCritical("qSlicerSubjectHierarchyTablesPlugin::getTableViewNode: Invalid MRML scene");
    return NULL;
    }

  vtkSmartPointer<vtkCollection> tableViewNodes =
    vtkSmartPointer<vtkCollection>::Take( scene->GetNodesByClass("vtkMRMLTableViewNode") );
  tableViewNodes->InitTraversal();
  vtkMRMLTableViewNode* tableViewNode = vtkMRMLTableViewNode::SafeDownCast( tableViewNodes->GetNextItemAsObject() );
  if (!tableViewNode)
    {
    qCritical("qSlicerSubjectHierarchyTablesPlugin::getTableViewNode: Unable to get table view node");
    return NULL;
    }

  return tableViewNode;
}
