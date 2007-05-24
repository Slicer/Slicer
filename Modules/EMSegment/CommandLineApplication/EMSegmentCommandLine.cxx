#include <iostream>
#include <vector>
#include <string>

#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkMRMLEMSSegmenterNode.h"
#include "EMSegmentCommandLineCLP.h"
#include <vtksys/SystemTools.hxx>
#include <stdexcept>

int main(int argc, char** argv)
{
  //
  // parse arguments using the CLP system; this creates variables.
  PARSE_ARGS;

  bool useDefaultParametersNode = parametersMRMLNodeName.empty();
  bool useDefaultTarget         = targetVolumeFileNames.empty();
  bool useDefaultOutput         = resultVolumeFileName.empty();
  bool segmentationSucceeded    = true;

  //
  // make sure arguments are sufficient and unique
  bool argsOK = true;
  if (mrmlSceneFileName.empty())
  {
    std::cerr << "Error: mrmlSceneFileName must be specified." << std::endl;
    argsOK = false;
  }
  if (!argsOK)
  {
    std::cerr << "Try --help for usage..." << std::endl;
    exit(EXIT_FAILURE);
  }

  //
  // make sure files exist
  if (!vtksys::SystemTools::FileExists(mrmlSceneFileName.c_str()))
  {
    std::cerr << "Error: MRML scene file does not exist." << std::endl;
    exit(EXIT_FAILURE);
  }

  //
  // create a mrml scene that will hold the parameters and data
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  mrmlScene->SetURL(mrmlSceneFileName.c_str());

  //
  // create an instance of vtkEMSegmentLogic and connect it with the
  // MRML scene
  vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->RegisterMRMLNodesWithScene();

  //
  // global try block makes sure data is cleaned up if anything goes
  // wrong
  try 
  {
    //
    // read the mrml scene
    try 
    {
      if (verbose) std::cerr << "Reading MRML scene...";
      mrmlScene->Connect();
      if (verbose) std::cerr << "DONE" << std::endl;
    }
    catch (...)
    {
      throw std::runtime_error("ERROR: failed to import mrml scene.");
    }

    int numParameterSets = emLogic->GetNumberOfParameterSets();
    if (verbose) std::cerr << "Imported: " << mrmlScene->GetNumberOfNodes()
                           << (mrmlScene->GetNumberOfNodes() == 1 
                               ? " node" : " nodes")
                           << ", including " << numParameterSets 
                           << " EM parameter "
                           << (numParameterSets == 1 ? "node." : "nodes.")
                           << std::endl;
  
    //
    // make sure there is at least one parameter set
    if (numParameterSets < 1)
    {
      throw std::
        runtime_error("ERROR: no EMSegment parameter nodes in scene.");
    }

    //
    // find the parameter set in the MRML scene
    int parameterNodeIndex = 0;
    if (useDefaultParametersNode)
    {
      if (verbose) std::cerr << "Using default parameter set named: " 
                             << emLogic->GetNthParameterSetName(0) 
                             << std::endl;    
    }
    else
    {
      // search for the named parameter set
      bool foundParameters = false;
      if (verbose) std::cerr << "Searching for an EM parameter node named: " 
                             << parametersMRMLNodeName << std::endl;
      
      for (int i = 0; i < numParameterSets; ++i)
      {
        std::string currentNodeName(emLogic->GetNthParameterSetName(i)); 
        if (verbose) std::cerr << "Node " << i 
                               << " name: " << currentNodeName << std::endl;
        if (parametersMRMLNodeName == currentNodeName)
        {
          parameterNodeIndex = i;
          foundParameters = true;
          break;
        }
        else
        {
          if (verbose) std::cerr << "Found non-matching EM parameters node: " 
                                 << currentNodeName << std::endl;
        }
      }
      
      // make sure the parameters were found
      if (!foundParameters)
      {
        throw std::
          runtime_error("ERROR: EMSegment parameters absent from scene.");
      }
    }

    //
    // populate the logic with the parameters
    try
    {
      emLogic->SetLoadedParameterSetIndex(parameterNodeIndex);
    }
    catch (...)
    {
      throw std::runtime_error("ERROR: failed to set EMSegment parameters.");
    }

    //
    // make sure the basic parameters are available
    // !!!
    // segmenter node
    // global parameters

    //
    // set the target images
    if (useDefaultTarget)
    {
      if (!emLogic->GetSegmenterNode()->GetTargetNode())
      {
        throw std::runtime_error("ERROR: no default target node available.");
      }
    }
    else
    {
      try 
      {
        // create target node
        vtkMRMLEMSTargetNode* targetNode = vtkMRMLEMSTargetNode::New();
        mrmlScene->AddNode(targetNode);        

        // connect target node to segmenter
        emLogic->GetSegmenterNode()->SetTargetNodeID(targetNode->GetID());
      }
      catch (...)
      {
        throw std::runtime_error("ERROR: failed to add target node.");
      }

      for (unsigned int imageIndex = 0; 
           imageIndex < targetVolumeFileNames.size(); ++imageIndex)
      {
        if (verbose) std::cerr << "Loading target image " << imageIndex
                               << "...";
        try
        {
          // load image into scene
          // emLogic->AddTargetSelectedVolumeByMRMLID()
        }
        catch(...)
        {
          throw std::runtime_error("ERROR: failed to load target image.");
        }
      }
    }

    //
    // set the result labelmap image

    //
    // update logic parameters from command line
    emLogic->SetEnableMultithreading(!disableMultithreading);
    if (verbose) 
      std::cerr << "Multithreading is " 
                << (disableMultithreading ? "disabled." : "enabled.")
                << std::endl;

    int segmentationBoundaryMin[3];
    int segmentationBoundaryMax[3];
    emLogic->GetSegmentationBoundaryMin(segmentationBoundaryMin);
    emLogic->GetSegmentationBoundaryMax(segmentationBoundaryMax);
    if (verbose) std::cerr 
      << "ROI is [" 
      << segmentationBoundaryMin[0] << ", "
      << segmentationBoundaryMin[1] << ", "
      << segmentationBoundaryMin[2] << "] -->> ["
      << segmentationBoundaryMax[0] << ", "
      << segmentationBoundaryMax[1] << ", "
      << segmentationBoundaryMax[2] << "]" << std::endl;

    //
    // check parameters' node structure
    if (!emLogic->CheckMRMLNodeStructure())
    {
      throw std::
        runtime_error("ERROR: EMSegment invalid parameter node structure");
    }

    //
    // run the segmentation
    try
    {
      if (verbose) std::cerr << "Starting segmentation..." << std::endl;
      emLogic->StartSegmentation();
      if (verbose) std::cerr << "Segmentation complete." << std::endl;
    }
    catch (...)
    {
      throw std::runtime_error("ERROR: failed to run segmentation.");
    }

    //
    // save the results in the MRML scene
  }
  catch (std::runtime_error& e)
  {
    std::cerr << "Errors detetected.  Segmentation failed." << std::endl;
    segmentationSucceeded = false;
  }
  catch (...)
  {
    std::cerr << "Unknown error detected.  Segmentation failed." << std::endl;
    segmentationSucceeded = false;
  }
  
  //
  // clean up
  if (verbose) std::cerr << "Cleaning up...";
  mrmlScene->Clear(true);
  mrmlScene->Delete();
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();
  if (verbose) std::cerr << "DONE" << std::endl;

  return segmentationSucceeded ? EXIT_SUCCESS : EXIT_FAILURE;  
}
