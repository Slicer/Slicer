#ifndef __qMRMLNodeSelector_h
#define __qMRMLNodeSelector_h

/// qVTK includes
#include <qVTKObject.h>

/// qCTK includes
#include <qCTKAddRemoveComboBox.h>
#include <qCTKPimpl.h>

/// QT includes
#include <QString>

#include "qMRMLWidgetsExport.h"

class qMRMLNodeFactory; 
class vtkMRMLScene; 
class vtkMRMLNode;
class qMRMLNodeSelectorPrivate;

class QMRML_WIDGETS_EXPORT qMRMLNodeSelector : public qCTKAddRemoveComboBox
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(QString NodeType READ nodeType WRITE setNodeType)
  Q_PROPERTY(bool ShowHidden READ showHidden WRITE setShowHidden)
  Q_PROPERTY(bool SelectNodeUponCreation READ selectNodeUponCreation
                                         WRITE setSelectNodeUponCreation)
  
public:
  /// Superclass typedef
  typedef qCTKAddRemoveComboBox Superclass;
  
  /// Constructors
  explicit qMRMLNodeSelector(QWidget* parent = 0);
  virtual ~qMRMLNodeSelector(){}
  
  /// 
  /// Set/Get node type 
  QString nodeType()const ;
  void setNodeType(const QString& nodeType);

  /// 
  /// If a vtkMRMLNode has the property HideFromEditors set to true,
  /// bypass the property and show the node anyway.
  void setShowHidden(bool);
  bool showHidden()const;
  
  /// 
  /// Set/Get MRML scene
  vtkMRMLScene* mrmlScene()const;

  /// 
  /// Return the node currently selected
  vtkMRMLNode* currentNode()const;

  /// 
  /// Add a node in the combobox
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
  const QString currentNodeId() const; 

public slots:
  /// 
  /// Set the scene the NodeSelector listens to.
  void setMRMLScene(vtkMRMLScene* scene);
  
  /// 
  /// Set the selected node.
  void setCurrentNode(vtkMRMLNode* node);

signals:
  /// 
  /// emit the current displayed node. NULL if
  /// the list is empty.
  void currentNodeChanged(vtkMRMLNode* node);

  /// Descritpion:
  /// Utility function emitted at the same time(right after)
  /// then currentNodeChanged(vtkMRMLNode*) signal is emitted
  /// emit true when the current node is changed.
  /// false when the list is empty. Useful to 
  /// enable/disable/show/hide other widgets
  /// depending on the validity of the current node.
  void currentNodeChanged(bool);

  /// 
  /// Emit when a node has been added to the list
  void nodeAdded(vtkMRMLNode* node);

  /// 
  /// Emit when a node is about to be removed from a scene.
  /// The node can still be found in the mrml scene.
  void nodeAboutToBeRemoved(vtkMRMLNode* node);

  /// 
  /// Emit when a node has been removed from the scene and
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
  void onMRMLSceneNodeRemoved(vtkObject * scene, vtkObject * node); 
  void onMRMLNodeModified(vtkObject * node);
  
  void onCurrentIndexChanged(int index);
  void onItemAdded(int index);
  void onItemAboutToBeRemoved(int index);
  void onItemRemoved(int index);
  
protected:
  virtual void addNodeInternal(vtkMRMLNode* mrmlNode);
  virtual void nodeIdSelected(int index);
  vtkMRMLNode* node(int index)const;
  vtkMRMLNode* node(const QString& id)const;

private:
  QCTK_DECLARE_PRIVATE(qMRMLNodeSelector);
};

#endif
