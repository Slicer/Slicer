#ifndef __qMRMLSceneFactoryWidget_h
#define __qMRMLSceneFactoryWidget_h

// Qt includes
#include <QPushButton>
#include <QString>

// CTK includes
#include <ctkPimpl.h>

#include "qMRMLWidgetsExport.h"

class vtkMRMLScene;
class vtkMRMLNode;
class qMRMLSceneFactoryWidgetPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSceneFactoryWidget : public QWidget
{
  Q_OBJECT
public:
  qMRMLSceneFactoryWidget(QWidget* parent = 0);
  ~qMRMLSceneFactoryWidget();
  
  vtkMRMLScene* mrmlScene()const;
  
public slots:
  void generateScene();
  void deleteScene();
  /// 
  /// Create and add a node given its classname to the scene associated with the factory
  /// Note: The scene has the ownership of the node and is responsible to delete it.
  vtkMRMLNode* generateNode(const QString& mrmlNodeClassName);
  void deleteNode(const QString& mrmlNodeID);

  vtkMRMLNode* generateNode();
  void deleteNode();
  
signals:
  void mrmlSceneChanged(vtkMRMLScene* scene);
  void mrmlNodeAdded(vtkMRMLNode* node);
  void mrmlNodeRemoved(vtkMRMLNode* node);

private:
  CTK_DECLARE_PRIVATE(qMRMLSceneFactoryWidget);
};

#endif
