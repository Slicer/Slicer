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

#ifndef __qSlicerSubjectHierarchyPluginHandler_h
#define __qSlicerSubjectHierarchyPluginHandler_h

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModuleWidgetsExport.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// Qt includes
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>

class vtkMRMLScene;
class vtkCallbackCommand;
class qSlicerSubjectHierarchyPluginLogic;
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

  /// Flag determining whether children of subject hierarchy nodes are automatically
  /// deleted upon deleting a parent subject hierarchy node.
  /// By default, a pop-up question asking the user to confirm the deletion of
  /// children nodes will be shown.
  Q_PROPERTY (bool autoDeleteSubjectHierarchyChildren READ autoDeleteSubjectHierarchyChildren WRITE setAutoDeleteSubjectHierarchyChildren)
  /// Flag determining whether the patient ID tag is included in the name of the patient
  /// subject hierarchy item name after loading from DICOM.
  /// True by default
  Q_PROPERTY (bool displayPatientIDInSubjectHierarchyItemName READ displayPatientIDInSubjectHierarchyItemName WRITE setDisplayPatientIDInSubjectHierarchyItemName)
  /// Flag determining whether the patient birth date tag is included in the name of the patient
  /// subject hierarchy item name after loading from DICOM.
  /// False by default
  Q_PROPERTY (bool displayPatientBirthDateInSubjectHierarchyItemName READ displayPatientBirthDateInSubjectHierarchyItemName WRITE setDisplayPatientBirthDateInSubjectHierarchyItemName)
  /// Flag determining whether the study ID tag is included in the name of the study
  /// subject hierarchy item name after loading from DICOM.
  /// False by default
  Q_PROPERTY (bool displayStudyIDInSubjectHierarchyItemName READ displayStudyIDInSubjectHierarchyItemName WRITE setDisplayStudyIDInSubjectHierarchyItemName)
  /// Flag determining whether the study date tag is included in the name of the study
  /// subject hierarchy item name after loading from DICOM.
  /// True by default
  Q_PROPERTY (bool displayStudyDateInSubjectHierarchyItemName READ displayStudyDateInSubjectHierarchyItemName WRITE setDisplayStudyDateInSubjectHierarchyItemName)

public:
  /// Instance getter for the singleton class
  /// \return Instance object
  Q_INVOKABLE static qSlicerSubjectHierarchyPluginHandler* instance();

  /// Allows cleanup of the singleton at application exit
  static void setInstance(qSlicerSubjectHierarchyPluginHandler* instance);

  /// Get subject hierarchy node
  Q_INVOKABLE vtkMRMLSubjectHierarchyNode* subjectHierarchyNode()const;
  /// Set MRML scene
  void setMRMLScene(vtkMRMLScene* scene);
  /// Get MRML scene
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;

  /// Get plugin logic
  Q_INVOKABLE qSlicerSubjectHierarchyPluginLogic* pluginLogic();
  /// Set plugin logic
  Q_INVOKABLE void setPluginLogic(qSlicerSubjectHierarchyPluginLogic* pluginLogic);

  /// Set current subject hierarchy item (single selection only)
  /// IMPORTANT NOTE: This function will not change the selection in individual widgets (tree views, comboboxes). This is
  ///                 solely used for plugin-provided context menus.
  Q_INVOKABLE void setCurrentItem(vtkIdType itemID);

  /// Get current subject hierarchy item (single selection only).
  /// This function is called from the plugins when exposing and performing the supported actions. As the plugin actions are not
  /// aggregated on multi-selection, this function is never called from plugins in that case (and thus invalid ID is returned).
  /// IMPORTANT NOTE: This function is solely used for plugin-provided context menus. This is NOT to be used for getting the
  ///                 selected item of individual widgets (tree views, comboboxes).
  /// \return Current item if only one is selected, otherwise INVALID_ITEM_ID
  Q_INVOKABLE vtkIdType currentItem();

  /// Set current subject hierarchy items in case of multi-selection
  /// IMPORTANT NOTE: This function will not change the selection in individual widgets (tree views, comboboxes). This is
  ///                 solely used for plugin-provided context menus.
  Q_INVOKABLE void setCurrentItems(QList<vtkIdType> items);

  /// Get current subject hierarchy items in case of multi-selection
  /// IMPORTANT NOTE: This function is solely used for plugin-provided context menus. This is NOT to be used for getting the
  ///                 selected items of individual widgets (tree views, comboboxes).
  Q_INVOKABLE QList<vtkIdType> currentItems();
  Q_INVOKABLE void currentItems(vtkIdList* selectedItems);

  Q_INVOKABLE bool autoDeleteSubjectHierarchyChildren()const;
  Q_INVOKABLE void setAutoDeleteSubjectHierarchyChildren(bool flag);
  Q_INVOKABLE bool displayPatientIDInSubjectHierarchyItemName()const;
  Q_INVOKABLE void setDisplayPatientIDInSubjectHierarchyItemName(bool on);
  Q_INVOKABLE bool displayPatientBirthDateInSubjectHierarchyItemName()const;
  Q_INVOKABLE void setDisplayPatientBirthDateInSubjectHierarchyItemName(bool on);
  Q_INVOKABLE bool displayStudyIDInSubjectHierarchyItemName()const;
  Q_INVOKABLE void setDisplayStudyIDInSubjectHierarchyItemName(bool on);
  Q_INVOKABLE bool displayStudyDateInSubjectHierarchyItemName()const;
  Q_INVOKABLE void setDisplayStudyDateInSubjectHierarchyItemName(bool on);

public:
  /// Register a plugin
  /// \return True if plugin registered successfully, false otherwise
  Q_INVOKABLE bool registerPlugin(qSlicerSubjectHierarchyAbstractPlugin* plugin);

  /// Get list of registered plugins
  Q_INVOKABLE QList<qSlicerSubjectHierarchyAbstractPlugin*> registeredPlugins() { return m_RegisteredPlugins; };

  /// Get default plugin instance
  Q_INVOKABLE qSlicerSubjectHierarchyDefaultPlugin* defaultPlugin();

  /// Returns all plugins (registered plugins and default plugin)
  Q_INVOKABLE QList<qSlicerSubjectHierarchyAbstractPlugin*> allPlugins();

  /// Get a plugin by name
  /// \return The plugin instance if exists, nullptr otherwise
  Q_INVOKABLE qSlicerSubjectHierarchyAbstractPlugin* pluginByName(QString name);

  /// Returns the plugin that can handle a node the best for adding it from outside to inside the subject hierarchy
  /// The best plugins are found based on the confidence numbers they return for the inputs.
  /// \param node Node to be added to the hierarchy
  /// \param parentItemID Prospective parent of the node to add.
  ///   Default value is invalid. In that case the parent will be ignored, the confidence numbers are got based on the to-be child node alone.
  /// \return The most suitable plugins if found, empty list otherwise
  QList<qSlicerSubjectHierarchyAbstractPlugin*> pluginsForAddingNodeToSubjectHierarchy(
    vtkMRMLNode* node,
    vtkIdType parentItemID=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID );

  /// Returns the plugin that can handle an item the best for reparenting it inside the subject hierarchy
  /// The best plugins are found based on the confidence numbers they return for the inputs.
  /// \param itemID Item to be reparented in the hierarchy
  /// \param parentItemID Prospective parent of the item to reparent.
  /// \return The most suitable plugins if found, empty list otherwise
  QList<qSlicerSubjectHierarchyAbstractPlugin*> pluginsForReparentingItemInSubjectHierarchy(
    vtkIdType itemID, vtkIdType parentItemID );

  /// Find plugin that is most suitable to own a subject hierarchy item.
  /// This method does not set it to the item!
  /// The best plugins are found based on the confidence numbers they return for the inputs.
  /// \param item Item to be owned
  qSlicerSubjectHierarchyAbstractPlugin* findOwnerPluginForSubjectHierarchyItem(vtkIdType itemID);

  /// Find and set plugin that is most suitable to own a subject hierarchy item
  /// The best plugins are found based on the confidence numbers they return for the inputs.
  /// \param item Item to be owned
  qSlicerSubjectHierarchyAbstractPlugin* findAndSetOwnerPluginForSubjectHierarchyItem(vtkIdType itemID);

  /// Get plugin owning a certain subject hierarchy item.
  /// This function doesn't try to find a suitable plugin, it just returns the one already assigned.
  Q_INVOKABLE qSlicerSubjectHierarchyAbstractPlugin* getOwnerPluginForSubjectHierarchyItem(vtkIdType itemID);

  /// Used when multiple plugins are found with the same confidence number for an item.
  /// Pops up a simple dialog asking to choose one plugin from a list.
  /// Note: This should happen very rarely. If happens frequently, then confidence numbers returned by plugins need review.
  /// \param textToDisplay Text assembled by the caller displaying the reason and basis (items) of the choice
  /// \param candidatePlugins List of plugins to choose from
  /// \return Plugin chosen by the user
  qSlicerSubjectHierarchyAbstractPlugin* selectPluginFromDialog(QString textToDisplay, QList<qSlicerSubjectHierarchyAbstractPlugin*> candidatePlugins);

  /// Add an observer on the given SH node to watch for changes and react
  /// as needed
  /// \param shNode the subject hierarchy node to observe
  void observeSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* shNode);

protected:
  /// Handle subject hierarchy node events
  static void onSubjectHierarchyNodeEvent(vtkObject* caller, unsigned long event, void* clientData, void* callData);

protected:
  /// List of registered plugin instances
  QList<qSlicerSubjectHierarchyAbstractPlugin*> m_RegisteredPlugins;

  /// Instance of the default plugin to access it if there is no suitable plugin found
  /// (the default plugin instance cannot be in the registered list, as then it would never be returned)
  qSlicerSubjectHierarchyDefaultPlugin* m_DefaultPlugin;

  /// Current subject hierarchy item(s)
  /// (selected items in the tree view e.g. for context menu request)
  QList<vtkIdType> m_CurrentItems;

  /// MRML scene (to get new subject hierarchy node if the stored one is deleted)
  vtkWeakPointer<vtkMRMLScene> m_MRMLScene;

  /// Plugin logic
  qSlicerSubjectHierarchyPluginLogic* m_PluginLogic{nullptr};

  /// Callback handling deletion of the subject hierarchy node
  vtkSmartPointer<vtkCallbackCommand> m_CallBack;

public:
  /// Private constructor made public to enable python wrapping
  /// IMPORTANT: Should not be used for creating plugin handler! Use instance() instead.
  qSlicerSubjectHierarchyPluginHandler(QObject* parent=nullptr);

  /// Private destructor made public to enable python wrapping
  ~qSlicerSubjectHierarchyPluginHandler() override;

  /// Timestamp of the last plugin registration. Used to allow context menus be repopulated if needed.
  QDateTime LastPluginRegistrationTime;

private:
  Q_DISABLE_COPY(qSlicerSubjectHierarchyPluginHandler);
  friend class qSlicerSubjectHierarchyPluginHandlerCleanup;

private:
  /// Instance of the singleton
  static qSlicerSubjectHierarchyPluginHandler* m_Instance;
};

#endif
