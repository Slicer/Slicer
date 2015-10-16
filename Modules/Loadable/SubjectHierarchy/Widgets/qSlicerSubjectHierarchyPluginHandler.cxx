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

// SubjectHierarchy plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"

// MRML includes
#include <vtkMRMLNode.h>

// Qt includes
#include <QDebug>
#include <QStringList>
#include <QInputDialog>

//----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginHandler *qSlicerSubjectHierarchyPluginHandler::m_Instance = NULL;

//----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyPluginHandlerCleanup
{
public:
  inline void use()   {   }

  ~qSlicerSubjectHierarchyPluginHandlerCleanup()
    {
    if (qSlicerSubjectHierarchyPluginHandler::m_Instance)
      {
      qSlicerSubjectHierarchyPluginHandler::setInstance(NULL);
      }
    }
};
static qSlicerSubjectHierarchyPluginHandlerCleanup qSlicerSubjectHierarchyPluginHandlerCleanupGlobal;

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginHandler* qSlicerSubjectHierarchyPluginHandler::instance()
{
  if(!qSlicerSubjectHierarchyPluginHandler::m_Instance)
    {
    if(!qSlicerSubjectHierarchyPluginHandler::m_Instance)
      {
      qSlicerSubjectHierarchyPluginHandlerCleanupGlobal.use();

      qSlicerSubjectHierarchyPluginHandler::m_Instance = new qSlicerSubjectHierarchyPluginHandler();
      }
    }
  // Return the instance
  return qSlicerSubjectHierarchyPluginHandler::m_Instance;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setInstance(qSlicerSubjectHierarchyPluginHandler* instance)
{
  if (qSlicerSubjectHierarchyPluginHandler::m_Instance==instance)
    {
    return;
    }
  // Preferably this will be NULL
  if (qSlicerSubjectHierarchyPluginHandler::m_Instance)
    {
    delete qSlicerSubjectHierarchyPluginHandler::m_Instance;
    }
  qSlicerSubjectHierarchyPluginHandler::m_Instance = instance;
  if (!instance)
    {
    return;
    }
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginHandler::qSlicerSubjectHierarchyPluginHandler(QObject* parent)
  : QObject(parent)
  , m_Scene(NULL)
{
  this->m_CurrentNodes.clear();
  this->m_RegisteredPlugins.clear();
  this->m_DefaultPlugin = new qSlicerSubjectHierarchyDefaultPlugin();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginHandler::~qSlicerSubjectHierarchyPluginHandler()
{
  QList<qSlicerSubjectHierarchyAbstractPlugin*>::iterator pluginIt;
  for (pluginIt = this->m_RegisteredPlugins.begin(); pluginIt != this->m_RegisteredPlugins.end(); ++pluginIt)
    {
    delete (*pluginIt);
    }
  this->m_RegisteredPlugins.clear();

  delete this->m_DefaultPlugin;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyPluginHandler::registerPlugin(qSlicerSubjectHierarchyAbstractPlugin* pluginToRegister)
{
  if (pluginToRegister == NULL)
    {
    qCritical() << "qSlicerSubjectHierarchyPluginHandler::RegisterPlugin: Invalid plugin to register!";
    return false;
    }
  if (pluginToRegister->name().isEmpty())
    {
    qCritical() << "qSlicerSubjectHierarchyPluginHandler::RegisterPlugin: SubjectHierarchy plugin cannot be registered with empty name!";
    return false;
    }

  // Check if the same plugin has already been registered
  qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = NULL;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    if (pluginToRegister->name().compare(currentPlugin->name()) == 0)
      {
      qDebug() << "qSlicerSubjectHierarchyPluginHandler::RegisterPlugin: "
                    "SubjectHierarchy plugin " << pluginToRegister->name() << " is already registered";
      return false;
      }
    }

  // Add the plugin to the list
  this->m_RegisteredPlugins << pluginToRegister;

  return true;
}

//---------------------------------------------------------------------------
qSlicerSubjectHierarchyDefaultPlugin* qSlicerSubjectHierarchyPluginHandler::defaultPlugin()
{
  return this->m_DefaultPlugin;
}

//---------------------------------------------------------------------------
QList<qSlicerSubjectHierarchyAbstractPlugin*> qSlicerSubjectHierarchyPluginHandler::allPlugins()
{
  QList<qSlicerSubjectHierarchyAbstractPlugin*> allPlugins = this->m_RegisteredPlugins;
  allPlugins << this->m_DefaultPlugin;
  return allPlugins;
}

//---------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyPluginHandler::pluginByName(QString name)
{
  // Return default plugin if requested
  if (name.compare("Default") == 0)
    {
    return this->m_DefaultPlugin;
    }
  else if (name.isEmpty())
    {
    return NULL;
    }

  // Find plugin with name
  qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = NULL;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    if (currentPlugin->name().compare(name) == 0)
      {
      return currentPlugin;
      }
    }

  qWarning() << "qSlicerSubjectHierarchyPluginHandler::pluginByName: Plugin named '" << name << "' cannot be found!";
  return NULL;
}

//---------------------------------------------------------------------------
QList<qSlicerSubjectHierarchyAbstractPlugin*> qSlicerSubjectHierarchyPluginHandler::pluginsForAddingToSubjectHierarchyForNode(
  vtkMRMLNode* node, vtkMRMLSubjectHierarchyNode* parent/*=NULL*/)
{
  QList<qSlicerSubjectHierarchyAbstractPlugin*> mostSuitablePlugins;
  double bestConfidence = 0.0;
  qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = NULL;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    double currentConfidence = currentPlugin->canAddNodeToSubjectHierarchy(node, parent);
    if (currentConfidence > bestConfidence)
      {
      bestConfidence = currentConfidence;

      // Set only the current plugin as most suitable plugin
      mostSuitablePlugins.clear();
      mostSuitablePlugins << currentPlugin;
      }
    else if (currentConfidence > 0.0 && currentConfidence == bestConfidence)
      {
      // Add current plugin to most suitable plugins
      mostSuitablePlugins << currentPlugin;
      }
    }

  return mostSuitablePlugins;
}

//---------------------------------------------------------------------------
QList<qSlicerSubjectHierarchyAbstractPlugin*> qSlicerSubjectHierarchyPluginHandler::pluginsForReparentingInsideSubjectHierarchyForNode(
  vtkMRMLSubjectHierarchyNode* node, vtkMRMLSubjectHierarchyNode* parent/*=NULL*/) //TODO is NULL possible?
{
  QList<qSlicerSubjectHierarchyAbstractPlugin*> mostSuitablePlugins;
  double bestConfidence = 0.0;
  qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = NULL;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    double currentConfidence = currentPlugin->canReparentNodeInsideSubjectHierarchy(node, parent);
    if (currentConfidence > bestConfidence)
      {
      bestConfidence = currentConfidence;

      // Set only the current plugin as most suitable plugin
      mostSuitablePlugins.clear();
      mostSuitablePlugins << currentPlugin;
      }
    else if (currentConfidence > 0.0 && currentConfidence == bestConfidence)
      {
      // Add current plugin to most suitable plugins
      mostSuitablePlugins << currentPlugin;
      }
    }

  return mostSuitablePlugins;
}

//---------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyPluginHandler::findOwnerPluginForSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)
{
  QList<qSlicerSubjectHierarchyAbstractPlugin*> mostSuitablePlugins;
  double bestConfidence = 0.0;
  qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = NULL;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    double currentConfidence = currentPlugin->canOwnSubjectHierarchyNode(node);
    if (currentConfidence > bestConfidence)
      {
      bestConfidence = currentConfidence;

      // Set only the current plugin as most suitable plugin
      mostSuitablePlugins.clear();
      mostSuitablePlugins << currentPlugin;
      }
    else if (currentConfidence > 0.0 && currentConfidence == bestConfidence)
      {
      // Add current plugin to most suitable plugins
      mostSuitablePlugins << currentPlugin;
      }
    }

  // Determine owner plugin based on plugins returning the highest non-zero confidence values for the input node
  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin = NULL;
  if (mostSuitablePlugins.size() > 1)
    {
    // Let the user choose a plugin if more than one returned the same non-zero confidence value
    vtkMRMLNode* associatedNode = (node->GetAssociatedNode() ? node->GetAssociatedNode() : node);
    QString textToDisplay = QString("Equal confidence number found for more than one subject hierarchy plugin.\n\n"
                                    "Select plugin to own node named\n'%1'\n(type %2):").arg(associatedNode->GetName()).arg(associatedNode->GetNodeTagName());
    ownerPlugin = this->selectPluginFromDialog(textToDisplay, mostSuitablePlugins);
    }
  else if (mostSuitablePlugins.size() == 1)
    {
    // One plugin found
    ownerPlugin = mostSuitablePlugins[0];
    }
  else
    {
    // Choose default plugin if all registered plugins returned confidence value 0
    ownerPlugin = m_DefaultPlugin;
    }

  return ownerPlugin;
}

//---------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyPluginHandler::findAndSetOwnerPluginForSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)
{
  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin = this->findOwnerPluginForSubjectHierarchyNode(node);
  node->SetOwnerPluginName(ownerPlugin->name().toLatin1().constData());
  return ownerPlugin;
}

//---------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyPluginHandler::getOwnerPluginForSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)
{
  if (!node->GetOwnerPluginName())
    {
    qCritical() << "qSlicerSubjectHierarchyPluginHandler::getOwnerPluginForSubjectHierarchyNode: Node '" << node->GetName()
      << "' is not owned by any plugin!";
    return NULL;
    }

  return this->pluginByName(node->GetOwnerPluginName());
}

//---------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyPluginHandler::selectPluginFromDialog(
  QString textToDisplay, QList<qSlicerSubjectHierarchyAbstractPlugin*> candidatePlugins)
{
  if (candidatePlugins.empty())
    {
    qCritical() << "qSlicerSubjectHierarchyPluginHandler::selectPluginFromDialog: Empty candidate plugin list! Returning default plugin.";
    return m_DefaultPlugin;
    }

  // Convert list of plugin objects to string list for the dialog
  QStringList candidatePluginNames;
  foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, candidatePlugins)
    {
    candidatePluginNames << plugin->name();
    }

  // Show dialog with a combobox containing the plugins in the input list
  bool ok = false;
  QString selectedPluginName = QInputDialog::getItem(NULL, "Select subject hierarchy plugin", textToDisplay, candidatePluginNames, 0, false, &ok);
  if (ok && !selectedPluginName.isEmpty())
    {
    // The user pressed OK, find the object for the selected plugin [1]
    foreach (qSlicerSubjectHierarchyAbstractPlugin* plugin, candidatePlugins)
      {
      if (!selectedPluginName.compare(plugin->name()))
        {
        return plugin;
        }
      }
    }

  // User pressed cancel (or [1] failed to find the plugin)
  qWarning() << "qSlicerSubjectHierarchyPluginHandler::selectPluginFromDialog: Plugin selection failed! Returning first available plugin";
  return candidatePlugins[0];
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setScene(vtkMRMLScene* scene)
{
  m_Scene = scene;
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerSubjectHierarchyPluginHandler::scene()
{
  return m_Scene;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setCurrentNode(vtkMRMLSubjectHierarchyNode* node)
{
  this->m_CurrentNodes.clear();
  this->m_CurrentNodes.append(node);
}

//-----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qSlicerSubjectHierarchyPluginHandler::currentNode()
{
  if (this->m_CurrentNodes.size() != 1)
    {
    return NULL;
    }
  return this->m_CurrentNodes.at(0);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setCurrentNodes(QList<vtkMRMLSubjectHierarchyNode*> nodes)
{
  this->m_CurrentNodes = nodes;
}

//-----------------------------------------------------------------------------
QList<vtkMRMLSubjectHierarchyNode*> qSlicerSubjectHierarchyPluginHandler::currentNodes()
{
  return this->m_CurrentNodes;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::reconnectOwnerPluginChanged(vtkObject* node, void* callData)
{
  char* oldPluginName = reinterpret_cast<char*>(callData);
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::SafeDownCast(node);
  if (!subjectHierarchyNode)
    {
    qCritical() << "qSlicerSubjectHierarchyPluginHandler::onOwnerPluginChanged: Invalid subject hierarchy node!";
    return;
    }

  qSlicerSubjectHierarchyAbstractPlugin* oldPlugin = this->pluginByName(oldPluginName);
  if (oldPlugin)
    {
    // Let the old plugin perform clean-up actions if necessary
    // Note: qvtkConnect did not work, thus the manual emit. The connection was not established again after disconnect.
    oldPlugin->emitOwnerPluginChanged(node, callData);
    }

  // Let the new plugin perform steps on claiming the node if necessary
  qSlicerSubjectHierarchyAbstractPlugin* newPlugin = this->pluginByName(subjectHierarchyNode->GetOwnerPluginName());
  newPlugin->emitOwnerPluginChanged(node, callData);
}
