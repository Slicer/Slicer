#include "vtkMRMLAnnotationLineDisplayNode.h"
#include "vtkMRMLAnnotationLinesNode.h"
#include "vtkMRMLAnnotationLinesStorageNode.h"
#include "vtkMRMLScene.h"

#include <sstream>

#include "vtkMRMLCoreTestingMacros.h"

void SetControlPointsAndText(vtkMRMLAnnotationLinesNode* node2)
{
  node2->AddText("TESTING 1",1,1);
  node2->AddText("TESTING 2",1,1);

  {
    double ctp[3] = { 1, 1, 1};
    node2->AddControlPoint(ctp,1,1);
  }
  {
    double ctp[3] = { 2, 2, 2};
    node2->AddControlPoint(ctp,0,1);
  }
  {
    double ctp[3] = { 1, 2, 3};
    node2->AddControlPoint(ctp,0,0);
  }
}

int vtkMRMLAnnotationLinesNodeTest1(int , char * [] )
{
  vtkNew< vtkMRMLAnnotationLinesNode > node1;
  vtkNew<vtkMRMLScene> mrmlScene;
  mrmlScene->AddNode(node1.GetPointer());
  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  mrmlScene->RemoveNode(node1.GetPointer());

  // ======================
  // Basic Setup
  // ======================
  mrmlScene->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationLinesNode>::New());
  mrmlScene->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationLineDisplayNode>::New());

  // ======================
  // Modify Properties
  // ======================
  vtkNew<vtkMRMLAnnotationLinesNode> node2;

  mrmlScene->AddNode(node2.GetPointer());
  node2->CreateAnnotationLineDisplayNode();
  CHECK_NOT_NULL(node2->GetAnnotationLineDisplayNode());

  SetControlPointsAndText(node2.GetPointer());
  CHECK_INT(node2->AddLine(0,1,1/*sel*/,0/*vis*/), 0);

  node2->ResetAnnotations();
  CHECK_INT(node2->GetNumberOfLines(), 0);

  SetControlPointsAndText(node2.GetPointer());
  CHECK_INT(node2->AddLine(0,1,1,0), 0);

  int sel = 0;
  int vis = 1;
  CHECK_INT(node2->AddLine(0,2,sel,vis), 1);

  vtkIdType ctrlPointID[2]={0};

  CHECK_BOOL(node2->GetEndPointsId(1,ctrlPointID), true);
  CHECK_INT(ctrlPointID[0], 0);
  CHECK_INT(ctrlPointID[1], 2);
  CHECK_INT(node2->GetAnnotationAttribute(1, vtkMRMLAnnotationLinesNode::LINE_SELECTED), sel);
  CHECK_INT(node2->GetAnnotationAttribute(1, vtkMRMLAnnotationLinesNode::LINE_VISIBLE), vis);

  node2->AddLine(1,2,0,0);
  CHECK_INT(node2->GetNumberOfLines(), 3);

  node2->DeleteLine(1);
  CHECK_INT(node2->GetNumberOfLines(), 2);

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  mrmlScene->SetURL("AnnotationLineNodeTest.mrml");
  mrmlScene->Commit(); // write

  vtkNew<vtkMRMLScene> mrmlScene2;
  mrmlScene2->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationLinesNode>::New());
  mrmlScene2->RegisterNodeClass(vtkSmartPointer<vtkMRMLAnnotationLineDisplayNode>::New());
  mrmlScene2->SetURL("AnnotationLineNodeTest.mrml");
  mrmlScene2->Connect(); // read

  CHECK_INT(mrmlScene2->GetNumberOfNodesByClass("vtkMRMLAnnotationLinesNode"),1);

  vtkMRMLAnnotationLinesNode *node3 = vtkMRMLAnnotationLinesNode::SafeDownCast(mrmlScene->GetFirstNodeByClass("vtkMRMLAnnotationLinesNode"));
  CHECK_NOT_NULL(node3);

  vtkIndent ind;
  std::stringstream initialAnnotation, afterAnnotation;

  node2->PrintAnnotationInfo(initialAnnotation,ind);
  node3->PrintAnnotationInfo(afterAnnotation,ind);
  CHECK_STRING(initialAnnotation.str().c_str(), afterAnnotation.str().c_str());

  std::cout << "Test passed" << std::endl;

  return EXIT_SUCCESS;
}


