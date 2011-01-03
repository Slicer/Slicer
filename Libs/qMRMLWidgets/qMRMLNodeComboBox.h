/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLNodeComboBox_h
#define __qMRMLNodeComboBox_h

// Qt includes
#include <QComboBox>

// qMRML includes
#include "qMRMLWidgetsExport.h"
#include "qMRMLSortFilterProxyModel.h"

class vtkMRMLNode;
class vtkMRMLScene;

class qMRMLNodeFactory;
class qMRMLNodeComboBoxPrivate;

class QMRML_WIDGETS_EXPORT qMRMLNodeComboBox : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QStringList nodeTypes READ nodeTypes WRITE setNodeTypes)
  Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden)
  Q_PROPERTY(bool showChildNodeTypes READ showChildNodeTypes WRITE setShowChildNodeTypes)
  Q_PROPERTY(QStringList hideChildNodeTypes READ hideChildNodeTypes WRITE setHideChildNodeTypes)
  Q_PROPERTY(bool selectNodeUponCreation READ selectNodeUponCreation WRITE setSelectNodeUponCreation)
  Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)
  Q_PROPERTY(bool addEnabled READ addEnabled WRITE setAddEnabled)
  Q_PROPERTY(bool removeEnabled READ removeEnabled WRITE setRemoveEnabled)
  Q_PROPERTY(bool editEnabled READ editEnabled WRITE setEditEnabled)
  Q_PROPERTY(bool renameEnabled READ renameEnabled WRITE setRenameEnabled)
  Q_PROPERTY(QString baseName READ baseName WRITE setBaseName)
  Q_PROPERTY(QString currentNodeId READ currentNodeId WRITE setCurrentNode DESIGNABLE false)

public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Construct an empty qMRMLNodeComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qMRMLNodeComboBox(QWidget* parent = 0);
  virtual ~qMRMLNodeComboBox();

  ///
  /// Get MRML scene
  vtkMRMLScene* mrmlScene()const;
  ///
  /// Retrieve the sortFilterProxyModel used to filter/sort
  /// the nodes
  qMRMLSortFilterProxyModel* sortFilterProxyModel()const;

  ///
  /// Set/Get node types to display in the list
  /// NodeTypes are the class names, i.e. vtkMRMLViewNode,
  /// vtkMRMLTransformNode
  QStringList nodeTypes()const;
  void setNodeTypes(const QStringList& nodeTypes);

  ///
  /// If a vtkMRMLNode has the property HideFromEditors set to true,
  /// bypass the property and show the node anyway.
  inline void setShowHidden(bool);
  inline bool showHidden()const;

  ///
  /// Display or not the nodes that are excluded by
  /// the ExcludedChildNodeTypes list.
  /// true by default.
  inline void setShowChildNodeTypes(bool show);
  inline bool showChildNodeTypes()const;

  ///
  /// If a node is a nodeType, hide the node if it is also
  /// a ExcludedChildNodeType. (this can happen if nodeType is a
  /// mother class of ExcludedChildNodeType)
  inline void setHideChildNodeTypes(const QStringList& nodeTypes);
  inline QStringList hideChildNodeTypes()const;

  ///
  /// Add node type attribute that filter the nodes to
  /// display. If a node is created via "AddNode", the attributes
  /// will be set to the new node
  Q_INVOKABLE void addAttribute(const QString& nodeType,
                                const QString& attributeName,
                                const QVariant& attributeValue);

  ///
  /// BaseName is the name used to generate a node name for all the new created
  /// nodes.
  /// TODO: Support different basename depending on the node type
  void setBaseName(const QString& baseName);
  QString baseName()const;

  ///
  /// return the number of nodes. it can be different from count()
  /// as count includes the "AddNode", "Remove Node"... items
  int nodeCount()const;

  ///
  /// return the vtkMRMLNode* at the corresponding index. 0 if the index is
  /// invalid
  /// \sa nodeCount(), setCurrentNode(int)
  vtkMRMLNode* nodeFromIndex(int index)const;

  ///
  /// Return the currently selected node. 0 if no node is selected
  Q_INVOKABLE vtkMRMLNode* currentNode()const;

  ///
  /// Return the currently selected node id . "" if no node is selected
  /// Utility function that is based on currentNode
  QString currentNodeId()const;

  ///
  /// if true, when the user create a node using "Add node", the node will be
  /// automatically selected. It doesn't apply if the node is programatically
  /// added (when the combobox is populated by the scene).
  bool selectNodeUponCreation()const;
  void setSelectNodeUponCreation(bool value);

  ///
  /// Set/Get NoneEnabled flags
  /// An additional item is added into the menu list, where the user can select
  /// "None".
  bool noneEnabled()const;
  void setNoneEnabled(bool enable);

  ///
  /// Allow the user to create a new node. An "Add node" item is added into the
  /// menu list.
  bool addEnabled()const;
  void setAddEnabled(bool enable);

  ///
  /// Allow the user to delete the currently selected node. A "Remove node" item
  /// is added to the menu list.
  bool removeEnabled()const;
  void setRemoveEnabled(bool enable);

  /// TODO
  /// Allow the user to modify the properties of the currently selected.
  bool editEnabled()const;
  void setEditEnabled(bool enable);

  ///
  /// Allow the user to rename the node.
  bool renameEnabled()const;
  void setRenameEnabled(bool enable);

  ///
  /// Internal model associated to the combobox
  QAbstractItemModel* model()const;

public slots:
  ///
  /// Set the scene the combobox listens to. The scene is observed and when new
  /// nodes are added to the scene, the menu list is populated.
  virtual void setMRMLScene(vtkMRMLScene* scene);

  ///
  /// Select the node to be current
  void setCurrentNode(vtkMRMLNode* node);

  ///
  /// Select the node to be current
  void setCurrentNode(const QString& nodeID);

  ///
  /// Select the current node by index. The index refers to the order of the nodes
  /// into the list. If index is 0, the first node will be selected (even if
  /// "NoneEnabled" is true).
  /// \sa nodeCount, setCurrentNode(vtkMRMLNode* ), setCurrentNode(const QString&)
  void setCurrentNode(int index);

  ///
  /// Create a node of the same type than on the "node types" properties
  virtual vtkMRMLNode* addNode();

  ///
  /// Remove the current node from the scene
  virtual void removeCurrentNode();

  ///
  /// Edit the currently selected node.
  virtual void editCurrentNode();
  
  ///
  /// Rename the currently selected node.
  virtual void renameCurrentNode();

signals:
  ///
  /// emit the current displayed node. NULL if
  /// the list is empty.
  void currentNodeChanged(vtkMRMLNode* node);

  ///
  /// Signal emitted just after currentNodeChanged(vtkMRMLNode*) is.
  /// \a validNode set to True when the current node is changed.
  /// Set to False when the list is empty. Useful to
  /// enable/disable/show/hide other widgets
  /// depending on the validity of the current node.
  void currentNodeChanged(bool validNode);

  ///
  /// TBD
  /// void nodeAboutToBeAdded(vtkMRMLNode*);

  ///
  /// Signal emitted when \a node is added to the comboBox
  /// Only nodes with valid type emit the signal
  void nodeAdded(vtkMRMLNode* node);

  ///
  /// Signal emitted when \a node is added by the user
  void nodeAddedByUser(vtkMRMLNode* node);

  ///
  /// Signal emitted when \a node is about to be removed from
  /// the comboBox. Only nodes with valid type emit the signal
  void nodeAboutToBeRemoved(vtkMRMLNode* node);

  ///
  /// TBD:
  /// void nodeRemoved(vtkMRMLNode*);
protected:
  /// qMRMLNodeComboBox will not take ownership on the model.
  qMRMLNodeComboBox(QAbstractItemModel* model, QWidget* parent = 0);
  QAbstractItemModel* rootModel()const;

  void setComboBox(QComboBox* comboBox);
  QComboBox* comboBox()const;

protected slots:
  void activateExtraItem(const QModelIndex& index);
  void emitCurrentNodeChanged(int index);
  void emitNodesAdded(const QModelIndex & parent, int start, int end);
  void emitNodesAboutToBeRemoved(const QModelIndex & parent, int start, int end);
  void refreshCurrentItem();

protected:
  QScopedPointer<qMRMLNodeComboBoxPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLNodeComboBox);
  Q_DISABLE_COPY(qMRMLNodeComboBox);
};

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setShowHidden(bool enable)
{
  this->sortFilterProxyModel()->setShowHidden(enable);
}

// --------------------------------------------------------------------------
bool qMRMLNodeComboBox::showHidden()const
{
  return this->sortFilterProxyModel()->showHidden();
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setShowChildNodeTypes(bool show)
{
  this->sortFilterProxyModel()->setShowChildNodeTypes(show);
}

// --------------------------------------------------------------------------
bool qMRMLNodeComboBox::showChildNodeTypes()const
{
  return this->sortFilterProxyModel()->showChildNodeTypes();
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setHideChildNodeTypes(const QStringList& _nodeTypes)
{
  this->sortFilterProxyModel()->setHideChildNodeTypes(_nodeTypes);
}

// --------------------------------------------------------------------------
QStringList qMRMLNodeComboBox::hideChildNodeTypes()const
{
  return this->sortFilterProxyModel()->hideChildNodeTypes();
}

#endif
