#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentTestUtilities.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSTargetNode.h"
#include <stdexcept>
#include <stdlib.h>

int main(int argc, char** argv)
{
  std::cerr << "Starting EM preprocessing manager test..." << std::endl;

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
      std::string 
        currentNodeName(emLogic->GetMRMLManager()->GetNthParameterSetName(i)); 
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

    bool localPass = true;

    // start preprocessing
    std::cerr << "Preprocessing: Initialize Input...";
    emLogic->StartPreprocessingInitializeInputData();
    
    // should start with only input target node
    vtkMRMLEMSTargetNode* inTargetNode = 
      m->GetWorkingDataNode()->GetInputTargetNode();
    vtkMRMLEMSTargetNode* normTargetNode = 
      m->GetWorkingDataNode()->GetNormalizedTargetNode();
    vtkMRMLEMSTargetNode* alignedTargetNode = 
      m->GetWorkingDataNode()->GetAlignedTargetNode();
    //vtkMRMLEMSTargetNode* workingTargetNode =
    //  m->GetWorkingDataNode()->GetWorkingTargetNode();

    if (inTargetNode == NULL)
      {
      std::cerr << "No input target node" << std::endl;
      pass = false;
      localPass = false;
      }
//     if (inTargetNode != workingTargetNode)
//       {
//       std::cerr 
//         << "Input target node should be working target node, but isn't" 
//         << std::endl;
//       pass = false;
//       localPass = false;
//       }
    if (inTargetNode->GetNumberOfVolumes() != 2)
      {
      std::cerr 
        << "Input target node has wrong number of volumes" 
        << inTargetNode->GetNumberOfVolumes() << " != 2"
        << std::endl;
      pass = false;
      localPass = false;        
      }
    if (normTargetNode || alignedTargetNode)
      {
      std::cerr 
        << "Normed or aligned target node is non-null, should be null" 
        << std::endl;
      pass = false;
      localPass = false;
      }
    std::cerr << (localPass ? "OK" : "FAILED") << std::endl;
    localPass = true;

    // intensity normalization
    emLogic->StartPreprocessingTargetIntensityNormalization();
      
    inTargetNode = 
      m->GetWorkingDataNode()->GetInputTargetNode();
    normTargetNode = 
      m->GetWorkingDataNode()->GetNormalizedTargetNode();
    alignedTargetNode = 
      m->GetWorkingDataNode()->GetAlignedTargetNode();
      
    if (inTargetNode == NULL)
      {
      std::cerr << "No input target node" << std::endl;
      pass = false;
      localPass = false;
      }
    if (normTargetNode == NULL)
      {
      std::cerr << "No normed target node" << std::endl;
      pass = false;
      localPass = false;
      }

    if (alignedTargetNode)
      {
      std::cerr 
        << "Aligned target node is non-null, should be null" 
        << std::endl;
      pass = false;
      localPass = false;
      }
    if (inTargetNode->GetNumberOfVolumes() != 2)
      {
      std::cerr 
        << "Input target node has wrong number of volumes" 
        << inTargetNode->GetNumberOfVolumes() << " != 2"
        << std::endl;
      pass = false;
      localPass = false;        
      }
    if (normTargetNode->GetNumberOfVolumes() != 2)
      {
      std::cerr 
        << "Normalized target node has wrong number of volumes" 
        << normTargetNode->GetNumberOfVolumes() << " != 2"
        << std::endl;
      pass = false;
      localPass = false;        
      }
    if (m->GetTargetNumberOfSelectedVolumes() != 2)
      {
      std::cerr 
        << "Segmenter knows wrong number of target volumes" 
        << m->GetTargetNumberOfSelectedVolumes() << " != 2"
        << std::endl;
      pass = false;
      localPass = false;        
      }
    std::cerr << (localPass ? "OK" : "FAILED") << std::endl;
    localPass = true;

    // target-to-target alignment
    emLogic->StartPreprocessingTargetToTargetRegistration();
      
    inTargetNode = 
      m->GetWorkingDataNode()->GetInputTargetNode();
    normTargetNode = 
      m->GetWorkingDataNode()->GetNormalizedTargetNode();
    alignedTargetNode = 
      m->GetWorkingDataNode()->GetAlignedTargetNode();
      
    if (inTargetNode == NULL)
      {
      std::cerr << "No input target node" << std::endl;
      pass = false;
      localPass = false;
      }
    if (normTargetNode == NULL)
      {
      std::cerr << "No normed target node" << std::endl;
      pass = false;
      localPass = false;
      }
    if (alignedTargetNode == NULL)
      {
      std::cerr << "No aligned target node" << std::endl;
      pass = false;
      localPass = false;
      }

    if (inTargetNode->GetNumberOfVolumes() != 2)
      {
      std::cerr 
        << "Input target node has wrong number of volumes" 
        << inTargetNode->GetNumberOfVolumes() << " != 2"
        << std::endl;
      pass = false;
      localPass = false;        
      }
    if (normTargetNode->GetNumberOfVolumes() != 2)
      {
      std::cerr 
        << "Normalized target node has wrong number of volumes" 
        << normTargetNode->GetNumberOfVolumes() << " != 2"
        << std::endl;
      pass = false;
      localPass = false;        
      }
    if (alignedTargetNode->GetNumberOfVolumes() != 2)
      {
      std::cerr 
        << "Aligned target node has wrong number of volumes" 
        << alignedTargetNode->GetNumberOfVolumes() << " != 2"
        << std::endl;
      pass = false;
      localPass = false;        
      }
    if (m->GetTargetNumberOfSelectedVolumes() != 2)
      {
      std::cerr 
        << "Segmenter knows wrong number of target volumes" 
        << m->GetTargetNumberOfSelectedVolumes() << " != 2"
        << std::endl;
      pass = false;
      localPass = false;        
      }
    std::cerr << (localPass ? "OK" : "FAILED") << std::endl;
    localPass = true;

    //
    // start actual segmentation
    emLogic->StartSegmentation();

    if (m->GetTargetNumberOfSelectedVolumes() != 2)
      {
      std::cerr 
        << "Segmenter knows wrong number of target volumes" 
        << m->GetTargetNumberOfSelectedVolumes() << " != 2"
        << std::endl;
      pass = false;
      localPass = false;        
      }
    std::cerr << (localPass ? "OK" : "FAILED") << std::endl;
    localPass = true;
    
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

  
