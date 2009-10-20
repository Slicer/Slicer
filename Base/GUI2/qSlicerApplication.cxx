#include "qSlicerApplication.h" 

#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFontInfo>
#include <QFontDatabase>

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
#ifdef Slicer3_USE_KWWidget
  vtkSlicerApplication* SlicerApplication;
#endif
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
  /*
  QFont f("Verdana", 9);
  QFontInfo ff(f);
  QFontDatabase database;
  foreach (QString family, database.families()) 
    {
    cout << family.toStdString() << endl;
    }

  cout << "Family: " << ff.family().toStdString() << endl;
  cout << "Size: " << ff.pointSize() << endl;
  this->setFont(f);
  */
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

#ifdef Slicer3_USE_KWWidget
void qSlicerApplication::setSlicerApplication(vtkSlicerApplication* app)
{
  this->Internal->SlicerApplication = app;
}

vtkSlicerApplication* qSlicerApplication::getSlicerApplication()const
{
  return this->Internal->SlicerApplication;
}
#endif
