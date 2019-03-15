#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationRulerNode.h"
#include "vtkSlicerAnnotationModuleLogic.h"

// MRML includes
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>

#include "vtkTestingOutputWindow.h"

void CheckTextColor(vtkMRMLAnnotationRulerNode *rnode)
{
  cout << "=== ColorCheck: " << rnode->GetAnnotationTextDisplayNode() << " " << rnode->GetAnnotationTextDisplayNode()->GetColor()[0] << " " << rnode->GetAnnotationTextDisplayNode()->GetColor()[1] << " " << rnode->GetAnnotationTextDisplayNode()->GetColor()[2] << endl;

}

// return 1 on failure, 0 on success
int colorTest(double *incolor, double *outcolor)
{
  if (incolor == nullptr ||
      outcolor == nullptr)
    {
    return 1;
    }
  if (!memcmp(incolor,outcolor,sizeof(double)*3))
    {
    return 0;
    }
  else
    {
    cout << "incolor  " << incolor[0] << " " << incolor[1] << " " << incolor[2] << endl;
    cout << "outcolor " << outcolor[0] << " " << outcolor[1] << " " << outcolor[2] << endl;
    return 1;
    }
}

int vtkSlicerAnnotationModuleLogicTest1(int , char * [] )
{

  // ======================
  // Basic Setup
  // ======================
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();
  vtkNew<vtkMRMLSelectionNode> selectionNode;
  mrmlScene->AddNode(selectionNode.GetPointer());
  vtkNew<vtkMRMLInteractionNode> interactionNode;
  mrmlScene->AddNode(interactionNode.GetPointer());

  vtkNew<vtkSlicerAnnotationModuleLogic > logic;
  EXERCISE_BASIC_OBJECT_METHODS(logic.GetPointer());

  logic->SetMRMLScene(mrmlScene);

  logic->StartPlaceMode(true);
  logic->StartPlaceMode(false);

  logic->StopPlaceMode(true);
  logic->StopPlaceMode(false);

  logic->AddAnnotationNode("AnnotationFiducialNode", false);
  logic->AddAnnotationNode("AnnotationTextNode", true);

  logic->AddNodeCompleted(nullptr);

  vtkSmartPointer<vtkMRMLAnnotationRulerNode> rnode1 = vtkSmartPointer<vtkMRMLAnnotationRulerNode>::New();
  mrmlScene->AddNode(rnode1 );
  logic->AddNodeCompleted(rnode1 );

  logic->CancelCurrentOrRemoveLastAddedAnnotationNode();

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  logic->RemoveAnnotationNode(nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();

  logic->RemoveAnnotationNode(rnode1 );

  // TODO: test mrml event processing?

  bool retval = logic->IsAnnotationNode(nullptr);
  if (retval)
    {
    std::cerr << "Error on checking a null annotation node id" << std::endl;
    return EXIT_FAILURE;
    }
  retval = logic->IsAnnotationHierarchyNode(nullptr);
  if (retval)
    {
    std::cerr << "Error on checking a null annotation hierarchy node id" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLNode *node = mrmlScene->GetNodeByID("vtkMRMLAnnotationFiducialNode");
  if (node)
    {
    retval = logic->IsAnnotationNode(node->GetID());
    if (!retval)
      {
      std::cerr << "Error on checking a fid annotation hierarchy node id" << std::endl;
      return EXIT_FAILURE;
      }
    }

  // add some display nodes
  double incolor1[3] = { 147.0/255.0, 0 , 0 };
  double incolor2[3] = { 0, 147.0/255.0, 0 };
  double incolor3[3] = { 0, 0, 147.0/255.0};

  vtkSmartPointer<vtkMRMLAnnotationRulerNode> rnode = vtkSmartPointer<vtkMRMLAnnotationRulerNode>::New();
  mrmlScene->AddNode(rnode);

  logic->SetAnnotationTextUnselectedColor(rnode->GetID(), incolor1);
  double *outcolor1 = nullptr;
  if (rnode->GetAnnotationTextDisplayNode())
    {
    outcolor1 = rnode->GetAnnotationTextDisplayNode()->GetColor();
    }
  if ( colorTest(incolor1,outcolor1))
    {
    std::cout << "Setting Text Color Failed!" << std::endl;
    //return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Setting Text Color Ok!" << std::endl;
    }
  logic->SetAnnotationPointColor(rnode->GetID(), incolor2);
  double *outcolor2 = nullptr;
  if (rnode->GetAnnotationPointDisplayNode())
    {
    outcolor2 = rnode->GetAnnotationPointDisplayNode()->GetColor();
    }
  if ( colorTest(incolor2,outcolor2))
    {
    std::cout << "Setting Point Color Failed!" << std::endl;
    //return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Setting Point Color Ok!" << std::endl;
    }
  logic->SetAnnotationLineColor(rnode->GetID(), incolor3);
  double *outcolor3 = nullptr;
  if (rnode->GetAnnotationLineDisplayNode())
    {
    outcolor3 = rnode->GetAnnotationLineDisplayNode()->GetColor();
    }
  if ( colorTest(incolor3,outcolor3))
    {
    std::cout << "Setting Line Color Failed!" << std::endl;
    //return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Setting Line Color Ok!" << std::endl;
    }

  double* getcolor1 = logic->GetAnnotationLineColor(rnode->GetID());

  if (colorTest(incolor1, getcolor1))
    {
    cout << "Correct DisplayNode: " << rnode->GetAnnotationTextDisplayNode() << " " <<   rnode->GetAnnotationLineDisplayNode() << endl;
    std::cout << "Getting Text Color Failed!" << std::endl;
//    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Getting Text Color Ok!" << std::endl;
    }
  double* getcolor2 = logic->GetAnnotationLineUnselectedColor(rnode->GetID());
  if (colorTest(incolor2, getcolor2))
    {
    std::cout << "Getting Point Color Failed!" << std::endl;
    //return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Getting Point Color Ok!" << std::endl;
    }
  double* getcolor3 = logic->GetAnnotationLineUnselectedColor(rnode->GetID());
  if (colorTest(incolor3, getcolor3))
    {
    std::cout << "Getting Line Color Failed!" << std::endl;
    //return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Getting Line Color Ok!" << std::endl;
    }

  // 1.) Test
  double ctp[3] = { 1, 1, 1};

  rnode->SetPosition1(ctp);
  rnode->SetPosition2(ctp);

  // 2.) Text Display Node Test..
  double scale = 20;
  if (rnode->GetAnnotationTextDisplayNode())
    {
    rnode->GetAnnotationTextDisplayNode()->SetTextScale( scale );
    }

  // test out hierarchy methods
  logic->AddHierarchy();

  char *toplevelid = logic->GetTopLevelHierarchyNodeID();
  std::cout << "Top level id = " << (toplevelid ? toplevelid : "null") << std::endl;
  retval = logic->IsAnnotationHierarchyNode(toplevelid);
  if (!retval)
    {
    std::cerr << "Error checking if is annot hierarchy node on id = " << (toplevelid ? toplevelid : "null") << std::endl;
    return EXIT_FAILURE;
    }
  vtkSmartPointer<vtkMRMLAnnotationFiducialNode> fnode = vtkSmartPointer<vtkMRMLAnnotationFiducialNode>::New();
  double pos[3]={1,2,5};
  fnode->SetControlPoint(0, pos, 0, 1);
  mrmlScene->AddNode(fnode);

  char *toplevelfidid = logic->GetTopLevelHierarchyNodeIDForNodeClass(fnode);
  std::cout << "Top level fid id = " << (toplevelfidid ? toplevelfidid : "null") << std::endl;

  vtkMRMLAnnotationHierarchyNode *activeHierarchy = logic->GetActiveHierarchyNode();
  if (activeHierarchy)
    {
    std::cout << "Active hierarchy = " << (activeHierarchy->GetName() ? activeHierarchy->GetName() : "null") << std::endl;
    }

  vtkMRMLAnnotationNode *nullNode = nullptr;
  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  const char *htmlRep = logic->GetHTMLRepresentation(nullNode, -1);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (htmlRep)
    {
    std::cout << htmlRep << std::endl;
    }
  htmlRep = logic->GetHTMLRepresentation(fnode, 1);
  if (htmlRep)
    {
    std::cout << htmlRep << std::endl;
    }
  htmlRep = logic->GetHTMLRepresentation(activeHierarchy, 0);
  if (htmlRep)
    {
    std::cout << htmlRep << std::endl;
    }
  TEST_SET_GET_STRING(logic, ActiveHierarchyNodeID);

  // test adding a display node for a hierarchy node

  TESTING_OUTPUT_ASSERT_ERRORS_BEGIN();
  const char *dID = logic->AddDisplayNodeForHierarchyNode(nullptr);
  TESTING_OUTPUT_ASSERT_ERRORS_END();
  if (dID != nullptr)
    {
    std::cerr << "Error testing null hierarchy node to add display node for, got a display node id of " << dID << std::endl;
    return EXIT_FAILURE;
    }
  dID = logic->AddDisplayNodeForHierarchyNode(activeHierarchy);
  if (dID == nullptr)
    {
    std::cerr << "Error testing adding display node for a hierarchy node, got a display node id of NULL" << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Added a display node with id " << dID << std::endl;

  return EXIT_SUCCESS;
}



