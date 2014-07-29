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
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"
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

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
QMap<QString, QString> qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap = QMap<QString, QString>();

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
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::name: Empty plugin name!";
    }
  return this->m_Name;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyAbstractPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  Q_UNUSED(node);

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
QIcon qSlicerSubjectHierarchyAbstractPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  Q_UNUSED(node);

  // Default implementation applies to plugins that do not define roles, only functions and/or levels
  // If there is no role, then there is no icon to set
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyAbstractPlugin::visibilityIcon(int visible)
{
  // Default implementation applies to plugins that do not define roles, only functions and/or levels
  // If there is no role, then there is no visibility icon to set
  return QIcon();
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  Q_UNUSED(node);

  // If there is no role, no edit properties action is needed
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyAbstractPlugin::nodeContextMenuActions()const
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
                                                                           vtkMRMLSubjectHierarchyNode* parent/*=NULL*/)const
{
  Q_UNUSED(node);
  Q_UNUSED(parent);

  // Many plugins do not perform steps additional to the default when
  // adding nodes to the hierarchy from outside, so return 0 by default
  return 0.0;
}

//----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy(vtkMRMLNode* nodeToAdd, vtkMRMLSubjectHierarchyNode* parentNode)
{
  if (!nodeToAdd || !parentNode)
    {
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy: Invalid node to add or parent node!";
    return false;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene)
    {
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy: Invalid MRML scene!";
    return false;
    }

  // Associate to a new hierarchy node and put it in the tree under the parent
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
    scene, parentNode, this->childLevel(parentNode->GetLevel()).toLatin1().constData(), nodeToAdd->GetName(), nodeToAdd);
  if (!subjectHierarchyNode)
    {
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy: Failed to create subject hierarchy node!";
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyAbstractPlugin::canReparentNodeInsideSubjectHierarchy(vtkMRMLSubjectHierarchyNode* node,
                                                                                    vtkMRMLSubjectHierarchyNode* parent)const
{
  Q_UNUSED(node);
  Q_UNUSED(parent);

  // Many plugins do not perform steps additional to the default
  // when reparenting inside the hierarchy, so return 0 by default
  return 0.0;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::reparentNodeInsideSubjectHierarchy(vtkMRMLSubjectHierarchyNode* nodeToReparent,
                                                                               vtkMRMLSubjectHierarchyNode* parentNode)
{
  nodeToReparent->SetParentNodeID(parentNode ? parentNode->GetID() : NULL);
  return true;
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::displayedName(vtkMRMLSubjectHierarchyNode* node)const
{
  QString nodeText(node->GetName());
  if (nodeText.endsWith(QString(vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_NODE_NAME_POSTFIX.c_str())))
    {
    nodeText = nodeText.left( nodeText.size() - vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_NODE_NAME_POSTFIX.size() );
    }

  return nodeText;
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::tooltip(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::tooltip: Subject hierarchy node is NULL!";
    return QString("Invalid!");
    }

  // Display node type and level in the tooltip
  QString tooltipString("");
  vtkMRMLNode* associatedNode = node->GetAssociatedNode();
  if (associatedNode)
    {
    tooltipString.append(associatedNode->GetNodeTagName());
    tooltipString.append(" (");
    }

  tooltipString.append("Level:");
  tooltipString.append(node->GetLevel());
  tooltipString.append(" Plugin:");
  tooltipString.append(node->GetOwnerPluginName() ? node->GetOwnerPluginName() : "None");

  if (associatedNode)
    {
    tooltipString.append(")");
    }

  return tooltipString;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::setDisplayVisibility(vtkMRMLSubjectHierarchyNode* node, int visible)
{
  Q_UNUSED(node);

  // Default behavior is to call SetDisplayVisibility on all displayable
  // associated nodes in the whole branch
  node->SetDisplayVisibilityForBranch(visible);
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyAbstractPlugin::getDisplayVisibility(vtkMRMLSubjectHierarchyNode* node)const
{
  Q_UNUSED(node);

  return node->GetDisplayVisibilityForBranch();
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyAbstractPlugin::childLevel(QString parentLevel)
{
  // Get child level from this plugin
  if (qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap.contains(parentLevel))
    {
    return qSlicerSubjectHierarchyAbstractPlugin::m_ChildLevelMap[parentLevel];
    }
  // If this plugin does not have child level for this parent level, then log a warning
  else
    {
    qWarning() << "qSlicerSubjectHierarchyAbstractPlugin::childLevel: Could not get child level for level '"
      << parentLevel << "'!";
    return QString("Invalid");
    }
}

//--------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qSlicerSubjectHierarchyAbstractPlugin::createChildNode(vtkMRMLSubjectHierarchyNode* parentNode,
                                                                                    QString nodeName,
                                                                                    vtkMRMLNode* associatedNode/*=NULL*/)
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();

  // If there is current node, parent level will be an empty string, which means the scene
  QString parentLevel;
  if (parentNode)
    {
    parentLevel = QString(parentNode->GetLevel());
    }
  QString childLevel = this->childLevel(parentLevel);

  // Create child subject hierarchy node
  vtkMRMLSubjectHierarchyNode* childSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::CreateSubjectHierarchyNode(
    scene, parentNode, childLevel.toLatin1().constData(), nodeName.toLatin1().constData(), associatedNode);

  emit requestExpandNode(childSubjectHierarchyNode);

  return childSubjectHierarchyNode;
}

//--------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::createChildForCurrentNode()
{
  vtkMRMLSubjectHierarchyNode* currentNode = qSlicerSubjectHierarchyPluginHandler::instance()->currentNode();

  // If there is current node, parent level will be an empty string, which means the scene
  QString parentLevel;
  if (currentNode)
    {
    parentLevel = QString(currentNode->GetLevel());
    }
  QString childLevel = this->childLevel(parentLevel);

  // Create child subject hierarchy node
  std::string childNodeName = vtkMRMLSubjectHierarchyConstants::SUBJECTHIERARCHY_NEW_NODE_NAME_PREFIX + childLevel.toLatin1().constData();
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  childNodeName = scene->GenerateUniqueName(childNodeName);

  this->createChildNode(currentNode, childNodeName.c_str());
}

//--------------------------------------------------------------------------
void qSlicerSubjectHierarchyAbstractPlugin::emitOwnerPluginChanged(vtkObject* node, void* callData)
{
  emit ownerPluginChanged(node, callData);
}

//--------------------------------------------------------------------------
bool qSlicerSubjectHierarchyAbstractPlugin::isThisPluginOwnerOfNode(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    return false;
    }

  return !strcmp(node->GetOwnerPluginName(), this->m_Name.toLatin1().constData());
}

//--------------------------------------------------------------------------
qSlicerAbstractModuleWidget* qSlicerSubjectHierarchyAbstractPlugin::switchToModule(QString moduleName)
{
  // Find module with name
  qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module(moduleName);
  qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
  if (!moduleWithAction)
    {
    qCritical() << "qSlicerSubjectHierarchyAbstractPlugin::switchToModule: Module with name '" << moduleName << "' not found!";
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
  allActions << this->nodeContextMenuActions();

  foreach (QAction* action, allActions)
    {
    action->setVisible(false);
    }
}
