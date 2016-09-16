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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLNodeComboBox_h
#define __qMRMLNodeComboBox_h

// Qt includes
#include <QComboBox>

// qMRML includes
#include "qMRMLSortFilterProxyModel.h"
#include "qMRMLWidgetsExport.h"
class qMRMLNodeFactory;
class qMRMLNodeComboBoxPrivate;
class qMRMLSceneModel;

// VTK includes
class vtkMRMLNode;
class vtkMRMLScene;

/// \class qMRMLNodeComboBox
/// \brief Combobox that automatically displays all the nodes of the scene that
/// match filtering criteria.
///
/// qMRMLNodeComboBox observes a MRML scene and gets automatically
/// populated with nodes of the scene that are of type \a nodeTypes,
/// and that are not hidden from editor (see vtkMRMLNode::GetHideFromEditor())
/// except if \a showHidden is true.
/// Sometimes, having no current node can be a valid choice, \a noneEnabled
/// allows the user to select or not "None" as the current node.
/// qMRMLNodeComboBox is disabled (grayed out) until a valid scene (not null)
/// is provided via setMRMLScene(). Setting the scene can be done from the
/// designer by connecting a mrmlSceneChanged(vtkMRMLScene*) signal with the
/// slot qMRMLNodeComboBox::setMRMLScene(vtkMRMLScene*).
/// In addition to the populated nodes, qMRMLNodeComboBox contains menu
/// items to add, delete, edit or rename the currently selected node. Each item
/// can be hidden.
class QMRML_WIDGETS_EXPORT qMRMLNodeComboBox
  : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString currentNodeID READ currentNodeID WRITE setCurrentNodeID NOTIFY currentNodeIDChanged DESIGNABLE false)
  Q_PROPERTY(QString currentNodeId READ currentNodeId WRITE setCurrentNode DESIGNABLE false) // \deprecated
  Q_PROPERTY(QStringList nodeTypes READ nodeTypes WRITE setNodeTypes)
  Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden)
  Q_PROPERTY(bool showChildNodeTypes READ showChildNodeTypes WRITE setShowChildNodeTypes)
  Q_PROPERTY(QStringList hideChildNodeTypes READ hideChildNodeTypes WRITE setHideChildNodeTypes)

  Q_PROPERTY(QString baseName READ baseName WRITE setBaseName)
  /// This property controls whether an additional item is added into the menu
  /// list, such item allows the user to select none of the nodes in the
  /// combobox list. By default, the display of the item is "None" but it can be
  /// changed with \a noneDisplay.
  /// \sa noneDisplay, addEnabled, removeEnabled, editEnabled, renameEnabled
  /// \sa noneEnabled(), setNoneEnabled()
  Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)
  Q_PROPERTY(bool addEnabled READ addEnabled WRITE setAddEnabled)
  Q_PROPERTY(bool removeEnabled READ removeEnabled WRITE setRemoveEnabled)
  Q_PROPERTY(bool editEnabled READ editEnabled WRITE setEditEnabled)
  Q_PROPERTY(bool renameEnabled READ renameEnabled WRITE setRenameEnabled)

  Q_PROPERTY(bool selectNodeUponCreation READ selectNodeUponCreation WRITE setSelectNodeUponCreation)
  /// This property controls the name that is displayed for the None item.
  /// "None" by default.
  /// \sa noneEnabled
  Q_PROPERTY(QString noneDisplay READ noneDisplay WRITE setNoneDisplay)

  Q_PROPERTY(QComboBox::SizeAdjustPolicy sizeAdjustPolicy READ sizeAdjustPolicy WRITE setSizeAdjustPolicy)

public:
  typedef QWidget Superclass;

  /// Construct an empty qMRMLNodeComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qMRMLNodeComboBox(QWidget* parent = 0);
  virtual ~qMRMLNodeComboBox();

  /// Get MRML scene that has been set by setMRMLScene(), there is no scene
  /// by default (0).
  /// \sa setMRMLScene
  Q_INVOKABLE vtkMRMLScene* mrmlScene()const;

  /// Set/Get node types to display in the list
  /// NodeTypes are the class names, i.e. vtkMRMLViewNode,
  /// vtkMRMLTransformNode
  QStringList nodeTypes()const;
  void setNodeTypes(const QStringList& nodeTypes);

  /// If a vtkMRMLNode has the property HideFromEditors set to true,
  /// bypass the property and show the node anyway.
  inline void setShowHidden(bool);
  inline bool showHidden()const;

  /// This property controls whether subclasses of \a nodeType
  /// are hidden or not. If false, only the nodes of 'final' type
  /// is \a nodeType are displayed, if true, all the nodes deriving
  /// from \a nodeType are visible except for the ones of
  /// type \a hideChildNodeTypes.
  /// true by default.
  inline void setShowChildNodeTypes(bool show);
  inline bool showChildNodeTypes()const;

  /// If a node is a nodeType, hide the node if it is also
  /// a ExcludedChildNodeType. (this can happen if nodeType is a
  /// mother class of ExcludedChildNodeType)
  inline void setHideChildNodeTypes(const QStringList& nodeTypes);
  inline QStringList hideChildNodeTypes()const;

  /// Add node type attribute that filter the nodes to
  /// display. For example, colormap categories are defined with the "Category"
  /// attribute. In the following, the combobox only display Discrete colormap
  /// nodes.
  /// \code addAttribute("vtkMRMLColorNode", "Category", "Discrete"); \endcode
  /// \note The attributes are used for filtering but also when "AddNode" is
  /// called: the attributes will be set to the new node
  /// \note An undefined attributeValue will match any value as long as the node
  /// has the attribute defined. An empty string will only match an empty
  /// string. Otherwise the attributeValue has to match the node's value exactly.
  Q_INVOKABLE void addAttribute(const QString& nodeType,
                                const QString& attributeName,
                                const QVariant& attributeValue = QVariant());
  /// Remove node type attribute filtering the displayed nodes
  /// \sa addAttribute
  Q_INVOKABLE void removeAttribute(const QString& nodeType,
                                const QString& attributeName);

  /// BaseName is the name used to generate a node name for all the new created
  /// nodes.
  /// If nodeType is not specified for setBaseName() then base name is set for all already defined node types.
  /// If nodeType is not specified for baseName() then base name of the first node type is returned.
  void setBaseName(const QString& baseName, const QString& nodeType = "");
  QString baseName(const QString& nodeType = "")const;

  /// NodeTypeLabel is the name displayed to the user as node type. By default the node's tag is used.
  /// Configuration is useful for cases when a more specific type name is preferred (e.g., instead of
  /// the generic "Create new SubjectHierarchy" option, a module can set up the widget to show
  /// "Create new Measurements"). If label is set to empty then the default label is used.
  Q_INVOKABLE void setNodeTypeLabel(const QString& label, const QString& nodeType);
  Q_INVOKABLE QString nodeTypeLabel(const QString& nodeType)const;

  /// return the number of nodes. it can be different from count()
  /// as count includes the "AddNode", "Remove Node"... items
  int nodeCount()const;

  /// return the vtkMRMLNode* at the corresponding index. 0 if the index is
  /// invalid
  /// \sa nodeCount(), setCurrentNode(int)
  vtkMRMLNode* nodeFromIndex(int index)const;

  /// Return the currently selected node. 0 if no node is selected
  Q_INVOKABLE vtkMRMLNode* currentNode()const;

  /// Return the currently selected node id . "" if no node is selected
  /// Utility function that is based on currentNode
  QString currentNodeID()const;

  /// \deprecated
  /// Use currentNodeID instead
  Q_INVOKABLE QString currentNodeId()const;

  /// if true, when the user create a node using "Add node", the node will be
  /// automatically selected. It doesn't apply if the node is programatically
  /// added (when the combobox is populated by the scene).
  bool selectNodeUponCreation()const;
  void setSelectNodeUponCreation(bool value);

  /// Return true if the "none" is in the comboBox list, false otherwise.
  /// \sa noneEnabled, setNoneEnabled()
  bool noneEnabled()const;
  /// Set whether the "none" item should be in the comboBox list or not.
  /// \sa noneEnabled, noneEnabled()
  void setNoneEnabled(bool enable);

  /// Allow the user to create a new node. An "Add node" item is added into the
  /// menu list.
  bool addEnabled()const;
  void setAddEnabled(bool enable);

  /// Allow the user to delete the currently selected node. A "Remove node" item
  /// is added to the menu list.
  bool removeEnabled()const;
  void setRemoveEnabled(bool enable);

  /// TODO
  /// Allow the user to modify the properties of the currently selected.
  bool editEnabled()const;
  void setEditEnabled(bool enable);

  /// Allow the user to rename the node.
  bool renameEnabled()const;
  void setRenameEnabled(bool enable);

  /// Return the name of the "none" item
  /// \sa noneDisplay, setNoneDisplay()
  QString noneDisplay()const;

  /// Set the name of the "none" item.
  /// \sa noneDisplay, noneDisplay()
  void setNoneDisplay(const QString& displayName);

  /// Return a list of all the nodes that are displayed in the combo box.
  QList<vtkMRMLNode*> nodes()const;

  /// Internal model associated to the combobox.
  /// It is usually not the scene model but the sort filter proxy model.
  /// \sa sortFilterProxyModel(), sceneModel()
  QAbstractItemModel* model()const;

  /// Retrieve the sortFilterProxyModel used to filter/sort
  /// the nodes.
  /// \sa sceneModel()
  Q_INVOKABLE qMRMLSortFilterProxyModel* sortFilterProxyModel()const;

  /// Retrieve the scene model internally used.
  /// The scene model is usually not used directly, but a sortFilterProxyModel
  /// is plugged in.
  /// \sa sortFilterProxyModel()
  qMRMLSceneModel* sceneModel()const;

  /// Return the node factory used to create nodes when "Add Node"
  /// is selected (property \a AddEnabled should be true).
  /// A typical use would be to connect the node factory signal
  /// nodeInitialized(vtkMRMLNode*) with your own initialization routine
  /// slot:
  /// connect(nodeComboBox->nodeFactory(), SIGNAL(nodeInitialized(vtkMRMLNode*)),
  ///         this, SLOT(initializeNode(vtkMRMLNode*)));
  qMRMLNodeFactory* nodeFactory()const;

  /// \sa QComboBox::sizeAdjustPolicy
  QComboBox::SizeAdjustPolicy sizeAdjustPolicy()const;
  void setSizeAdjustPolicy(QComboBox::SizeAdjustPolicy policy);

  /// Allow addition of menu actions in addition to the default actions that
  /// can be flagged on or off via \a setAddEnabled() etc.
  /// New actions are saved to the UserMenuActions list, and are added to the
  /// extra items list in \a updateActionItems(bool resetRootIndex) and set as
  /// post items on the scene model. The new actions are checked for and
  /// triggered in \a activateExtraItem(const QModelIndex& index)
  /// Checks for action text duplicates and doesn't add them.
  /// Also checks for action text that will be hidden by the default action
  /// texts and doesn't add it.
  Q_INVOKABLE virtual void addMenuAction(QAction *newAction);

public slots:
  /// Set the scene the combobox listens to. The scene is observed and when new
  /// nodes are added to the scene, the menu list is populated.
  virtual void setMRMLScene(vtkMRMLScene* scene);

  /// Select the node to be current
  void setCurrentNode(vtkMRMLNode* node);

  /// \deprecated
  /// Use setCurrentNodeID instead
  void setCurrentNode(const QString& nodeID);

  /// Select the node to be current. If \a nodeID is invalid (or can't be found
  /// in the scene), the current node becomes 0.
  void setCurrentNodeID(const QString& nodeID);

  /// Select the current node by index. The index refers to the order of the nodes
  /// into the list. If \a index is 0, the first node will be selected (even if
  /// "NoneEnabled" is true).
  /// \sa nodeCount, setCurrentNode(vtkMRMLNode* ), setCurrentNodeID(const QString&)
  void setCurrentNodeIndex(int index);

  /// \brief Creates a node of the same type as in the "node types" property.
  ///
  /// Its name is generated using \a basename.
  ///
  /// \return The new node or NULL if \a nodeType is not among the allowed
  /// node types specified using setNodeTypes().
  ///
  /// \sa nodeTypes()
  /// \sa baseName()
  virtual vtkMRMLNode* addNode(QString nodeType);

  /// \brief Creates a node of the same type as the first in the "node types" property.
  ///
  /// Its name is generated using \a basename.
  ///
  /// \sa nodeTypes()
  /// \sa baseName()
  virtual vtkMRMLNode* addNode();

  /// Removes the current node from the scene. The node reference count gets
  /// decremented which might lead to deletion (if it reaches 0).
  virtual void removeCurrentNode();

  /// Edits the currently selected node.
  virtual void editCurrentNode();

  /// Renames the currently selected node.
  /// It shows an input dialog box with the current name of the node
  /// \sa vtkMRMLNode::GetName(), vtkMRMLNode::SetName()
  virtual void renameCurrentNode();

signals:
  /// This signal is sent anytime the current node is changed. NULL if
  /// no node is current or the current item is "None".
  void currentNodeChanged(vtkMRMLNode* node);

  /// This signal is sent anytime the current node is changed. "" if
  /// no node is current or the current item is "None".
  void currentNodeIDChanged(const QString& id);

  /// Advanced function.
  /// This signal is sent when the user chooses a node in the combobox.
  /// The item's node is passed. Note that this signal is sent even when the
  /// choice is not changed. If you need to know when the choice actually
  /// changes, use signal currentNodeChanged().
  /// \sa QComboBox::activated.
  void nodeActivated(vtkMRMLNode* node);

  /// Signal emitted just after currentNodeChanged(vtkMRMLNode*) is.
  /// \a validNode set to True when the current node is changed.
  /// Set to False when the list is empty. Useful to
  /// enable/disable/show/hide other widgets
  /// depending on the validity of the current node.
  void currentNodeChanged(bool validNode);

  /// TBD
  /// void nodeAboutToBeAdded(vtkMRMLNode*);

  /// Signal emitted when \a node is added to the comboBox
  /// Only nodes with valid type emit the signal
  void nodeAdded(vtkMRMLNode* node);

  /// Signal emitted when \a node is added by the user
  void nodeAddedByUser(vtkMRMLNode* node);

  /// Signal emitted when \a node is about to be removed from
  /// the comboBox. Only nodes with valid type emit the signal
  void nodeAboutToBeRemoved(vtkMRMLNode* node);

  /// TBD:
  /// void nodeRemoved(vtkMRMLNode*);

  /// Signal emitted when the current node is renamed
  void currentNodeRenamed(const QString& newName);

  /// Signal emitted when edit node menu action is selected
  void nodeAboutToBeEdited(vtkMRMLNode* node);

protected:
  /// qMRMLNodeComboBox will not take ownership on the model.
  qMRMLNodeComboBox(QAbstractItemModel* model, QWidget* parent = 0);
  qMRMLNodeComboBox(qMRMLNodeComboBoxPrivate* pimpl, QWidget* parent = 0);
  QAbstractItemModel* rootModel()const;

  void setComboBox(QComboBox* comboBox);

  /// Exposed internal combobox to tweak its behavior such as changing the
  /// QComboBox view or item delegate.
  QComboBox* comboBox()const;

  virtual void changeEvent(QEvent* event);

protected slots:
  void activateExtraItem(const QModelIndex& index);
  void emitCurrentNodeChanged();
  void emitNodeActivated(int currentIndex);
  void emitNodesAdded(const QModelIndex & parent, int start, int end);
  void emitNodesAboutToBeRemoved(const QModelIndex & parent, int start, int end);
  void refreshIfCurrentNodeHidden();

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
