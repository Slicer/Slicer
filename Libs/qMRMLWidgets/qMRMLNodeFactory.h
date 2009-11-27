#ifndef __qMRMLNodeFactory_h
#define __qMRMLNodeFactory_h

// QT includes
#include <QObject>

#include "qMRMLWidgetsWin32Header.h"
 
class vtkMRMLNode;
class vtkMRMLScene; 

class QMRML_WIDGETS_EXPORT qMRMLNodeFactory : public QObject
{
  
public:
  
  // Self/Superclass typedef
  typedef qMRMLNodeFactory  Self;
  typedef QObject           Superclass;
  
  // Constructors
  qMRMLNodeFactory(QObject* parent);
  virtual ~qMRMLNodeFactory();
  
  // Description:
  // Set/Get MRML scene
  void setMRMLScene(vtkMRMLScene* mrmlScene);
  vtkMRMLScene* mrmlScene();

  // Description:
  // Create and add a node given its classname to the scene associated with the factory
  // Note: The scene has the ownership of the node and is responsible to delete it.
  vtkMRMLNode* createNode(const char* className);
  vtkMRMLNode* createNode(const QString& className);

private:
  struct qInternal; 
  qInternal* Internal; 
}; 

#endif
