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

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerAbstractModule.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractModuleWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin::qSlicerSubjectHierarchyAbstractPlugin(QObject *parent)
  : Superclass(parent)
  , m_Name(QString())
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin::~qSlicerSubjectHierarchyAbstractPlugin()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::name()const
{
  if (m_Name.isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ": Empty plugin name!";
    }
  return this->m_Name;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::setName(QString name)
{
  Q_UNUSED(name);
  qCritical() << Q_FUNC_INFO << ": Cannot set plugin name by method, only in constructor!";
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyAbstractPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
{
  Q_UNUSED(itemID);

  // Default implementation applies to plugins that do not define roles, only functions and/or levels
  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyAbstractPlugin::roleForPlugin()const
{
  // Default implementation applies to plugins that do not define roles, only functions and/or levels
  return QString("N/A");
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyAbstractPlugin::helpText()const
{
  // No need to define this function if there is no help text for a plugin
  return QString("");
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyAbstractPlugin::icon(vtkIdType itemID)
{
  Q_UNUSED(itemID);

  // Default implementation applies to plugins that do not define roles, only functions and/or levels
  // If there is no role, then there is no icon to set
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyAbstractPlugin::visibilityIcon(int visible)
{
  Q_UNUSED(visible);

  // Default implementation applies to plugins that do not define roles, only functions and/or levels
  // If there is no role, then there is no visibility icon to set
  return QIcon();
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::editProperties(vtkIdType itemID)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  qSlicerApplication::application()->openNodeModule(shNode->GetItemDataNode(itemID));
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyAbstractPlugin::itemContextMenuActions()const
{
  return QList<QAction*>();
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyAbstractPlugin::sceneContextMenuActions()const
{
  return QList<QAction*>();
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyAbstractPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node,
  vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(node);
  Q_UNUSED(parentItemID);

  // Only role plugins can add node to the hierarchy, so default is 0
  return 0.0;
}

//----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy(
  vtkMRMLNode* nodeToAdd, vtkIdType parentItemID, std::string level/*=""*/)
{
  if (!nodeToAdd)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid node to add";
    return false;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }

  // If parent is invalid, then add it under the scene
  if (!parentItemID)
    {
    parentItemID = shNode->GetSceneItemID();
    }
  // If level is undefined, then add as series
  if (level.empty())
    {
    level = vtkMRMLSubjectHierarchyConstants::GetDICOMLevelSeries();
    }

  // Create subject hierarchy item with added node
  vtkIdType addedItemID = shNode->CreateItem(parentItemID, nodeToAdd, level);
  if (!addedItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to add subject hierarchy item for data node " << nodeToAdd->GetName();
    return false;
    }

  // Set owner plugin to make sure the plugin that adds the item is the owner
  shNode->SetItemOwnerPluginName(addedItemID, this->m_Name.toLatin1().constData());

  return true;
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyAbstractPlugin::canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)const
{
  Q_UNUSED(itemID);
  Q_UNUSED(parentItemID);

  // Many plugins do not perform steps additional to the default
  // when reparenting inside the hierarchy, so return 0 by default
  return 0.0;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::reparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }

  shNode->SetItemParent(itemID, parentItemID);
  return true;
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::displayedItemName(vtkIdType itemID)const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QString();
    }

  return QString(shNode->GetItemName(itemID).c_str());
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::tooltip(vtkIdType itemID)const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QString();
    }

  // Display node type and level in the tooltip
  QString tooltipString("");
  vtkMRMLNode* dataNode = shNode->GetItemDataNode(itemID);
  if (dataNode)
    {
    tooltipString.append(dataNode->GetNodeTagName());
    }

  return tooltipString;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Default behavior is to call SetDisplayVisibility on all displayable associated data nodes in the whole branch
  shNode->SetDisplayVisibilityForBranch(itemID, visible);
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyAbstractPlugin::getDisplayVisibility(vtkIdType itemID)const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0;
    }

  return shNode->GetDisplayVisibilityForBranch(itemID);
}

//--------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::isThisPluginOwnerOfItem(vtkIdType itemID)const
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }

  return !shNode->GetItemOwnerPluginName(itemID).compare(this->m_Name.toLatin1().constData());
}

//--------------------------------------------------------------------------
qSlicerAbstractModuleWidget* qSlicerSubjectHierarchyAbstractPlugin::switchToModule(QString moduleName)
{
  // Find module with name
  qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module(moduleName);
  qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
  if (!moduleWithAction)
    {
    qCritical() << Q_FUNC_INFO << ": Module with name '" << moduleName << "' not found!";
    return NULL;
    }

  // Switch to module
  moduleWithAction->widgetRepresentation(); // Make sure it's created before showing
  moduleWithAction->action()->trigger();

  // Get node selector combobox
  return dynamic_cast<qSlicerAbstractModuleWidget*>(moduleWithAction->widgetRepresentation());
}

//--------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::hideAllContextMenuActions()const
{
  QList<QAction*> allActions;
  allActions << this->sceneContextMenuActions();
  allActions << this->itemContextMenuActions();

  foreach (QAction* action, allActions)
    {
    action->setVisible(false);
    }
}
