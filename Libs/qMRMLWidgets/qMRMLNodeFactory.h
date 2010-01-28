#ifndef __qMRMLNodeFactory_h
#define __qMRMLNodeFactory_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>
#include <QHash>

#include "qMRMLWidgetsExport.h"
 
class vtkMRMLNode;
class vtkMRMLScene;
class qMRMLNodeFactoryPrivate;

class QMRML_WIDGETS_EXPORT qMRMLNodeFactory : public QObject
{
  Q_OBJECT
public:

  // Convenient typedef 
  typedef QHash<QString,QString> AttributeType;
  
  // Constructors
  typedef QObject Superclass;
  explicit qMRMLNodeFactory(QObject* parent = 0);
  virtual ~qMRMLNodeFactory(){}
  
  // Description:
  // Set/Get MRML scene
  vtkMRMLScene* mrmlScene()const;

  // Description:
  // Create and add a node given its classname to the scene associated with the factory
  // Note: The scene has the ownership of the node and is responsible to delete it.
  vtkMRMLNode* createNode(const QString& className);

  // Description:
  // Convenient method allowing to create a new node and add it to the scene
  static vtkMRMLNode* createNode(vtkMRMLScene* scene, const QString& className,
    const AttributeType& attributes = AttributeType());

  // Description:
  // Add attribute
  // Note: If an attribute already exist, it's value will be overwritten.
  void addAttribute(const QString& name, const QString& value);
public slots:
  // Description:
  // Set/Get MRML scene
  void setMRMLScene(vtkMRMLScene* mrmlScene);
  
private:
  QCTK_DECLARE_PRIVATE(qMRMLNodeFactory);
}; 

#endif
