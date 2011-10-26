/*==========================================================================

  Portions (c) Copyright 2009 Atsushi Yamada (Fujimoto Lab, Nagoya Institute of Technology (NIT) All Rights Reserved.
 
 Acknowledgement: K. Chinzei (AIST), Y. Hayashi(Nagoya Univ.), T. Takeuchi (SFC Corp.), J. Tokuda(BWH), N. Hata(BWH), and H. Fujimoto(NIT) 
 CMakeLists.txt, FindOpenCV.cmake, and FindOpenIGTLink.cmake are contributions of K. Chinzei(AIST) and T. Takeuchi (SFC Corp.).

  See README.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   OpenCV
  Module:    $HeadURL: $
  Date:      $Date:07/17/2009 $
  Version:   $Revision: $

==========================================================================*/

// .NAME vtkOpenCVLogic - slicer logic class for Locator module 
// .SECTION Description
// This class manages the logic associated with tracking device for
// IGT. 


#ifndef __vtkOpenCVLogic_h
#define __vtkOpenCVLogic_h

#include "vtkOpenCVWin32Header.h"

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerApplication.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLSliceNode.h"

#include <cstdlib>
#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <math.h>

#include "vtkBMPReader.h"
#include <vtkPNGReader.h>
#include "vtkDataSetMapper.h"
#include "vtkRenderer.h"
#include <vtkImageImport.h>
#include <vtkImageViewer.h>
#include "vtkActor2D.h"
#include "vtkImageMapper.h"
#include "vtkRenderWindow.h"

#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkCamera.h"

#include "igtlOSUtil.h"
#include "igtlImageMessage.h"
#include "igtlServerSocket.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include <vtkTextProperty.h>
#include <vtkTextSource.h>
#include <vtkTextActor.h>
#include <vtkTextMapper.h>

#include <vtkTexture.h>
#include <vtkPlaneSource.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataNormals.h>

#include <vtkProperty.h>
#include <vtkProperty2D.h>

#include "vtkImageBlend.h"

#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"



class vtkIGTLConnector;

class VTK_OpenCV_EXPORT vtkOpenCVLogic : public vtkSlicerModuleLogic 
{
 public:
  enum {  // Events
    //LocatorUpdateEvent      = 50000,
    StatusUpdateEvent       = 50001,
  };

 public:

  static vtkOpenCVLogic *New();
  
  vtkTypeRevisionMacro(vtkOpenCVLogic,vtkObject);
  void PrintSelf(ostream&, vtkIndent);
    
  vtkMRMLModelNode* SetVisibilityOfCaptureData(const char* nodeName, int v);
  vtkMRMLModelNode* SetCloseOfCaptureData(const char* nodeName, int v);
    
  void cvCaptureAndView(void);
    
  vtkMRMLModelNode* initCapture(const char* nodeName, double r, double g, double b);
  void cvGetImageFromCamera(void);
    
 protected:
  
  vtkOpenCVLogic();
  ~vtkOpenCVLogic();

  void operator=(const vtkOpenCVLogic&);
  vtkOpenCVLogic(const vtkOpenCVLogic&);

  static void DataCallback(vtkObject*, unsigned long, void *, void *);
  void UpdateAll();

  vtkCallbackCommand *DataCallbackCommand;

 private:


};

#endif


  
