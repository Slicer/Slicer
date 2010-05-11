#ifndef __qMRMLNodeFactory_h
#define __qMRMLNodeFactory_h

// Qt includes
#include <QObject>
#include <QHash>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"
 
class vtkMRMLNode;
class vtkMRMLScene;
class qMRMLNodeFactoryPrivate;

class QMRML_WIDGETS_EXPORT qMRMLNodeFactory : public QObject
{
  Q_OBJECT
public:

  /// Convenient typedef 
  typedef QHash<QString,QString> AttributeType;
  
  /// Constructors
  typedef QObject Superclass;
  explicit qMRMLNodeFactory(QObject* parent = 0);
  virtual ~qMRMLNodeFactory(){}
  
  /// 
  /// Set/Get MRML scene
  vtkMRMLScene* mrmlScene()const;

  /// 
  /// Create and add a node given its \a className to the scene associated with the factory
  /// Note: The scene has the ownership of the node and is responsible to delete it.
  vtkMRMLNode* createNode(const QString& className);

  /// 
  /// Convenient method allowing to create a new node and add it to the \a scene
  static vtkMRMLNode* createNode(vtkMRMLScene* scene, const QString& className,
    const AttributeType& attributes = AttributeType());

  /// 
  /// Add attribute
  /// Note: If an attribute already exist, it's value will be overwritten.
  void addAttribute(const QString& name, const QString& value);
public slots:
  /// 
  /// Set/Get MRML scene
  void setMRMLScene(vtkMRMLScene* mrmlScene);
  
private:
  CTK_DECLARE_PRIVATE(qMRMLNodeFactory);
}; 

#endif
