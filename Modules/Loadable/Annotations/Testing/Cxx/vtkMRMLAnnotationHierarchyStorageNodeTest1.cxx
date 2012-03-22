#include "vtkMRMLAnnotationHierarchyStorageNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLScene.h"
#include "vtkURIHandler.h"

#include <sstream>

#include <vtksys/SystemTools.hxx>

#include "vtkMRMLCoreTestingMacros.h"

int vtkMRMLAnnotationHierarchyStorageNodeTest1(int argc, char * argv[] )
{
  vtkSmartPointer< vtkMRMLAnnotationHierarchyStorageNode > node1 = vtkSmartPointer< vtkMRMLAnnotationHierarchyStorageNode >::New();
  EXERCISE_BASIC_OBJECT_METHODS( node1 );
  EXERCISE_BASIC_STORAGE_MRML_METHODS(vtkMRMLAnnotationHierarchyStorageNode, node1);

  vtkSmartPointer< vtkMRMLAnnotationHierarchyStorageNode > node2 = vtkSmartPointer< vtkMRMLAnnotationHierarchyStorageNode >::New();
  vtkSmartPointer< vtkMRMLAnnotationHierarchyNode > annNode = vtkSmartPointer< vtkMRMLAnnotationHierarchyNode >::New();
  vtkSmartPointer< vtkMRMLScene> scene =  vtkSmartPointer<vtkMRMLScene>::New();
  scene->AddNode(node2);
  scene->AddNode(annNode);

  const char *tempDir = NULL;
  if (argc > 4)
    {
    tempDir = argv[4];
    std::cout << "Temp dir = " << tempDir << std::endl;
    }

  // Test reading
  std::string fileName = "Fiducials List.acsv";
  if (tempDir)
    {
    std::vector<std::string> pathComponents;
    vtksys::SystemTools::SplitPath(tempDir,pathComponents);
    pathComponents.push_back(fileName);
    fileName = vtksys::SystemTools::JoinPath(pathComponents);
    }
  std::cout << "Attempting to read from file " << fileName.c_str() << std::endl;
  node2->SetFileName(fileName.c_str());

  int retval = node2->ReadData(annNode);
  if (!retval)
    {
    return EXIT_FAILURE;
    }

  int numNodes = scene->GetNumberOfNodesByClass("vtkMRMLAnnotationFiducialNode");
  if (numNodes == 0)
    {
    std::cerr << "Failed to read in any fiducial nodes from the file " << node2->GetFileName() << std::endl;
    return EXIT_FAILURE;
    }
  else
    {
    std::cout << "Read in " << numNodes << " fiducial nodes from the file " << node2->GetFileName() << std::endl;
    }
  return EXIT_SUCCESS;
}
