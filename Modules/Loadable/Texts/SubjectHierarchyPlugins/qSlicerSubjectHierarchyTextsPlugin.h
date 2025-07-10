/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __qSlicerSubjectHierarchyTextsPlugin_h
#define __qSlicerSubjectHierarchyTextsPlugin_h

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyAbstractPlugin.h"

#include "qSlicerTextsSubjectHierarchyPluginsExport.h"

class qSlicerSubjectHierarchyTextsPluginPrivate;

class Q_SLICER_TEXTS_SUBJECT_HIERARCHY_PLUGINS_EXPORT qSlicerSubjectHierarchyTextsPlugin
  : public qSlicerSubjectHierarchyAbstractPlugin
{
public:
  Q_OBJECT

public:
  typedef qSlicerSubjectHierarchyAbstractPlugin Superclass;
  qSlicerSubjectHierarchyTextsPlugin(QObject* parent = nullptr);
  ~qSlicerSubjectHierarchyTextsPlugin() override;

public:
  /// Determines if a data node can be placed in the hierarchy using the actual plugin,
  /// and gets a confidence value for a certain MRML node (usually the type and possibly attributes are checked).
  /// \param node Node to be added to the hierarchy
  /// \param parentItemID Prospective parent of the node to add.
  ///   Default value is invalid. In that case the parent will be ignored, the confidence numbers are got based on the
  ///   to-be child node alone.
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   node, and 1 means that the plugin is the only one that can handle the node (by node type or identifier
  ///   attribute)
  double canAddNodeToSubjectHierarchy(
    vtkMRMLNode* node,
    vtkIdType parentItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID) const override;

  /// Determines if the actual plugin can handle a subject hierarchy item. The plugin with
  /// the highest confidence number will "own" the item in the subject hierarchy (set icon, tooltip,
  /// set context menu etc.)
  /// \param item Item to handle in the subject hierarchy tree
  /// \return Floating point confidence number between 0 and 1, where 0 means that the plugin cannot handle the
  ///   item, and 1 means that the plugin is the only one that can handle the item (by node type or identifier
  ///   attribute)
  double canOwnSubjectHierarchyItem(vtkIdType itemID) const override;

  /// Get role that the plugin assigns to the subject hierarchy item.
  ///   Each plugin should provide only one role.
  Q_INVOKABLE const QString roleForPlugin() const override;

  /// Get icon of an owned subject hierarchy item
  /// \return Icon to set, empty icon if nothing to set
  QIcon icon(vtkIdType itemID) override;

  /// Generate tooltip for a owned subject hierarchy item
  QString tooltip(vtkIdType itemID) const override;

protected:
  QScopedPointer<qSlicerSubjectHierarchyTextsPluginPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSubjectHierarchyTextsPlugin);
  Q_DISABLE_COPY(qSlicerSubjectHierarchyTextsPlugin);
};

#endif
