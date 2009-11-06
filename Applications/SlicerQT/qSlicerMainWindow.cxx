#include "qSlicerMainWindow.h" 

#include "ui_qSlicerMainWindow.h" 

#include "qSlicerApplication.h"
#include "qSlicerMainWindowCore.h"

#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerMainWindow::qInternal : public Ui::qSlicerMainWindow
{
  qInternal()
    {
    this->Core = 0;
    }
  qSlicerMainWindowCore* Core; 
};

//-----------------------------------------------------------------------------
qSlicerMainWindow::qSlicerMainWindow(QWidget *parent)
 :Superclass(parent)
{
  this->Internal = new qInternal;
  this->Internal->setupUi(this);
  
  // Main window core helps to coordinate various widgets and panels
  this->Internal->Core = new qSlicerMainWindowCore(this);
  
  this->setupMenuActions(); 
}

//-----------------------------------------------------------------------------
qSlicerMainWindow::~qSlicerMainWindow()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
qSlicerMainWindowCore* qSlicerMainWindow::core()
{
  return this->Internal->Core;
}

//-----------------------------------------------------------------------------
void qSlicerMainWindow::setupMenuActions()
{
  this->connect(
    this->Internal->actionFileExit, SIGNAL(triggered()), 
    qSlicerApplication::instance(), SLOT(quit()));
}
