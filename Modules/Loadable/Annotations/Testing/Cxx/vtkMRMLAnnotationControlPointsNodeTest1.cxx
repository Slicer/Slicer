#include "vtkMRMLAnnotationControlPointsNode.h"
#include "vtkMRMLAnnotationControlPointsStorageNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include "vtkMRMLScene.h"

#include <sstream>

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationControlPointsNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLAnnotationControlPointsNode> node1;
  vtkNew<vtkMRMLScene> mrmlScene;
  mrmlScene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  mrmlScene->RemoveNode(node1.GetPointer());

  // ======================
  // Basic Setup
  // ======================
  mrmlScene->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationControlPointsNode>::New());
  mrmlScene->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationPointDisplayNode>::New());

  // ======================
  // Modify Properties
  // ======================
  vtkNew<vtkMRMLAnnotationControlPointsNode> node2;
  node2->Reset(nullptr);

  mrmlScene->AddNode(node2.GetPointer());
  node2->CreateAnnotationPointDisplayNode();
  CHECK_NOT_NULL(node2->GetAnnotationPointDisplayNode());

  {
    double ctp[3] = { 1, 1, 1};
    node2->AddControlPoint(ctp,1,1);
    CHECK_BOOL(node2->GetAnnotationAttribute(0, vtkMRMLAnnotationControlPointsNode::CP_VISIBLE), true);
    CHECK_BOOL(node2->GetAnnotationAttribute(0, vtkMRMLAnnotationControlPointsNode::CP_SELECTED), true);
  }
  {
    double ctp[3] = { 2, 2, 2};
    node2->AddControlPoint(ctp,0,1);
  }
  {
    double ctp[3] = { 1, 2, 3};
    node2->SetControlPoint(3,ctp,0,0);
    node2->SetControlPoint(2,ctp,0,0);
  }
  CHECK_INT(node2->GetNumberOfControlPoints(), 4);

  node2->DeleteControlPoint(3);
  CHECK_INT(node2->GetNumberOfControlPoints(), 3);

  node2->AddText("TESTING",1,1);
  CHECK_INT(node2->GetNumberOfTexts(), 1);
  CHECK_STRING(node2->GetText(0), "TESTING");

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  mrmlScene->SetURL("AnnotationControlPointNodeTest.mrml");
  mrmlScene->Commit(); // write

  vtkNew<vtkMRMLScene> mrmlScene2;
  mrmlScene2->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationControlPointsNode>::New());
  mrmlScene2->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationPointDisplayNode>::New());
  mrmlScene2->SetURL("AnnotationControlPointNodeTest.mrml");
  mrmlScene2->Connect(); // read

  CHECK_INT(mrmlScene2->GetNumberOfNodesByClass("vtkMRMLAnnotationControlPointsNode"),1);

  vtkMRMLAnnotationControlPointsNode *node3 = vtkMRMLAnnotationControlPointsNode::SafeDownCast(
    mrmlScene->GetFirstNodeByClass("vtkMRMLAnnotationControlPointsNode"));
  CHECK_NOT_NULL(node3);

  vtkIndent ind;
  std::stringstream initialAnnotation, afterAnnotation;

  node2->PrintAnnotationInfo(initialAnnotation,ind);
  node3->PrintAnnotationInfo(afterAnnotation,ind);
  CHECK_STRING(initialAnnotation.str().c_str(), afterAnnotation.str().c_str());

  std::cout << "Test passed" << std::endl;

  return EXIT_SUCCESS;
}
