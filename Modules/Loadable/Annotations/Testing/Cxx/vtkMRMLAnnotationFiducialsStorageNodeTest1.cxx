#include <sstream>

#include <vtksys/SystemTools.hxx>

#include "vtkMRMLAnnotationFiducialsStorageNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLAnnotationPointDisplayNode.h"
#include <vtkMRMLScene.h>


#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationFiducialsStorageNodeTest1(int argc, char * argv[] )
{
  for (int a = 0; a < argc; a++)
    {
    std::cout << "argc = " << argc<< ", arg " << a << " = " << (argv[a] ? argv[a] : "null") << std::endl;
    }
  // argv[4] is the temporary dir
  const char *tempDir = nullptr;
  if (argc > 4)
    {
    tempDir = argv[4];
    }
  vtkNew<vtkMRMLAnnotationFiducialsStorageNode> node2;
  EXERCISE_ALL_BASIC_MRML_METHODS(node2.GetPointer());

  vtkMRMLAnnotationFiducialsStorageNode* node1 = dynamic_cast <  vtkMRMLAnnotationFiducialsStorageNode *> (node2->CreateNodeInstance());
  if( node1 == nullptr )
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
  // changing to use the display node visibilty, update this to show the
  // control point visibility
  std::cout << "Set annNode visible to 0, GetDisplayVisibility = " << annNode->GetDisplayVisibility() << std::endl;

  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();
  mrmlScene->AddNode(annNode);

  annNode->CreateAnnotationTextDisplayNode();
  if (!annNode->GetAnnotationTextDisplayNode())
    {
    std::cerr << "Error in vtkMRMLAnnotationNode::AnnotationTextDisplayNode() " << std::endl;
    return EXIT_FAILURE;
    }
  double textColor[3] = {0.5, 0.25, 1.0};
  annNode->GetAnnotationTextDisplayNode()->SetColor(textColor);


  annNode->CreateAnnotationPointDisplayNode();
  if (!annNode->GetAnnotationPointDisplayNode())
    {
       std::cerr << "Error in vtkMRMLAnnotationFiducialNode::AnnotationPointDisplayNode() " << std::endl;
       return EXIT_FAILURE;
    }
  double pointColor[3] = {1.0, 0.5, 0.33};
  annNode->GetAnnotationPointDisplayNode()->SetColor(pointColor);

  cout << "AnnotationPointDisplayNode Passed" << endl;

  annNode->Modified();

  mrmlScene->AddNode(node2.GetPointer());
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
  std::cout << "After Write Data, annNode GetDisplayVisibility = " << annNode->GetDisplayVisibility() << std::endl;

  vtkIndent in;
  std::stringstream initialAnnotation, afterAnnotation;
  annNode->PrintAnnotationInfo(initialAnnotation,in);
  annNode->ResetAnnotations();
  if (annNode->GetAnnotationTextDisplayNode())
    {
    annNode->GetAnnotationTextDisplayNode()->SetColor(0.0, 0.0, 0.0);
    }
  if (annNode->GetAnnotationPointDisplayNode())
    {
    annNode->GetAnnotationPointDisplayNode()->SetColor(0.0, 0.0, 0.0);
    }
  std::cout << "After ResetAnnotations, annNode GetDisplayVisibility = " << annNode->GetDisplayVisibility() << std::endl;
  node2->ReadData(annNode);
  std::cout << "After ReadData, annNode GetDisplayVisibility = " << annNode->GetDisplayVisibility() << std::endl;
  annNode->PrintAnnotationInfo(afterAnnotation,in);
  if (initialAnnotation.str().compare(afterAnnotation.str()))
  {
    std::cerr << endl << "Error in WriteData() or ReadData()" << std::endl;
    std::cerr << "Before:" << std::endl << initialAnnotation.str() <<std::endl;
    std::cerr << "After:" << std::endl << afterAnnotation.str() <<std::endl;
    std::cerr << "KP : need to fix annText field ones we have AnnotationFiducials defined" <<std::endl;
    return EXIT_FAILURE;
  }

  // check the colors manually
  if (!annNode->GetAnnotationTextDisplayNode())
    {
    std::cerr << "Error in getting AnnotationTextDisplayNode() after read data" << std::endl;
    return EXIT_FAILURE;
    }
  double afterTextColor[3] = {0.0, 0.0, 0.0};
  annNode->GetAnnotationTextDisplayNode()->GetColor(afterTextColor);
  if (fabs(afterTextColor[0] - textColor[0]) > 0.1 ||
      fabs(afterTextColor[1] - textColor[1]) > 0.1 ||
      fabs(afterTextColor[2] - textColor[2]) > 0.1)
    {
    std::cerr << "Error in retrieving the text display colour after write/read, expected " << textColor[0] << ", " << textColor[1] << ", " << textColor[2] << ", but got " << afterTextColor[0] << ", " << afterTextColor[1] << ", " << afterTextColor[2] << std::endl;
    return EXIT_FAILURE;
    }

  if (!annNode->GetAnnotationPointDisplayNode())
    {
    std::cerr << "Error in getting AnnotationPointDisplayNode() after read data" << std::endl;
    return EXIT_FAILURE;
    }
  double afterPointColor[3] = {0.0, 0.0, 0.0};
  annNode->GetAnnotationPointDisplayNode()->GetColor(afterPointColor);
  if (fabs(afterPointColor[0] - pointColor[0]) > 0.1 ||
      fabs(afterPointColor[1] - pointColor[1]) > 0.1 ||
      fabs(afterPointColor[2] - pointColor[2]) > 0.1)
    {
    std::cerr << "Error in retrieving the point display colour after write/read, expected " << pointColor[0] << ", " << pointColor[1] << ", " << pointColor[2] << ", but got " << afterPointColor[0] << ", " << afterPointColor[1] << ", " << afterPointColor[2] << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;

}
