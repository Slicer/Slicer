/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/OpenIGTLinkIF/vtkIGTLToMRMLBase.h $
  Date:      $Date: 2009-01-05 13:28:20 -0500 (Mon, 05 Jan 2009) $
  Version:   $Revision: 8267 $

==========================================================================*/

/*
 * ZFrame Fiducial Registration Algorithm
 * The method and code is originally developoed by Simon DiMaio from BWH.
 */


#ifndef __vtkZFrameRobotToImageRegistration2_h
#define __vtkZFrameRobotToImageRegistration2_h

#include "vtkObject.h"
#include "vtkProstateNavWin32Header.h" 

#include "vtkRobotToImageRegistration.h"

#include <vector>
#include <string>

#include "ZLinAlg.h"
#include "newmatap.h"
#include "newmat.h"

class VTK_PROSTATENAV_EXPORT vtkZFrameRobotToImageRegistration2 : public vtkRobotToImageRegistration
{

 public:

  static vtkZFrameRobotToImageRegistration2 *New();
  vtkTypeRevisionMacro(vtkZFrameRobotToImageRegistration2,vtkRobotToImageRegistration);

  void PrintSelf(ostream& os, vtkIndent indent);

  virtual int DoRegistration();

  void SetSliceRange(int param1, int param2);

 protected:
  vtkZFrameRobotToImageRegistration2();
  ~vtkZFrameRobotToImageRegistration2();

  //BTX
  typedef float  Matrix4x4[4][4];

  // Methods related to finding the fiducial artifacts in the MR images.
  void Init(int xsize, int ysize);
  int  ZFrameRegistration(vtkMRMLScalarVolumeNode* volumeNode,
                          vtkMRMLLinearTransformNode* transformNode, int slindex_s, int slindex_e);
  int  ZFrameRegistrationQuaternion(float position[3], float quaternion[4],
                                    Matrix& srcImage, int xsize, int ysize);
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

  //BTX
  void PrintMatrix(Matrix4x4 &matrix);
  void QuaternionToMatrix(float* q, Matrix4x4& m);
  void MatrixToQuaternion(Matrix4x4& m, float* q);
  void Cross(float *a, float *b, float *c);
  void IdentityMatrix(Matrix4x4 &matrix);
  //ETX
  
 protected:

  short  *InputImage;
  int    imgxsize;
  int    imgysize;

  //BTX
  Matrix SourceImage, MaskImage;
  Matrix IFreal, IFimag, MFreal, MFimag, zeroimag;
  Matrix PFreal, PFimag;
  Matrix PIreal, PIimag;
  //ETX

  int SliceRangeLow;
  int SliceRangeHigh;


};


#endif //__vtkZFrameRobotToImageRegistration2_h
