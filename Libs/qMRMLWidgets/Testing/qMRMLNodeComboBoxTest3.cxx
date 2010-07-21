// QT includes
#include <QApplication>
#include <QTimer>

// qMRML includes
#include "qMRMLNodeComboBox.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include "vtkSmartPointer.h"

// STD includes
#include <stdlib.h>
#include <iostream>

int qMRMLNodeComboBoxTest3( int argc, char * argv [] )
{
  if (argc < 2)
    {
    std::cerr<< "Wrong number of arguments." << std::endl;
    return EXIT_FAILURE;
    }
  QApplication app(argc, argv);

  qMRMLNodeComboBox nodeSelector;
  nodeSelector.show();
  nodeSelector.setNodeTypes(QStringList("vtkMRMLViewNode"));
  vtkSmartPointer<vtkMRMLScene> scene =  vtkSmartPointer<vtkMRMLScene>::New();
  nodeSelector.setMRMLScene(scene);
  scene->SetURL(argv[1]);
  scene->Connect();

  QTimer autoExit;
  if (argc < 3 || QString(argv[]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }

  return app.exec();
}
