#ifndef __qMRMLNodeFactoryButton_h
#define __qMRMLNodeFactoryButton_h

/// qCTK includes
#include <qCTKPimpl.h>

/// qVTK includes
#include <qVTKObject.h>

/// QT includes
#include <QPushButton>

#include "qMRMLWidgetsExport.h"
 
class vtkMRMLNode;
class vtkMRMLScene;
class qMRMLNodeFactoryButtonPrivate;

class QMRML_WIDGETS_EXPORT qMRMLNodeFactoryButton : public QPushButton
{
  Q_OBJECT
  Q_PROPERTY(ActionType Action READ action WRITE setAction)
  Q_ENUMS(ActionType)
  QVTK_OBJECT
public:

  /// Constructors
  typedef QPushButton Superclass;
  explicit qMRMLNodeFactoryButton(QWidget* parent = 0);
  virtual ~qMRMLNodeFactoryButton(){}
  
  /// 
  /// Set/Get MRML scene
  vtkMRMLScene* mrmlScene()const;

  enum ActionType
  {
    GenerateRandom, 
    DeleteRandom
  };
  
  void setAction(ActionType action);
  ActionType action()const;

public slots:
  /// 
  /// Set/Get MRML scene
  void setMRMLScene(vtkMRMLScene* mrmlScene);

  /// 
  /// Create and add a node given its classname to the scene associated with the factory
  /// Note: The scene has the ownership of the node and is responsible to delete it.
  vtkMRMLNode* generateRandomNode();
  void deleteRandomNode();

protected slots:
  void onClick();
  void onMRMLSceneChanged();

private:
  CTK_DECLARE_PRIVATE(qMRMLNodeFactoryButton);
}; 

#endif
