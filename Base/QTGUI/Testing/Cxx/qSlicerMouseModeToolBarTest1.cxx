// Qt includes
//#include <QApplication>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerMouseModeToolBar.h"

// MRML includes
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>

// Logic includes
#include <vtkSlicerApplicationLogic.h>

// STD includes

int qSlicerMouseModeToolBarTest1(int argc, char * argv[] )
{
  //QApplication app(argc, argv);
  qSlicerApplication app(argc, argv);
  qSlicerMouseModeToolBar mouseToolBar;

  // set the scene without the app logic
  vtkMRMLScene* scene = vtkMRMLScene::New();
  mouseToolBar.setMRMLScene(scene);

  // now reset it to null and set with app logic
  vtkSlicerApplicationLogic *appLogic = vtkSlicerApplicationLogic::New();
  mouseToolBar.setMRMLScene(NULL);
  appLogic->SetMRMLScene(scene);
  mouseToolBar.setApplicationLogic(appLogic);
  mouseToolBar.setMRMLScene(scene);

  std::cout << "Done set up, starting testing..." << std::endl;

  // exercise public slots
  mouseToolBar.switchToViewTransformMode();
  mouseToolBar.switchPlaceMode();
  mouseToolBar.setPersistence(true);
  // without a qSlicerApplication, setting the cursor is a noop
  mouseToolBar.changeCursorTo(QCursor(Qt::BusyCursor));

  QString activeActionText;
  activeActionText = mouseToolBar.activeActionText();
  std::cout << "Active action text = " << qPrintable(activeActionText) << std::endl;

  // get the selection and interaction nodes that the mouse mode tool bar
  // listens to
  vtkMRMLNode *mrmlNode;
  vtkMRMLSelectionNode *selectionNode = NULL;
  //QString activeActionText;
  mrmlNode = scene->GetNodeByID("vtkMRMLSelectionNodeSingleton");
  if (mrmlNode)
    {
    selectionNode = vtkMRMLSelectionNode::SafeDownCast(mrmlNode);
    }
  if (selectionNode)
    {
    std::cout << "Got selection node" << std::endl;
    // add the new annotation types to it
    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationFiducialNode", ":/Icons/AnnotationPointWithArrow.png", "Fiducial");
    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLAnnotationRulerNode", ":/Icons/AnnotationDistanceWithArrow.png", "Ruler");
    selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsFiducialNode", ":/Icons/MarkupsMouseModePlace.png");

    selectionNode->SetReferenceActivePlaceNodeClassName("vtkMRMLAnnotationFiducialNode");
    activeActionText = mouseToolBar.activeActionText();
    std::cout << "After setting selection node activeplace node class name to "
              << selectionNode->GetActivePlaceNodeClassName()
              << ", mouse tool bar active action text = "
              << qPrintable(activeActionText) << std::endl;
    if (activeActionText.compare(QString("Fiducial")) != 0)
      {
      std::cerr << "Error! Expected active action text of 'Fiducial', got '"
                << qPrintable(activeActionText) << "'" << std::endl;
      return EXIT_FAILURE;
      }

    selectionNode->SetReferenceActivePlaceNodeClassName("vtkMRMLAnnotationRulerNode");
    activeActionText = mouseToolBar.activeActionText();
    std::cout << "After setting selection node active place node class name to "
              << selectionNode->GetActivePlaceNodeClassName()
              << ", mouse tool bar active action text = "
              << qPrintable(activeActionText) << std::endl;
    if (activeActionText.compare(QString("Ruler")) != 0)
        {
        std::cerr << "Error! Expected active action text of 'Ruler', got '"
                  << qPrintable(activeActionText) << "'" << std::endl;
        return EXIT_FAILURE;
        }

    // test with no action text
    selectionNode->SetReferenceActivePlaceNodeClassName("vtkMRMLMarkupsFiducialNode");
    activeActionText = mouseToolBar.activeActionText();
    std::cout << "After setting selection node active place node class name to "
              << selectionNode->GetActivePlaceNodeClassName()
              << ", mouse tool bar active action text = '"
              << qPrintable(activeActionText) << "'" << std::endl;
    if (activeActionText.compare(QString("")) != 0)
      {
      std::cerr << "Error! Expected active action text of '', got '"
                << qPrintable(activeActionText) << "'" << std::endl;
      return EXIT_FAILURE;
      }
    }

  vtkMRMLInteractionNode *interactionNode = NULL;
  mrmlNode = scene->GetNodeByID("vtkMRMLInteractionNodeSingleton");
  if (mrmlNode)
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(mrmlNode);
    }
  if (interactionNode)
    {
    std::cout << "Got interaction node" << std::endl;
    interactionNode->SetPlaceModePersistence(1);
    interactionNode->SetPlaceModePersistence(0);
    interactionNode->SwitchToSinglePlaceMode();
    if (selectionNode)
      {
      selectionNode->SetReferenceActivePlaceNodeClassName("vtkMRMLAnnotationFiducialNode");
      activeActionText = mouseToolBar.activeActionText();
      std::cout << "After setting selection node active place node class name to "
                << selectionNode->GetActivePlaceNodeClassName()
                << ", mouse tool bar active action text = "
                << qPrintable(activeActionText) << std::endl;
      if (activeActionText.compare(QString("Fiducial")) != 0)
        {
        std::cerr << "Error! Expected active action text of 'Fiducial', got '"
                  << qPrintable(activeActionText) << "'" << std::endl;
        return EXIT_FAILURE;
        }
      }
    interactionNode->SwitchToViewTransformMode();
    activeActionText = mouseToolBar.activeActionText();
    std::cout << "After switching interaction node to view transform, active action text = " << qPrintable(activeActionText) << std::endl;
    // after a change in the tool bar (removed the Rotate action), this should still
    // be fiducial.
    if (activeActionText.compare(QString("Fiducial")) != 0)
      {
      std::cerr << "Error! Expected active action text of 'Fiducial', got '" << qPrintable(activeActionText) << "'" << std::endl;
      return EXIT_FAILURE;
      }
    }


  // clean up
  appLogic->Delete();
  scene->Delete();


  return EXIT_SUCCESS;
}

