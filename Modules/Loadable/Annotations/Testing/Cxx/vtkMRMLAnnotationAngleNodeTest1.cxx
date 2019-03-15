#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLAnnotationAngleStorageNode.h"
#include "vtkMRMLScene.h"

#include <sstream>

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationAngleNodeTest1(int , char * [] )
{
  // ======================
  // Basic Setup
  // ======================

  vtkNew<vtkMRMLAnnotationAngleNode> node2;
  vtkNew<vtkMRMLScene> mrmlScene;
  vtkNew<vtkMRMLAnnotationAngleNode> node1;
  mrmlScene->AddNode(node1.GetPointer());

  node1->UpdateReferences();
  node2->Copy( node1.GetPointer() );

  mrmlScene->RegisterNodeClass(node1.GetPointer());
  mrmlScene->AddNode(node2.GetPointer());

  vtkNew<vtkMRMLAnnotationAngleStorageNode> prototypeStorageNode;
  mrmlScene->RegisterNodeClass(prototypeStorageNode.GetPointer());
  vtkMRMLAnnotationAngleStorageNode *storNode = vtkMRMLAnnotationAngleStorageNode::SafeDownCast(
    node2->CreateDefaultStorageNode());

  EXERCISE_ALL_BASIC_MRML_METHODS(node1.GetPointer());
  mrmlScene->RemoveNode(node1.GetPointer());

  if( !storNode )
    {
    std::cerr << "Error in CreateDefaultStorageNode()" << std::endl;
    return EXIT_FAILURE;
    }
  storNode->Delete();

  std::cout << "Passed StorageNode" << std::endl;

  // ======================
  // Modify Properties
  // ======================
  node2->Reset(nullptr);
  node2->StartModify();

  node2->SetName("AnnotationAngleNodeTest") ;

  std::string nodeTagName = node2->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

  {
    double ctp[3] = { 1, 1, 1};
    node2->SetPosition1(ctp);
  }
  {
    double ctp[3] = { 2, 2, 2};
    node2->SetPosition2(ctp);
  }
  {
    double ctp[3] = { 1, 2, 3};
    node2->SetPositionCenter(ctp);
  }

  int vis = 1;
  node2->SetRay1Visibility(vis);

  double *ctrlPointID = node2->GetPositionCenter();

  if (ctrlPointID[0]!= 1 || (ctrlPointID[1] != 2) || (ctrlPointID[2] != 3) ||  node2->GetRay1Visibility() != vis)
    {
    std::cerr << "Error in Array Attributes: "  << ctrlPointID[0] << "!=1, " << ctrlPointID[1]<< "!=2" << ctrlPointID[2]<< "!=3, "
      << node2->GetRay1Visibility() <<"!="<< vis << std::endl;
    return EXIT_FAILURE;
    }

  vtkIndent ind;
  node2->PrintAnnotationInfo(cout,ind);

  cout << "Passed Adding and Deleting Data" << endl;

  node2->Modified();

  // ======================
  // Test WriteXML and ReadXML
  // ======================

  mrmlScene->SetURL("AnnotationAngleNodeTest.mrml");
  mrmlScene->Commit();

  // Now Read in File to see if ReadXML works - it first disconnects from node2 !
  mrmlScene->Connect();
  vtkIndent ij;

  if (mrmlScene->GetNumberOfNodesByClass("vtkMRMLAnnotationAngleNode") != 1)
    {
    std::cerr << "Error in ReadXML() or WriteXML() - Did not create a class called vtkMRMLAnnotationAngleNode" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMRMLAnnotationAngleNode *node3 = vtkMRMLAnnotationAngleNode::SafeDownCast(mrmlScene->GetFirstNodeByClass("vtkMRMLAnnotationAngleNode"));
  if (!node3)
    {
    std::cerr << "Error in ReadXML() or WriteXML(): could not find vtkMRMLAnnotationAngleNode" << std::endl;
    return EXIT_FAILURE;
    }

  std::stringstream initialAnnotation, afterAnnotation;

  node2->PrintAnnotationInfo(initialAnnotation,ind);

  node3->PrintAnnotationInfo(afterAnnotation,ind);
  if (initialAnnotation.str().compare(afterAnnotation.str()))
  {
    std::cerr << "Error in ReadXML() or WriteXML()" << std::endl;
    std::cerr << "Before:" << std::endl << initialAnnotation.str() <<std::endl;
    std::cerr << "After:" << std::endl << afterAnnotation.str() <<std::endl;
    return EXIT_FAILURE;
  }
  cout << "Passed XML" << endl;

  return EXIT_SUCCESS;
}
