/*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

// true=mean detection algo; false=circle detection algo
static const bool USE_MARKER_MEAN_DETECTION=true;
// true: report error if marker is not found; false: if a marker is not found then just use the initial guess
static bool REQUIRE_MARKER_DETECTION=true;
// completely ignore all voxels that are farther from the initial guess line than radius*MAX_RADIUS_TOLERANCE
const double MAX_RADIUS_TOLERANCE=1.8;

// circle detection algo params
static const int CIRCLE_VOTE_NEEDED=18;

// mean detection algo params
static const int MIN_CIRCLE_PIXEL_COUNT=5;

// debug params
static const bool WRITE_DEBUG_IMAGES=false;
static const char DEBUG_IMAGE_PATH[]="c:\\";

//----------------------------------------------------------------------------

#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkTransRectalFiducialCalibrationAlgo.h"

#include "vtkMRMLScalarVolumeNode.h"

#include <vnl/vnl_matrix.h>
#include <vnl/algo/vnl_matrix_inverse.h>
#include <vnl/vnl_transpose.h>

#include "vtkMath.h"
#include "vtkImageMedian3D.h"
#include "vtkImageClip.h"
#include "vtkPointData.h"
#include "vtkMetaImageWriter.h"
#include "vtkImageReslice.h"
#include "vtkImageEllipsoidSource.h"
#include "vtkImageMask.h"

#include "ProstateNavMath.h"
#include "vtkProstateNavTargetDescriptor.h"

#include <algorithm>

#include "vtkImageCast.h"
#include "vtkTIFFWriter.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkTransRectalFiducialCalibrationAlgo);
vtkCxxRevisionMacro(vtkTransRectalFiducialCalibrationAlgo, "$Revision: 1.0 $");
//------------------------------------------------------------------------------

vtkTransRectalFiducialCalibrationAlgo::vtkTransRectalFiducialCalibrationAlgo()
{
  CalibMarkerPreProcOutput.resize(CALIB_MARKER_COUNT);
  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
    {
    CalibMarkerPreProcOutput[i]=vtkImageData::New();
    }
  this->EnableMarkerCenterpointAdjustment=true;
  this->CalibrationData.CalibrationValid=false;
  this->CalibMarkerPreProcOutputIJKToRAS=vtkMatrix4x4::New();
}

vtkTransRectalFiducialCalibrationAlgo::~vtkTransRectalFiducialCalibrationAlgo()
{
    for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
    {
    if (CalibMarkerPreProcOutput[i]!=0)
      {
      CalibMarkerPreProcOutput[i]->Delete();
      CalibMarkerPreProcOutput[i]=0;
      }
    }
    if (this->CalibMarkerPreProcOutputIJKToRAS!=NULL)
    {
      this->CalibMarkerPreProcOutputIJKToRAS->Delete();
      this->CalibMarkerPreProcOutputIJKToRAS=NULL;
    }
}

//----------------------------------------------------------------------------
void vtkTransRectalFiducialCalibrationAlgo::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//--------------------------------------------------------------------------------------
bool vtkTransRectalFiducialCalibrationAlgo::CalibrateFromImage(const TRProstateBiopsyCalibrationFromImageInput &input, TRProstateBiopsyCalibrationFromImageOutput &output)
{ 
  // segement the axis using initial guesses
  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    output.MarkerFound[i]=false;
  }
  double P1[3], v1[3];
  this->SegmentAxis(input.MarkerInitialPositions[0], input.MarkerInitialPositions[1], input.VolumeIJKToRASMatrix, input.VolumeImageData,
    input.MarkerSegmentationThreshold[0], input.MarkerSegmentationThreshold[1], input.MarkerDimensionsMm, input.MarkerRadiusMm, input.RobotInitialAngle,
    P1, v1, output.MarkerPositions[0], output.MarkerPositions[1], output.MarkerFound[0], output.MarkerFound[1], CalibMarkerPreProcOutput[0], CalibMarkerPreProcOutput[1], &CoordinatesVectorAxis1);
  double P2[3], v2[3];
  this->SegmentAxis(input.MarkerInitialPositions[2], input.MarkerInitialPositions[3], input.VolumeIJKToRASMatrix, input.VolumeImageData,
    input.MarkerSegmentationThreshold[2], input.MarkerSegmentationThreshold[3], input.MarkerDimensionsMm, input.MarkerRadiusMm, input.RobotInitialAngle,
    P2, v2, output.MarkerPositions[2], output.MarkerPositions[3], output.MarkerFound[2], output.MarkerFound[3], CalibMarkerPreProcOutput[2], CalibMarkerPreProcOutput[3], &CoordinatesVectorAxis2);

  this->CalibMarkerPreProcOutputIJKToRAS->DeepCopy(input.VolumeIJKToRASMatrix);

  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    if (!output.MarkerFound[i])
    {      
      // cannot complete calibration, not all markers have been found
      return false;
    }
  }
  
  // markers segmented, now register probe
  double I1[3], I2[3];
  double axesAngleDegrees=0;
  double axesDistance=0;
  if (this->FindProbe(P1, P2, v1, v2, I1, I2, axesAngleDegrees, axesDistance))
  {
    this->CalibrationData.FoR=input.FoR;
    this->CalibrationData.CalibrationValid=true;
    this->CalibrationData.AxesDistance=axesDistance;
    this->CalibrationData.RobotRegistrationAngleDegrees=input.RobotInitialAngle;
    this->CalibrationData.AxesAngleDegrees=axesAngleDegrees;
    for (int i=0; i<3; i++)
    {
      this->CalibrationData.I1[i]=I1[i];
      this->CalibrationData.I2[i]=I2[i];
      this->CalibrationData.v1[i]=v1[i];
      this->CalibrationData.v2[i]=v2[i];
    }
    return true;
  }
  else
  {
    this->CalibrationData.CalibrationValid=false;
    return false;
  }
}
//--------------------------------------------------------------------------------------
void vtkTransRectalFiducialCalibrationAlgo::SegmentAxis(const double initPos1Ras[3], const double initPos2Ras[3], vtkMatrix4x4 *volumeIJKToRASMatrix, vtkImageData* calibVol,
    double thresh1, double thresh2, const double fidDimsMm[3], double radiusMm, double initialAngle, 
    double P1[3], double v1[3], double finalPos1Ras[3], double finalPos2Ras[3], bool &found1, bool &found2, vtkImageData* img1, vtkImageData* img2, std::vector<PointType> *CoordinatesVectorAxis)
{
  /// \todo Show resliced object in a window, like ITK-SNAP

  // Slice plane normal
  double PNormal1Ras[3]={ initPos1Ras[0]-initPos2Ras[0],
    initPos1Ras[1]-initPos2Ras[1],
    initPos1Ras[2]-initPos2Ras[2]};  

  if ( vtkMath::Norm(PNormal1Ras)<0.1 /*mm*/ )
  {
    vtkWarningMacro("Markers are coincidental");       
    PNormal1Ras[0]=0.0;
    PNormal1Ras[1]=0.0;
    PNormal1Ras[2]=1.0;
  }
  vtkMath::Normalize(PNormal1Ras);

  // /* Automatic segmentation    //std::vector<PointType> CoordinatesVector; // contains the candidate marker centerpoint positions

  // First set of circle centers
  for (int i=0; i<3; i++)
  {
    finalPos1Ras[i] = initPos1Ras[i];
    finalPos2Ras[i] = initPos2Ras[i];
  }

  CoordinatesVectorAxis->clear();

  found1 = SegmentCircle(finalPos1Ras, PNormal1Ras, thresh1, fidDimsMm, radiusMm, volumeIJKToRASMatrix, calibVol, *CoordinatesVectorAxis, img1); 
  if (!found1)
  {
    if (!REQUIRE_MARKER_DETECTION)
    {
      found1=true;
    }
  } 

  // 2nd set of circle centers
  found2 = SegmentCircle( finalPos2Ras, PNormal1Ras, thresh2, fidDimsMm, radiusMm, volumeIJKToRASMatrix, calibVol, *CoordinatesVectorAxis, img2);
  if (!found2)
  {
    if (!REQUIRE_MARKER_DETECTION)
    {
      found2=true;
    }
  }

  if (!found1 || !found2)
  {
    // calib marker not found
    return;
  }

  //int vecSize = CoordinatesVectorAxis->size(); // :TODO: remove, for debugging only

  // Use CoordVector to find the line 
  for (int i=0; i<3; i++)
  {
    P1[i]=0;
    v1[i]=0;
  }
  this->RemoveOutliners(P1,v1, initPos1Ras, initPos2Ras, *CoordinatesVectorAxis);

  // we did not find anything, so take the clicked points (too bad)
  if (vtkMath::Norm(v1)<0.001) 
  {
    CoordinatesVectorAxis->clear();

    // CoordVector.clear(); // otherwise it will throw out badly \todo new alg?
    itk::Point<double,3> coord;

    coord[0] = initPos1Ras[0];
    coord[1] = initPos1Ras[1];
    coord[2] = initPos1Ras[2];
    CoordinatesVectorAxis->push_back(coord); // def1

    coord[0] = initPos2Ras[0];
    coord[1] = initPos2Ras[1];
    coord[2] = initPos2Ras[2];
    CoordinatesVectorAxis->push_back(coord); // def2

    RemoveOutliners(P1,v1, initPos1Ras, initPos2Ras, *CoordinatesVectorAxis);
  }

  // End Automatic segmentation */
}
//--------------------------------------------------------------------------------------
// Extract a VOI (defined by the markerCenterGuess and fiducial dimensions)
// Filter the VOI (median filter, thresholding)
// Extract slices that are perpendicular to the normal_RAS vector
// Perform circle detection on all slices and store detected centerpoints in CoordinatesVector
bool vtkTransRectalFiducialCalibrationAlgo::SegmentCircle(double markerCenterGuess_RAS[3],const double normal_RAS[3],  double thresh, const double fidDimsMm[3], 
  double radiusMm, vtkMatrix4x4 *ijkToRAS, vtkImageData *calibVol, std::vector<PointType> &CoordinatesVector, vtkImageData *preprocOutput/*=NULL*/)
{
    vtkSmartPointer<vtkMatrix4x4> rasToIJK = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkMatrix4x4::Invert(ijkToRAS, rasToIJK);
    
    // Spacing and origin information in the imagedata is ignored, because the real spacing and origin info is stored in the ijkToRAS matrix

    // :TODO: set the spacing info in the image when performing the slice extraction
    // (it makes a big difference if the volume is strongly non-isotropic or if the spacing is very different from 1.0)

    // Compute VOI extents (the VOI shall contain the whole fiducial in all possible orientations and should fit within the volume)
    int voiExtent_IJK[6]={0,0,0,0,0,0};
    double spacing[3]={0,0,0};
    {  
      // get volume spacing      
      {
        int col;
        for (col=0; col<3; col++) 
        {
          double len =0;
          int row;
          for (row=0; row<3; row++) 
          {
            len += ijkToRAS->GetElement(row, col) * ijkToRAS->GetElement(row, col);
          }
          spacing[col] = sqrt(len);
        }
      }
    
      double voiCenter_RAS[4]={markerCenterGuess_RAS[0],markerCenterGuess_RAS[1],markerCenterGuess_RAS[2],1};
      double voiCenter_IJK[4]={0,0,0,1};
      rasToIJK->MultiplyPoint(voiCenter_RAS, voiCenter_IJK);
      for (int i=0; i<3; i++)
        {
        // 2*maxFidDimMm to make sure that the whole fiducial fits into the VOI in any orientation
        voiExtent_IJK[i*2]= (int)(voiCenter_IJK[i]-2*fidDimsMm[i]/spacing[i]); 
        voiExtent_IJK[i*2+1]= (int)(voiCenter_IJK[i]+2*fidDimsMm[i]/spacing[i]);
        }
      // Limit VOI to volume extent
      int volumeExtent_IJK[6]={0,0,0,0,0,0};
      calibVol->GetExtent(volumeExtent_IJK);
      for (int i=0; i<3; i++)
      {
        if (voiExtent_IJK[i*2]<volumeExtent_IJK[i*2])
        {
          voiExtent_IJK[i*2]=volumeExtent_IJK[i*2];
        }
        if (voiExtent_IJK[i*2]>volumeExtent_IJK[i*2+1])
        {
          voiExtent_IJK[i*2]=volumeExtent_IJK[i*2+1];
        }
        if (voiExtent_IJK[i*2+1]<volumeExtent_IJK[i*2])
        {
          voiExtent_IJK[i*2+1]=volumeExtent_IJK[i*2];
        }
        if (voiExtent_IJK[i*2+1]>volumeExtent_IJK[i*2+1])
        {
          voiExtent_IJK[i*2+1]=volumeExtent_IJK[i*2+1];
        }
      }
    }
    
    // extract voi
    vtkSmartPointer<vtkImageClip> voiFilter = vtkSmartPointer<vtkImageClip>::New();
    voiFilter->SetInput(calibVol);  
    voiFilter->SetOutputWholeExtent(voiExtent_IJK);

    // median filter
    vtkSmartPointer<vtkImageMedian3D> medianFilter = vtkSmartPointer<vtkImageMedian3D>::New();
    medianFilter->SetInputConnection(voiFilter->GetOutputPort());
    medianFilter->SetKernelSize(3,3,3); //:TODO: check if it is spacing independent or not

    // threshold filter
    vtkSmartPointer<vtkImageThreshold> thresholdFilter = vtkSmartPointer<vtkImageThreshold>::New();
    thresholdFilter->SetInputConnection(medianFilter->GetOutputPort());
    double range[2];
    calibVol->GetScalarRange(range);        
    double nThreshold = (range[1]-range[0])* (thresh/100.0) + range[0];
    thresholdFilter->ThresholdByUpper(nThreshold);  
    thresholdFilter->SetOutValue(range[0]);
    thresholdFilter->SetInValue(range[1]);
    thresholdFilter->SetOutputScalarTypeToUnsignedChar();
    thresholdFilter->Update();
    
    // remove points that are too far from the centerline
    vtkSmartPointer<vtkImageData> radiusCroppedImage=vtkSmartPointer<vtkImageData>::New();
    CropWithCylinder(radiusCroppedImage, thresholdFilter->GetOutput(), markerCenterGuess_RAS, normal_RAS, ijkToRAS, radiusMm);  

    // the output voi to use
    vtkImageData *binaryVOI = radiusCroppedImage;

    // Copy the preprocessed image output for display purposes
    if (preprocOutput!=0)
    {
      preprocOutput->DeepCopy(binaryVOI);
      
      if (WRITE_DEBUG_IMAGES)
      {
        vtkMetaImageWriter *writer = vtkMetaImageWriter::New();        
        std::ostrstream os;            
        os << DEBUG_IMAGE_PATH << "CalibMarkerVoi.mha" << std::ends;
        writer->SetFileName(os.str());
        os.rdbuf()->freeze();
        writer->SetInput(binaryVOI);
        writer->Write();
        writer->Delete();
      }
      
    }

    if (!this->EnableMarkerCenterpointAdjustment)
    {
      // no need for automatic adjustment, just use the initial guess as result
      itk::Point<double,3> coord;
      coord[0] = markerCenterGuess_RAS[0];
      coord[1] = markerCenterGuess_RAS[1];
      coord[2] = markerCenterGuess_RAS[2];
      CoordinatesVector.push_back(coord);
      return true;
    }

    // Define a coordinate system (planeX_RAS, planeY_RAS, planeNormal_RAS) with one axis parallel to the planeNormal_RAS vector
    double planeX_RAS[4]={0,0,0, 0/*vector*/};
    double planeY_RAS[4]={0,0,0, 0/*vector*/};
    double planeNormal_RAS[4]={normal_RAS[0],normal_RAS[1],normal_RAS[2], 0/*vector*/};
    {
      double point1_RAS[4]={1,0,0,1}; // Any vector
      if (fabs(planeNormal_RAS[0])>fabs(planeNormal_RAS[1]))
      {
        // to make sure that it is not parallel to the planeNormal_RAS vector
        point1_RAS[0]=0;
        point1_RAS[1]=1;
        point1_RAS[2]=0;
      }
      // planeX_RAS is perpendicular to planeNormal_RAS
      vtkMath::Cross(planeNormal_RAS, point1_RAS, planeX_RAS);
      vtkMath::Normalize(planeX_RAS);

      // planeY_RAS is perpendicular to planeNormal_RAS and planeX_RAS
      vtkMath::Cross(planeNormal_RAS, planeX_RAS, planeY_RAS);
      vtkMath::Normalize(planeY_RAS);
    }
    
    // slice plane origin (in RAS): Starts from the clicked coordinate
    double planeOrigin_RAS[4]={0,0,0,1};
    planeOrigin_RAS[0] = markerCenterGuess_RAS[0]; // Marker position --> planeOrigin_RAS
    planeOrigin_RAS[1] = markerCenterGuess_RAS[1];
    planeOrigin_RAS[2] = markerCenterGuess_RAS[2];

    // bounds checking max error
    double delta[4]={0.1,0.1,0.1,1};

    // slicing steps (the planeNormal_RAS is in RAS cooordinates and it is normalized, so step size will be 1mm)
    double slicingSteps_RAS[4] = {planeNormal_RAS[0], planeNormal_RAS[1], planeNormal_RAS[2], 1};

    // Search for the first slice in normal direction, along the line intersecting the markerCenterGuess
/*    int voiExtent_IJK[6]={0,0,0,0,0,0};
    binaryVOI->GetExtent(voiExtent_IJK);*/
    double voiExtentDouble_IJK[6]={voiExtent_IJK[0],voiExtent_IJK[1],voiExtent_IJK[2],voiExtent_IJK[3],voiExtent_IJK[4],voiExtent_IJK[5]};
    double planeOrigin_IJK[4]={0,0,0,1};
    do
    {
      planeOrigin_RAS[0]-=slicingSteps_RAS[0];
      planeOrigin_RAS[1]-=slicingSteps_RAS[1];
      planeOrigin_RAS[2]-=slicingSteps_RAS[2];        
      rasToIJK->MultiplyPoint(planeOrigin_RAS, planeOrigin_IJK);
    }
    while (vtkMath::PointIsWithinBounds(planeOrigin_IJK, voiExtentDouble_IJK, delta));

    // Too much, one step forward (no longer within bounds)
    planeOrigin_RAS[0]+=slicingSteps_RAS[0];
    planeOrigin_RAS[1]+=slicingSteps_RAS[1];
    planeOrigin_RAS[2]+=slicingSteps_RAS[2];

    // Create slicer extractor
    vtkSmartPointer<vtkImageReslice> imageReslicer = vtkSmartPointer<vtkImageReslice>::New();    
    imageReslicer->SetInput(binaryVOI);
    imageReslicer->SetOutputDimensionality(2);

    double planeX_IJK[4]={0,0,0,1};
    double planeY_IJK[4]={0,0,0,1};
    double normal_IJK[4]={0,0,0,1};
    rasToIJK->MultiplyPoint(planeX_RAS, planeX_IJK);
    rasToIJK->MultiplyPoint(planeY_RAS, planeY_IJK);
    rasToIJK->MultiplyPoint(planeNormal_RAS, normal_IJK);
    imageReslicer->SetResliceAxesDirectionCosines(planeX_IJK, planeY_IJK, normal_IJK);   

    // Output parameters
    // Gobbi: I _always_ call SetOutputOrigin, SetOutputExtent, and SetOutputSpacing
    imageReslicer->SetOutputSpacing(1,1,1); // 1mm if the spacing of the input is correctly set
    int dispsize = 2*16; /// \todo - make the "16" a variable!
    imageReslicer->SetOutputOrigin(-(dispsize/2), -(dispsize/2), 0);
    imageReslicer->SetOutputExtent(0,dispsize,0,dispsize,0,0);
    imageReslicer->SetBackgroundColor( range[0], range[0], range[0], range[0] );
    imageReslicer->SetInterpolationModeToCubic();
    imageReslicer->Update();
           
    // Looking for mean center along the resliced planes
    double meanx=0.0, meany=0.0, meanz=0.0;
    int meanNr=0;

    // Resliced image slice
    vtkImageData *planeImageData = NULL;
     
    // Temporary storage will be used for counting the circle centers (optimization)
    int tempStorageSize = 0;
    unsigned int *tempStorage = NULL;

    // number of slices generated from this dataset (Volume Of Interest)
    int nTotalSlices = 0;

    // For every every slice    
    rasToIJK->MultiplyPoint(planeOrigin_RAS, planeOrigin_IJK);
    while (vtkMath::PointIsWithinBounds(planeOrigin_IJK, voiExtentDouble_IJK, delta) )     
    {   
      nTotalSlices++;

      imageReslicer->SetResliceAxesOrigin(planeOrigin_IJK);  
      imageReslicer->Update();

      planeImageData = imageReslicer->GetOutput();

      // allocate temporary buffer (reallocated only if it is necessary)
      {
        int *planeExtent = planeImageData->GetWholeExtent();
        int width=abs(planeExtent[0]-planeExtent[1])+1;
        int height=abs(planeExtent[2]-planeExtent[3])+1;
        int neededTempStorageSize = width*height;
        if (neededTempStorageSize>tempStorageSize)
        {
          delete[] tempStorage;
          tempStorageSize=neededTempStorageSize;
          tempStorage = new unsigned int[tempStorageSize+8];
        }
      }    



      if (WRITE_DEBUG_IMAGES)
      {
        // dump the slice image (for debug only)
        vtkImageCast *cast=vtkImageCast::New();
        cast->SetInput(planeImageData);
        cast->SetOutputScalarTypeToUnsignedShort();
        cast->ClampOverflowOn();
        cast->Update();
        vtkTIFFWriter *writer = vtkTIFFWriter::New();
        writer->SetCompressionToNoCompression();
        std::ostrstream os;            
        os << std::setiosflags(ios::fixed | ios::showpoint) << std::setprecision(2);
        os << DEBUG_IMAGE_PATH << "CalibMarkerSlice_";
        if (nTotalSlices<10)
        {
          os << "0";
        }
        os << nTotalSlices << ".tif" << std::ends;
        writer->SetFileName(os.str());
        os.rdbuf()->freeze();
        writer->SetInput(cast->GetOutput());
        writer->Write();
        writer->Delete();
        cast->Delete();
      }

      // if returns false, the center was not found (not enough votes?)
      bool lCircleFound;
      double ix, iy;
      double *minMax=planeImageData->GetScalarRange();
      double binaryImageThreshold=minMax[0]+1; // set the threshold to slightly above the minimum value
      if (USE_MARKER_MEAN_DETECTION)
      {
        
        lCircleFound = CalculateCircleCenterMean(planeImageData, radiusMm, binaryImageThreshold, ix, iy);
      }
      else
      {
        lCircleFound = CalculateCircleCenter(planeImageData, tempStorage, tempStorageSize,  binaryImageThreshold, 2*radiusMm/10.0, ix, iy, CIRCLE_VOTE_NEEDED, true);
      }

      if (lCircleFound) 
      {
        
        double pos_slice[4]={ix, iy, 0, 1};
        double pos_IJK[4]={0,0,0,1};
        imageReslicer->GetResliceAxes()->MultiplyPoint(pos_slice, pos_IJK);                  

        double pos_RAS[4]={0,0,0,1};
        ijkToRAS->MultiplyPoint(pos_IJK, pos_RAS);

        // Mean circle center - for visualization only
        meanx += pos_RAS[0];
        meany += pos_RAS[1];
        meanz += pos_RAS[2];
        meanNr++;

        // Collect circle centers into CoordVector
        itk::Point<double,3> coord;
        coord[0] = pos_RAS[0];
        coord[1] = pos_RAS[1];
        coord[2] = pos_RAS[2];
        CoordinatesVector.push_back(coord);

        //ShowSegmentedCenter(scanner[0],scanner[1],scanner[2]);
      }

      // planeOrigin_RAS: move along the normal (get next slice)
      planeOrigin_RAS[0]+=slicingSteps_RAS[0];
      planeOrigin_RAS[1]+=slicingSteps_RAS[1];
      planeOrigin_RAS[2]+=slicingSteps_RAS[2];

      rasToIJK->MultiplyPoint(planeOrigin_RAS, planeOrigin_IJK);

    } // while(all slices)

    delete[] tempStorage;
    
    // Return value: compute mean value of circle centers (for visualisation only)
    if (meanNr>0) {     
        markerCenterGuess_RAS[0]=meanx/meanNr;
        markerCenterGuess_RAS[1]=meany/meanNr;
        markerCenterGuess_RAS[2]=meanz/meanNr;      
        return true; // done, we found at least 1 center
    }
    return false; // failed, no Circle center found
}


//------------------------------------------------------------------------------------------------------------
/// \brief Calculates the circle center.
/// Assumes spacingX == spacingY (for the circle)
/// Assumes 2D planeImageData
bool vtkTransRectalFiducialCalibrationAlgo::CalculateCircleCenter(vtkImageData *planeImageData, unsigned int *tempStorage, int tempStorageSize, double nThersholdVal, double nRadiusMm, double &gx, double &gy, int nVotedNeeded, bool lDebug)
{/// \todo Make all calculations floating point!
    
    // "0" the Temporal data storage
    memset(tempStorage, 0, tempStorageSize*sizeof(unsigned int)); // tCircleAccumulate = int 
    
    // spacing should not be 0!
    double spacing[3];
    planeImageData->GetSpacing(spacing);

    if ( this->DoubleEqual(spacing[0],0.0) || this->DoubleEqual(spacing[1],0.0) || this->DoubleEqual(spacing[2],0.0) )  {
        return false;
    }

    // assume spacingY==spacingY -  depends on vtkImageReslicer settings!
    

    // convert mm to pixel
    double r = nRadiusMm/fabs(spacing[0]);

    // Calculate width & height
    int *extent = planeImageData->GetWholeExtent();
    int width = abs(extent[0]-extent[1])+1;
    int height = abs(extent[2]-extent[3])+1;
    //assert( width*height <= tempStorageSize );
    tempStorageSize=width*height;

    // Increment circle centers (possibile centers are also on a circle)
    vtkDataArray * da = planeImageData->GetPointData()->GetScalars();

    double r_exp=r*r; // r^2
    double x_exp;     // x^2
    double d;
    int x=-1;
    int y=0;
    unsigned int max = 0;
    int x1, x2, y1, y2;
    int idx;

    // Cumulate circle centers
    for (int i=0;i<tempStorageSize;i++) {
        x++;
        if (x>=width) {
            x=0; y++;
        }
        double value=*(da->GetTuple(i));
        if ( value < nThersholdVal ) 
        {
          continue;
        }
        x1=x-vtkMath::Round(r);
        x2=x+vtkMath::Round(r);

        if ( x1>=0 ) {
            idx=x1+y*width;
            if (idx>=tempStorageSize)
                idx=tempStorageSize-1;
            tempStorage[idx]++;
            if (tempStorage[idx]>max)
                max=tempStorage[idx];
        }

        if ( x2<width ) {
            idx=x2+y*width;
            if (idx>=tempStorageSize)
                idx=tempStorageSize-1;
            tempStorage[idx]++;
            if (tempStorage[idx]>max)
                max=tempStorage[idx];
        }

        if (x1<0)
            x1=0;
        if (x2>=width)
            x2=width-1;


        for (int cx=x1;cx<=x2;cx++) {
            x_exp=(x-cx)*(x-cx); // (x-j)^2 
            if (x_exp>r_exp) { // out
                continue;
            }
            d=sqrt( r_exp-x_exp );
            if (d<1.0) 
                d=1.0;

            y1=vtkMath::Round(y-d); // top
            y2=vtkMath::Round(y+d); // bottom

            if ( (y1>=0) && (y1<height) ) { // top
                idx=cx+y1*width;
                if (idx>=tempStorageSize)
                    idx=tempStorageSize-1;
                tempStorage[idx]++;
                if (tempStorage[idx]>max) 
                    max=tempStorage[idx];
            } // top

            if ( (y2>=0) && (y2<height) ) { // bottom
                idx=cx+y2*width;
                if (idx>=tempStorageSize)
                    idx=tempStorageSize-1;
                tempStorage[idx]++;
                if (tempStorage[idx]>max)
                    max=tempStorage[idx];
            } // bottom
        } // for cx
    } // for i - Cumulate circle centers

    // calculate mean center position
    x=y=0;
    double meanx=0.0, meany=0.0;
    int meannr=0;
    for (int i=0;i<tempStorageSize;i++) {
        if (tempStorage[i]==max) {
              meanx+=x;
            meany+=y;
            meannr++;
        }
        x++;
        if (x>=width) {
            x=0;
            y++;
        }
    }

    // since max exists for sure!
    //assert(meannr>0);

    // Get the mean of the centers
    gx=meanx/meannr;
    gy=meany/meannr;

    //assert(extent[4]==extent[5]); // 2D

    gx-=( (width-1)  /2.0 );
    gy-=( (height-1) /2.0 ); 

    // debug
    x=y=0;
    if (lDebug) {
    //  f << "Max:" << max << " Needed: " << nVotedNeeded << " 0:" << *gx << " Y:" << *gy << " Centers:" << meannr << std::endl;
        //if (max>=nVotedNeeded) f << "Center found!" << endl;
        //f << "Width:" << width << 
        //  " Height:" << height << 
        //  " DataSize:" << tempStorageSize << 
        //  " (" << width*height << ")" << std::endl;
        //f << "Radius:" << nRadius << " In pixels:" << r << std::endl;
        //f << extent[0] << " - " << extent[1] << "   "   << extent[2] << " - " << extent[3] << "   "  << extent[4] << " - " << extent[5] << std::endl;
        //f << std::endl;
    }

/*  if (lDebug) {
        f << std::endl << std::endl;
        x=y=0;
        for (int i=0;i<tempStorageSize;i++) {
            if ( (x==vtkMath::Round(meanx/meannr)) && (y==vtkMath::Round(meany/meannr)) ) {
                f << "0 ";
            } else {
                if ( *(da->GetTuple(i)) > nThersholdVal ) {
                    f << "H ";
                } else {
                    f << "· ";
                }
            }

            x++;
            if (x>=width) {
                f << std::endl;
                x=0;
                y++;
            }
        }

        f << std::endl;
    } // debug
*/
    if ((int)max<nVotedNeeded) 
        return false;

    return true;
}

bool vtkTransRectalFiducialCalibrationAlgo::CalculateCircleCenterMean(vtkImageData *planeImageData, double nRadiusMm, double threshold, double &gx, double &gy)
{
    // spacing should not be 0!
    double spacing[3];
    planeImageData->GetSpacing(spacing);

    if ( this->DoubleEqual(spacing[0],0.0) || this->DoubleEqual(spacing[1],0.0) || this->DoubleEqual(spacing[2],0.0) )  {
        return false;
    }

    // assume spacing[0]==spacing[1] -  depends on vtkImageReslicer settings!
    

    // convert mm to pixel
    //double r = nRadiusMm/fabs(spacing[0]);

    // Calculate width & height
    int *extent = planeImageData->GetWholeExtent();
    int width = abs(extent[0]-extent[1])+1;
    int height = abs(extent[2]-extent[3])+1;
 
    // Increment circle centers (possibile centers are also on a circle)
    vtkDataArray * da = planeImageData->GetPointData()->GetScalars();

    double sumx=0;
    double sumy=0;
    double sumweight=0;

    // compute a weighed sum of pixels above threshold
    int validPixelCount=0;
    int x=0;
    int y=0;
    for (int i=0;i<width*height;i++) 
    {
        x++;
        if (x>=width) 
        {
            x=0; y++;
        }
        double value=*(da->GetTuple(i));
        if (value>threshold)
        {
          sumx+=x*value;
          sumy+=y*value;
          sumweight+=value-threshold;
          validPixelCount++;
        }
    }
    
    if (validPixelCount<MIN_CIRCLE_PIXEL_COUNT)
    {
        return false;
    }

    // Get the mean of the centers
    gx=sumx/sumweight;
    gy=sumy/sumweight;

    gx-=(double)width/2.0;
    gy-=(double)height/2.0;

    return true;
}

/// Calculate probe position from fiducial pairs
bool vtkTransRectalFiducialCalibrationAlgo::FindProbe(const double P1[3], const double P2[3], double v1[3], double v2[3], 
    double I1[3], double I2[3], double &axesAngleDegrees, double &axesDistance)
{
    /*
    // plane normal to display lines
    window->GetTransformedNormal(normal);
    normal[0]=-(normal[0]/5.0);
    normal[1]=-(normal[1]/5.0);
    normal[2]=-(normal[2]/5.0);

    // to show the lines found
    #define DDIST 120.0 
    double e1[3], e2[3];

    // Transfer to data coordinate system
    // Assume: patient HFS
    // 0 is increasing toward the patient's left
    // Y is increasing to the patient's posterior
    // Z is increasing to the head (superior) direction
    // DICOm: LPS, Algor: LPS, Normally: RAS

    if (v1[2]<0) {
        v1[0]=-v1[0];v1[1]=-v1[1];v1[2]=-v1[2];
    }

    if (v2[2]<0) {
        v2[0]=-v2[0];v2[1]=-v2[1];v2[2]=-v2[2];
    }

    // P1, v1 from Linefinder
    e1[0]=P1[0]+(-DDIST)*v1[0];
    e1[1]=P1[1]+(-DDIST)*v1[1];
    e1[2]=P1[2]+(-DDIST)*v1[2];

    e2[0]=P1[0]+(+DDIST)*v1[0];
    e2[1]=P1[1]+(+DDIST)*v1[1];
    e2[2]=P1[2]+(+DDIST)*v1[2];
    FidLine1->SetPoint1(e1[0]+normal[0], e1[1]+normal[1], e1[2]+normal[2]);
    FidLine1->SetPoint2(e2[0]+normal[0], e2[1]+normal[1], e2[2]+normal[2]);

    // P2, v2 from Linefinder
    e1[0]=P2[0]+(-DDIST)*v2[0];
    e1[1]=P2[1]+(-DDIST)*v2[1];
    e1[2]=P2[2]+(-DDIST)*v2[2];

    e2[0]=P2[0]+(+DDIST)*v2[0];
    e2[1]=P2[1]+(+DDIST)*v2[1];
    e2[2]=P2[2]+(+DDIST)*v2[2];
    FidLine2->SetPoint1(e1[0]+normal[0], e1[1]+normal[1], e1[2]+normal[2]);
    FidLine2->SetPoint2(e2[0]+normal[0], e2[1]+normal[1], e2[2]+normal[2]);

    // show lines
    if (line_window) {
        line_window->RenderAll();
    }

    */

    //this->OverlayFiducialAxisLines();

    // Axel algorithm, part 1 (see Targeting for part 2)

    double p12[3];
    p12[0]=P2[0]-P1[0];
    p12[1]=P2[1]-P1[1];
    p12[2]=P2[2]-P1[2];

  // Normalize axis vector direction
  { 
    double t1=vtkMath::Dot(p12,v1);
    if (t1>0)
    {
      // v1 points towards probe tip 
    }
    else
    {
      // v1 does not point towards probe tip; reverse it!
      v1[0]=-v1[0];
      v1[1]=-v1[1];
      v1[2]=-v1[2];
    }
  }
  // Normalize needle vector direction
  { 
    double t1=vtkMath::Dot(p12,v2);
    if (t1<0)
    {
      // v2 points towards hinge point 
    }
    else
    {
      // v1 does not point towards hinge point; reverse it!
      v2[0]=-v2[0];
      v2[1]=-v2[1];
      v2[2]=-v2[2];
    }
  }

    // Find intersecting points

    // vtkMath::Normalize(p12); - not ntomalized here!
    double v12=vtkMath::Dot(v1,v2);

    double t1=(vtkMath::Dot(p12,v1)-vtkMath::Dot(p12,v2)*v12)/(1-v12*v12);
    double t2=-(vtkMath::Dot(p12,v2)-vtkMath::Dot(p12,v1)*v12)/(1-v12*v12);

    // I1 global
    I1[0] = P1[0] + t1*v1[0];
    I1[1] = P1[1] + t1*v1[1];
    I1[2] = P1[2] + t1*v1[2];

    // I2 global
    I2[0] = P2[0] + t2*v2[0];
    I2[1] = P2[1] + t2*v2[1];
    I2[2] = P2[2] + t2*v2[2];

    //Calculate and display distance and angle
    double Id[3];
    Id[0]=I2[0]-I1[0];
    Id[1]=I2[1]-I1[1];
    Id[2]=I2[2]-I1[2]; 
    axesDistance=vtkMath::Norm(Id);

    // Angle between lines (alpha global)
    double alpha= acos(  fabs(vtkMath::Dot(v1,v2)) );
    axesAngleDegrees=alpha*180.0/vtkMath::Pi();

    return true;
}

struct PointTooFar 
{
  PointTooFar(const double defp[3],const double defv[3],double max_dist):
    defp_p((double*)(defp)),
    defv_p((double*)(defv)),
    max_distance(max_dist) 
  {
  }
  double *defp_p;
  double *defv_p;
  double max_distance;

  bool operator()( itk::Point<double, 3> x) {
    return ProstateNavMath::PointDistanceFromLineM(x,defp_p,defv_p)>max_distance;
  }
};

/// Remove outlaying points
void vtkTransRectalFiducialCalibrationAlgo::RemoveOutliners(double P_[3], double v_[3], const double def1[3], const double def2[3], std::vector<PointType> &CoordinatesVector)
{
    // Initial threshold to disregard center points, which are too far away from initial line.
    double outlier_thres_initial=4;

    // Threshold to call center points outliers.
    double outlier_thres=.5;

    
    std::vector<itk::Point<double,3> > CoordVectorCopy (CoordinatesVector);
    

    
    int vecSize = CoordVectorCopy.size();
    double def_v[3];
    def_v[0]=def2[0]-def1[0];
    def_v[1]=def2[1]-def1[1];
    def_v[2]=def2[2]-def1[2];
    vtkMath::Normalize(def_v);  

    // Remove outside points (>4mm)
    CoordVectorCopy.erase( std::remove_if( CoordVectorCopy.begin(), CoordVectorCopy.end(), PointTooFar(def1,def_v,outlier_thres_initial) ),
                  CoordVectorCopy.end() );

    vecSize = CoordVectorCopy.size();

    double dist, max_distance;
    std::vector<itk::Point<double,3> >::const_iterator i;

    do {
        this->Linefinder(P_,v_,CoordVectorCopy);
        max_distance=0;
        for (i=CoordVectorCopy.begin(); i!=CoordVectorCopy.end(); ++i){
            dist=ProstateNavMath::PointDistanceFromLineM(*i,P_,v_);
            if (dist>max_distance) max_distance=dist;
        }
        if (max_distance>=outlier_thres) {
            CoordVectorCopy.erase( std::remove_if( CoordVectorCopy.begin(), CoordVectorCopy.end(), PointTooFar(P_,v_,max_distance-0.001) ),
                 CoordVectorCopy.end() );
        }
    } while (max_distance>=outlier_thres);

    
    
}


/// Find the best line fitting CoordVector
void vtkTransRectalFiducialCalibrationAlgo::Linefinder(double P_[3], double v_[3], std::vector<itk::Point<double,3> > CoordVector)
{// Axel line algorithm



    // CoordVector is "A"
    int m=CoordVector.size();
    if (m<2) {
        // At least two points are necessary to calculate a line
        return;
    }

    std::vector<itk::Point<double,3> >::iterator it;

    // calculate the mean point P
    // P=sum(A)/m;
    double P[3];
    P[0] = 0.0;
    P[1] = 0.0;
    P[2] = 0.0;

    it = CoordVector.begin();
    while (it != CoordVector.end()) {
        P[0] += (*it)[0];
        P[1] += (*it)[1];
        P[2] += (*it)[2];
        it++;
    }
    P[0]/=m;
    P[1]/=m;
    P[2]/=m;

    // P_m=(P'*ones(1,m))';
    // subtract the mean point from each matrix A coordinate
    // B=A-P_m;
    /*it = CoordVector.begin();
    while (it != CoordVector.end()) {
        (*it)[0] -= P[0];
        (*it)[1] -= P[1];
        (*it)[2] -= P[2];
        it++;
    }
    */
    double curVal[3];
    for (int k = 0; k<(int)CoordVector.size(); k++)
    {
      curVal[0] = CoordVector[k][0];
      curVal[1] = CoordVector[k][1];
      curVal[2] = CoordVector[k][2];
      CoordVector[k][0] -= P[0];
      CoordVector[k][1] -= P[1];
      CoordVector[k][2] -= P[2];
      curVal[0] = CoordVector[k][0];
      curVal[1] = CoordVector[k][1];
      curVal[2] = CoordVector[k][2];
    }
    vnl_matrix<double> A(m, 3);
    it = CoordVector.begin();
    int i =0;
    while ( (it != CoordVector.end()) && (i<m) ) {
        A(i,0) = (*it)[0];
        A(i,1) = (*it)[1];
        A(i,2) = (*it)[2];
        it++;
        i++;
    }
    vnl_svd<double> svdData(A);
    /*try {
        svdData.recompose();
        //nmrSVD(A, svdData);
    } catch(...) {
        assert(false); // "An exception occured, check cisstLog.txt." 
        return;
    }*/

    P_[0]=P[0]; P_[1]=P[1]; P_[2]=P[2];

    const vnl_matrix<double>& V = svdData.V();
    /*for (int l = 0; l<3; l++)
        for (int m = 0; m<3; m++)
            slicerCerr(V(l,m));*/
    vnl_matrix<double> trV = vnl_transpose(V);

    v_[0]=trV(0,0);
    v_[1]=trV(0,1);
    v_[2]=trV(0,2);
    vtkMath::Normalize(v_);
}

//-------------------------------------------------------------------------------
/// Calculations to find the targeting parameters (point -> rotation & deepth)
bool vtkTransRectalFiducialCalibrationAlgo::FindTargetingParams(vtkProstateNavTargetDescriptor *target)
{
    if (!this->CalibrationData.CalibrationValid) 
    {
      target->SetTargetingParametersValid(false);
      return false;
    }

    target->SetCalibrationFoRStr(this->CalibrationData.FoR);

    // Axel algorithm, part 1 :
    //  When fiducials segmented

    // Axel algorithm, part 2
    //  Calculate Targeting Parameters

    double targetRas[3]; // Target
    target->GetRASLocation(targetRas); 
    
    double targetLps[3];
    targetLps[0] = -targetRas[0];
    targetLps[1] = -targetRas[1];
    targetLps[2] = targetRas[2];
    
    double I1[3];
    I1[0] = -this->CalibrationData.I1[0];
    I1[1] = -this->CalibrationData.I1[1];
    I1[2] = this->CalibrationData.I1[2];

    double v1[3]; // v1 is inverted and then converted from RAS to LPS 
    v1[0] = -(-this->CalibrationData.v1[0]);
    v1[1] = -(-this->CalibrationData.v1[1]);
    v1[2] = (-this->CalibrationData.v1[2]);

    double v2[3];
    v2[0] = -this->CalibrationData.v2[0];
    v2[1] = -this->CalibrationData.v2[1];
    v2[2] = this->CalibrationData.v2[2];
    
    double H_zero[3];
    
    // Original hinge point before rotation
    double l=14.5/sin(this->CalibrationData.AxesAngleDegrees * vtkMath::Pi()/180);

    double H_before[3];
    H_before[0] = I1[0] - l*v2[0];
    H_before[1] = I1[1] - l*v2[1];
    H_before[2] = I1[2] - l*v2[2];

    // Axis of Rotation is v1
    // ToZeroRotMatrix=eye(3)+sin(-registration_angle_rad)*skew+skew*skew*(1-cos(-registration_angle_rad));
    RotatePoint(H_before, this->CalibrationData.RobotRegistrationAngleDegrees*vtkMath::Pi()/180, this->CalibrationData.AxesAngleDegrees * vtkMath::Pi()/180, v1, I1, H_zero);


    // The rotated (to zero) needle axis
    double v2_zero[3];
    v2_zero[0] = I1[0] - H_zero[0];
    v2_zero[1] = I1[1] - H_zero[1];
    v2_zero[2] = I1[2] - H_zero[2];
    vtkMath::Normalize(v2_zero);

    //  Calculate Rotation
    // n1: Orthogonal to probe (v1) and "0" needle
    double n1[3];
    vtkMath::Cross(v1,v2_zero,n1);
    vtkMath::Normalize(n1);

    // Distance from Target T to the plane with direction, is: 
    double tVec[3];
    tVec[0] = targetLps[0] - I1[0];
    tVec[1] = targetLps[1] - I1[1];
    tVec[2] = targetLps[2] - I1[2];

    // n2: 0 needle axes at 90 deg
    double n2[3];
    vtkMath::Cross(v1,n1,n2);
    vtkMath::Normalize(n2);

    // Distance from Target T to the plane with direction, is: 
    double x = -vtkMath::Dot(n2,tVec);
    double y = -vtkMath::Dot(n1,tVec);

    // Rotation with atan2
    double rotation = atan2(y,x);
    double rotation_degree = -rotation*180.0/vtkMath::Pi();

    // Text -------------------------------------------------------------
    target->SetAxisRotation(rotation_degree);

    // RotMatrix=eye(3)+sin(-rotation)*skew+skew*skew*(1-cos(-rotation));
    double H_afterLps[3];
    RotatePoint(H_zero, rotation, this->CalibrationData.AxesAngleDegrees*vtkMath::Pi()/180, v1, I1, H_afterLps);
        
    // Needle Trajectory Vector
    double v_needle[3];
    v_needle[0] = targetLps[0] - H_afterLps[0];
    v_needle[1] = targetLps[1] - H_afterLps[1];
    v_needle[2] = targetLps[2] - H_afterLps[2];

    double H_afterRas[3];
    H_afterRas[0] = -H_afterLps[0];
    H_afterRas[1] = -H_afterLps[1];
    H_afterRas[2] = H_afterLps[2];
    target->SetHingePosition(H_afterRas); // convert to RAS, because internal calculations are done in LPS but all outputs (and inputs) are in RAS reference

    /// \todo Pipe from H_afterLps (along to v_needle traj.) ending "Overshoot" mm after the target
    // T[?]+overshoot*v_needle[?]

    vtkMath::Normalize(v_needle);

    // Needle Angle: fix it!
    double needle_angle = acos( 1.0 * vtkMath::Dot(v_needle,v1) );

    double needle_angle_degree = needle_angle*180.0/vtkMath::Pi();
    // The absolute needle angle to reach the target is %d degrees
        
    /// \todo Temp function! - compensate (no cheating?) // ccs
    double alpha_degree = this->CalibrationData.AxesAngleDegrees;

    // Cheating ot not? 
    needle_angle_degree = needle_angle_degree - (alpha_degree-37.0); // cheating

    if ( (needle_angle_degree < 17.5) || (needle_angle_degree > 37.0 + 1.8 /* !!! */ ) ) {
        // can't reach the target!
        target->SetIsOutsideReach(true);
        //target->SetColor(0.3, 0.3, 1.0);
    } else {
        target->SetIsOutsideReach(false);
    }

    // Text -------------------------------------------------------------
    target->SetNeedleAngle(needle_angle_degree);

    // Insertion depth
    double needle_angle_initial = alpha_degree*vtkMath::Pi()/180;

    double n_length = 29.16 + 4 + 14.5 / sin(needle_angle_initial);
    double n_slide = n_length-14.5/sin(needle_angle);

    double insM[3];
    insM[0] = H_afterLps[0] - targetLps[0];
    insM[1] = H_afterLps[1] - targetLps[1];
    insM[2] = H_afterLps[2] - targetLps[2];

    // Insertion depth offset (in mm)
    // overshoot>0: biopsy
    // overshoot<0: seed placement
    double overshoot = target->GetNeedleOvershoot();

    double n_insertion=vtkMath::Norm(insM)+n_slide+overshoot;

    // Text -------------------------------------------------------------
    target->SetDepthCM(n_insertion/10.0);

    target->SetTargetingParametersValid(true);
    return true;
}


/// Internal function used by FindTargetingParams
bool vtkTransRectalFiducialCalibrationAlgo::RotatePoint(double H_before[3], double rotation_rad, double alpha_rad, double mainaxis[3], double I[3], /*out*/double H_after[3])
{
        double skew[3][3] = {
           {             0, -mainaxis[2],  mainaxis[1] },
           {  mainaxis[2],             0, -mainaxis[0] },
           { -mainaxis[1],  mainaxis[0],             0 }
        };

        // RotMatrix=eye(3)+sin(-rotation_rad)*skew+skew*skew*(1-cos(-rotation_rad));
        double RotMatrix[3][3];
        vtkMath::Identity3x3(RotMatrix);

        RotMatrix[0][0]+=sin(-rotation_rad)*skew[0][0];
        RotMatrix[0][1]+=sin(-rotation_rad)*skew[0][1];
        RotMatrix[0][2]+=sin(-rotation_rad)*skew[0][2];

        RotMatrix[1][0]+=sin(-rotation_rad)*skew[1][0];
        RotMatrix[1][1]+=sin(-rotation_rad)*skew[1][1];
        RotMatrix[1][2]+=sin(-rotation_rad)*skew[1][2];

        RotMatrix[2][0]+=sin(-rotation_rad)*skew[2][0];
        RotMatrix[2][1]+=sin(-rotation_rad)*skew[2][1];
        RotMatrix[2][2]+=sin(-rotation_rad)*skew[2][2];

        double skew2[3][3];
        vtkMath::Multiply3x3(skew, skew, skew2);

        skew2[0][0]*=1-cos(-rotation_rad);
        skew2[0][1]*=1-cos(-rotation_rad);
        skew2[0][2]*=1-cos(-rotation_rad);

        skew2[1][0]*=1-cos(-rotation_rad);
        skew2[1][1]*=1-cos(-rotation_rad);
        skew2[1][2]*=1-cos(-rotation_rad);

        skew2[2][0]*=1-cos(-rotation_rad);
        skew2[2][1]*=1-cos(-rotation_rad);
        skew2[2][2]*=1-cos(-rotation_rad);

        RotMatrix[0][0]+=skew2[0][0];
        RotMatrix[0][1]+=skew2[0][1];
        RotMatrix[0][2]+=skew2[0][2];

        RotMatrix[1][0]+=skew2[1][0];
        RotMatrix[1][1]+=skew2[1][1];
        RotMatrix[1][2]+=skew2[1][2];

        RotMatrix[2][0]+=skew2[2][0];
        RotMatrix[2][1]+=skew2[2][1];
        RotMatrix[2][2]+=skew2[2][2];

        double rotation_point[3];
        rotation_point[0]=I[0]-14.5/tan(alpha_rad)*mainaxis[0];
        rotation_point[1]=I[1]-14.5/tan(alpha_rad)*mainaxis[1];
        rotation_point[2]=I[2]-14.5/tan(alpha_rad)*mainaxis[2];

        double rotation_vector[3];
        rotation_vector[0]=H_before[0]-rotation_point[0];
        rotation_vector[1]=H_before[1]-rotation_point[1];
        rotation_vector[2]=H_before[2]-rotation_point[2];

        double after_rotation_vector[3];
        vtkMath::Multiply3x3(RotMatrix, rotation_vector, after_rotation_vector);

        H_after[0]=rotation_point[0]+after_rotation_vector[0];
        H_after[1]=rotation_point[1]+after_rotation_vector[1];
        H_after[2]=rotation_point[2]+after_rotation_vector[2];

        return true;
}

//----------------------------------------------------------------------------
bool vtkTransRectalFiducialCalibrationAlgo::DoubleEqual(double val1, double val2)
{
  bool result = false;
    
  if(fabs(val2-val1) < 0.0001)
      result = true;

  return result;
}

//----------------------------------------------------------------------------
vtkImageData* vtkTransRectalFiducialCalibrationAlgo::GetCalibMarkerPreProcOutput(int i)
{
  return this->CalibMarkerPreProcOutput[i];
}

//----------------------------------------------------------------------------
vtkMatrix4x4* vtkTransRectalFiducialCalibrationAlgo::GetCalibMarkerPreProcOutputIJKToRAS()
{
  return this->CalibMarkerPreProcOutputIJKToRAS;
}

//----------------------------------------------------------------------------
void vtkTransRectalFiducialCalibrationAlgo::GetAxisCenterpoints(vtkPoints *points, int i)
{
  if (points==NULL)
    {
    // TODO: log error
    vtkErrorMacro("GetAxisCenterpoints: points are null");
    return;
    }

  std::vector<PointType> *coords;
  if (i==0)
  {
    coords=&CoordinatesVectorAxis1;
  }
  else if (i==1)
  {
    coords=&CoordinatesVectorAxis2;
  }
  else
    {
    // i is out of valid range, return
    vtkErrorMacro("GetAxisCenterpoints: i " << i << " is out of range 0-1");
    return;
    }
  std::vector<PointType>::iterator it = coords->begin();
  int insertPos=points->GetNumberOfPoints();
  while (it != coords->end()) 
  {
    double x=(*it)[0];
    double y=(*it)[1];
    double z=(*it)[2];
    points->InsertPoint(insertPos,x,y,z);
    it++;
    insertPos++;
  }  
}

//----------------------------------------------------------------------------
void vtkTransRectalFiducialCalibrationAlgo::SetEnableMarkerCenterpointAdjustment(bool enable)
{
  this->EnableMarkerCenterpointAdjustment=enable;
}

void vtkTransRectalFiducialCalibrationAlgo::CropWithCylinder(vtkImageData* output, vtkImageData* input, const double linePoint_RAS[3],const double lineVector_RAS[3], vtkMatrix4x4 *ijkToRAS, double radiusMm)
{
  output->DeepCopy(input);

  double lineVectorNorm_RAS[4]={0,0,0,0};
  lineVectorNorm_RAS[0]=lineVector_RAS[0];
  lineVectorNorm_RAS[1]=lineVector_RAS[1];
  lineVectorNorm_RAS[2]=lineVector_RAS[2];
  vtkMath::Normalize(lineVectorNorm_RAS);

  int *extent = output->GetWholeExtent(); 
  double coord_IJK[4]={0,0,0,1};
  for (coord_IJK[2]=extent[4]; coord_IJK[2]<=extent[5]; coord_IJK[2]++)
  {
    for (coord_IJK[1]=extent[2]; coord_IJK[1]<=extent[3]; coord_IJK[1]++)
    {
      for (coord_IJK[0]=extent[0]; coord_IJK[0]<=extent[1]; coord_IJK[0]++)
      {
        double coord_RAS[4]={0,0,0,1};
        ijkToRAS->MultiplyPoint(coord_IJK, coord_RAS);

        double distFromLine=0;
        {
          double v[3];
          v[0] = coord_RAS[0] - linePoint_RAS[0];
          v[1] = coord_RAS[1] - linePoint_RAS[1];
          v[2] = coord_RAS[2] - linePoint_RAS[2];
          double dot = vtkMath::Dot(lineVectorNorm_RAS,v);
          double w[3];
          w[0] = v[0] - dot*lineVectorNorm_RAS[0];
          w[1] = v[1] - dot*lineVectorNorm_RAS[1];
          w[2] = v[2] - dot*lineVectorNorm_RAS[2];
          distFromLine=vtkMath::Norm(w);
        }
        if (distFromLine>radiusMm*MAX_RADIUS_TOLERANCE)
          {
          output->SetScalarComponentFromFloat((int)coord_IJK[0],
                                              (int)coord_IJK[1],
                                              (int)coord_IJK[2],0,0.0);
          }
      }
    }
  }
}
