#ifndef __qMRMLSceneGeneratorButton_h
#define __qMRMLSceneGeneratorButton_h

/// qCTK includes
#include <qCTKPimpl.h>

/// QT includes
#include <QPushButton>
#include <QString>

#include "qMRMLWidgetsExport.h"

class vtkMRMLScene;
class qMRMLSceneGeneratorButtonPrivate;

class QMRML_WIDGETS_EXPORT qMRMLSceneGeneratorButton : public QPushButton
{
  Q_OBJECT
public:
  qMRMLSceneGeneratorButton(QWidget* parent = 0);
  ~qMRMLSceneGeneratorButton();
  
  void clear();
  
  vtkMRMLScene* mrmlScene()const;
  
public slots:
  void generateScene();
  void generateEmptyScene();

signals:
  void mrmlSceneSet(vtkMRMLScene* scene);

private:
  QCTK_DECLARE_PRIVATE(qMRMLSceneGeneratorButton);
};

#endif
