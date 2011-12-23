#include <sstream>

#include <vtksys/SystemTools.hxx>

#include "vtkMRMLAnnotationFiducialsStorageNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include <vtkMRMLScene.h>


#include "TestingMacros.h"

int vtkMRMLAnnotationFiducialsStorageNodeTest1(int argc, char * argv[] )
{
  for (int a = 0; a < argc; a++)
    {
    std::cout << "argc = " << argc<< ", arg " << a << " = " << (argv[a] ? argv[a] : "null") << std::endl;
    }
  // argv[4] is the temporary dir
  const char *tempDir = NULL;
  if (argc > 4)
    {
    tempDir = argv[4];
    }
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
  annNode->SetName("AnnotationFiducialNodeTest") ;
  double ctp[3] = { 1, 2, 3};
  const char* text = "Test 1 2";
  annNode->SetFiducialLabel(text);
  if (!annNode->SetFiducial(ctp,1,0)) 
    {
    vtkIndent f;
    annNode->PrintSelf(cout,f);
    std::cerr << "Error: Could not define Fiducial " << std::endl;
    return EXIT_FAILURE;
    }
  std::cout << "Set annNode visible to 0, GetVisible = " << annNode->GetVisible() << std::endl;
  
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

  mrmlScene->AddNode(node2);
  node2->InitializeSupportedWriteFileTypes();
  // node2->SetDataDirectory("./log");
  std::string fileName = "AnnotationFiducialList.acsv";
  std::vector<std::string> pathComponents;
  if (tempDir)
    {
    vtksys::SystemTools::SplitPath(tempDir,pathComponents);
    pathComponents.push_back(fileName);
    fileName = vtksys::SystemTools::JoinPath(pathComponents);
    std::cout << "Using temporary directory for file name: " << fileName.c_str() << std::endl;
    }
  node2->SetFileName(fileName.c_str());
  node2->WriteData(annNode);
  std::cout << "After Write Data, annNode GetVisible = " << annNode->GetVisible() << std::endl;

  vtkIndent in;
  std::stringstream initialAnnotation, afterAnnotation;
  annNode->PrintAnnotationInfo(initialAnnotation,in);
  annNode->ResetAnnotations();
  std::cout << "After ResetAnnotations, annNode GetVisible = " << annNode->GetVisible() << std::endl;
  node2->ReadData(annNode);
  std::cout << "After ReadData, annNode GetVisible = " << annNode->GetVisible() << std::endl;
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
