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

int qMRMLNodeComboBoxTest4( int argc, char * argv [] )
{
  QApplication app(argc, argv);

  qMRMLNodeComboBox nodeSelector;
  nodeSelector.setNodeTypes(QStringList("vtkMRMLScalarVolumeNode"));
  nodeSelector.addAttribute("vtkMRMLScalarVolumeNode", "LabelMap", "1");

  vtkSmartPointer<vtkMRMLScene> scene =  vtkSmartPointer<vtkMRMLScene>::New();
  nodeSelector.setMRMLScene(scene);

  nodeSelector.addNode();
  if (nodeSelector.nodeCount() != 1)
    {
    std::cerr << "qMRMLNodeComboBox::addAttribute is broken" << std::endl;
    return EXIT_FAILURE;
    }

  nodeSelector.show();
  QTimer autoExit;
  if (argc < 2 || QString(argv[1]) != "-I")
    {
    QObject::connect(&autoExit, SIGNAL(timeout()), &app, SLOT(quit()));
    autoExit.start(1000);
    }

  return app.exec();
}
