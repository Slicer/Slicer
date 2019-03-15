#include "vtkMRMLAnnotationAngleStorageNode.h"
#include "vtkMRMLAnnotationAngleNode.h"
#include "vtkMRMLScene.h"

#include <sstream>

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationAngleStorageNodeTest1(int , char * [] )
{
  vtkNew<vtkMRMLAnnotationAngleStorageNode> node2;
  EXERCISE_ALL_BASIC_MRML_METHODS(node2.GetPointer());

  vtkMRMLAnnotationAngleStorageNode* node1 = dynamic_cast <  vtkMRMLAnnotationAngleStorageNode *> (node2->CreateNodeInstance());
  if( node1 == nullptr )
    {
      std::cerr << "Error in CreateNodeInstance()" << std::endl;
      return EXIT_FAILURE;
    }
  node1->Delete();


  vtkSmartPointer< vtkMRMLAnnotationAngleNode > annNode = vtkSmartPointer< vtkMRMLAnnotationAngleNode >::New();
  annNode->StartModify();
  annNode->SetName("AnnotationAngleStorageNodeTest") ;
  std::string nodeTagName = annNode->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;

 {
    double ctp[3] = { 1, 1, 1};
    annNode->SetPosition1(ctp);
  }
  {
    double ctp[3] = { 2, 2, 2};
    annNode->SetPosition2(ctp);
  }
  {
    double ctp[3] = { 1, 2, 3};
    annNode->SetPositionCenter(ctp);
  }


  int vis = 1;
  annNode->SetRay1Visibility(vis);


  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();
  mrmlScene->AddNode(annNode);

  annNode->CreateAnnotationTextDisplayNode();
  if (!annNode->GetAnnotationTextDisplayNode())
    {
       std::cerr << "Error in vtkMRMLAnnotationNode::AnnotationTextDisplayNode() " << std::endl;
       return EXIT_FAILURE;
    }

  annNode->CreateAnnotationPointDisplayNode();
  if (!annNode->GetAnnotationPointDisplayNode())
    {
       std::cerr << "Error in vtkMRMLAnnotationControlPointsNode::AnnotationPointDisplayNode() " << std::endl;
       return EXIT_FAILURE;
    }

  annNode->CreateAnnotationLineDisplayNode();
  if (!annNode->GetAnnotationLineDisplayNode())
    {
       std::cerr << "Error in vtkMRMLAnnotationLineNode::AnnotationPointDisplayNode() " << std::endl;
       return EXIT_FAILURE;
    }
  cout << "AnnotationPointDisplayNode Passed" << endl;


  annNode->Modified();

  // node2->SetDataDirectory("./log");
  node2->SetFileName("AnnotationAngleStorageNodeTest.acsv");
  node2->WriteData(annNode);

  vtkIndent in;
  std::stringstream initialAnnotation, afterAnnotation;
  annNode->PrintAnnotationInfo(initialAnnotation,in);
  annNode->ResetAnnotations();
  node2->ReadData(annNode);
  annNode->PrintAnnotationInfo(afterAnnotation,in);
  if (initialAnnotation.str().compare(afterAnnotation.str()))
  {
    std::cerr << endl << "Error in WriteData() or ReadData()" << std::endl;
    std::cerr << "Before:" << std::endl << initialAnnotation.str() <<std::endl;
    std::cerr << "After:" << std::endl << afterAnnotation.str() <<std::endl;
    return EXIT_FAILURE;
  }

  std::cout << initialAnnotation.str()<< std::endl;

  return EXIT_SUCCESS;

}
