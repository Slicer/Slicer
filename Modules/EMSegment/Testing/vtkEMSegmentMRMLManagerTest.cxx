#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentTestUtilities.h"
#include <stdexcept>

#define vtkTestSetGetMacro(pass, obj, var, val, index)           \
std::cerr << "Testing Set/Get " #var "...";                      \
obj->Set##var (index, val);                                      \
pass = pass && (val == obj->Get##var(index));                    \
std::cerr << (val==obj->Get##var(index) ? "OK" : "FAILED") << std::endl;

int main(int argc, char** argv)
{
  std::cerr << "Starting EM mrml manager test..." << std::endl;

  std::string mrmlSceneFilename           = argv[1];
  std::string parametersNodeName          = argv[2];

#ifdef WIN32
  //
  // strip backslashes from parameter node name (present if spaces were used)
  std::string tmpNodeName = parametersNodeName;
  parametersNodeName.clear();
  for (unsigned int i = 0; i < tmpNodeName.size(); ++i)
    {
      if (tmpNodeName[i] != '\\')
        {
        parametersNodeName.push_back(tmpNodeName[i]);
        }
      else if (i > 0 && tmpNodeName[i-1] == '\\')
        {
        parametersNodeName.push_back(tmpNodeName[i]);
        }
    }
#endif

  bool pass = true;
  int returnValue = EXIT_SUCCESS;

  //
  // create a mrml scene that will hold the data parameters
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  mrmlScene->SetURL(mrmlSceneFilename.c_str());

  //
  // create an instance of vtkEMSegmentLogic and connect it with the
  // MRML scene
  vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->RegisterMRMLNodesWithScene();
  vtkIntArray *emsEvents                 = vtkIntArray::New();
  emsEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  emsEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  emLogic->SetAndObserveMRMLSceneEvents(mrmlScene, emsEvents);
  emsEvents->Delete();

  try 
    {
    try 
      {
      mrmlScene->Import();
      std::cerr << "Imported: " << mrmlScene->GetNumberOfNodes()
                << " nodes." << std::endl;
      }
    catch (...)
      {
      std::cerr << "Error reading/setting mrml scene: " << std::endl;
      throw;
      }

    //
    // populate the logic class with testing data
    int numParameterSets = emLogic->GetMRMLManager()->
      GetNumberOfParameterSets();
    std::cerr << "Found " << numParameterSets << " EM top level nodes."
              << std::endl;
    bool foundParameters = false;
    std::cerr << "Searching for an EM parameter node named: " 
              << parametersNodeName << std::endl;

    for (int i = 0; i < numParameterSets; ++i)
      {
      std::string currentNodeName(emLogic->GetMRMLManager()->GetNthParameterSetName(i)); 
      std::cerr << "Node " << i << " name: " << currentNodeName << std::endl;
      if (parametersNodeName == currentNodeName)
        {
        try
          {
          emLogic->GetMRMLManager()->SetLoadedParameterSetIndex(i);
          }
        catch (...)
          {
          std::cerr << "Error setting parameter set: " << std::endl;
          throw;
          }
        foundParameters = true;
        break;
        }
      else
        {
        std::cerr << "Found non-matching EM parameters node: " 
                  << currentNodeName << std::endl;
        }
      }

    if (!foundParameters)
      {
      std::cerr << "Error: parameters not found in scene" << std::endl;
      throw std::runtime_error("Parameters Not Found!");
      }

    vtkEMSegmentMRMLManager* m = emLogic->GetMRMLManager();

    // normalization parameters
    int numTargets = m->GetTargetNumberOfSelectedVolumes();
    vtkIdType targetID = m->GetTargetSelectedVolumeNthID(numTargets-1);
    
    vtkTestSetGetMacro(pass, m, 
                       NthTargetVolumeIntensityNormalizationNormValue, 
                       3.14159, numTargets-1);
    vtkTestSetGetMacro(pass, m, 
                         TargetVolumeIntensityNormalizationNormValue,
                         3.14159, targetID);
      
      vtkTestSetGetMacro(pass, m,
                         NthTargetVolumeIntensityNormalizationNormType,
                         5, numTargets-1);
      vtkTestSetGetMacro(pass, m,
                         TargetVolumeIntensityNormalizationNormType,
                         5, targetID);

      vtkTestSetGetMacro(pass, m,
                         NthTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth,
                         5, numTargets-1);
      vtkTestSetGetMacro(pass, m,
                         TargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth,
                         5, targetID);

      vtkTestSetGetMacro(pass, m,
                         NthTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth,
                         5, numTargets-1);
      vtkTestSetGetMacro(pass, m,
                         TargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth,
                         5, targetID);

      vtkTestSetGetMacro(pass, m,
                         NthTargetVolumeIntensityNormalizationRelativeMaxVoxelNum,
                         5, numTargets-1);
      vtkTestSetGetMacro(pass, m,
                         TargetVolumeIntensityNormalizationRelativeMaxVoxelNum,
                         5, targetID);

      vtkTestSetGetMacro(pass, m,
                         NthTargetVolumeIntensityNormalizationPrintInfo,
                         5, numTargets-1);
      vtkTestSetGetMacro(pass, m,
                         TargetVolumeIntensityNormalizationPrintInfo,
                         5, targetID);

      vtkTestSetGetMacro(pass, m,
                         NthTargetVolumeIntensityNormalizationEnabled,
                         5, numTargets-1);
      vtkTestSetGetMacro(pass, m,
                         TargetVolumeIntensityNormalizationEnabled,
                         5, targetID);

    }
  catch(...)
    {
    returnValue = EXIT_FAILURE;
    }
  
  // clean up
  mrmlScene->Clear(true);
  mrmlScene->Delete();
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();
  
  return (pass ? returnValue : EXIT_FAILURE);
}

