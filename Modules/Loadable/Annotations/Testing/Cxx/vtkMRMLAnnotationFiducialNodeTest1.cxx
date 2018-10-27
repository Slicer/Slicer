#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLScene.h"

#include <sstream>

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationFiducialNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLAnnotationFiducialNode> node1;
  vtkNew<vtkMRMLScene> mrmlScene;
  mrmlScene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  mrmlScene->RemoveNode(node1.GetPointer());

  // ======================
  // Basic Setup
  // ======================
  mrmlScene->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationFiducialNode>::New());
  mrmlScene->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationPointDisplayNode>::New());

  // ======================
  // Modify Properties
  // ======================
  vtkNew<vtkMRMLAnnotationFiducialNode> node2;

  mrmlScene->AddNode(node2.GetPointer());
  node2->CreateAnnotationPointDisplayNode();
  CHECK_NOT_NULL(node2->GetAnnotationPointDisplayNode());

  double ctp[3] = { 1, 2, 3};
  const char* text = "Test 1 2";
  node2->SetFiducialLabel(text);
  CHECK_BOOL(node2->SetFiducial(ctp,1,0), true);
  node2->SetSelected(1);
  node2->SetDisplayVisibility(0);

  CHECK_INT(node2->GetNumberOfTexts(), 1);
  CHECK_STRING(node2->GetFiducialLabel(), text);

  double *ctrlPointPos = node2->GetFiducialCoordinates();
  CHECK_NOT_NULL(ctrlPointPos);
  CHECK_INT(ctrlPointPos[0], 1);
  CHECK_INT(ctrlPointPos[1], 2);
  CHECK_INT(ctrlPointPos[2], 3);

  CHECK_BOOL(node2->GetSelected(), true);
  CHECK_BOOL(node2->GetDisplayVisibility(), false);

  vtkIndent ind;
  node2->PrintAnnotationInfo(cout,ind);

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  mrmlScene->SetURL("AnnotationFiducialNodeTest.mrml");
  mrmlScene->Commit(); // write

  vtkNew<vtkMRMLScene> mrmlScene2;
  mrmlScene2->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationFiducialNode>::New());
  mrmlScene2->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationPointDisplayNode>::New());
  mrmlScene2->SetURL("AnnotationFiducialNodeTest.mrml");
  mrmlScene2->Connect(); // read

  CHECK_INT(mrmlScene2->GetNumberOfNodesByClass("vtkMRMLAnnotationFiducialNode"),1);

  vtkMRMLAnnotationFiducialNode *node3 = vtkMRMLAnnotationFiducialNode::SafeDownCast(mrmlScene->GetFirstNodeByClass("vtkMRMLAnnotationFiducialNode"));
  CHECK_NOT_NULL(node3);

  std::stringstream initialAnnotation, afterAnnotation;

  node2->PrintAnnotationInfo(initialAnnotation,ind);
  node3->PrintAnnotationInfo(afterAnnotation,ind);
  CHECK_STRING(initialAnnotation.str().c_str(), afterAnnotation.str().c_str());

  std::cout << "Test passed" << std::endl;

  return EXIT_SUCCESS;
}
