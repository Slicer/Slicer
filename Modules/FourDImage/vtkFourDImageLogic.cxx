/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
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
#include <vector>
#include <sstream>

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
/* NOT USED
//----------------------------------------------------------------------------
void vtkFourDImageLogic::ProcessLogicEvents(vtkObject *vtkNotUsed(caller), 
                                            unsigned long event, 
                                            void *callData)
{
  if (event ==  vtkCommand::ProgressEvent) 
    {
    this->InvokeEvent ( vtkCommand::ProgressEvent,callData );
    }
}
*/

//---------------------------------------------------------------------------
void vtkFourDImageLogic::DataCallback(vtkObject *vtkNotUsed(caller), 
                                       unsigned long vtkNotUsed(eid),
                                       void *clientData, void *vtkNotUsed(callData))
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
vtkMRMLTimeSeriesBundleNode* vtkFourDImageLogic::AddNewFourDBundleNode (  )
{

 vtkMRMLNode *node = NULL;
 vtkMRMLNode *retNode = NULL;

    node = this->MRMLScene->CreateNodeByClass( "vtkMRMLTimeSeriesBundleNode" );
    if (node == NULL)
      {
      return NULL;
      }

    node->SetScene(this->MRMLScene);

    std::stringstream ss;
    const char *name = "TimeSeriesBundle";
    ss << this->MRMLScene->GetUniqueNameByString(name);
    node->SetName(ss.str().c_str());
    retNode = this->MRMLScene->AddNode(node);
    node->Delete();
    return ( vtkMRMLTimeSeriesBundleNode::SafeDownCast(retNode) );
}



//---------------------------------------------------------------------------
int vtkFourDImageLogic::SortFilesFromDirToCreateFileList ( const char *path,
                                                           std::vector<ReaderType::FileNamesContainer>& fileNamesContainerList)
{
  typedef itk::Image< PixelValueType, 2 > SliceType;
  typedef itk::ImageFileReader< SliceType > SliceReaderType;
  typedef itk::GDCMImageIO ImageIOType;
  typedef itk::GDCMSeriesFileNames InputNamesGeneratorType;
  typedef itk::ImageFileWriter< VolumeType >  WriterType;
  typedef itk::VectorImage< PixelValueType, 3 > NRRDImageType;
  
  fileNamesContainerList.clear();
  StatusMessageType statusMessage;
  
  statusMessage.show = 1;
  statusMessage.progress = 0.0;
  statusMessage.message = "Checking directory....";
  this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
  
  //--- Things to convert DICOM tags into
  //--- FourDImage sorting parameters.
  int numberOfVolumesInBundle;
  int bundleIdentifier;
  
  // Search files with compatible types (except DICOM)
  vtkGlobFileNames* gfn = vtkGlobFileNames::New();
  gfn->SetDirectory(path);
  gfn->AddFileNames("*.nhdr");
  gfn->AddFileNames("*.nrrd");
  gfn->AddFileNames("*.hdr");
  gfn->AddFileNames("*.mha");
  gfn->AddFileNames("*.mhd");
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
    numberOfVolumesInBundle = nFiles;  // ??
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
      statusMessage.message = "Splitting series.... getting file names.";
      this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
      
      const ReaderType::FileNamesContainer & filenames = inputNames->GetFileNames(seriesUIDs[0]);
      
      ReaderType::Pointer reader = ReaderType::New();
      reader->SetImageIO( gdcmIO );
      reader->SetFileNames( filenames );
      
      try
        {
        statusMessage.message = "Splitting series.... updating reader.";
        this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
        reader->Update();
        }
      catch (itk::ExceptionObject &excp)
        {
        statusMessage.message = "Error encountered: exiting.";
        this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);

        std::cerr << "Exception thrown while reading the series" << std::endl;
        std::cerr << excp << std::endl;
        return EXIT_FAILURE;
        }

      //--- get DICOM header info
      statusMessage.message = "Splitting series.... getting DICOM metadata.";
      this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
      ReaderType::DictionaryArrayRawPointer inputDict = reader->GetMetaDataDictionaryArray();
      int nSlices = inputDict->size();
      
      //--- pull out some important information for sorting.
      std::string tag;
      tag.clear();

      //--- 
      //--- Use DICOM header info to help sorting if possible.
      //--- 
      // NumberOfTemporalPositions
      itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|0105",  tag);
      if ( tag.c_str() != NULL && strcmp (tag.c_str(), "" ))
        {
        //--- using DICOM metadata for "Number of Temporal Positions"
        numberOfVolumesInBundle = atoi ( tag.c_str() ); 

        statusMessage.progress = 0.0;
        statusMessage.message = "Sorting slices into volume bundles...";
        this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);

        //--- keep track of which volume has had room for 
        //--- its file list allocated in the fileNamesContainerList.
        //--- Each sub-vector is a list of elements containing the sliceLocation
        //--- for a corresponding volume in fileNamesContainerList.
        //--- The sorting of these by sliceLocation guides the sorting
        //--- of files in the fileNameList. (or maybe we don't have to.
        fileNamesContainerList.resize(numberOfVolumesInBundle);

        //--- for each slice...check its header information
        //--- and put its filename in the proper bundle container.
        //--- here assume that filename[i] order is same as inputDict[i] info order.
        //--- !!! check this assumption!!!!
        int bundle;
        std::stringstream ss;
        for (int i = 0; i < nSlices; i ++)
          {
          //--- to which volume does slice belong?
          tag.clear();
          itk::ExposeMetaData<std::string> ( *(*inputDict)[i], "0020|0100",  tag);
          bundleIdentifier = atoi( tag.c_str() );
        
          //--- this filenamelist for this volume isn't yet allocated.
          bundle = bundleIdentifier - 1;
          fileNamesContainerList[bundle].push_back(filenames[i]);        
          }
        }
      else
        {
        // Slice Location
        itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|1041",  tag);
        float firstSliceLocation = atof( tag.c_str() ); // first slice location
        int nSlicesInVolume = 0;
      
        statusMessage.message = "No explicit DICOM timepoints; sorting by slice locations...";
        this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
        statusMessage.progress = 0.0;
        for (int i = 1; i < nSlices; i ++)
        {
        statusMessage.progress = (double)i/(double)nSlices;
        this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
        
        tag.clear();
        // Slice Location for this slice
        itk::ExposeMetaData<std::string> ( *(*inputDict)[i], "0020|1041",  tag);
        float sliceLocation = atof( tag.c_str() );
        //std::cerr << "location = " << tag.c_str() << std::endl;
        if (sliceLocation == firstSliceLocation)
          {
          nSlicesInVolume = i;
          break;
          }
        }
      
        if (nSlicesInVolume > 0 )
          {
        numberOfVolumesInBundle = nSlices / nSlicesInVolume;
          }
        else
          {
          numberOfVolumesInBundle = 0;
          }
        
        fileNamesContainerList.resize(numberOfVolumesInBundle);
      
        for (int i = 0; i < numberOfVolumesInBundle; i ++)
          {
          fileNamesContainerList[i].resize(nSlicesInVolume);
          for (int j = 0; j < nSlicesInVolume; j ++)
            {
            //std::cerr << "fileNamesContainerList " << i << ", " << j << std::endl;
            fileNamesContainerList[i][j] = filenames[i*nSlicesInVolume + j];
            }
          }
        }
      }
    else // if the directory contains multiple series UIDs
      {
      numberOfVolumesInBundle = seriesUIDs.size();
      fileNamesContainerList.clear();
      
      itk::SerieUIDContainer::iterator iter;
      for (iter = seriesUIDs.begin(); iter != seriesUIDs.end(); iter ++)
        {
        //std::cerr << "UID = " << *iter << std::endl;
        fileNamesContainerList.push_back(inputNames->GetFileNames(*iter));
        }
      }
    }
  return numberOfVolumesInBundle;
}




//---------------------------------------------------------------------------
//--- no longer used;
//---  SortFilesFromDirToCreateFileList ( ... ) used instead.
//---------------------------------------------------------------------------
int vtkFourDImageLogic::CreateFileListFromDir(const char* path,
                                             std::vector<ReaderType::FileNamesContainer>& fileNamesContainerList)
{

  typedef itk::Image< PixelValueType, 2 > SliceType;
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
  gfn->AddFileNames("*.mha");
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
      
      const ReaderType::FileNamesContainer & filenames = inputNames->GetFileNames(seriesUIDs[0]);
      
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

      //--- get DICOM header info
      ReaderType::DictionaryArrayRawPointer inputDict = reader->GetMetaDataDictionaryArray();
      int nSlices = inputDict->size();
      
      // search "cycle" of slice location
      std::string tag;
      tag.clear();
        // Slice Location
      itk::ExposeMetaData<std::string> ( *(*inputDict)[0], "0020|1041",  tag);
      float firstSliceLocation = atof( tag.c_str() ); // first slice location
      int nSlicesInVolume = 1;
      
      statusMessage.progress = 0.0;
      statusMessage.message = "Checking slice locations...";
      this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
      
      for (int i = 1; i < nSlices; i ++)
        {
        statusMessage.progress = (double)i/(double)nSlices;
        this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
        
        tag.clear();
        // Slice Location
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
void vtkFourDImageLogic::CreateFileListFromDir(const char* path,
                                               std::vector<ReaderType::FileNamesContainer>& fileNamesContainerList,
                                               const char* order, const char* filter,
                                               int nFrames, int nSlices, int nChannels, int channel)
{

  // Search files with compatible types (except DICOM)
  vtkGlobFileNames* gfn = vtkGlobFileNames::New();
  gfn->SetDirectory(path);
  gfn->AddFileNames(filter);

  // Get and check number of files in the directory
  int nFiles = gfn->GetNumberOfFileNames();

  if (nFiles < nFrames*nSlices*nChannels)
    {
    std::cerr << "The number of files is not enough." << std::endl;
    fileNamesContainerList.clear();
    return;
    }
  
  int tOffset = 1;  // increment for time point index
  int cOffset = 1;  // increment for channel index
  int sOffset = 1;  // increment for slicer index

  if (strcmp(order, "T-S-C") == 0)
    {
    tOffset = nSlices*nChannels;
    sOffset = nChannels;
    cOffset = 1;
    }
  else if (strcmp(order, "T-C-S") == 0)
    {
    tOffset = nSlices*nChannels;
    sOffset = 1;
    cOffset = nSlices;
    }
  else if (strcmp(order, "S-T-C") == 0)
    {
    tOffset = nChannels;
    sOffset = nChannels*nFrames;
    cOffset = 1;
    }
  else if (strcmp(order, "S-C-T") == 0)
    {
    tOffset = 1;
    sOffset = nChannels*nFrames;
    cOffset = nFrames;
    }
  else if (strcmp(order, "C-T-S") == 0)
    {
    tOffset = nSlices;
    sOffset = 1;
    cOffset = nFrames*nSlices;
    }
  else if (strcmp(order, "C-S-T") == 0)
    {
    tOffset = 1;
    sOffset = nFrames;
    cOffset = nFrames*nSlices;
    }

  fileNamesContainerList.clear();
  for (int f = 0; f < nFrames; f ++)
    {
    ReaderType::FileNamesContainer container;
    container.clear();
    for (int s = 0; s < nSlices; s ++)
      {
      //int index = f * (nSlices*nChannels) + channel*(nSlices) + s;
      int index = f * tOffset + channel*cOffset + s*sOffset;
      if (index < nFiles)
        {
        container.push_back(gfn->GetNthFileName(index));
        }
      else
        {
        std::cerr << "Error: creating file list." << std::endl;
        return;
        }
      }
    fileNamesContainerList.push_back(container);
    }
}


//---------------------------------------------------------------------------
vtkMRMLTimeSeriesBundleNode* vtkFourDImageLogic::LoadImagesByList(const char* bundleNodeName,
                                                                  std::vector<ReaderType::FileNamesContainer>& fileNamesContainerList)
{
  
  StatusMessageType statusMessage;
  double rangeLower;
  double rangeUpper;
  
  // Analyze the directory and create the file list
  if (fileNamesContainerList.size() <= 0)
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

  /*
  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  transform->Identity();
  bundleNode->ApplyTransform(transform);
  transform->Delete();
  */
  scene->AddNode(bundleNode);
  bundleNode->Delete();


  // Time stamp
  // Tentatively, we assume that the frame rate is 1 fps.
  // (We should obtain time stamps from the files.)
  const int fps = 1;
  vtkMRMLTimeSeriesBundleNode::TimeStamp ts;
  ts.second = 0;
  ts.nanosecond = 0;

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
    bundleNode->SetTimeStamp(i, &ts);
    ts.second += fps;

    volumeNode->Delete();
    storageNode->Delete();
    displayNode->Delete();
    }

  if (bundleNode && bundleNode->GetDisplayBufferNode(0) == NULL)
    {
    AddDisplayBufferNode(bundleNode, 0);
    }
  else
    {
    UpdateDisplayBufferNode(bundleNode, 0);
    }
  if (bundleNode && bundleNode->GetDisplayBufferNode(1) == NULL)
    {
    AddDisplayBufferNode(bundleNode, 1);
    }
  else
    {
    UpdateDisplayBufferNode(bundleNode, 1);
    }

  statusMessage.show = 0;
  this->InvokeEvent ( vtkFourDImageLogic::ProgressDialogEvent, &statusMessage);
  
  return bundleNode;
}


//---------------------------------------------------------------------------
vtkMRMLTimeSeriesBundleNode* vtkFourDImageLogic::LoadImagesFromDir(const char* path, const char* bundleNodeName)
{
  std::cerr << "loading from " << path << std::endl;

  std::vector<ReaderType::FileNamesContainer> fileNamesContainerList;


  if ( SortFilesFromDirToCreateFileList ( path, fileNamesContainerList) <= 0 )
    {
    std::cerr << "Couldn't find files" << std::endl;
    return NULL;
    }

  return LoadImagesByList(bundleNodeName, fileNamesContainerList);
  
}


//---------------------------------------------------------------------------
vtkMRMLTimeSeriesBundleNode* vtkFourDImageLogic::LoadImagesFromDir(const char* path, const char* bundleNodeName,
                                                                   const char* order, const char* filter,
                                                                   int nFrames, int nSlices, int nChannels, int channel)
{
  std::cerr << "loading from " << path << std::endl;

  std::vector<ReaderType::FileNamesContainer> fileNamesContainerList;

  CreateFileListFromDir(path, fileNamesContainerList, order, filter, nFrames, nSlices, nChannels, channel);
  return LoadImagesByList(bundleNodeName, fileNamesContainerList);

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
  else 
    // if there is no frame in the bundle node (this likely happens
    // when the node has been created from the node selector.)
    {
    vtkImageData* image = vtkImageData::New();
    image->SetDimensions(256, 256, 1);
    image->SetExtent(0, 255, 0, 255, 0, 0 );
    image->SetSpacing(1.0, 1.0, 1.0);
    image->SetOrigin(0.0, 0.0, 0.0);
    image->SetNumberOfScalarComponents(1);
    image->SetScalarTypeToShort();
    image->AllocateScalars();
    image->Update();
    volumeNode->SetAndObserveImageData(image);
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
void vtkFourDImageLogic::UpdateDisplayBufferNode(vtkMRMLTimeSeriesBundleNode* bundleNode, int index)
{
  if (!bundleNode)
    {
    return;
    }
//  vtkMRMLScene* scene = this->GetMRMLScene();

  // Get display and volume node
  vtkMRMLScalarVolumeNode *volumeNode
    = vtkMRMLScalarVolumeNode::SafeDownCast(bundleNode->GetDisplayBufferNode(index));
  vtkMRMLScalarVolumeDisplayNode* displayNode 
    = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(volumeNode->GetDisplayNode());
    
  vtkImageData* imageData = volumeNode->GetImageData();
  vtkMRMLScalarVolumeNode *firstFrameNode 
    = vtkMRMLScalarVolumeNode::SafeDownCast(bundleNode->GetFrameNode(0));

  if (firstFrameNode && firstFrameNode->GetImageData())
    {
    volumeNode->Copy(firstFrameNode);
    volumeNode->SetAndObserveTransformNodeID(NULL);
    vtkImageData* firstImageData = firstFrameNode->GetImageData();
    imageData->ShallowCopy(firstImageData);
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
    
    volumeNode->Modified();
    displayNode->Modified();
    }

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

