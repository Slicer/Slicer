#ifndef __qMRMLNodeComboBox_h
#define __qMRMLNodeComboBox_h

// Qt includes
#include <QComboBox>

/// qCTKWidgets includes
#include <qCTKPimpl.h>

// qMRMLWidgets includes
#include "qMRMLWidgetsExport.h"
#include "qMRMLSortFilterProxyModel.h"

class vtkMRMLNode;
class vtkMRMLScene;

class qMRMLNodeFactory;
class qMRMLNodeComboBoxPrivate;
//class qMRMLSortFilterProxyModel;

class QMRML_WIDGETS_EXPORT qMRMLNodeComboBox : public QComboBox
{
  Q_OBJECT
  Q_PROPERTY(QStringList nodeTypes READ nodeTypes WRITE setNodeTypes) 
  Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden)
  Q_PROPERTY(bool showChildNodeTypes READ showChildNodeTypes WRITE setShowChildNodeTypes)
  Q_PROPERTY(QStringList hideChildNodeTypes READ hideChildNodeTypes WRITE setHideChildNodeTypes)
  Q_PROPERTY(bool selectNodeUponCreation READ selectNodeUponCreation WRITE setSelectNodeUponCreation)

public:
  /// Superclass typedef
  typedef QComboBox Superclass;
  
  /// Constructors
  explicit qMRMLNodeComboBox(QWidget* parent = 0);
  virtual ~qMRMLNodeComboBox(){}

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
  inline QStringList nodeTypes()const;
  inline void setNodeTypes(const QStringList& nodeTypes);
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
  /// Return the currently selected node. 0 if no node is selected
  vtkMRMLNode* currentNode()const;

  /// 
  /// Set/Get SelectNodeUponCreation flags
  bool selectNodeUponCreation()const;
  void setSelectNodeUponCreation(bool value);

public slots:
  /// 
  /// Set the scene the NodeSelector listens to.
  void setMRMLScene(vtkMRMLScene* scene);
  
  ///
  /// Select the node to be current
  void setCurrentNode(vtkMRMLNode* node);

  /// 
  /// Create a node of the same type than on the "node types" properties
  virtual void addNode();

  /// 
  /// Remove the current node from the scene
  virtual void removeCurrentNode();

  /// 
  /// Edit the current node
  virtual void editCurrentNode();

protected:
  virtual QAbstractItemModel* createSceneModel();

protected slots:
  void activateExtraItem(const QModelIndex& index);

private:
  QCTK_DECLARE_PRIVATE(qMRMLNodeComboBox);
};

// --------------------------------------------------------------------------
QStringList qMRMLNodeComboBox::nodeTypes()const
{
  return this->sortFilterProxyModel()->nodeTypes();
}

// --------------------------------------------------------------------------
void qMRMLNodeComboBox::setNodeTypes(const QStringList& _nodeTypes)
{
  this->sortFilterProxyModel()->setNodeTypes(_nodeTypes);
}

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
