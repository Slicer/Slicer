#ifndef __qMRMLNodeFactory_h
#define __qMRMLNodeFactory_h

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QObject>

#include "qMRMLWidgetsExport.h"
 
class vtkMRMLNode;
class vtkMRMLScene;
class qMRMLNodeFactoryPrivate;

class QMRML_WIDGETS_EXPORT qMRMLNodeFactory : public QObject
{
  
public:
  
  // Self/Superclass typedef
  typedef qMRMLNodeFactory  Self;
  typedef QObject           Superclass;
  
  // Constructors
  explicit qMRMLNodeFactory(QObject* parent);
  virtual ~qMRMLNodeFactory(){}
  
  // Description:
  // Set/Get MRML scene
  void setMRMLScene(vtkMRMLScene* mrmlScene);
  vtkMRMLScene* mrmlScene()const;

  // Description:
  // Create and add a node given its classname to the scene associated with the factory
  // Note: The scene has the ownership of the node and is responsible to delete it.
  vtkMRMLNode* createNode(const char* className);
  vtkMRMLNode* createNode(const QString& className);

  // Description:
  // Add attribute
  // Note: If an attribute already exist, it's value will be overwritten.
  void addAttribute(const QString& name, const QString& value);

private:
  QCTK_DECLARE_PRIVATE(qMRMLNodeFactory);
}; 

#endif
