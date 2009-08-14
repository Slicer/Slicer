/*==========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.na-mic.org/NAMICSandBox/trunk/IGTLoadableModules/FourDAnalysis/vtkFourDAnalysisLogic.h $
  Date:      $Date: 2009-02-03 12:05:00 -0500 (Tue, 03 Feb 2009) $
  Version:   $Revision: 3633 $

==========================================================================*/

// .NAME vtkFourDAnalysisLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for
// IGT. 


#ifndef __vtkFourDAnalysisLogic_h
#define __vtkFourDAnalysisLogic_h

#include "vtkFourDAnalysisWin32Header.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkCallbackCommand.h"
#include "vtkDoubleArray.h"

#include "vtkMRMLSliceNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLTimeSeriesBundleNode.h"
#include "vtkMRMLCurveAnalysisNode.h"


#include "itkOrientedImage.h"
#include "itkImageSeriesReader.h"

#include "vtkIntensityCurves.h"
#include "vtkCurveAnalysisPythonInterface.h"

#include <string>
#include <map>


class vtkIGTLConnector;
class vtkMutexLock;

class VTK_FourDAnalysis_EXPORT vtkFourDAnalysisLogic : public vtkSlicerModuleLogic 
{
 public:
  //BTX
  enum {  // Events
    StatusUpdateEvent       = 50003,
    ProgressDialogEvent     = 50004,
  };
  enum {
    TYPE_MEAN,
    TYPE_SD,
  };
  typedef struct {
    int         show;
    double      progress;
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

  typedef std::map<std::string, CurveDataSetType> CurveCacheType;

  typedef std::map<std::string, vtkImageData*>            ParameterImageMapType;
  typedef std::map<std::string, vtkMRMLScalarVolumeNode*> ParameterVolumeNodeMapType;

  typedef struct {
    int                              id;
    vtkFourDAnalysisLogic*           ptr;
    vtkCurveAnalysisPythonInterface* curveAnalysisInterface;
    std::vector<vtkImageData*>       imageVector;
    std::vector<double>              imageTimeStampVector;
    ParameterImageMapType            parameterImageMap;
    vtkMutexLock*                    mutex;
    
    int rangei[2];
    int rangej[2];
    int rangek[2];
  } ThreadInfo;
  //ETX

 public:
  
  static vtkFourDAnalysisLogic *New();
  
  vtkTypeRevisionMacro(vtkFourDAnalysisLogic,vtkObject);
  void PrintSelf(ostream&, vtkIndent);

  // Description:
  // Loads series of volumes from the directory that contains the file
  // specified by 'path' argument.
  // Returns number of volumes in the series.

  //BTX
  //const int SpaceDim = 3;
  typedef short PixelValueType;
  typedef itk::OrientedImage< PixelValueType, 3 > VolumeType;
  typedef itk::ImageSeriesReader< VolumeType > ReaderType;
  //ETX

  vtkMRMLScalarVolumeNode* AddMapVolumeNode(vtkMRMLTimeSeriesBundleNode* bundleNode,
                                            const char* nodeName);
  int         GetNumberOfFrames();
  const char* GetFrameNodeID(int index);

  int  SaveIntensityCurves(vtkIntensityCurves* curves, const char* fileNamePrefix);
  int  SaveCurve(vtkDoubleArray* curve, const char* fileNamePrefix);

  void SetApplication(vtkSlicerApplication *app) { this->Application = app; };
  vtkSlicerApplication* GetApplication() { return this->Application; };


  void GenerateParameterMapMT(const char* scriptFile,
                              vtkMRMLCurveAnalysisNode* curveNode,
                              vtkMRMLTimeSeriesBundleNode* bundleNode,
                              const char* outputNodeNamePrefix,
                              int start, int end,
                              int imin, int imax, int jmin, int jmax, int kmin, int kmax);
  //BTX
  static void* CurveAnalysisThread(void* ptr);
  //ETX

  void GenerateParameterMap(vtkCurveAnalysisPythonInterface* script,
                            vtkMRMLCurveAnalysisNode* curveNode,
                            vtkMRMLTimeSeriesBundleNode* bundleNode, 
                            const char* outputNodeNamePrefix,
                            int start, int end,
                            int imin, int imax, int jmin, int jmax, int kmin, int kmax);


 protected:
  
  vtkFourDAnalysisLogic();
  ~vtkFourDAnalysisLogic();

  void operator=(const vtkFourDAnalysisLogic&);
  vtkFourDAnalysisLogic(const vtkFourDAnalysisLogic&);

  virtual void ProcessLogicEvents ( vtkObject * /*caller*/, 
                                  unsigned long /*event*/, 
                                  void * /*callData*/ );  

  static void DataCallback(vtkObject*, unsigned long, void *, void *);
  void UpdateAll();


  vtkSlicerApplication *Application;
  vtkCallbackCommand *DataCallbackCommand;

 private:

  vtkMRMLScene* MRMLScene;

  //BTX
  FrameNodeVectorType FrameNodeVector;
  FrameNodeVectorType RegisteredFrameNodeVector;
  std::string VolumeBundleID;
  std::string RegisteredVolumeBundleID;

  CurveCacheType CurveCache;  // CurveCache[<4d bundle name>][<label number>].<member of CurveDataType>
  //ETX

};

#endif


  
