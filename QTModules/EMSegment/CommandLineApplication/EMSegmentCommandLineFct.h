#include "vtkMRMLEMSAtlasNode.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSGlobalParametersNode.h"
#include "vtkMRMLEMSVolumeCollectionNode.h"
#include "vtkEMSegmentMRMLManager.h"
#include "vtkEMSegmentLogic.h"
#include "vtkSlicerColorLogic.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkImageMathematics.h"
#include "vtkImageAccumulate.h"
#include "vtkSlicerCommonInterface.h"

// -------------------------------------------------------------------------------------------
class ProgressReporter
{
 public:
  void ReportProgress(const std::string& message,
                      float totalProgress = 0.0f,
                      float stageProgress = 0.0f)
  {
    // unused variable
    (void)(message);
    (void)(totalProgress);
    (void)(stageProgress);
  }
};

// -------------------------------------------------------------------------------------------
// does not actually read an image from disk, this is intended for
// creating an image that you will later want to write to
vtkMRMLVolumeNode*
AddNewScalarArchetypeVolume(vtkMRMLScene* mrmlScene,
                            const char* filename,
                            int centerImage,
                            int labelMap,
                            const char* volname)
{
  vtkMRMLScalarVolumeNode  *scalarNode   = vtkMRMLScalarVolumeNode::New();
  scalarNode->SetLabelMap(labelMap);
  vtkMRMLVolumeNode        *volumeNode   = scalarNode;

  // i/o mechanism
  vtkMRMLVolumeArchetypeStorageNode *storageNode =
    vtkMRMLVolumeArchetypeStorageNode::New();
  storageNode->SetFileName(filename);
  storageNode->SetCenterImage(centerImage);

  // set the volume's name
  if (volname == NULL)
    {
      const vtksys_stl::string fname(filename);
      vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
      volumeNode->SetName(name.c_str());
    }
  else
    {
      volumeNode->SetName(volname);
    }

  // add nodes to scene
  mrmlScene->AddNodeNoNotify(storageNode);
  mrmlScene->AddNodeNoNotify(volumeNode);

  volumeNode->SetAndObserveStorageNodeID(storageNode->GetID());

  if (scalarNode)
    {
      scalarNode->Delete();
    }
  if (storageNode)
    {
      storageNode->Delete();
    }
  return volumeNode;
}

// -------------------------------------------------------------------------------------------
// This function checks to see if the image stored in standardFilename
// differs from resultData.  True is returned if the images differ,
// false is returned if they are identical.
bool ImageDiff(vtkImageData* resultData, std::string standardFilename)
{
  bool imagesDiffer = false;

  //
  // read segmentation result standard
  vtkITKArchetypeImageSeriesReader* standardReader =
    vtkITKArchetypeImageSeriesScalarReader::New();
  standardReader->SetArchetype(standardFilename.c_str());
  standardReader->SetOutputScalarTypeToNative();
  standardReader->SetDesiredCoordinateOrientationToNative();
  standardReader->SetUseNativeOriginOn();
  try
    {
      standardReader->Update();
    }
  catch (...)
    {
      std::cerr << "Error: EMSegmenter: Error reading standard image: " << std::endl;
      standardReader->Delete();
      return true;
    }

  //
  // compare image origin and spacing
  for (unsigned int i = 0; i < 3; ++i)
    {
      if (resultData->GetSpacing()[i] !=
          standardReader->GetOutput()->GetSpacing()[i] ||
          resultData->GetOrigin()[i] !=
          standardReader->GetOutput()->GetOrigin()[i])
        {
          //
          // display spacing and origin info for resultData
          std::cerr << "Error: EMSegmenter: Image spacing and/or origin does not match standard!" 
                    << std::endl;
          std::cerr << "Error: EMSegmenter: result origin: " 
                    << resultData->GetOrigin()[0] << " "
                    << resultData->GetOrigin()[1] << " "
                    << resultData->GetOrigin()[2] << std::endl;
          std::cerr << "Error: EMSegmenter: result spacing: " 
                    << resultData->GetSpacing()[0] << " "
                    << resultData->GetSpacing()[1] << " "
                    << resultData->GetSpacing()[2] << std::endl;

          std::cerr << "Error: EMSegmenter: Standard origin: " 
                    << standardReader->GetOutput()->GetOrigin()[0] << " "
                    << standardReader->GetOutput()->GetOrigin()[1] << " "
                    << standardReader->GetOutput()->GetOrigin()[2] << std::endl;
          std::cerr << "Error: EMSegmenter: Standard spacing: " 
                    << standardReader->GetOutput()->GetSpacing()[0] << " "
                    << standardReader->GetOutput()->GetSpacing()[1] << " "
                    << standardReader->GetOutput()->GetSpacing()[2] << std::endl;
          imagesDiffer = true;
        }
    }
  if (!imagesDiffer)
    {
      std::cout << "Result image origin and spacing match." << std::endl;
    }

  //
  // compare image voxels
  vtkImageMathematics* imageDifference = vtkImageMathematics::New();
  imageDifference->SetOperationToSubtract();
  imageDifference->SetInput1(resultData);
  imageDifference->SetInput2(standardReader->GetOutput());

  vtkImageAccumulate* differenceAccumulator = vtkImageAccumulate::New();
  differenceAccumulator->SetInputConnection(imageDifference->GetOutputPort());
  //differenceAccumulator->IgnoreZeroOn();
  differenceAccumulator->Update();

  //imagesDiffer = differenceAccumulator->GetVoxelCount() > 0;
  imagesDiffer =
    differenceAccumulator->GetMin()[0] != 0.0 ||
    differenceAccumulator->GetMax()[0] != 0.0;

  if (imagesDiffer)
    {
      std::cerr << "Error: EMSegmenter: ((temporarily not) ignoring zero) Num / Min / Max / Mean difference = " 
                << differenceAccumulator->GetVoxelCount()  << " / "
                << differenceAccumulator->GetMin()[0]      << " / "
                << differenceAccumulator->GetMax()[0]      << " / "
                << differenceAccumulator->GetMean()[0]     << std::endl;
    }
  else
    {
      std::cout << "Result image voxels match." << std::endl;
    }

  standardReader->Delete();
  imageDifference->Delete();
  differenceAccumulator->Delete();

  return imagesDiffer;
}


// -------------------------------------------------------------------------------------------
int LoadMRMLScene ( vtkEMSegmentMRMLManager* emMRMLManager, vtkMRMLScene* mrmlScene,  int verbose) {
    try
      {
         if (verbose) std::cout << "Reading MRML scene...";  
         mrmlScene->Connect();
         if (verbose) std::cout << "DONE" << std::endl;
      }
    catch (...)
      {
    return 1; 
      }
   
    if (verbose) {
        std::cout << "=============== Print EMSegmentMRMLManager" << std::endl;
        emMRMLManager->Print(std::cout);

        std::cout << "=============== Print PrintVolumeInfo(mrmlScene)" << std::endl;
        emMRMLManager->PrintVolumeInfo(mrmlScene);

        //std::cout << "=============== Print Tree" << std::endl;
        //emMRMLManager->PrintTree();
    }

    int numParameterSets = emMRMLManager->GetNumberOfParameterSets();
    if (verbose) std::cout << "Imported: " << mrmlScene->GetNumberOfNodes()
                           << (mrmlScene->GetNumberOfNodes() == 1
                               ? " node" : " nodes")
                           << ", including " << numParameterSets
                           << " EM parameter "
                           << (numParameterSets == 1 ? "node." : "nodes.")
                           << std::endl;
    return 0;
}

// -------------------------------------------------------------------------------------------
int DefineTargetVolume ( int useDefaultTarget,  std::vector<std::string>  targetVolumeFileNames,vtkEMSegmentLogic* EMSLogic, vtkMRMLScene* mrmlScene,  vtkSlicerApplicationLogic* appLogic, int verbose, bool centered)  {

  vtkEMSegmentMRMLManager* emMRMLManager = EMSLogic->GetMRMLManager();

  if (!useDefaultTarget)
    {
      if (verbose) std::cout << "Adding a target node...";

      // create target node
      vtkMRMLEMSVolumeCollectionNode* targetNode = vtkMRMLEMSVolumeCollectionNode::New();
      mrmlScene->AddNodeNoNotify(targetNode);

      // remove default target node
      mrmlScene->RemoveNode(emMRMLManager->GetTargetInputNode());
   
      // connect target node to segmenter
      emMRMLManager->GetWorkingDataNode()->SetInputTargetNodeID(targetNode->GetID());
   
      if (verbose) std::cout << targetNode->GetID() << " DONE" << std::endl;
   
      targetNode->Delete();
   
      if (verbose) 
        {
          std::cout << "Segmenter's target node is now: " << emMRMLManager->GetTargetInputNode()->GetID() << std::endl;
          std::cout << "Adding " << targetVolumeFileNames.size()  << " target images..." << std::endl;
        }

      for (unsigned int imageIndex = 0; imageIndex < targetVolumeFileNames.size(); ++imageIndex)
        {
          if (verbose) std::cout << "Loading target image " << imageIndex << "..." << std::endl;
          try
            {
              // load image into scene
              vtkMRMLVolumeNode* volumeNode = EMSLogic->AddArchetypeScalarVolume(targetVolumeFileNames[imageIndex].c_str(),targetVolumeFileNames[imageIndex].c_str(), appLogic, mrmlScene, centered);

              if (!volumeNode)
                {
                  throw std::runtime_error("failed to load image.");
                }

              // set volume name and ID in map
              emMRMLManager->GetTargetInputNode()->AddVolume(volumeNode->GetID(), volumeNode->GetID());
            }
          catch(...)
            {
              std::cerr << "ERROR: failed to load target image " << targetVolumeFileNames[imageIndex] << endl;
              return 1;
            }
        }
    }

  // CHECK: make sure the number of input channels matches the expected value in the parameters
  if (emMRMLManager->GetGlobalParametersNode()->GetNumberOfTargetInputChannels() != emMRMLManager->GetTargetInputNode()->GetNumberOfVolumes())
    {
      std::cerr << "ERROR: Number of input channels (" << emMRMLManager->GetTargetInputNode()->GetNumberOfVolumes()
                << ") does not match expected value from parameters (" << emMRMLManager->GetGlobalParametersNode()-> GetNumberOfTargetInputChannels()
                << ")" <<endl;
      return 1;
    }
  else
    {
      if (verbose)
        std::cout << "Number of input channels (" << emMRMLManager->GetTargetInputNode()->GetNumberOfVolumes() << ") matches expected value from parameters (" 
                  << emMRMLManager->GetGlobalParametersNode()->GetNumberOfTargetInputChannels() << ")" << std::endl;
    }
  return 0;
}

// -------------------------------------------------------------------------------------------
int LoadUserDefinedAtlas (std::vector<std::string> atlasVolumeFileNames,  vtkEMSegmentLogic* EMSLogic, vtkMRMLScene* mrmlScene, vtkSlicerApplicationLogic* appLogic, int verbose, bool centered) {

  vtkEMSegmentMRMLManager* emMRMLManager = EMSLogic->GetMRMLManager();
        vtkMRMLEMSAtlasNode* atlasNode = emMRMLManager->GetAtlasInputNode();
        if (!atlasNode)
          {
            std::cerr << "ERROR: parameters must already contain an atlas node if you wish to specify atlas volumes." << endl;
            return 1 ;
          }

        if (int(atlasNode->GetNumberOfVolumes()) != int(atlasVolumeFileNames.size()))
          {
            std::cerr << "ERROR: number of volumes defined by atlasVolumeFileNames ("<< int(atlasVolumeFileNames.size())
                         << ") does not match number of atlas volumes originally defined by template (" << atlasNode->GetNumberOfVolumes() << ")" << endl;
            return 1;
          }

        if (verbose)
          {
            std::cout << "Adding " << atlasVolumeFileNames.size() << " atlas images..." << std::endl;
          }

        for (unsigned int imageIndex = 0; imageIndex < atlasVolumeFileNames.size(); ++imageIndex)
          {
            if (verbose) std::cout << "Loading atlas image " << imageIndex << "..." << std::endl;
            try
              {
                // load image into scene
                vtkMRMLVolumeNode* volumeNode = EMSLogic->AddArchetypeScalarVolume( atlasVolumeFileNames[imageIndex].c_str(), atlasVolumeFileNames[imageIndex].c_str(), appLogic, mrmlScene, centered);
                if (!volumeNode)
                  {
                    std::cerr << "ERROR: failed to load image." << endl;
                    throw std::runtime_error("");
                  }

                // set volume name and ID in map
                // cout << "Updated Node :" << atlasNode->AddVolume( atlasNode->GetNthKey(imageIndex),  volumeNode->GetID() ) << endl;
                atlasNode->SetNthNodeID(imageIndex,volumeNode->GetID());
              }
            catch(...)
              {
                std::cerr << "ERROR: failed to load atlas image " << atlasVolumeFileNames[imageIndex] << endl;
                return 1;
              }
          }

          if (verbose)
          {
            cout << "Done downloading atlases - here is the assignment between class and atlas volumes " << endl;
             // iterate over tree nodes
            for (unsigned int imageIndex = 0; imageIndex < atlasVolumeFileNames.size(); ++imageIndex)
              {
                // This assumes that the keys are defined by the EMSTree node ids - which they were so far !
                // if needed can be made more fancy
                const char* volumeNodeID =  atlasNode->GetNthNodeID(imageIndex);
                vtkMRMLNode* vNode =  mrmlScene->GetNodeByID(volumeNodeID );
                if (vNode)
                  {
                     const char* treeNodeID =  atlasNode->GetNthKey(imageIndex);
                     vtkMRMLNode* tNode = mrmlScene->GetNodeByID(treeNodeID);
                      if (tNode)
                        {
                          cout << "Class: " << setw(30) << std::left << tNode->GetName() <<  " " << vNode->GetName() << " " << volumeNodeID << endl;
                        }
                      else
                        {
                           cout << "AtlasTemplateID: " << setw(30) << std::left << treeNodeID <<  " " << vNode->GetName() <<  " " << volumeNodeID << endl;
                        }
                  }
                else
                  {
                    // then just make the method more fancy so that you first go through all the nodes in the tree - and then look where the index of the corresponding key defined by the spatialnode ID
                    cout << "Could not retrieve class name or volume for: " << atlasVolumeFileNames[imageIndex].c_str() << endl;
                  }
              }
          }
      return 0;
}

// -------------------------------------------------------------------------------------------
int DefineEMSTemplate (int useDefaultParametersNode, std::string parametersMRMLNodeName, vtkEMSegmentMRMLManager* emMRMLManager, int verbose) 
{
    //
    // make sure there is at least one parameter set
    // this also modifies mrml to new mrml structure 
    //
    int numParameterSets = emMRMLManager->GetNumberOfParameterSets();
    if (numParameterSets < 1)
       {
     std::cerr << "ERROR: no EMSegment parameter nodes in scene." << endl;
         return 1;
       }

    //
    // find the parameter set in the MRML scene
    int parameterNodeIndex = 0;
    if (useDefaultParametersNode)
      {
      if (verbose) std::cout << "Using default parameter set named: " << emMRMLManager->GetNthParameterSetName(0) << std::endl;
      }
    else
      {
      // search for the named parameter set
      bool foundParameters = false;
      if (verbose) std::cout << "Searching for an EM parameter node named: "
                             << parametersMRMLNodeName << std::endl;

      for (int i = 0; i < numParameterSets; ++i)
        {
        std::string currentNodeName(emMRMLManager->GetNthParameterSetName(i));
        if (verbose) std::cout << "Node " << i << " name: " << currentNodeName << std::endl;
        if (parametersMRMLNodeName == currentNodeName)
          {
          parameterNodeIndex = i;
          foundParameters = true;
          break;
          }
        else
          {
            if (verbose) std::cout << "Found non-matching EM parameters node: " << currentNodeName << std::endl;
          }
        }

      // make sure the parameters were found
      if (!foundParameters)
        {
      std::cerr << "ERROR: no EMSegment parameters found in scene with name " << parametersMRMLNodeName << endl;
          return 1; 
        }
      }

    //
    // populate the mrmlManager with the parameters
    //  
    try
      {
      emMRMLManager->SetLoadedParameterSetIndex(parameterNodeIndex);
      }
    catch (...)
      {
        return 1;
      }

    return 0;
}

// -------------------------------------------------------------------------------------------
int DefineFinalOutput(  int useDefaultOutput, std::string resultVolumeFileName, vtkEMSegmentMRMLManager* emMRMLManager, vtkMRMLScene* mrmlScene, int verbose) 
{
    if (useDefaultOutput)
      {
        if (!emMRMLManager->GetOutputVolumeNode())
        {
      std::cerr << "ERROR: no default output volume node available." << endl;
      return 1;
        }
        if (verbose)
          std::cout << "Using default output volume node named: " << emMRMLManager->GetOutputVolumeNode()->GetName() << std::endl;
    return 0;
      }

   try
        {
        // create volume node
        if (verbose) std::cout << "Creating output volume node...";
   
        vtkstd::string absolutePath = resultVolumeFileName;
   
        // the gui uses <image>, the command line uses actual files
        //
        // Set up the filename so that a relative filename will be
        // relative to the current directory, not relative to the mrml
        // scene's path.
        if (!vtksys::SystemTools::FileIsFullPath(resultVolumeFileName.c_str()))
          {
          absolutePath = vtksys::SystemTools::
            CollapseFullPath(resultVolumeFileName.c_str());
          }
   
        vtkMRMLVolumeNode* outputNode =
          AddNewScalarArchetypeVolume(mrmlScene,
                                      absolutePath.c_str(),
                                      true,
                                      true,
                                      NULL);
   
        if (!outputNode)
          {
           std::cerr << "ERROR: failed to create output image" << endl;
            return 1;
          }
   
          // connect output volume node to segmenter
          emMRMLManager->SetOutputVolumeMRMLID(outputNode->GetID());
   
         if (verbose)  std::cout << "DONE" << std::endl;
        }
      catch (...)
        {
         std::cerr << "ERROR: failed to add result volume node." << endl;
         return 1;
        }
      return 0;
}

// -------------------------------------------------------------------------------------------
int RunPreprocessing(vtkEMSegmentLogic* EMSLogic, std::string EMSLogicTcl, std::string emMRMLManagerTcl, vtkSlicerCommonInterface* slicerCommon, vtkEMSegmentMRMLManager* emMRMLManager, int verbose)
{
   try
       {
         if (verbose) std::cout << "Starting preprocessing ..." << std::endl;

         emMRMLManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(0);
         emMRMLManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(0);


         if (EMSLogic->SourcePreprocessingTclFiles())
           {
             throw std::runtime_error("ERROR: could not source tcl files. ");
           }

        // Have to init variables again bc first time EMLogic was not fully set up
        std::string CMD = "::EMSegmenterPreProcessingTcl::InitVariables " + EMSLogicTcl + " " + emMRMLManagerTcl + " NULL";
        if (atoi(slicerCommon->EvaluateTcl(CMD.c_str())))
        {
           throw std::runtime_error("ERROR: could not init files. ");
        }

         if (atoi(slicerCommon->EvaluateTcl("::EMSegmenterPreProcessingTcl::Run")))
           {
             throw std::runtime_error("ERROR: Pre-processing did not execute correctly");
           }
       }
      catch (std::runtime_error& e)
      {
          std::cerr << e.what() << std::endl;
          std::cerr << "Errors detected.  Writing failed." << std::endl;
          return 1;  
      }

      emMRMLManager->GetWorkingDataNode()->SetAlignedTargetNodeIsValid(1);
      emMRMLManager->GetWorkingDataNode()->SetAlignedAtlasNodeIsValid(1);

      if (verbose) std::cout << "EMSEG: Preprocessing complete." << std::endl;
      return 0;
}

// -------------------------------------------------------------------------------------------
bool WriteResultsToFile(int disableCompression, vtkEMSegmentMRMLManager* emMRMLManager, int verbose)
{
   
     if (verbose) std::cout << "Saving segmentation results..." << std::endl;
    try
      {
      vtkstd::cout << "Writing segmentation result: " <<
        emMRMLManager->GetOutputVolumeNode()->GetStorageNode()->GetFileName()
                   << vtkstd::endl;
      emMRMLManager->GetOutputVolumeNode()->GetStorageNode()->
        SetUseCompression(!disableCompression);
      emMRMLManager->GetOutputVolumeNode()->GetStorageNode()->
        WriteData(emMRMLManager->GetOutputVolumeNode());
      }
    catch (std::runtime_error& e)
      {
         std::cerr << e.what() << std::endl;
         std::cerr << "Errors detected.  Writing failed." << std::endl;
         return false;
      }
    catch (...)
      {
         std::cerr << "Unknown error detected.  Writing failed." << std::endl;
         return  false;
      }
  if (verbose) std::cout << "DONE" << std::endl;

  return true;
}


// -------------------------------------------------------------------------------------------
bool CompareResultsToStandard(std::string resultStandardVolumeFileName, int disableCompression, vtkEMSegmentMRMLManager* emMRMLManager, vtkMRMLScene* mrmlScene, int verbose)
{
   if (verbose) cout << "Comparing results with standard..." << std::endl;

    try
      {

      // get a pointer to the results
      std::string resultMRMLID = emMRMLManager->GetOutputVolumeMRMLID();
      std::cout << "Extracting results from mrml node: " << resultMRMLID << std::endl;
      vtkImageData* resultImage = NULL;
      vtkMRMLVolumeNode* node   = vtkMRMLVolumeNode::
        SafeDownCast(mrmlScene->GetNodeByID(resultMRMLID.c_str()));
      resultImage = node->GetImageData();
      resultImage->SetSpacing(node->GetSpacing());
      resultImage->SetOrigin(node->GetOrigin());

      // compare result with standard image
      bool imagesDiffer = ImageDiff(resultImage, resultStandardVolumeFileName);
      if (imagesDiffer)
        {
            if (verbose) cerr << "Result DOES NOT match standard!" << std::endl;
            return false;
        }
      else
        {
             if (verbose) cout << "Result matches standard!" << std::endl;
             return true;
        }
      }
    catch (std::runtime_error& e)
      {
        std::cerr << e.what() << std::endl;
        std::cerr << "Errors detetected.  Comparison failed." << std::endl;
        return false;
      }
    catch (...)
      {
         std::cerr << "Unknown error detected.  Comparison failed." << std::endl;
         return false ;
      }

  return true;
}

// -------------------------------------------------------------------------------------------
void GenerateEmptyMRMLScene(const char* filename)
{
  // create scene, logic, and add parameters to scene
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  mrmlScene->SetURL(filename);

  vtkEMSegmentLogic* emLogic             = vtkEMSegmentLogic::New();
  emLogic->SetModuleName("EMSegment");
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->RegisterMRMLNodesWithScene();

  vtkIntArray *emsEvents                 = vtkIntArray::New();
  emsEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  emsEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  emLogic->SetAndObserveMRMLSceneEvents(mrmlScene, emsEvents);
  emsEvents->Delete();

  emLogic->GetMRMLManager()->CreateAndObserveNewParameterSet();

  // write the scene
  try
    {
      mrmlScene->Commit();
    }
  catch (...)
    {
      std::cerr << "ERROR: could not write mrml scene." << std::endl;
    }

  // clean up
  mrmlScene->Clear(true);
  mrmlScene->Delete();
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();
}



