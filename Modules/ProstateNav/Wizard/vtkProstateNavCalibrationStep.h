/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkProstateNavCalibrationStep_h
#define __vtkProstateNavCalibrationStep_h

#include "vtkProstateNavStep.h"

#include "vtkKWPushButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWFrame.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "ZLinAlg.h"
#include "newmatap.h"
#include "newmat.h"
//#include <math.h>
#include "vtkMRMLLinearTransformNode.h"

#define FORCE_SIZEX     256
#define FORCE_SIZEY     256
#define FORCE_FOV       160.0
#define MEPSILON        (1e-10)

class VTK_PROSTATENAV_EXPORT vtkProstateNavCalibrationStep : public vtkProstateNavStep
{
public:
  static vtkProstateNavCalibrationStep *New();
  vtkTypeRevisionMacro(vtkProstateNavCalibrationStep,vtkProstateNavStep);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ShowUserInterface();
  virtual void ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData);  

  void ShowZFrameModel();
  void HideZFrameModel();

  const char* AddZFrameModel(const char* nodeName); // returns Node ID
  const char* AddZFrameTransform(const char* nodeName);

  void PerformZFrameCalibration(const char* filename);

  void Init(int xsize, int ysize);
  int  ZFrameRegistration(vtkMRMLScalarVolumeNode* volumeNode,
                          vtkMRMLLinearTransformNode* transformNode,
                          int slindex);

  short  *InputImage;
  int    imgxsize;
  int    imgysize;

  //BTX
  Matrix SourceImage, MaskImage;
  Matrix IFreal, IFimag, MFreal, MFimag, zeroimag;
  Matrix PFreal, PFimag;
  Matrix PIreal, PIimag;
  //ETX

  /*
  vtkImageData* SourceImage;
  vtkImageData* MaskImage;
  vtkImageData* IFreal;
  vtkImageData* IFimag;
  vtkImageData* MFreal;
  vtkImageData* MFimag;
  vtkImageData* zeroimag;
  vtkImageData* PFreal;
  vtkImageData* PFimag;
  vtkImageData* PIreal;
  vtkImageData* PIimag;
  */

  //BTX
  // Methods related to finding the fiducial artifacts in the MR images.
  bool LocateFiducials(Matrix &image, int xsize, int ysize, 
                       int Zcoordinates[7][2], float tZcoordinates[7][2]);
  void FindSubPixelPeak(int Zcoordinate[2], float tZcoordinate[2],
                        Real Y0, Real Yx1, Real Yx2, Real Yy1, Real Yy2);
  bool CheckFiducialGeometry(int Zcoordinates[7][2], int xsize, int ysize);
  void FindFidCentre(float points[7][2], float &rmid, float &cmid);
  void FindFidCorners(float points[7][2], float *pmid);
  void OrderFidPoints(float points[7][2], float rmid, float cmid);
  
  // Methods related to solving for the frame pose w.r.t. the imaging plane.
  bool LocalizeFrame(float Zcoordinates[7][2], Column3Vector &Zposition,
                     Quaternion &Zorientation);
  void SolveZ(Column3Vector P1, Column3Vector P2, Column3Vector P3,
              Column3Vector Oz, Column3Vector Vz, Column3Vector &P2f);
  
  // Method for computing the imaging plane update.
  void Update_Scan_Plane(Column3Vector &pcurrent, Quaternion &ocurrent,
                         Column3Vector Zposition, Quaternion Zorientation);
  
  // Methods for finding matrix maxima.
  Real ComplexMax(Matrix &realmat, Matrix &imagmat);
  Real RealMax(Matrix &realmat);
  Real FindMax(Matrix &inmatrix, int &row, int &col);
  float CoordDistance(float *p1, float *p2);
  //ETX
  
protected:
  vtkProstateNavCalibrationStep();
  ~vtkProstateNavCalibrationStep();

  vtkKWFrame       *SelectImageFrame;
  vtkKWLoadSaveButtonWithLabel *SelectImageButton;
  vtkKWPushButton  *CalibrateButton;
  vtkKWCheckButton *ShowZFrameCheckButton;
  vtkKWFrame       *ZFrameSettingFrame;

  //vtkMRMLModelNode* ZFrameModelNode;
  //BTX
  std::string ZFrameModelNodeID;
  std::string ZFrameTransformNodeID;
  //ETX
  

private:
  vtkProstateNavCalibrationStep(const vtkProstateNavCalibrationStep&);
  void operator=(const vtkProstateNavCalibrationStep&);
};

#endif
