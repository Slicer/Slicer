#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentTestUtilities.h"

int main(int argc, char** argv)
{
  std::cerr << "Starting EM read/write MRML test..." << std::endl;
  std::string mrmlSceneFilenameInput;
  std::string mrmlSceneFilenameOutput;
  bool success = true;

  //
  // parse command line
  if (argc < 3)
  {
    std::cerr 
      << "Usage: vtkEMSegmentReadWriteMRMLTest"      << std::endl
      <<         "<mrmlSceneFilenameInput>"          << std::endl
      <<         "<mrmlSceneFilenameOutput>"         << std::endl
      << std::endl;
    return EXIT_FAILURE;
  }

  mrmlSceneFilenameInput           = argv[1];
  mrmlSceneFilenameOutput          = argv[2];

  //
  // create a mrml scene that will hold the data parameters
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  mrmlScene->SetURL(mrmlSceneFilenameInput.c_str());

  //
  // create an instance of vtkEMSegmentLogic and connect it with the
  // MRML scene
  vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->RegisterMRMLNodesWithScene();

  try 
  {
    mrmlScene->Import();
    mrmlScene->Commit(mrmlSceneFilenameOutput.c_str());
  }
  catch (...)
  {
    std::cerr << "Error: " << std::endl;
    success = false;
  }

  if (success)
    {
      std::cerr << "Read/Wrote: " << mrmlScene->GetNumberOfNodes()
                << " nodes." << std::endl;
    }

  // clean up
  mrmlScene->Clear(true);
  mrmlScene->Delete();
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();
  
  return success ? EXIT_SUCCESS : EXIT_FAILURE;    
}

