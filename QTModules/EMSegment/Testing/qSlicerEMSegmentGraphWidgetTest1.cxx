// QT includes
#include <QApplication>

// EMSegment includes
#include "vtkSlicerEMSegmentLogic.h"
#include "qSlicerEMSegmentGraphWidget.h"

// MRML includes
#include "vtkMRMLScene.h"

// STD includes
#include <iostream>

int qSlicerEMSegmentGraphWidgetTest1( int argc, char * argv [] )
{
  QApplication app(argc, argv);
  qSlicerEMSegmentGraphWidget graph;
  graph.show();

  if (argc < 2)
    {
    return EXIT_SUCCESS;
    }

  vtkSlicerEMSegmentLogic* logic = vtkSlicerEMSegmentLogic::New();
  vtkMRMLScene* scene =vtkMRMLScene::New();
  logic->SetMRMLScene(scene);
  scene->SetURL(argv[1]);
  scene->Import();

  graph.setMRMLManager(logic->GetMRMLManager());
  return EXIT_SUCCESS;
}
