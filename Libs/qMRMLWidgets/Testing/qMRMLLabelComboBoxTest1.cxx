// QT includes
#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QTimer>

// qMRML includes
#include "qMRMLLabelComboBox.h"
//#include "qMRMLSceneFactoryWidget.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLdGEMRICProceduralColorNode.h"
//#include "vtkMRMLFreeSurferProceduralColorNode.h"


// VTK includes
#include <vtkSmartPointer.h>

// STD includes
//#include <stdlib.h>
//#include <iostream>

int qMRMLLabelComboBoxTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qMRMLLabelComboBox colorViewer1,colorViewer2;
  
  //test 1 
  colorViewer1.setDisplayOption(qMRMLLabelComboBox::WithNone);
  vtkSmartPointer<vtkMRMLColorTableNode> newNode1 =
    vtkSmartPointer<vtkMRMLColorTableNode>::New();
  newNode1->SetType(vtkMRMLColorTableNode::Labels);
  
  colorViewer1.setMRMLColorNode(newNode1);
  colorViewer1.setCurrentColor(20);
  colorViewer1.setCurrentColor(-1);

  //test 2
  vtkSmartPointer<vtkMRMLColorTableNode> newNode2 =
    vtkSmartPointer<vtkMRMLColorTableNode>::New();
  newNode2->SetType(vtkMRMLColorTableNode::Labels);

  colorViewer2.setMRMLColorNode(newNode2);
  colorViewer2.setCurrentColor(-2);
  colorViewer2.setCurrentColor(600);

  colorViewer1.show();
  colorViewer2.show();
  
  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }

  return app.exec();
}
