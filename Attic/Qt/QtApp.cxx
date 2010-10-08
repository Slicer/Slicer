
//# ifdef Slicer_USE_PYTHON
//#   include "PythonQt.h"
//#   include "PythonQt_QtAll.h"
//# endif
#include <QApplication>
#include <QTextBrowser>
#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLineEdit>

#include "QtSlicerNodeSelectorWidget.h"
#include "QtApp.h"


QtApp::QtApp()
{
  
}

QtApp::~QtApp()
{
  
}
void QtApp::BuildGUI()
{

  // create a small Qt GUI
  QVBoxLayout*  vbox = new QVBoxLayout;
  QGroupBox*    box  = new QGroupBox;
  //QTextBrowser* browser = new QTextBrowser(box);
  QLineEdit*    edit = new QLineEdit(box);
  QPushButton*  button = new QPushButton(box);
  button->setObjectName("button1");
  edit->setObjectName("edit");
  //browser->setObjectName("browser");
  //vbox->addWidget(browser);
  vbox->addWidget(edit);
  vbox->addWidget(button);


  this->qNodeSelector = new QtSlicerNodeSelectorWidget();
  this->qNodeSelector->setObjectName("selector");
  this->qNodeSelector->SetMRMLScene(this->MRMLScene);
  this->qNodeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->qNodeSelector->SetNoneEnabled(1);
  this->qNodeSelector->SetNewNodeEnabled(1);
  vbox->addWidget(this->qNodeSelector);

  box->setLayout(vbox);

  this->qNodeSelector->connect(this->qNodeSelector, SIGNAL(NodeSelected(const QString&)), edit, SLOT(setText ( const QString &)));
  this->qNodeSelector->connect(this->qNodeSelector, SIGNAL(NodeSelected(const QString&)), button, SLOT(setText ( const QString &)));

  box->show();

}
