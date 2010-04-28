#include "vtkMRMLAnnotationStorageNode.h"
#include "vtkMRMLAnnotationNode.h"

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>

#include "TestingMacros.h"

int vtkMRMLAnnotationStorageNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLAnnotationStorageNode > node2 = vtkSmartPointer< vtkMRMLAnnotationStorageNode >::New();
  EXERCISE_BASIC_OBJECT_METHODS( node2 );

  vtkMRMLAnnotationStorageNode* node1 = dynamic_cast <  vtkMRMLAnnotationStorageNode *> (node2->CreateNodeInstance());
  if( node1 == NULL )
    {
      std::cerr << "Error in CreateNodeInstance()" << std::endl;
      return EXIT_FAILURE;
    }
  node1->Delete();


  vtkSmartPointer< vtkMRMLAnnotationNode > annNode = vtkSmartPointer< vtkMRMLAnnotationNode >::New();
  annNode->StartModify();
  annNode->SetName("AnnotationStorageNodeTest") ;
  std::string nodeTagName = annNode->GetNodeTagName();
  std::cout << "Node Tag Name = " << nodeTagName << std::endl;
  annNode->AddText("Test 1" , 1, 0);
  annNode->AddText("Test ,2"  , 0, 1);
  annNode->AddText("Test3"  , 1, 1);
  annNode->Modified();

  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();
  mrmlScene->AddNode(annNode);

  annNode->CreateAnnotationTextDisplayNode();
  if (!annNode->GetAnnotationTextDisplayNode())
    {
       std::cerr << "Error in vtkMRMLAnnotationNode::AnnotationTextDisplayNode() " << std::endl;
       return EXIT_FAILURE;
    }  

  annNode->Modified();

  node2->InitializeSupportedWriteFileTypes();
  // node2->SetDataDirectory("./log");
  node2->SetFileName("AnnotationStorageNodeTest.acsv");
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
    std::cerr << "KP : need to fix annText field ones we have AnnotationFiducials defined" <<std::endl; 
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
  
}
