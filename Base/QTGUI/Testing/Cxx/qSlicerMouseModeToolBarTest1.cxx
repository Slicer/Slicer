// Qt includes
//#include <QApplication>
//
// Slicer includes
#include "vtkSlicerConfigure.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerMouseModeToolBar.h"

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>

// Logic includes
#include <vtkSlicerApplicationLogic.h>

// VTK includes

// STD includes

#define CHECK_PLACE_ACTION_TEXT(expected, mouseToolBar) \
  { \
  QString activeActionText; \
  activeActionText = activePlaceActionText(mouseToolBar); \
  std::cout << "Line " << __LINE__ << " Active place action text = " << qPrintable(activeActionText) << std::endl; \
  if (activeActionText.compare(QString(expected)) != 0) \
    { \
    std::cerr << "Line " << __LINE__ << " Error: Expected active action text of '" << #expected << "', got '" \
      << qPrintable(activeActionText) << "'" << std::endl; \
    return EXIT_FAILURE; \
    } \
  };

QString activePlaceActionText(qSlicerMouseModeToolBar& mouseModeToolBar)
{
  QAction* placeAction = mouseModeToolBar.actions()[2];
  if (!placeAction->isEnabled())
    {
    return QString();
    }
  return placeAction->text();
  /*
  return mouseModeToolBar.actions()[2]->text();
  foreach(QAction* action, mouseModeToolBar.actions())
    {
    if (action->objectName() == QString("ToolBarAction"))
      {
      return action->text();
      break;
      }
    }
  return QString();
  */
}

QString getActiveActionText(qSlicerMouseModeToolBar& mouseModeToolBar)
  {
  foreach(QAction * action, mouseModeToolBar.actions())
    {
    std::cout << "action name: " << qPrintable(action->objectName()) << std::endl;;
    if (action->isChecked())
      {
      return action->text();
      break;
      }

    }
  return QString();
  }

int qSlicerMouseModeToolBarTest1(int argc, char * argv[] )
{
  qSlicerApplication app(argc, argv);
  qSlicerMouseModeToolBar mouseToolBar;

  // set the scene without the app logic
  vtkMRMLScene* scene = vtkMRMLScene::New();

  // Check that setting a scene without interaction node does not cause any problem
  // (it may log messages that interaction node is not found)
  mouseToolBar.setMRMLScene(scene);

  // Now reset scene in the toolbar to null and set the scene again now after app logic
  // adds interaction and selection nodes.
  mouseToolBar.setMRMLScene(nullptr);
  vtkSlicerApplicationLogic *appLogic = vtkSlicerApplicationLogic::New();
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

  CHECK_PLACE_ACTION_TEXT("Toggle Markups Toolbar", mouseToolBar);

  // get the selection and interaction nodes that the mouse mode tool bar
  // listens to
  vtkMRMLSelectionNode *selectionNode = vtkMRMLSelectionNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLSelectionNodeSingleton"));
  CHECK_NOT_NULL(selectionNode);

  // add markups
  selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsFiducialNode", ":/Icons/MarkupsFiducialMouseModePlace.png", "Point List");
  selectionNode->AddNewPlaceNodeClassNameToList("vtkMRMLMarkupsCurveNode", ":/Icons/MarkupsCurveMouseModePlace.png", "Curve");

  selectionNode->SetReferenceActivePlaceNodeClassName("vtkMRMLMarkupsFiducialNode");
  selectionNode->SetActivePlaceNodeID("vtkMRMLMarkupsFiducialNode1");
  selectionNode->SetActivePlaceNodePlacementValid(true);
  CHECK_PLACE_ACTION_TEXT("Point List", mouseToolBar);

  selectionNode->SetReferenceActivePlaceNodeClassName("vtkMRMLMarkupsCurveNode");
  selectionNode->SetActivePlaceNodeID("vtkMRMLMarkupsCurveNode1");
  selectionNode->SetActivePlaceNodePlacementValid(true);
  CHECK_PLACE_ACTION_TEXT("Curve", mouseToolBar);

  selectionNode->SetReferenceActivePlaceNodeClassName("vtkMRMLMarkupsFiducialNode");
  selectionNode->SetActivePlaceNodeID("vtkMRMLMarkupsFiducialNode1");
  selectionNode->SetActivePlaceNodePlacementValid(true);
  CHECK_PLACE_ACTION_TEXT("Point List", mouseToolBar);

  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast(
    scene->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
  CHECK_NOT_NULL(interactionNode);

  interactionNode->SetPlaceModePersistence(1);
  interactionNode->SetPlaceModePersistence(0);
  interactionNode->SwitchToSinglePlaceMode();
  CHECK_PLACE_ACTION_TEXT("Point List", mouseToolBar);

  interactionNode->SwitchToViewTransformMode();
  CHECK_PLACE_ACTION_TEXT("Point List", mouseToolBar);

  // clean up
  appLogic->Delete();
  scene->Delete();

  return EXIT_SUCCESS;
}
