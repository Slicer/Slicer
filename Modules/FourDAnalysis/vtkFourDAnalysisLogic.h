/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

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
#include "vtkMRML4DBundleNode.h"
#include "vtkMRMLCurveAnalysisNode.h"


#include "itkOrientedImage.h"
#include "itkImageSeriesReader.h"


#include "vtkIntensityCurves.h"

#include <string>
#include <map>


class vtkIGTLConnector;

class VTK_FourDAnalysis_EXPORT vtkFourDAnalysisLogic : public vtkSlicerModuleLogic 
{
 public:
  //BTX
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

  typedef std::map<std::string, CurveDataSetType> CurveCacheType;
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

  vtkMRMLScalarVolumeNode* AddMapVolumeNode(vtkMRML4DBundleNode* bundleNode,
                                            const char* nodeName);
  int         GetNumberOfFrames();
  const char* GetFrameNodeID(int index);

  int  SaveIntensityCurves(vtkIntensityCurves* curves, const char* fileNamePrefix);
  int  SaveCurve(vtkDoubleArray* curve, const char* fileNamePrefix);

  void SetApplication(vtkSlicerApplication *app) { this->Application = app; };
  vtkSlicerApplication* GetApplication() { return this->Application; };

  void GenerateParameterMap(const char* script,
                            vtkMRML4DBundleNode* bundleNode, 
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

  //vtkCurveAnalysisPythonScript* Script;


};

#endif


  
