/*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/
// .NAME vtkTransRectalFiducialCalibrationAlgo - ...
// .SECTION Description
// vtkTransRectalFiducialCalibrationAlgo ... TODO: to be completed

#ifndef __vtkTransRectalFiducialCalibrationAlgo_h
#define __vtkTransRectalFiducialCalibrationAlgo_h

#include "vtkObject.h"
#include "vtkProstateNavWin32Header.h"

class vtkMatrix4x4;
class vtkImageData;
class vtkProstateNavTargetDescriptor;
class vtkMRMLTRProstateBiopsyModuleNode;
class vtkPoints;

#include <vector>
#include "itkPoint.h"

const unsigned int CALIB_MARKER_COUNT=4;

//BTX
struct TRProstateBiopsyCalibrationFromImageInput
{
  double MarkerInitialPositions[CALIB_MARKER_COUNT][3]; // in RAS coordinates
  double MarkerSegmentationThreshold[CALIB_MARKER_COUNT];
  double MarkerDimensionsMm[3];
  double MarkerRadiusMm;
  double RobotInitialAngle;
  vtkMatrix4x4 *VolumeIJKToRASMatrix;
  vtkImageData *VolumeImageData;
  std::string FoR; // frame of reference
};

struct TRProstateBiopsyCalibrationFromImageOutput
{
  bool MarkerFound[CALIB_MARKER_COUNT];
  double MarkerPositions[CALIB_MARKER_COUNT][3]; // in RAS coordinates
};

struct TRProstateBiopsyCalibrationData
{
  bool CalibrationValid;
  double AxesDistance;
  double RobotRegistrationAngleDegrees; // registration angle in degrees
  double AxesAngleDegrees; // angle alpha between two axes in degrees
  double I1[3];
  double I2[3]; 
  double v1[3];
  double v2[3];
  std::string FoR; // frame of reference UID
};

//ETX

class VTK_PROSTATENAV_EXPORT vtkTransRectalFiducialCalibrationAlgo :
  public vtkObject
{
public:

  static vtkTransRectalFiducialCalibrationAlgo *New();
  vtkTypeRevisionMacro(vtkTransRectalFiducialCalibrationAlgo,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //BTX
  typedef itk::Point<double, 3> PointType;

  // Description
  // ... TODO: to be completed
  bool CalibrateFromImage(const TRProstateBiopsyCalibrationFromImageInput &input, TRProstateBiopsyCalibrationFromImageOutput &output);  
  //ETX

  bool FindTargetingParams(vtkProstateNavTargetDescriptor *target);

  vtkImageData *GetCalibMarkerPreProcOutput(int i);
  vtkMatrix4x4* GetCalibMarkerPreProcOutputIJKToRAS();
  void GetAxisCenterpoints(vtkPoints *points, int i);

  const TRProstateBiopsyCalibrationData& GetCalibrationData() { return this->CalibrationData; }

  // Description
  // Enable automatic adjustment of the clicked marker centerpoint position based on the
  // image contents. If disabled, then the manually defined marker positions considered to be
  // the centerpoints.
  // By default it is enabled.
  void SetEnableMarkerCenterpointAdjustment(bool enable);

protected:

  bool RotatePoint(double H_before[3], double rotation_rad, double alpha_rad, double mainaxis[3], double I[3], /*out*/double H_after[3]);

  //BTX
  void SegmentAxis(const double initPos1[3], const double initPos2[3], vtkMatrix4x4 *volumeIJKToRASMatrix, vtkImageData* calibVol,
    double thresh1, double thresh2, const double fidDimsMm[3], double radius, double initialAngle, 
    double P1[3], double v1[3], double finalPos1[3], double finalPos2[3], bool &found1, bool &found2, vtkImageData* img1, vtkImageData* img2, std::vector<PointType> *CoordinatesVectorAxis);
  // thresh: 0..100, binary threshold value (0 corresponds to min voxel value; 100 corresponds to max voxel value in the image)
  bool SegmentCircle(double markerCenterGuessRas[3],const double normalRas[3],  double thresh, const double fidDimsMm[3], double radius, vtkMatrix4x4 *ijkToRAS, vtkImageData *calibVol, std::vector<PointType> &CoordinatesVector, vtkImageData *preprocOutput=NULL);
  //ETX
  bool CalculateCircleCenter(vtkImageData *inData, unsigned int *tempStorage, int tempStorageSize, double nThersholdVal, double nRadius, double &gx, double &gy, int nVotedNeeded, bool lDebug);  
  bool CalculateCircleCenterMean(vtkImageData *inData, double nRadius, double threshold, double &gx, double &gy);  
  //BTX
  void RemoveOutliners(double P_[3], double v_[3], const double def1[3], const double def2[3], std::vector<PointType> &CoordinatesVector);
  //ETX
  bool FindProbe(const double P1[3], const double P2[3], double v1[3], double v2[3], 
    double I1[3], double I2[3], double &axesAngleDegrees, double &axesDistance);
  //BTX
  void Linefinder(double P_[3], double v_[3], std::vector<itk::Point<double,3> > CoordVector);
  //ETX

  void CropWithCylinder(vtkImageData* output, vtkImageData* input, const double linePoint_RAS[3],const double normal_RAS[3], vtkMatrix4x4 *ijkToRAS, double radiusMm);

  bool DoubleEqual(double val1, double val2);

  vtkTransRectalFiducialCalibrationAlgo();
  virtual ~vtkTransRectalFiducialCalibrationAlgo();

  //BTX
  std::vector<PointType> CoordinatesVectorAxis1;
  std::vector<PointType> CoordinatesVectorAxis2;
  std::vector<vtkImageData*> CalibMarkerPreProcOutput;
  vtkMatrix4x4* CalibMarkerPreProcOutputIJKToRAS;
  //ETX

  TRProstateBiopsyCalibrationData CalibrationData;

  bool EnableMarkerCenterpointAdjustment;

private:
  vtkTransRectalFiducialCalibrationAlgo(const vtkTransRectalFiducialCalibrationAlgo&);  // Not implemented.
  void operator=(const vtkTransRectalFiducialCalibrationAlgo&);  // Not implemented.
};

#endif
