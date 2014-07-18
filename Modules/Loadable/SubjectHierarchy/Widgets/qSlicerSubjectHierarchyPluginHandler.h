/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

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

#ifndef __qSlicerSubjectHierarchyPluginHandler_h
#define __qSlicerSubjectHierarchyPluginHandler_h

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

// Qt includes
#include <QObject>
#include <QList>
#include <QString>

class vtkObject;
class vtkMRMLNode;
class vtkMRMLSubjectHierarchyNode;
class vtkMRMLScene;
class qSlicerSubjectHierarchyAbstractPlugin;
class qSlicerSubjectHierarchyDefaultPlugin;
class qSlicerSubjectHierarchyPluginHandlerCleanup;


/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
///    In Widgets, not Plugins because the paths and libs need to be exported to extensions
/// \class qSlicerSubjectHierarchyPluginHandler
/// \brief Singleton class managing Subject Hierarchy plugins
class Q_SLICER_MODULE_SUBJECTHIERARCHY_WIDGETS_EXPORT qSlicerSubjectHierarchyPluginHandler : public QObject
{
  Q_OBJECT

public:
  /// Instance getter for the singleton class
  /// \return Instance object
  static qSlicerSubjectHierarchyPluginHandler* instance();

  /// Allows cleanup of the singleton at application exit
  static void setInstance(qSlicerSubjectHierarchyPluginHandler* instance);

  /// Set MRML scene
  void setScene(vtkMRMLScene* scene);

  /// Get MRML scene
  vtkMRMLScene* scene();

  /// Set current subject hierarchy node
  void setCurrentNode(vtkMRMLSubjectHierarchyNode* node);

  /// Get current subject hierarchy node
  vtkMRMLSubjectHierarchyNode* currentNode();

public:
  /// Register a plugin
  /// \return True if plugin registered successfully, false otherwise
  bool registerPlugin(qSlicerSubjectHierarchyAbstractPlugin* plugin);

  /// Get list of registered plugins
  QList<qSlicerSubjectHierarchyAbstractPlugin*> registeredPlugins() { return m_RegisteredPlugins; };

  /// Get default plugin instance
  qSlicerSubjectHierarchyDefaultPlugin* defaultPlugin();

  /// Returns all plugins (registered plugins and default plugin)
  QList<qSlicerSubjectHierarchyAbstractPlugin*> allPlugins();

  /// Get a plugin by name
  /// \return The plugin instance if exists, NULL otherwise
  qSlicerSubjectHierarchyAbstractPlugin* pluginByName(QString name);

  /// Returns the plugin that can handle a node the best for adding it from outside to inside the subject hierarchy
  /// The best plugins are found based on the confidence numbers they return for the inputs.
  /// \param node Node to be added to the hierarchy
  /// \param parent Prospective parent of the node to add.
  ///   Default value is NULL. In that case the parent will be ignored, the confidence numbers are got based on the to-be child node alone.
  /// \return The most suitable plugins if found, empty list otherwise
  QList<qSlicerSubjectHierarchyAbstractPlugin*> pluginsForAddingToSubjectHierarchyForNode(vtkMRMLNode* node, vtkMRMLSubjectHierarchyNode* parent=NULL);

  /// Returns the plugin that can handle a node the best for reparenting it inside the subject hierarchy
  /// The best plugins are found based on the confidence numbers they return for the inputs.
  /// \param node Node to be reparented in the hierarchy
  /// \param parent Prospective parent of the node to reparent.
  /// \return The most suitable plugins if found, empty list otherwise
  QList<qSlicerSubjectHierarchyAbstractPlugin*> pluginsForReparentingInsideSubjectHierarchyForNode(vtkMRMLSubjectHierarchyNode* node,
                                                                                                   vtkMRMLSubjectHierarchyNode* parent);

  /// Find plugin that is most suitable to own a subject hierarchy node.
  /// This method does not set it to the node!
  /// The best plugins are found based on the confidence numbers they return for the inputs.
  /// \param node Node to be owned
  qSlicerSubjectHierarchyAbstractPlugin* findOwnerPluginForSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node);

  /// Find and set plugin that is most suitable to own a subject hierarchy node
  /// The best plugins are found based on the confidence numbers they return for the inputs.
  /// \param node Node to be owned
  qSlicerSubjectHierarchyAbstractPlugin* findAndSetOwnerPluginForSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node);

  /// Get plugin owning a certain subject hierarchy node.
  /// This function doesn't try to find a suitable plugin, it just returns the one already assigned.
  qSlicerSubjectHierarchyAbstractPlugin* getOwnerPluginForSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node);

  /// Used when multiple plugins are found with the same confidence number for a node.
  /// Pops up a simple dialog asking to choose one plugin from a list.
  /// Note: This should happen very rarely. If happens frequently, then confidence numbers returned by plugins need review.
  /// \param textToDisplay Text assembled by the caller displaying the reason and basis (nodes) of the choice
  /// \param candidatePlugins List of plugins to choose from
  /// \return Plugin chosen by the user
  qSlicerSubjectHierarchyAbstractPlugin* selectPluginFromDialog(QString textToDisplay, QList<qSlicerSubjectHierarchyAbstractPlugin*> candidatePlugins);

protected slots:
  /// Reconnect 'owner plugin changed' signal to the new owner plugin.
  /// When the owner plugin is changed, both the old and the new plugins have the chance to
  /// take actions (e.g. the new plugin can force a level) by connecting a locally defined
  /// slot to the \sa ownerPluginChanged signal.
  void reconnectOwnerPluginChanged(vtkObject* node, void* callData);

protected:
  /// List of registered plugin instances
  QList<qSlicerSubjectHierarchyAbstractPlugin*> m_RegisteredPlugins;

  /// Instance of the default plugin to access it if there is no suitable plugin found
  /// (the default plugin instance cannot be in the registered list, as then it would never be returned)
  qSlicerSubjectHierarchyDefaultPlugin* m_DefaultPlugin;

  //TODO: Is there a way to avoid having this member? (Having a member for the tree view is equally bad)
  /// Current subject hierarchy node
  /// (selected node in the tree view e.g. for context menu request)
  vtkMRMLSubjectHierarchyNode* m_CurrentNode;

  //TODO: is there a better way to access the scene without a node?
  /// MRML scene
  vtkMRMLScene* m_Scene;

private:
  /// Private constructor
  qSlicerSubjectHierarchyPluginHandler();

  /// Private destructor
  virtual ~qSlicerSubjectHierarchyPluginHandler();

private:
  Q_DISABLE_COPY(qSlicerSubjectHierarchyPluginHandler);
  friend class qSlicerSubjectHierarchyPluginHandlerCleanup;
  friend class qSlicerSubjectHierarchyModule;

private:
  /// Instance of the singleton
  static qSlicerSubjectHierarchyPluginHandler* m_Instance;
};

#endif
