
// Qt includes
#include <QApplication>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QTreeView>

// Slicer includes
#include "vtkSlicerConfigure.h"

// CTK includes
#include <ctkModelTester.h>

// qMRML includes
#include "qMRMLSceneFactoryWidget.h"
#include "qMRMLSceneDisplayableModel.h"
#include "qMRMLUtils.h"

// VTK includes
#include "vtkMRMLCoreTestingMacros.h"
#include <vtkEventBroker.h>
#include "qMRMLWidget.h"

#include "GUI/qMRMLAnnotationTreeView.h"
#include "Logic/vtkSlicerAnnotationModuleLogic.h"
#include "MRML/vtkMRMLAnnotationRulerNode.h"


// STD includes
#include <cstdlib>
#include <iostream>

// MRML includes
#include <vtkMRMLDisplayableHierarchyNode.h>

int qMRMLSceneAnnotationModelAndAnnotationTreeViewTest1(int argc, char * argv [])
{
  qMRMLWidget::preInitializeApplication();
  QApplication app(argc, argv);
  qMRMLWidget::postInitializeApplication();

  qMRMLSceneFactoryWidget sceneFactory(0);

  sceneFactory.generateScene();

  qMRMLAnnotationTreeView* view = new qMRMLAnnotationTreeView(0);

  //view->setSelectionBehavior(QAbstractItemView::SelectRows);

  vtkSlicerAnnotationModuleLogic* logic = vtkSlicerAnnotationModuleLogic::New();

  logic->SetMRMLScene(sceneFactory.mrmlScene());

  view->setAndObserveLogic(logic);
  view->setMRMLScene(sceneFactory.mrmlScene());
  view->hideScene();

  view->show();
  view->resize(500, 500);

  QTreeView view3;
  view3.setModel(view->sceneModel());
  view3.show();
/*
  qMRMLTreeView view2;
  view2.setSceneModelType("Displayable");
  view2.sceneModel()->setMRMLScene(sceneFactory.mrmlScene());
  view2.show();

  QTreeView view3;
  view3.setModel(view->sceneModel());
  view3.show();
*/
  double worldCoordinates1[3] = {0,0,0};
  double worldCoordinates2[3] = {50,50,50};

  double distance = sqrt(vtkMath::Distance2BetweenPoints(worldCoordinates1,worldCoordinates2));

  vtkMRMLAnnotationRulerNode *rulerNode = vtkMRMLAnnotationRulerNode::New();

  rulerNode->SetPosition1(worldCoordinates1);
  rulerNode->SetPosition2(worldCoordinates2);
  rulerNode->SetDistanceMeasurement(distance);

  rulerNode->SetName(sceneFactory.mrmlScene()->GetUniqueNameByString("AnnotationRuler"));

  rulerNode->Initialize(sceneFactory.mrmlScene());

  rulerNode->Delete();

  std::cout << "Measurement in rulerNode: " << rulerNode->GetDistanceMeasurement() << std::endl;
/*
  QModelIndex index = view->d_func()->SceneModel->indexFromNode(sceneFactory.mrmlScene()->GetFirstNodeByClass("vtkMRMLAnnotationRulerNode"));

  qMRMLAbstractItemHelper* helper = view->d_func()->SceneModel->itemFromIndex(index);
  std::cout << helper->data(Qt::DisplayRole) << std::endl;
*/

  if (argc < 2 || QString(argv[1]) != "-I" )
    {
    QTimer::singleShot(200, &app, SLOT(quit()));
    }

  return app.exec();
}

