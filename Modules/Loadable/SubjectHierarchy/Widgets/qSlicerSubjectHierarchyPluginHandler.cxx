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
#include "qSlicerSubjectHierarchyPluginLogic.h"
#include "qSlicerSubjectHierarchyAbstractPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Subject hierarchy logic includes
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// Qt includes
#include <QDebug>
#include <QStringList>
#include <QInputDialog>
#include <QSettings>

// Slicer includes
#include "qSlicerApplication.h"

// VTK includes
#include <vtkCallbackCommand.h>

//----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginHandler *qSlicerSubjectHierarchyPluginHandler::m_Instance = nullptr;

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
      qSlicerSubjectHierarchyPluginHandler::setInstance(nullptr);
      }
    }
};
static qSlicerSubjectHierarchyPluginHandlerCleanup qSlicerSubjectHierarchyPluginHandlerCleanupGlobal;

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginHandler* qSlicerSubjectHierarchyPluginHandler::instance()
{
  if(!qSlicerSubjectHierarchyPluginHandler::m_Instance)
    {
    qSlicerSubjectHierarchyPluginHandlerCleanupGlobal.use();
    qSlicerSubjectHierarchyPluginHandler::m_Instance = new qSlicerSubjectHierarchyPluginHandler();
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
  // Preferably this will be nullptr
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
  , m_MRMLScene(nullptr)
{
  this->m_CurrentItems.clear();

  this->m_RegisteredPlugins.clear();
  this->m_DefaultPlugin = new qSlicerSubjectHierarchyDefaultPlugin();

  this->m_CallBack = vtkSmartPointer<vtkCallbackCommand>::New();
  this->m_CallBack->SetClientData(this);
  this->m_CallBack->SetCallback(qSlicerSubjectHierarchyPluginHandler::onSubjectHierarchyNodeEvent);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginHandler::~qSlicerSubjectHierarchyPluginHandler()
{
  if (m_MRMLScene != nullptr)
    {
    m_MRMLScene->RemoveObserver(m_CallBack);
    }

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
  if (pluginToRegister == nullptr)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid plugin to register";
    return false;
    }
  if (pluginToRegister->name().isEmpty())
    {
    qCritical() << Q_FUNC_INFO << ": SubjectHierarchy plugin cannot be registered with empty name";
    return false;
    }

  // Check if the same plugin has already been registered
  qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = nullptr;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    if (pluginToRegister->name().compare(currentPlugin->name()) == 0)
      {
      qDebug() << Q_FUNC_INFO << ": SubjectHierarchy plugin " << pluginToRegister->name() << " is already registered";
      return false;
      }
    }

  // Add view menu actions from plugin to plugin logic
  if (this->m_PluginLogic)
    {
    foreach(QAction* action, pluginToRegister->viewContextMenuActions())
      {
      this->m_PluginLogic->registerViewMenuAction(action);
      }
    }

  // Add the plugin to the list
  this->m_RegisteredPlugins << pluginToRegister;

  // Update timestamp
  this->LastPluginRegistrationTime = QDateTime::currentDateTimeUtc();

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
    return nullptr;
    }

  // Find plugin with name
  qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = nullptr;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    if (currentPlugin->name().compare(name) == 0)
      {
      return currentPlugin;
      }
    }

  qWarning() << Q_FUNC_INFO << ": Plugin named '" << name << "' cannot be found";
  return nullptr;
}

//---------------------------------------------------------------------------
QList<qSlicerSubjectHierarchyAbstractPlugin*> qSlicerSubjectHierarchyPluginHandler::pluginsForAddingNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/ )
{
  QList<qSlicerSubjectHierarchyAbstractPlugin*> mostSuitablePlugins;
  double bestConfidence = 0.0;
  qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = nullptr;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    double currentConfidence = currentPlugin->canAddNodeToSubjectHierarchy(node, parentItemID);
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
QList<qSlicerSubjectHierarchyAbstractPlugin*>
qSlicerSubjectHierarchyPluginHandler::pluginsForReparentingItemInSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)
{
  QList<qSlicerSubjectHierarchyAbstractPlugin*> mostSuitablePlugins;
  double bestConfidence = 0.0;
  qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = nullptr;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    double currentConfidence = currentPlugin->canReparentItemInsideSubjectHierarchy(itemID, parentItemID);
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
qSlicerSubjectHierarchyAbstractPlugin*
qSlicerSubjectHierarchyPluginHandler::findOwnerPluginForSubjectHierarchyItem(vtkIdType itemID)
{
  if (this->m_MRMLScene != nullptr && this->m_MRMLScene->GetSubjectHierarchyNode() == nullptr)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy node";
    return nullptr;
    }

  QList<qSlicerSubjectHierarchyAbstractPlugin*> mostSuitablePlugins;
  double bestConfidence = 0.0;
  qSlicerSubjectHierarchyAbstractPlugin* currentPlugin = nullptr;
  foreach (currentPlugin, this->m_RegisteredPlugins)
    {
    double currentConfidence = currentPlugin->canOwnSubjectHierarchyItem(itemID);
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

  // Determine owner plugin based on plugins returning the highest non-zero confidence values for the input item
  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin = nullptr;
  if (mostSuitablePlugins.size() > 1)
    {
    // Let the user choose a plugin if more than one returned the same non-zero confidence value
    vtkMRMLNode* dataNode = this->m_MRMLScene->GetSubjectHierarchyNode()->GetItemDataNode(itemID);
    QString textToDisplay = QString("Equal confidence number found for more than one subject hierarchy plugin.\n\n"
                                    "Select plugin to own node named\n'%1'\n(type %2):").arg(
                                    dataNode?dataNode->GetName():"NULL").arg(dataNode?dataNode->GetNodeTagName():"None");
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
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyPluginHandler::findAndSetOwnerPluginForSubjectHierarchyItem(vtkIdType itemID)
{
  if (this->m_MRMLScene != nullptr && this->m_MRMLScene->GetSubjectHierarchyNode() == nullptr)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy node";
    return nullptr;
    }

  qSlicerSubjectHierarchyAbstractPlugin* ownerPlugin = this->findOwnerPluginForSubjectHierarchyItem(itemID);
  this->m_MRMLScene->GetSubjectHierarchyNode()->SetItemOwnerPluginName(itemID, ownerPlugin->name().toUtf8().constData());
  return ownerPlugin;
}

//---------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyPluginHandler::getOwnerPluginForSubjectHierarchyItem(vtkIdType itemID)
{
  if (this->m_MRMLScene != nullptr && this->m_MRMLScene->GetSubjectHierarchyNode() == nullptr)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid subject hierarchy node";
    return nullptr;
    }

  std::string ownerPluginName = this->m_MRMLScene->GetSubjectHierarchyNode()->GetItemOwnerPluginName(itemID);
  if (ownerPluginName.empty())
    {
    if (itemID != this->m_MRMLScene->GetSubjectHierarchyNode()->GetSceneItemID())
      {
      qCritical() << Q_FUNC_INFO << ": Item '" << this->m_MRMLScene->GetSubjectHierarchyNode()->GetItemName(itemID).c_str() << "' is not owned by any plugin";
      }
    return nullptr;
    }

  return this->pluginByName(ownerPluginName.c_str());
}

//---------------------------------------------------------------------------
qSlicerSubjectHierarchyAbstractPlugin* qSlicerSubjectHierarchyPluginHandler::selectPluginFromDialog(
  QString textToDisplay, QList<qSlicerSubjectHierarchyAbstractPlugin*> candidatePlugins)
{
  if (candidatePlugins.empty())
    {
    qCritical() << Q_FUNC_INFO << ": Empty candidate plugin list! Returning default plugin.";
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
  QString selectedPluginName = QInputDialog::getItem(nullptr, "Select subject hierarchy plugin", textToDisplay, candidatePluginNames, 0, false, &ok);
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
  qWarning() << Q_FUNC_INFO << ": Plugin selection failed! Returning first available plugin";
  return candidatePlugins[0];
}

//----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::observeSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* shNode)
{
  if (m_MRMLScene != nullptr)
  {
    m_MRMLScene->GetSubjectHierarchyNode()->RemoveObserver(m_CallBack);
  }

  if (shNode != nullptr)
  {
    this->setMRMLScene(shNode->GetScene());

    shNode->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent, m_CallBack);
    shNode->AddObserver(vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemOwnerPluginSearchRequested, m_CallBack);
  }
}

//-----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* qSlicerSubjectHierarchyPluginHandler::subjectHierarchyNode()const
{
  return m_MRMLScene == nullptr ? nullptr : m_MRMLScene->GetSubjectHierarchyNode();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setMRMLScene(vtkMRMLScene* scene)
{
  if (scene == this->m_MRMLScene)
    {
    return;
    }

  if (m_MRMLScene != nullptr)
    {
    m_MRMLScene->RemoveObserver(m_CallBack);
    }

  m_MRMLScene = scene;

  if (m_MRMLScene != nullptr)
    {
    scene->AddObserver(vtkMRMLScene::NodeRemovedEvent, m_CallBack);
    this->observeSubjectHierarchyNode(m_MRMLScene->GetSubjectHierarchyNode());
    }
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerSubjectHierarchyPluginHandler::mrmlScene()const
{
  return m_MRMLScene;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyPluginLogic* qSlicerSubjectHierarchyPluginHandler::pluginLogic()
{
  return m_PluginLogic;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setPluginLogic(qSlicerSubjectHierarchyPluginLogic* pluginLogic)
{
  if (m_PluginLogic && pluginLogic != m_PluginLogic)
    {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy plugin logic is a singleton and must not change";
    return;
    }

  m_PluginLogic = pluginLogic;

  // Register view menu actions of those plugins that were registered before the PluginLogic was set.
  if (this->m_PluginLogic)
    {
    foreach(qSlicerSubjectHierarchyAbstractPlugin* pluginToRegister, this->m_RegisteredPlugins)
      {
      foreach(QAction * action, pluginToRegister->viewContextMenuActions())
        {
        this->m_PluginLogic->registerViewMenuAction(action);
        }
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setCurrentItem(vtkIdType itemID)
{
  this->m_CurrentItems.clear();
  this->m_CurrentItems.append(itemID);
}

//-----------------------------------------------------------------------------
vtkIdType qSlicerSubjectHierarchyPluginHandler::currentItem()
{
  if (this->m_CurrentItems.size() != 1)
    {
    return vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    }
  return this->m_CurrentItems.at(0);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setCurrentItems(QList<vtkIdType> items)
{
  this->m_CurrentItems = items;
}

//-----------------------------------------------------------------------------
QList<vtkIdType> qSlicerSubjectHierarchyPluginHandler::currentItems()
{
  return this->m_CurrentItems;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::currentItems(vtkIdList* selectedItems)
{
  if (!selectedItems)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid item list";
    return;
    }

  foreach (vtkIdType item, this->m_CurrentItems)
    {
    selectedItems->InsertNextId(item);
    }
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyPluginHandler::autoDeleteSubjectHierarchyChildren()const
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  if (settings->contains("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren"))
    {
    return settings->value("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren").toBool();
    }

  return false; // Default value
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setAutoDeleteSubjectHierarchyChildren(bool on)
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  settings->setValue("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren", on);
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyPluginHandler::displayPatientIDInSubjectHierarchyItemName()const
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  if (settings->contains("SubjectHierarchy/DisplayPatientIDInSubjectHierarchyItemName"))
    {
    return settings->value("SubjectHierarchy/DisplayPatientIDInSubjectHierarchyItemName").toBool();
    }

  return true; // Default value
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setDisplayPatientIDInSubjectHierarchyItemName(bool on)
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  settings->setValue("SubjectHierarchy/DisplayPatientIDInSubjectHierarchyItemName", on);
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyPluginHandler::displayPatientBirthDateInSubjectHierarchyItemName()const
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  if (settings->contains("SubjectHierarchy/DisplayPatientBirthDateInSubjectHierarchyItemName"))
    {
    return settings->value("SubjectHierarchy/DisplayPatientBirthDateInSubjectHierarchyItemName").toBool();
    }

  return false; // Default value
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setDisplayPatientBirthDateInSubjectHierarchyItemName(bool on)
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  settings->setValue("SubjectHierarchy/DisplayPatientBirthDateInSubjectHierarchyItemName", on);
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyPluginHandler::displayStudyIDInSubjectHierarchyItemName()const
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  if (settings->contains("SubjectHierarchy/DisplayStudyIDInSubjectHierarchyItemName"))
    {
    return settings->value("SubjectHierarchy/DisplayStudyIDInSubjectHierarchyItemName").toBool();
    }

  return false; // Default value
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setDisplayStudyIDInSubjectHierarchyItemName(bool on)
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  settings->setValue("SubjectHierarchy/DisplayStudyIDInSubjectHierarchyItemName", on);
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyPluginHandler::displayStudyDateInSubjectHierarchyItemName()const
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  if (settings->contains("SubjectHierarchy/DisplayStudyDateInSubjectHierarchyItemName"))
    {
    return settings->value("SubjectHierarchy/DisplayStudyDateInSubjectHierarchyItemName").toBool();
    }

  return true; // Default value
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::setDisplayStudyDateInSubjectHierarchyItemName(bool on)
{
  QSettings* settings = qSlicerApplication::application()->settingsDialog()->settings();
  settings->setValue("SubjectHierarchy/DisplayStudyDateInSubjectHierarchyItemName", on);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyPluginHandler::onSubjectHierarchyNodeEvent(
  vtkObject* caller, unsigned long event, void* clientData, void* callData )
{
  vtkMRMLSubjectHierarchyNode* shNode = reinterpret_cast<vtkMRMLSubjectHierarchyNode*>(caller);
  vtkMRMLScene* scene = reinterpret_cast<vtkMRMLScene*>(caller);
  qSlicerSubjectHierarchyPluginHandler* pluginHandler = reinterpret_cast<qSlicerSubjectHierarchyPluginHandler*>(clientData);
  if (!pluginHandler || (!shNode && !scene))
    {
    qCritical() << Q_FUNC_INFO << ": Invalid event parameters";
    return;
    }

  // Handle subject hierarchy node events
  if ( ( event == vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemAddedEvent
      || event == vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemOwnerPluginSearchRequested )
      && shNode->GetScene() && !shNode->GetScene()->IsImporting() )
    {
    // Get item ID
    vtkIdType itemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
    if (callData)
      {
      vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
      if (itemIdPtr)
        {
        itemID = *itemIdPtr;
        }
      }

      // Find plugin for added subject hierarchy item and "claim" it
      pluginHandler->findAndSetOwnerPluginForSubjectHierarchyItem(itemID);
    }
  // Handle scene events
  else if (event == vtkMRMLScene::NodeRemovedEvent)
    {
    // Get node (if any)
    vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(reinterpret_cast<vtkObject*>(callData));

    if (!scene->IsClosing() && node->IsA("vtkMRMLSubjectHierarchyNode"))
      {
      // Make sure there is one subject hierarchy node in the scene
      vtkMRMLSubjectHierarchyNode* newSubjectHierarchyNode = vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(scene);
      if (!newSubjectHierarchyNode)
        {
        qCritical() << Q_FUNC_INFO << ": No subject hierarchy node could be retrieved from the scene";
        }
      }
    }
}
