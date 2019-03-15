
// MRML includes
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLCoreTestingUtilities.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkPlane.h>
#include <vtkPlanes.h>
#include <vtkTransform.h>

/* this test has been adopted from vtkMRMLAnnotationAngleTest1 by
 * Andrey Fedorov to demonstrate some of the problems observed with the ROI
 * annotation node
 */

using namespace vtkMRMLCoreTestingUtilities;

//----------------------------------------------------------------------------
int TestGetTransformedPlanes(vtkMRMLScene* scene, vtkMRMLAnnotationROINode* node);

//----------------------------------------------------------------------------
int vtkMRMLAnnotationROINodeTest1(int , char * [] )
{
  // ======================
  // Basic Setup
  // ======================

  vtkNew<vtkMRMLAnnotationROINode> node2;
  vtkNew<vtkMRMLScene> mrmlScene;

  vtkNew<vtkMRMLAnnotationROINode> node1;
  mrmlScene->AddNode(node1.GetPointer());

  node1->UpdateReferences();
  node2->Copy(node1.GetPointer());

  mrmlScene->RegisterNodeClass(node1.GetPointer());
  mrmlScene->AddNode(node2.GetPointer());

  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  mrmlScene->RemoveNode(node1.GetPointer());

  // ======================
  // Modify Properties
  // ======================
  node2->Reset(nullptr);

  vtkNew<vtkMRMLNodeCallback> spy;
  node2->AddObserver(vtkCommand::AnyEvent, spy.GetPointer());

  int wasModified = node2->StartModify();

  node2->SetName("AnnotationROINodeTest") ;

  std::string nodeTagName = node2->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  CHECK_BOOL(node2->SetXYZ(3, -5, 0), true);
  CHECK_BOOL(node2->SetRadiusXYZ(100, 200, -10), true);

  std::cout << "PrintAnnotationInfo:" << std::endl;
  vtkIndent ind;
  node2->PrintAnnotationInfo(std::cout, ind);

  double center[3];
  node2->GetXYZ(center);
  double radius[3];
  node2->GetRadiusXYZ(radius);

  CHECK_DOUBLE(center[0], 3);
  CHECK_DOUBLE(center[1], -5);
  CHECK_DOUBLE(center[2], 0);

  CHECK_DOUBLE(radius[0], 100);
  CHECK_DOUBLE(radius[1], 200);
  CHECK_DOUBLE(radius[2], -10);

  node2->EndModify(wasModified);

  CHECK_INT(spy->GetNumberOfEvents(vtkCommand::ModifiedEvent), 1);

  CHECK_EXIT_SUCCESS(TestGetTransformedPlanes(mrmlScene.GetPointer(), node2.GetPointer()));

  return EXIT_SUCCESS;
}

//----------------------------------------------------------------------------
int TestGetTransformedPlanes(vtkMRMLScene* scene, vtkMRMLAnnotationROINode* node)
{
  vtkNew<vtkPlanes> planes;
  node->GetTransformedPlanes(planes.GetPointer());
  CHECK_INT(planes->GetNumberOfPlanes(), 6);

  double* planeOrigin = nullptr;
  double* planeNormal = nullptr;

  // Without transform

  // plane 0
  planeOrigin = planes->GetPlane(0)->GetOrigin();
  CHECK_DOUBLE(planeOrigin[0], -97);
  CHECK_DOUBLE(planeOrigin[1], -205);
  CHECK_DOUBLE(planeOrigin[2], 10);

  planeNormal = planes->GetPlane(0)->GetNormal();
  CHECK_DOUBLE(planeNormal[0], 1);
  CHECK_DOUBLE(planeNormal[1], 0);
  CHECK_DOUBLE(planeNormal[2], 0);

  // plane 1
  planeOrigin = planes->GetPlane(1)->GetOrigin();
  CHECK_DOUBLE(planeOrigin[0], -97);
  CHECK_DOUBLE(planeOrigin[1], -205);
  CHECK_DOUBLE(planeOrigin[2], 10);

  planeNormal = planes->GetPlane(1)->GetNormal();
  CHECK_DOUBLE(planeNormal[0], 0);
  CHECK_DOUBLE(planeNormal[1], 1);
  CHECK_DOUBLE(planeNormal[2], 0);

  // plane 2
  planeOrigin = planes->GetPlane(2)->GetOrigin();
  CHECK_DOUBLE(planeOrigin[0], -97);
  CHECK_DOUBLE(planeOrigin[1], -205);
  CHECK_DOUBLE(planeOrigin[2], 10);

  planeNormal = planes->GetPlane(2)->GetNormal();
  CHECK_DOUBLE(planeNormal[0], 0);
  CHECK_DOUBLE(planeNormal[1], 0);
  CHECK_DOUBLE(planeNormal[2], -1);

  // plane 3
  planeOrigin = planes->GetPlane(3)->GetOrigin();
  CHECK_DOUBLE(planeOrigin[0], 103);
  CHECK_DOUBLE(planeOrigin[1], 195);
  CHECK_DOUBLE(planeOrigin[2], -10);

  planeNormal = planes->GetPlane(3)->GetNormal();
  CHECK_DOUBLE(planeNormal[0], -1);
  CHECK_DOUBLE(planeNormal[1], 0);
  CHECK_DOUBLE(planeNormal[2], 0);

  // plane 4
  planeOrigin = planes->GetPlane(4)->GetOrigin();
  CHECK_DOUBLE(planeOrigin[0], 103);
  CHECK_DOUBLE(planeOrigin[1], 195);
  CHECK_DOUBLE(planeOrigin[2], -10);

  planeNormal = planes->GetPlane(4)->GetNormal();
  CHECK_DOUBLE(planeNormal[0], 0);
  CHECK_DOUBLE(planeNormal[1], -1);
  CHECK_DOUBLE(planeNormal[2], 0);

  // plane 5
  planeOrigin = planes->GetPlane(5)->GetOrigin();
  CHECK_DOUBLE(planeOrigin[0], 103);
  CHECK_DOUBLE(planeOrigin[1], 195);
  CHECK_DOUBLE(planeOrigin[2], -10);

  planeNormal = planes->GetPlane(5)->GetNormal();
  CHECK_DOUBLE(planeNormal[0], 0);
  CHECK_DOUBLE(planeNormal[1], 0);
  CHECK_DOUBLE(planeNormal[2], 1);

  // With transform (translation + rotation)
  vtkNew<vtkMRMLTransformNode> transform;
  vtkNew<vtkTransform> tr;
  tr->Translate(5, 10, 20);
  tr->RotateX(30);
  tr->RotateY(15);
  tr->RotateZ(5);
  vtkNew<vtkMatrix4x4> matrix;
  tr->GetMatrix(matrix.GetPointer());
  transform->SetMatrixTransformToParent(matrix.GetPointer());

  scene->AddNode(transform.GetPointer());
  node->SetAndObserveTransformNodeID(transform->GetID());

  node->GetTransformedPlanes(planes.GetPointer());
  CHECK_INT(planes->GetNumberOfPlanes(), 6);

  // plane 0
  planeOrigin = planes->GetPlane(0)->GetOrigin();
  CHECK_DOUBLE_TOLERANCE(planeOrigin[0], -68.492, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[1], -189.204, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[2], -60.3174, 1e-3);

  planeNormal = planes->GetPlane(0)->GetNormal();
  CHECK_DOUBLE_TOLERANCE(planeNormal[0], 0.96225, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[1], 0.204396, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[2], -0.179713, 1e-3);

  // plane 1
  planeOrigin = planes->GetPlane(1)->GetOrigin();
  CHECK_DOUBLE_TOLERANCE(planeOrigin[0], -68.492, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[1], -189.204, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[2], -60.3174, 1e-3);

  planeNormal = planes->GetPlane(1)->GetNormal();
  CHECK_DOUBLE_TOLERANCE(planeNormal[0], -0.084186, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[1], 0.851451, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[2], 0.517633, 1e-3);

  // plane 2
  planeOrigin = planes->GetPlane(2)->GetOrigin();
  CHECK_DOUBLE_TOLERANCE(planeOrigin[0], -68.492, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[1], -189.204, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[2], -60.3174, 1e-3);

  planeNormal = planes->GetPlane(2)->GetNormal();
  CHECK_DOUBLE_TOLERANCE(planeNormal[0], -0.258819, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[1], 0.482963, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[2], -0.836516, 1e-3);

  // plane 3
  planeOrigin = planes->GetPlane(3)->GetOrigin();
  CHECK_DOUBLE_TOLERANCE(planeOrigin[0], 85.1073, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[1], 201.915, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[2], 94.0628, 1e-3);

  planeNormal = planes->GetPlane(3)->GetNormal();
  CHECK_DOUBLE_TOLERANCE(planeNormal[0], -0.96225, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[1], -0.204396, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[2], 0.179713, 1e-3);

  // plane 4
  planeOrigin = planes->GetPlane(4)->GetOrigin();
  CHECK_DOUBLE_TOLERANCE(planeOrigin[0], 85.1073, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[1], 201.915, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[2], 94.0628, 1e-3);

  planeNormal = planes->GetPlane(4)->GetNormal();
  CHECK_DOUBLE_TOLERANCE(planeNormal[0], 0.084186, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[1], -0.851451, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[2], -0.517633, 1e-3);

  // plane 5
  planeOrigin = planes->GetPlane(5)->GetOrigin();
  CHECK_DOUBLE_TOLERANCE(planeOrigin[0], 85.1073, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[1], 201.915, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeOrigin[2], 94.0628, 1e-3);

  planeNormal = planes->GetPlane(5)->GetNormal();
  CHECK_DOUBLE_TOLERANCE(planeNormal[0], 0.258819, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[1], -0.482963, 1e-3);
  CHECK_DOUBLE_TOLERANCE(planeNormal[2], 0.836516, 1e-3);

  return EXIT_SUCCESS;
}
