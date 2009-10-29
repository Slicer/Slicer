#include "qSlicerApplication.h" 

#include <QPalette>
#include <QColor>
#include <QFont>
#include <QFontInfo>
#include <QFontDatabase>
#include <QWidget>
#include <QMap>

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
  QMap<QWidget*,bool> TopLevelWidgetsSavedVisibilityState; 
};

//-----------------------------------------------------------------------------
qSlicerApplication::qSlicerApplication(int &argc, char **argv)
  : Superclass(argc, argv)
{
  this->Internal = new qInternal;
  this->initFont(); 
  this->initPalette(); 
  this->loadStyleSheet(); 
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
void qSlicerApplication::initPalette()
{
  QPalette p = this->palette();
  p.setColor(QPalette::Window, Qt::white);
  p.setColor(QPalette::Base, Qt::white);
  p.setColor(QPalette::AlternateBase, QColor("#e4e4fe"));
  p.setColor(QPalette::Button, Qt::white);
  this->setPalette(p);
}

//-----------------------------------------------------------------------------
void qSlicerApplication::initFont()
{
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
void qSlicerApplication::loadStyleSheet()
{
//   QString styleSheet = 
//     "background-color: white;"
//     "alternate-background-color: #e4e4fe;";
//   
//   this->setStyleSheet(styleSheet);
}

//-----------------------------------------------------------------------------
void qSlicerApplication::setTopLevelWidgetsVisible(bool visible)
{
  foreach(QWidget * widget, this->topLevelWidgets())
    {
    // Store current visibility state
    if (!visible)
      {
      this->Internal->TopLevelWidgetsSavedVisibilityState[widget] = widget->isVisible(); 
      widget->hide();
      }
    else
      {
      QMap<QWidget*,bool>::const_iterator it = 
        this->Internal->TopLevelWidgetsSavedVisibilityState.find(widget); 
        
      // If widget state was saved, restore it. Otherwise skip.
      if (it != this->Internal->TopLevelWidgetsSavedVisibilityState.end())
        {
        widget->setVisible(it.value());
        }
      }
    }
  
  // Each time widget are set visible. Internal Map can be cleared.
  if (visible)
    {
    this->Internal->TopLevelWidgetsSavedVisibilityState.clear();
    }
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
