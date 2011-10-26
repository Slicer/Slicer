/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.na-mic.org/NAMICSandBox/trunk/IGTLoadableModules/FourDImage/vtkFourDImageLogic.h $
  Date:      $Date: 2009-02-03 12:05:00 -0500 (Tue, 03 Feb 2009) $
  Version:   $Revision: 3633 $

==========================================================================*/

// .NAME vtkFourDImageLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for
// IGT. 


#ifndef __vtkFourDImageLogic_h
#define __vtkFourDImageLogic_h

#include "vtkFourDImageWin32Header.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkCallbackCommand.h"
#include "vtkDoubleArray.h"

#include "vtkMRMLSliceNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLTimeSeriesBundleNode.h"

#include "itkImage.h"
#include "itkImageSeriesReader.h"

#include <string>
#include <map>


class vtkIGTLConnector;

class VTK_FourDImage_EXPORT vtkFourDImageLogic : public vtkSlicerModuleLogic 
{
 public:
  enum {  // Events
    //LocatorUpdateEvent    = 50000,
    StatusUpdateEvent       = 50001,
    ProgressDialogEvent     = 50002,
  };
  enum {
    TYPE_MEAN,
    TYPE_SD,
  };
  typedef struct {
    int    show;
    double progress;
    std::string message;
  } StatusMessageType;

  typedef std::vector<std::string> FrameNodeVectorType;
  typedef struct {
    int x;
    int y;
    int z;
  } CoordType;

  typedef std::vector<CoordType>                       IndexTableType;

  typedef struct {
    unsigned long    MaskModifiedTime;
    vtkDoubleArray*  Mean;
    vtkDoubleArray*  SD;
  } CurveDataType;
  
  typedef std::map<int, CurveDataType> CurveDataListType;
  typedef struct {
    std::string       MaskNodeID;
    CurveDataListType CurveList;
  } CurveDataSetType;

  //typedef std::map<std::string, CurveDataSetType> CurveCacheType;

  typedef std::map<std::string, std::string> RegistrationParametersType;

 public:
  
  static vtkFourDImageLogic *New();
  
  vtkTypeRevisionMacro(vtkFourDImageLogic,vtkObject);
  void PrintSelf(ostream&, vtkIndent);

  // Description:
  // Loads series of volumes from the directory that contains the file
  // specified by 'path' argument.
  // Returns number of volumes in the series.

  //const int SpaceDim = 3;
  typedef short PixelValueType;
  typedef itk::Image< PixelValueType, 3 > VolumeType;
  typedef itk::ImageSeriesReader< VolumeType > ReaderType;

  // Description:
  // Used to sort volumes into bundles based on
  // DICOM header information.
  int SortFilesFromDirToCreateFileList ( const char* path,
                                         std::vector<ReaderType::FileNamesContainer>& fileNamesContainerList);
  // Description:
  // No longer used. replaced by SortFilesFromDirToCreateFileList() method
  int CreateFileListFromDir(const char* path,
                            std::vector<ReaderType::FileNamesContainer>& fileNamesContainerList);
  // Description:
  // Used to sort volumes into bundles based on user input
  void CreateFileListFromDir(const char* path,
                             std::vector<ReaderType::FileNamesContainer>& fileNamesContainerList,
                             const char* order, const char* filter, int nFrames, int nSlices, int nChannels, int channel);
  vtkMRMLTimeSeriesBundleNode* LoadImagesByList(const char* bundleNodeName,
                                                std::vector<ReaderType::FileNamesContainer>& fileNamesContainerList);

  vtkMRMLTimeSeriesBundleNode* LoadImagesFromDir(const char* path, const char* bundleNodeName);
  vtkMRMLTimeSeriesBundleNode* LoadImagesFromDir(const char* path, const char* bundleNodeName,
                                                 const char* order, const char* filter,
                                                 int nFrames, int nSlices, int nChannels, int channel);

  int SaveImagesToDir(const char* path, const char* bundleID, const char* prefix, const char* suffix);

  vtkMRMLScalarVolumeNode* AddDisplayBufferNode(vtkMRMLTimeSeriesBundleNode* bundleNode, 
                                                int index);
  void UpdateDisplayBufferNode(vtkMRMLTimeSeriesBundleNode* bundleNode, int index);
  int         GetNumberOfFrames();
  const char* GetFrameNodeID(int index);
  const char* GetRegisteredFrameNodeID(int index);

  void SetApplication(vtkSlicerApplication *app) { this->Application = app; };
  vtkSlicerApplication* GetApplication() { return this->Application; };
  //int  RunSeriesRegistration(int sIndex, int eIndex, int kIndex, RegistrationParametersType& param);

  int GenerateBundleFrames(vtkMRMLTimeSeriesBundleNode* inputBundleNode,
                           vtkMRMLTimeSeriesBundleNode* outputBundleNode);


  // Description:
  // This method is used to handle NULL pointers.
  // If there's a null time series bundle node,
  // this method creates one using the same
  // mechanisms as the node selector widget
  // before any operations on the node are
  // performed.
  vtkMRMLTimeSeriesBundleNode* AddNewFourDBundleNode ();
  
  //----------------------------------------------------------------
  // Editor functions
  //----------------------------------------------------------------
  //void AddFrame(const char* bundleID, int index, const char* nodeID);
  //void DeleteFrame();

 protected:
  
  vtkFourDImageLogic();
  ~vtkFourDImageLogic();

  void operator=(const vtkFourDImageLogic&);
  vtkFourDImageLogic(const vtkFourDImageLogic&);

  // TBD: not used
  //virtual void ProcessLogicEvents(vtkObject * caller,
  //                                unsigned long event,
  //                               void * callData);
  using vtkSlicerLogic::ProcessLogicEvents;

  static void DataCallback(vtkObject*, unsigned long, void *, void *);
  void UpdateAll();

  vtkSlicerApplication *Application;
  vtkCallbackCommand *DataCallbackCommand;

 private:

  FrameNodeVectorType FrameNodeVector;
  FrameNodeVectorType RegisteredFrameNodeVector;
  std::string VolumeBundleID;
  std::string RegisteredVolumeBundleID;

  //CurveCacheType CurveCache;  // CurveCache[<4d bundle name>][<label number>].<member of CurveDataType>

};

#endif


  
