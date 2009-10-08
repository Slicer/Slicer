#include "qSlicerApplication.h" 

#include "QPalette"
#include "QColor"

#include "vtkMRMLScene.h"

//-----------------------------------------------------------------------------
class qSlicerApplication::qInternal
{
public:
  qInternal()
    {
    this->MRMLScene = 0;
    }
  vtkMRMLScene * MRMLScene; 
};

//-----------------------------------------------------------------------------
qSlicerApplication::qSlicerApplication(int &argc, char **argv)
  : Superclass(argc, argv)
{
  this->Internal = new qInternal;
  QPalette p = this->palette();
  p.setColor(QPalette::Window, Qt::white);
  p.setColor(QPalette::Base, Qt::white);
  p.setColor(QPalette::AlternateBase, QColor("#e4e4fe"));
  p.setColor(QPalette::Button, Qt::white);
  this->setPalette(p);
}

//-----------------------------------------------------------------------------
qSlicerApplication::~qSlicerApplication()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
qSlicerApplication* qSlicerApplication::application()
{
  qSlicerApplication* app =
    qobject_cast<qSlicerApplication*>(QApplication::instance());
  return app;
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setMRMLScene(vtkMRMLScene * scene)
{
  this->Internal->MRMLScene = scene; 
}

//-----------------------------------------------------------------------------
vtkMRMLScene* qSlicerApplication::getMRMLScene()
{
  return this->Internal->MRMLScene; 
}
