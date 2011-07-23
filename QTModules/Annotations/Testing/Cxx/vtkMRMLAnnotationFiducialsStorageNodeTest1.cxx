#include "vtkMRMLAnnotationFiducialsStorageNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include <vtkMRMLScene.h>

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>

#include "TestingMacros.h"

int vtkMRMLAnnotationFiducialsStorageNodeTest1(int , char * [] )
{
  vtkSmartPointer< vtkMRMLAnnotationFiducialsStorageNode > node2 = vtkSmartPointer< vtkMRMLAnnotationFiducialsStorageNode >::New();
  EXERCISE_BASIC_OBJECT_METHODS( node2 );

  vtkMRMLAnnotationFiducialsStorageNode* node1 = dynamic_cast <  vtkMRMLAnnotationFiducialsStorageNode *> (node2->CreateNodeInstance());
  if( node1 == NULL )
    {
      std::cerr << "Error in CreateNodeInstance()" << std::endl;
      return EXIT_FAILURE;
    }
  node1->Delete();


  vtkSmartPointer< vtkMRMLAnnotationFiducialNode > annNode = vtkSmartPointer< vtkMRMLAnnotationFiducialNode >::New();
  annNode->StartModify();
  annNode->SetName("AnnotationFidcucialNodeTest") ;
  double ctp[3] = { 1, 2, 3};
  const char* text = "Test 1 2";
  if (!annNode->SetFiducial(text,ctp,1,0)) 
      {
    vtkIndent f;
    annNode->PrintSelf(cout,f);
    std::cerr << "Error: Could not define Fiducial " << std::endl;
    return EXIT_FAILURE;
      }

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
       std::cerr << "Error in vtkMRMLAnnotationFiducialNode::AnnotationPointDisplayNode() " << std::endl;
       return EXIT_FAILURE;
    }  
  cout << "AnnotationPointDisplayNode Passed" << endl;

  annNode->Modified();

  node2->InitializeSupportedWriteFileTypes();
  // node2->SetDataDirectory("./log");
  node2->SetFileName("AnnotationFiducialList.fcsv");
  // node2->WriteData(annNode);

  vtkIndent in;
  // std::stringstream initialAnnotation, afterAnnotation;
  // annNode->PrintAnnotationInfo(initialAnnotation,in);
  annNode->ResetAnnotations();
  node2->ReadData();

  // annNode->PrintAnnotationInfo(afterAnnotation,in);
  // if (initialAnnotation.str().compare(afterAnnotation.str())) 
  // {
  //   std::cerr << endl << "Error in WriteData() or ReadData()" << std::endl;
  //   std::cerr << "Before:" << std::endl << initialAnnotation.str() <<std::endl;
  //   std::cerr << "After:" << std::endl << afterAnnotation.str() <<std::endl;
  //   std::cerr << "KP : need to fix annText field ones we have AnnotationFiducials defined" <<std::endl; 
  //   return EXIT_FAILURE;
  // }

  return EXIT_SUCCESS;
  
}
