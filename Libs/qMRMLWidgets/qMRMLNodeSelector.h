#ifndef __qMRMLNodeSelector_h
#define __qMRMLNodeSelector_h

// Qt includes
#include <QString>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>
#include <ctkAddRemoveComboBox.h>

#include "qMRMLWidgetsExport.h"

class qMRMLNodeFactory; 
class vtkMRMLScene; 
class vtkMRMLNode;
class qMRMLNodeSelectorPrivate;

class QMRML_WIDGETS_EXPORT qMRMLNodeSelector : public ctkAddRemoveComboBox
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(QStringList nodeTypes READ nodeTypes WRITE setNodeTypes)
  Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden)
  Q_PROPERTY(bool selectNodeUponCreation READ selectNodeUponCreation
                                         WRITE setSelectNodeUponCreation)
  Q_PROPERTY(bool showChildNodeTypes READ showChildNodeTypes WRITE setShowChildNodeTypes)
  Q_PROPERTY(QStringList hideChildNodeTypes READ hideChildNodeTypes WRITE setHideChildNodeTypes)

public:
  /// Superclass typedef
  typedef ctkAddRemoveComboBox Superclass;
  
  /// Constructors
  explicit qMRMLNodeSelector(QWidget* parent = 0);
  virtual ~qMRMLNodeSelector(){}
  virtual void printAdditionalInfo();
  
  /// 
  /// Set/Get node types to display in the list
  /// NodeTypes are the class names, i.e. vtkMRMLViewNode, 
  /// vtkMRMLTransformNode
  QStringList nodeTypes()const ;
  void setNodeTypes(const QStringList& nodeTypes);

  /// 
  /// If a vtkMRMLNode has the property HideFromEditors set to true,
  /// bypass the property and show the node anyway.
  void setShowHidden(bool);
  bool showHidden()const;
  
  ///
  /// Add node type attribute that filter the nodes to
  /// display
  void addAttribute(const QString& nodeType, 
                    const QString& attributeName,
                    const QVariant& attributeValue);


  ///
  /// Display or not the nodes that are excluded by
  /// the ExcludedChildNodeTypes list.
  /// true by default.
  void setShowChildNodeTypes(bool show);
  bool showChildNodeTypes()const;

  ///
  /// If a node is a nodeType, hide the node if it is also 
  /// a ExcludedChildNodeType. (this can happen if nodeType is a 
  /// mother class of ExcludedChildNodeType)
  void setHideChildNodeTypes(const QStringList& nodeTypes);
  QStringList hideChildNodeTypes()const;

  /// 
  /// Set/Get MRML scene
  vtkMRMLScene* mrmlScene()const;

  /// 
  /// Return the node currently selected
  vtkMRMLNode* currentNode()const;

  /// 
  /// Add a node in the combobox
  /// The node will be added if not filtered
  void addNode(vtkMRMLNode* node);

  /// 
  /// Set/Get MRML node factory
  void setMRMLNodeFactory(qMRMLNodeFactory* factory);
  qMRMLNodeFactory* factory()const;

  /// 
  /// Set/Get SelectNodeUponCreation flags
  bool selectNodeUponCreation()const;
  void setSelectNodeUponCreation(bool value);

  /// 
  /// Convenient method returning the current node id
  QString currentNodeId() const; 

public slots:
  /// 
  /// Set the scene the NodeSelector listens to.
  void setMRMLScene(vtkMRMLScene* scene);
  
  /// 
  /// Set the selected node.
  void setCurrentNode(vtkMRMLNode* node);

  ///
  /// Convenient function allowing to set the selected node using the Node Id
  inline void setCurrentNode(const QString& nodeId)
    { this->setCurrentIndex(this->findData(nodeId)); }

signals:
  /// 
  /// Emitted when the current displayed \a node changed. NULL if
  /// the list is empty.
  void currentNodeChanged(vtkMRMLNode* node);

  /// 
  /// Utility function emitted at the same time(right after)
  /// then currentNodeChanged(vtkMRMLNode*) signal is emitted
  /// emit true when the current node is changed.
  /// false when the list is empty. Useful to 
  /// enable/disable/show/hide other widgets
  /// depending on the validity of the current node.
  void currentNodeChanged(bool);

  /// 
  /// Emitted when \a node has been added to the list
  void nodeAdded(vtkMRMLNode* node);

  ///
  /// Signal emitted when \a node is added by the user
  void nodeAddedByUser(vtkMRMLNode* node);

  /// 
  /// Emitted when a node is about to be removed from a scene.
  /// The node can still be found in the mrml scene.
  void nodeAboutToBeRemoved(vtkMRMLNode* node);

  /// 
  /// Emitted when a node has been removed from the scene and
  /// the list. Warning, the node can't be found in the mrml
  /// scene anymore.
  void nodeRemoved(vtkMRMLNode* node);

protected slots:
  virtual void onAdd();
  virtual void onRemove();
  virtual void onEdit();
  
  /// 
  /// Triggered upon MRML scene updates
  void onMRMLSceneNodeAdded(vtkObject * scene, vtkObject * node); 
  void onMRMLSceneNodeAboutToBeRemoved(vtkObject * scene, vtkObject * node); 
  void onMRMLNodeModified(vtkObject * node);
  void onMRMLSceneDeleted(vtkObject * scene); 
  
  void onCurrentIndexChanged(int index);
  void onItemAdded(int index);
  void onItemAboutToBeRemoved(int index);
  void onItemRemoved(int index);
  
protected:
  virtual void addNodeInternal(vtkMRMLNode* mrmlNode);
  virtual void nodeIdSelected(int index);
  vtkMRMLNode* node(int index)const;
  vtkMRMLNode* node(const QString& id)const;

  void populateItems();
private:
  CTK_DECLARE_PRIVATE(qMRMLNodeSelector);
};

#endif
