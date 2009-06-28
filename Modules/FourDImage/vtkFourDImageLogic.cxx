/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.na-mic.org/NAMICSandBox/trunk/IGTLoadableModules/FourDImage/vtkFourDImageLogic.cxx $
  Date:      $Date: 2009-02-03 12:05:00 -0500 (Tue, 03 Feb 2009) $
  Version:   $Revision: 3633 $

==========================================================================*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkSlicerColorLogic.h"
#include "vtkFourDImageLogic.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"

#include "itkMetaDataDictionary.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkNumericSeriesFileNames.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
#include "itkImageFileWriter.h"
#include "itkTimeProbesCollectorBase.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLBSplineTransformNode.h"
#include "itksys/DynamicLoader.hxx"

#include "vtkMRMLTimeSeriesBundleNode.h"

#include "vtkGlobFileNames.h"

#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerVolumesLogic.h"

#include <cmath>

vtkCxxRevisionMacro(vtkFourDImageLogic, "$Revision: 3633 $");
vtkStandardNewMacro(vtkFourDImageLogic);

//---------------------------------------------------------------------------
vtkFourDImageLogic::vtkFourDImageLogic()
{

  // Timer Handling

  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkFourDImageLogic::DataCallback);

  this->FrameNodeVector.clear();

  this->VolumeBundleID = "";
  this->RegisteredVolumeBundleID = "";

}


//---------------------------------------------------------------------------
vtkFourDImageLogic::~vtkFourDImageLogic()
{

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

}


//---------------------------------------------------------------------------
void vtkFourDImageLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkFourDImageLogic:             " << this->GetClassName() << "\n";

}

//----------------------------------------------------------------------------
void vtkFourDImageLogic::ProcessLogicEvents(vtkObject *caller, 
                                            unsigned long event, 
                                            void *callData)
{
  if (event ==  vtkCommand::ProgressEvent) 
    {
    this->InvokeEvent ( vtkCommand::ProgressEvent,callData );
    }
}

//---------------------------------------------------------------------------
void vtkFourDImageLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
  vtkFourDImageLogic *self = reinterpret_cast<vtkFourDImageLogic *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkFourDImageLogic DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkFourDImageLogic::UpdateAll()
{
}


//---------------------------------------------------------------------------
int vtkFourDImageLogic::CreateFileListFromDir(const char* path,
                                             std::vector<ReaderType::FileNamesContainer>& fileNamesContainerList)
{

  typedef itk::OrientedImage< PixelValueType, 2 > SliceType;
  typedef itk::ImageFileReader< SliceType > SliceReaderType;
  typedef itk::GDCMImageIO ImageIOType;
  typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;
  typedef itk::ImageFileWriter< VolumeType >  WriterType;
  //typedef itk::VectorImage< PixelValueType, SpaceDim > NRRDImageType;
  typedef itk::VectorImage< PixelValueType, 3 > NRRDImageType;
  
  fileNamesContainerList.clear();
  
  StatusMessageType statusMessage;
  
  //itk::TimeProbesCollectorBase collector;

  statusMessage.show = 1;
  statusMessage.progress = 0.0;
  statusMessage.message = "Checking directory....";
  this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
  
  int nVolumes;

  // Search files with compatible types (except DICOM)
  vtkGlobFileNames* gfn = vtkGlobFileNames::New();
  gfn->SetDirectory(path);
  gfn->AddFileNames("*.nhdr");
  gfn->AddFileNames("*.nrrd");
  gfn->AddFileNames("*.hdr");
  gfn->AddFileNames("*.img");
  gfn->AddFileNames("*.nii");
  gfn->AddFileNames("*.nia");

  int nFiles = gfn->GetNumberOfFileNames();
  if (nFiles > 0)
    {
    //std::cerr << "find non-dicom files" << std::endl;
    for (int i = 0; i < nFiles; i ++)
      {
      ReaderType::FileNamesContainer container;
      container.clear();
      //std::cerr << "FileName #" << i << " " << gfn->GetNthFileName(i) << std::endl;
      container.push_back(gfn->GetNthFileName(i));
      fileNamesContainerList.push_back(container);
      }
    nVolumes = nFiles;  // ??
    }
  else
    {
    // in case of dicom series
    ImageIOType::Pointer gdcmIO = ImageIOType::New();
    gdcmIO->LoadPrivateTagsOn();
    
    InputNamesGeneratorType::Pointer inputNames = InputNamesGeneratorType::New();
    inputNames->SetUseSeriesDetails(true);
    inputNames->SetDirectory(path);
    
    itk::SerieUIDContainer seriesUIDs = inputNames->GetSeriesUIDs();
    if (seriesUIDs.size() == 1)  // if single series UID is used
      {
      statusMessage.message = "Splitting series....";
      this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
      
      const ReaderType::FileNamesContainer & filenames = 
        inputNames->GetFileNames(seriesUIDs[0]);
      
      ReaderType::Pointer reader = ReaderType::New();
      reader->SetImageIO( gdcmIO );
      reader->SetFileNames( filenames );
      
      try
        {
        //collector.Start( "Checking series ...." );
        reader->Update();
        //collector.Stop( "Checking series ...." );
        }
      catch (itk::ExceptionObject &excp)
        {
        std::cerr << "Exception thrown while reading the series" << std::endl;
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
        }
      ReaderType::DictionaryArrayRawPointer inputDict = reader->GetMetaDataDictionaryArray();
      int nSlices = inputDict->size();
      
      // search "cycle" of slice location
      std::string tag;
      tag.clear();
      itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|1041",  tag);
      float firstSliceLocation = atof( tag.c_str() ); // first slice location
      int nSlicesInVolume;
      
      statusMessage.progress = 0.0;
      statusMessage.message = "Checking slice locations...";
      this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
      
      for (int i = 1; i < nSlices; i ++)
        {
        statusMessage.progress = (double)i/(double)nSlices;
        this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
        
        tag.clear();
        itk::ExposeMetaData<std::string> ( *(*inputDict)[i], "0020|1041",  tag);
        float sliceLocation = atof( tag.c_str() );
        //std::cerr << "location = " << tag.c_str() << std::endl;
        if (sliceLocation == firstSliceLocation)
          {
          nSlicesInVolume = i;
          break;
          }
        }
      /*
        std::cerr << "Number of slices in Volume is " << nSlicesInVolume << std::endl;
        std::cerr << "Number of slices " << nSlices << std::endl;
        std::cerr << "Number of slices " << filenames.size() << std::endl;
      */
      
      nVolumes = nSlices / nSlicesInVolume;
      fileNamesContainerList.resize(nVolumes);
      
      for (int i = 0; i < nVolumes; i ++)
        {
        fileNamesContainerList[i].resize(nSlicesInVolume);
        for (int j = 0; j < nSlicesInVolume; j ++)
          {
          //std::cerr << "fileNamesContainerList " << i << ", " << j << std::endl;
          fileNamesContainerList[i][j] = filenames[i*nSlicesInVolume + j];
          }
        }
      
      }
    else // if the directory contains multiple series UIDs
      {
      nVolumes = seriesUIDs.size();
      //fileNamesContainerList.resize(nVolumes);
      fileNamesContainerList.clear();
      
      itk::SerieUIDContainer::iterator iter;
      for (iter = seriesUIDs.begin(); iter != seriesUIDs.end(); iter ++)
        {
        //std::cerr << "UID = " << *iter << std::endl;
        fileNamesContainerList.push_back(inputNames->GetFileNames(*iter));
        }
      }
    }

  return nVolumes;
}


//---------------------------------------------------------------------------
vtkMRMLTimeSeriesBundleNode* vtkFourDImageLogic::LoadImagesFromDir(const char* path, const char* bundleNodeName)
{
  StatusMessageType statusMessage;
  double rangeLower;
  double rangeUpper;
  
  std::cerr << "loading from " << path << std::endl;

  std::vector<ReaderType::FileNamesContainer> fileNamesContainerList;

  // Analyze the directory and create the file list
  if (CreateFileListFromDir(path, fileNamesContainerList) <= 0)
    {
    std::cerr << "Couldn't find files" << std::endl;
    return NULL;
    }

  int nVolumes = fileNamesContainerList.size();
  std::cerr << "nVolumes = " << nVolumes << std::endl;


  //std::vector<ReaderType::FileNamesContainer>::iterator fnciter;
  //for (fnciter = fileNamesContainerList.begin(); fnciter != fileNamesContainerList.end(); fnciter ++)

  //vtkMRMLScene* scene = vtkMRMLScene::New();
  vtkMRMLScene* scene = this->GetMRMLScene();
  scene->SaveStateForUndo();
  this->FrameNodeVector.clear();

  statusMessage.show = 1;
  statusMessage.progress = 0.0;
  statusMessage.message = "Reading Volumes....";
  this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);

  rangeLower = 0.0;
  rangeUpper = 0.0;

  // prepare 4D bundle
  vtkMRMLTimeSeriesBundleNode* bundleNode = vtkMRMLTimeSeriesBundleNode::New();
  //vtkMRMLLinearTransformNode* bundleNode = vtkMRMLLinearTransformNode::New();
  //bundleNode->SetName("TimeSeriesBundle");
  bundleNode->SetName(bundleNodeName);
  bundleNode->SetDescription("Created by FourDImage");

  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  transform->Identity();
  bundleNode->ApplyTransform(transform);
  transform->Delete();
  scene->AddNode(bundleNode);

  for (int i = 0; i < nVolumes; i ++)
    {
    //std::cerr << "i = " << i << std::endl;

    statusMessage.progress = (double)i / (double)nVolumes;
    this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);

    vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::New();
    vtkMRMLVolumeArchetypeStorageNode*storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
    vtkMRMLScalarVolumeDisplayNode* displayNode = vtkMRMLScalarVolumeDisplayNode::New();

    //storageNode->SetFileName(filename);
    char nodeName[128];
    std::cerr << "filename = " << fileNamesContainerList[i][0].c_str() << std::endl;
    ReaderType::FileNamesContainer::iterator fnciter;
    storageNode->SetFileName(fileNamesContainerList[i][0].c_str());
    storageNode->ResetFileNameList();
    for (fnciter = fileNamesContainerList[i].begin(); fnciter != fileNamesContainerList[i].end(); fnciter ++)
      {
      storageNode->AddFileName(fnciter->c_str());
      }
    storageNode->SetSingleFile(0);
    //storageNode->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);
    storageNode->ReadData(volumeNode);

    sprintf(nodeName, "Vol_%03d", i);
    volumeNode->SetName(nodeName);

    volumeNode->SetScene(scene);
    storageNode->SetScene(scene);
    displayNode->SetScene(scene);

    double range[2];
    volumeNode->GetImageData()->GetScalarRange(range);
    /*
      range[0] = 0.0;
      range[1] = 256.0;
    */
    if (range[0] < rangeLower) rangeLower = range[0];
    if (range[1] > rangeUpper) rangeUpper = range[1];
    displayNode->SetAutoWindowLevel(0);
    displayNode->SetAutoThreshold(0);
    displayNode->SetLowerThreshold(range[0]);
    displayNode->SetUpperThreshold(range[1]);
    displayNode->SetWindow(range[1] - range[0]);
    displayNode->SetLevel(0.5 * (range[1] + range[0]) );
    vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
    displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
    colorLogic->Delete();

    scene->AddNode(displayNode);  
    scene->AddNode(storageNode);  
    volumeNode->SetAndObserveStorageNodeID(storageNode->GetID());
    volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
    scene->AddNode(volumeNode);  
    this->FrameNodeVector.push_back(std::string(volumeNode->GetID()));

    // Add to 4D bundle
    volumeNode->SetAndObserveTransformNodeID(bundleNode->GetID());
    volumeNode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
    bundleNode->AddFrame(volumeNode->GetID());

    volumeNode->Delete();
    storageNode->Delete();
    displayNode->Delete();
    }

  AddDisplayBufferNode(bundleNode, 0);
  AddDisplayBufferNode(bundleNode, 1);

  statusMessage.show = 0;
  this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
  
  return bundleNode;
}


//---------------------------------------------------------------------------
int vtkFourDImageLogic::SaveImagesToDir(const char* path,
                                       const char* bundleID,
                                       const char* prefix,
                                       const char* suffix)
{
  vtkMRMLTimeSeriesBundleNode* bundleNode 
    = vtkMRMLTimeSeriesBundleNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(bundleID));

  if (!bundleNode)
    {
    return 0;
    }

  StatusMessageType statusMessage;
  statusMessage.show = 1;
  statusMessage.progress = 0.0;
  statusMessage.message = "Writing Volumes....";
  this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);

  int nFrames = bundleNode->GetNumberOfFrames();
  for (int i = 0; i < nFrames; i ++)
    {
    statusMessage.progress = (double)i / (double) nFrames;
    this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);

    vtkMRMLScalarVolumeNode* inode
      = vtkMRMLScalarVolumeNode::SafeDownCast(bundleNode->GetFrameNode(i));
    if (inode)
      {
      vtkSlicerApplication* app = this->GetApplication();
      vtkSlicerVolumesGUI* volumesGUI = 
        vtkSlicerVolumesGUI::SafeDownCast(app->GetModuleGUIByName ("Volumes"));
      vtkSlicerVolumesLogic* volumesLogic = volumesGUI->GetLogic(); 

      char fileName[256];
      sprintf(fileName, "%s/%s_%03d.%s", path, prefix, i, suffix);
      std::cerr << "writing " << fileName << std::endl;
      volumesLogic->SaveArchetypeVolume(fileName, inode);
      }
    }

  statusMessage.show = 0;
  this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);

  return 1;
}


//---------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkFourDImageLogic::AddDisplayBufferNode(vtkMRMLTimeSeriesBundleNode* bundleNode, int index)
{
  vtkMRMLScene* scene = this->GetMRMLScene();

  // Create Display Buffer Node
  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::New();
  //vtkMRMLVolumeArchetypeStorageNode*storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
  vtkMRMLScalarVolumeDisplayNode* displayNode = vtkMRMLScalarVolumeDisplayNode::New();
  

  volumeNode->SetScene(scene);
  //storageNode->SetScene(scene);
  displayNode->SetScene(scene);

  vtkImageData* imageData = vtkImageData::New();
  vtkMRMLScalarVolumeNode *firstFrameNode 
    = vtkMRMLScalarVolumeNode::SafeDownCast(bundleNode->GetFrameNode(0));

  if (firstFrameNode && firstFrameNode->GetImageData())
    {
    volumeNode->Copy(firstFrameNode);
    volumeNode->SetAndObserveTransformNodeID(NULL);
    vtkImageData* firstImageData = firstFrameNode->GetImageData();
    imageData->ShallowCopy(firstImageData);
    }
  
  // J. Tokuda -- Jan 26, 2009: The display buffer node is placed outside
  // the bundle to allow the users to make a label map with the buffer image.
  volumeNode->SetAndObserveImageData(imageData);


  char nodeName[128];
  sprintf(nodeName, "%s_Display%d", bundleNode->GetName(), index);
  volumeNode->SetName(nodeName);
  
  double rangeLower = 0.0;
  double rangeUpper = 0.0;

  double range[2];
  volumeNode->GetImageData()->GetScalarRange(range);
  if (range[0] < rangeLower) rangeLower = range[0];
  if (range[1] > rangeUpper) rangeUpper = range[1];
  displayNode->SetAutoWindowLevel(0);
  displayNode->SetAutoThreshold(0);
  displayNode->SetLowerThreshold(range[0]);
  displayNode->SetUpperThreshold(range[1]);
  displayNode->SetWindow(range[1] - range[0]);
  displayNode->SetLevel(0.5 * (range[1] - range[0]) );
  vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
  displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
  colorLogic->Delete();
  scene->AddNode(displayNode);  
  //scene->AddNode(storageNode);  
  //volumeNode->SetAndObserveStorageNodeID(storageNode->GetID());
  volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  scene->AddNode(volumeNode);  
  //this->FrameNodeVector.push_back(std::string(volumeNode->GetID()));

  bundleNode->SetDisplayBufferNodeID(index, volumeNode->GetID());
  
  //volumeNode->Delete();
  //storageNode->Delete();
  displayNode->Delete();
  return volumeNode;

}


//---------------------------------------------------------------------------
int vtkFourDImageLogic::GenerateBundleFrames(vtkMRMLTimeSeriesBundleNode* inputBundleNode,
                                                vtkMRMLTimeSeriesBundleNode* outputBundleNode)
{
  int nfInput  = inputBundleNode->GetNumberOfFrames();
  int nfOutput = outputBundleNode->GetNumberOfFrames();

  if (nfInput == nfOutput)
    {
    // nothing to do
    return 0;
    }

  if (nfOutput > nfInput)
    {
    for (int i = nfOutput-1; i >= nfInput; i --)
      {
      outputBundleNode->RemoveFrame(i);
      }
    return -1;
    }

  // need to create new frames

  int nVolumes = nfInput - nfOutput;

  StatusMessageType statusMessage;
  statusMessage.show = 1;
  statusMessage.progress = 0.0;
  statusMessage.message = "Allocating image nodes....";
  this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);

  vtkMRMLScene* scene = this->GetMRMLScene();
  for (int i = 0; i < nVolumes; i ++)
    {
    statusMessage.progress = (double)i / (double)nVolumes;
    this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);

    vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::New();
    vtkMRMLScalarVolumeDisplayNode* displayNode = vtkMRMLScalarVolumeDisplayNode::New();
    //vtkMRMLVolumeArchetypeStorageNode*storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
    char nodeName[128];

    sprintf(nodeName, "RegVol_%03d", i);
    vtkMRMLScalarVolumeNode *inFrameNode 
      = vtkMRMLScalarVolumeNode::SafeDownCast(inputBundleNode->GetFrameNode(nfOutput+i));
    if (inFrameNode)
      {
      volumeNode->Copy(inFrameNode);
      vtkImageData* inImageData = inFrameNode->GetImageData();
      vtkImageData* imageData = vtkImageData::New();
      imageData->DeepCopy(inImageData);
      volumeNode->SetAndObserveImageData(imageData);
      }
    volumeNode->SetName(nodeName);
    volumeNode->SetScene(scene);
    displayNode->SetScene(scene);

    vtkImageData* imageData = vtkImageData::New();
    volumeNode->SetAndObserveImageData(imageData);
    
    vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
    displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
    
    //scene->AddNode(storageNode);  
    scene->AddNode(displayNode);  
    volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
    scene->AddNode(volumeNode);  
    //this->RegisteredFrameNodeVector.push_back(std::string(volumeNode->GetID()));

    volumeNode->SetAndObserveTransformNodeID(outputBundleNode->GetID());
    volumeNode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
    outputBundleNode->AddFrame(volumeNode->GetID());
    
    volumeNode->Delete();
    colorLogic->Delete();
    displayNode->Delete();
    }

  if (!outputBundleNode->GetDisplayBufferNode(0))
    {
    AddDisplayBufferNode(outputBundleNode, 0);
    }
  if (!outputBundleNode->GetDisplayBufferNode(1))
    {
    AddDisplayBufferNode(outputBundleNode, 1);
    }

  statusMessage.show = 0;
  this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
  
  return 1;
}


//---------------------------------------------------------------------------
int vtkFourDImageLogic::GetNumberOfFrames()
{
  return this->FrameNodeVector.size();
}

//---------------------------------------------------------------------------
const char* vtkFourDImageLogic::GetFrameNodeID(int index)
{
  if (index >= 0 && index < (int)this->FrameNodeVector.size())
    {
    return this->FrameNodeVector[index].c_str();
    }
  else
    {
    return NULL;
    }
}

//---------------------------------------------------------------------------
const char* vtkFourDImageLogic::GetRegisteredFrameNodeID(int index)
{
  if (index >= 0 && index < (int)this->RegisteredFrameNodeVector.size())
    {
    return this->RegisteredFrameNodeVector[index].c_str();
    }
  else
    {
    return NULL;
    }
}

