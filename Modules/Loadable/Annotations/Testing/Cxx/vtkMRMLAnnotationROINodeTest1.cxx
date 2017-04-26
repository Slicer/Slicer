
// MRML includes
#include <vtkMRMLAnnotationROINode.h>
#include <vtkMRMLCoreTestingMacros.h>
#include <vtkMRMLCoreTestingUtilities.h>
#include <vtkMRMLScene.h>

/* this test has been adopted from vtkMRMLAnnotationAngleTest1 by
 * Andrey Fedorov to demonstrate some of the problems observed with the ROI
 * annotation node
 */

using namespace vtkMRMLCoreTestingUtilities;

//----------------------------------------------------------------------------
int vtkMRMLAnnotationROINodeTest1(int , char * [] )
{

  // ======================
  // Basic Setup
  // ======================

  vtkSmartPointer<vtkMRMLAnnotationROINode > node2 = vtkSmartPointer< vtkMRMLAnnotationROINode >::New();
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();

  {

    vtkNew<vtkMRMLAnnotationROINode> node1;

    EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());

    node1->UpdateReferences();
    node2->Copy(node1.GetPointer());

    mrmlScene->RegisterNodeClass(node1.GetPointer());
    mrmlScene->AddNode(node2);
  }

  // ======================
  // Modify Properties
  // ======================
  node2->Reset(NULL);

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

  return EXIT_SUCCESS;
}
