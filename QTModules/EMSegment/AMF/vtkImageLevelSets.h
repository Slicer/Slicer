/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageLevelSets.h,v $

=========================================================================auto=*/
/*  ==================================================
    Module : vtkImageLevelSets
             slightly altered version from vtkLevelSet by  Karl Krissian 
             (from initial code by Liana Lorigo and Renaud Keriven)
    ==================================================
*/



// .NAME vtkImageLevelSets -  Vessels segmentation method
// .SECTION Description
// Implementation of Liana Lorigo's method to segment automatically
// vessels or cilindrical structures.
// 

#ifndef __vtkImageLevelSets_h
#define __vtkImageLevelSets_h

#include <vtkEMSegment.h>
#include "vtkImageData.h"
#include "vtkImageToImageFilter.h"

#include "vtkLevelSets.h"
#include "vtkLevelSetFastMarching.h"
#include "vtkImageIsoContourDist.h"
#include "vtkImageFastSignedChamfer.h"
#include "vtkImagePropagateDist2.h"

// Kilian: IPMI07 
#define DISTMAP_LOGODDS      4


class VTK_EMSEGMENT_EXPORT vtkImageLevelSets : public vtkImageToImageFilter
{
public:
  static vtkImageLevelSets *New();
  vtkTypeMacro(vtkImageLevelSets,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  vtkSetMacro(isotropic_voxels,int);
  vtkGetMacro(isotropic_voxels,int);
  vtkBooleanMacro(isotropic_voxels,int);

  // Description
  // Rescale data to 0-255
  vtkSetMacro(RescaleImage,int);
  vtkGetMacro(RescaleImage,int);
  vtkBooleanMacro(RescaleImage,int);
  // Description
  // Input parameters of method: InitThreshold, check_freq,numiters, reinit_freq,
  // converged_thresh, imscale, do_mean and band
  vtkSetMacro(InitThreshold,float);
  vtkGetMacro(InitThreshold,float);

  //
  vtkSetMacro(UseLowThreshold,int);
  vtkGetMacro(UseLowThreshold,int);
  vtkBooleanMacro(UseLowThreshold,int);

  //
  vtkSetMacro(LowThreshold,float);
  vtkGetMacro(LowThreshold,float);

  //
  vtkSetMacro(UseHighThreshold,int);
  vtkGetMacro(UseHighThreshold,int);
  vtkBooleanMacro(UseHighThreshold,int);

  //
  vtkSetMacro(HighThreshold,float);
  vtkGetMacro(HighThreshold,float);

  //
  vtkSetMacro(ProbabilityHighThreshold,float);
  vtkGetMacro(ProbabilityHighThreshold,float);

  //
  vtkSetMacro(StepDt,float);
  vtkGetMacro(StepDt,float);
  //

  // current iteration number
  //
  vtkSetMacro(step,int);
  vtkGetMacro(step,int);
  //
  vtkSetMacro(CheckFreq,int);
  vtkGetMacro(CheckFreq,int);
  //
  vtkSetMacro(NumIters,int);
  vtkGetMacro(NumIters,int);
  //
  vtkSetMacro(ReinitFreq,int);
  vtkGetMacro(ReinitFreq,int);
  //
  vtkSetMacro(ConvergedThresh,float);
  vtkGetMacro(ConvergedThresh,float);
  //
  vtkSetMacro(AdvectionCoeff,float);
  vtkGetMacro(AdvectionCoeff,float);
  //
  vtkSetMacro(DoMean,int);
  vtkGetMacro(DoMean,int);
  vtkBooleanMacro(DoMean,int);

  //
  vtkSetMacro(Band,int);
  vtkGetMacro(Band,int);

  //
  vtkSetMacro(ShapeMinDist,int);
  vtkGetMacro(ShapeMinDist,int);

  //
  vtkSetMacro(Tube,int);
  vtkGetMacro(Tube,int);
  
  // Distance Map algorithm
  // 0: old curve method
  // 1: fast marching method with conservation of the sub-voxel surface position
  // 2: Fast signed Chamfer 
  // 3: Shape-based levelset: propagation Danielsson
  // 4: LogOdds 
  vtkSetMacro(DMmethod,int); 
  vtkGetMacro(DMmethod,int);


  //
  vtkSetMacro(EvolveThreads,int);
  vtkGetMacro(EvolveThreads,int);

  //
  vtkSetMacro(savedistmap,int);
  vtkGetMacro(savedistmap,int);
  vtkBooleanMacro(savedistmap,int);

  //
  vtkSetMacro(savesecdergrad,int);
  vtkGetMacro(savesecdergrad,int);
  vtkBooleanMacro(savesecdergrad,int);

  //
  vtkSetMacro(advection_scheme,int);
  vtkGetMacro(advection_scheme,int);
  //  vtkBooleanMacro(advection_scheme,int);


  //
  vtkSetMacro(balloon_scheme,int);
  vtkGetMacro(balloon_scheme,int);


  //
  vtkSetMacro(UseCosTerm,int);
  vtkGetMacro(UseCosTerm,int);
  vtkBooleanMacro(UseCosTerm,int);

  //
  vtkSetMacro(IsoContourBin,int);
  vtkGetMacro(IsoContourBin,int);
  vtkBooleanMacro(IsoContourBin,int);

  //
  // Dimension 2 for 2D, 3 for 3D
  //
  vtkSetMacro(Dimension,int);
  vtkGetMacro(Dimension,int);

  // slice number for 2D level set
  vtkSetMacro(SliceNum,int);
  vtkGetMacro(SliceNum,int);

  //
  // Image of velocity vectors
  //
  vtkSetObjectMacro(velocity,vtkImageData);
  vtkGetObjectMacro(velocity,vtkImageData);

  //
  vtkSetMacro(coeff_velocity,float);
  vtkGetMacro(coeff_velocity,float);

  // Balloon expansion force

  // Set Constant expansion
  vtkSetMacro(balloon_value,float);
  vtkGetMacro(balloon_value,float);

  // Set impact of balloon force onto curve evalution
  vtkSetMacro(balloon_coeff,float);
  vtkGetMacro(balloon_coeff,float);

  // Set Image based Expansion 
  vtkSetObjectMacro(balloon_image,vtkImageData);
  vtkGetObjectMacro(balloon_image,vtkImageData);

  //
  // Use initial image for the level set
  // initial level set function 
  vtkSetObjectMacro(initImage,vtkImageData);
  vtkGetObjectMacro(initImage,vtkImageData);

  void SetInitIntensityBright() { InitIntensity=Bright; }
  void SetInitIntensityDark()   { InitIntensity=Dark; }

  // Kilian: IPMI 07
  // Use initial image for the level set
  // initial level set function 
  vtkSetObjectMacro(logCondIntensityImage,vtkImageData);
  vtkGetObjectMacro(logCondIntensityImage,vtkImageData);


  void SetlogCondIntensityCoefficient(float init);
  vtkGetMacro(logCondIntensityCoefficient,float);

  // Inside is greater 0
  void SetLogCondIntensityInsideBright() { this->logCondIntensityInside= -1; }
  // Inside is smaller 0 
  void SetLogCondIntensityInsideDark()   { this->logCondIntensityInside= 1; }

  // The minum of the global weight term q*(1-q) on all update coefficients 
  // where the sigmoid function of the level set represeentation q = 1/(1 + e^-d) 
  // when probCondWeightMin is set to 1 (which it is by default) then the term is ignored ! 
  vtkSetMacro(probCondWeightMin,float);
  vtkGetMacro(probCondWeightMin,float);
 

  // Set/Get Skeleton Image
  vtkSetObjectMacro(SkeletonImage,vtkImageData);
  vtkGetObjectMacro(SkeletonImage,vtkImageData);

  //
  vtkSetMacro(HistoGradThreshold,float);
  vtkGetMacro(HistoGradThreshold,float);

  //
  vtkSetMacro(coeff_curvature,float);
  vtkGetMacro(coeff_curvature,float);

  //
  vtkSetMacro(ProbabilityThreshold,float);
  vtkGetMacro(ProbabilityThreshold,float);

  //
  vtkSetMacro(verbose,unsigned char);
  vtkGetMacro(verbose,unsigned char);

  void SetNumInitPoints( int n);
  void SetInitPoint(int num, int x, int y, int z, int radius);

  void SetNumGaussians( int n);
  void SetGaussian(int num, float mean, float sd);

  // Description
  // Reset the distance map (the old function reinit)
  void DistanceMap();

  // initial code of curves
  void DistanceMapCurves();

  // fast marching distance map
  void DistanceMapFMOld();
  void DistanceMapFM();    // use vtkImageIsoContourDist

  // fast signed Chamfer distance map
  void DistanceMapChamfer();
      
  // Propagating Danielsson Distance + 
  // skeleton and distance to the skeleton
  void DistanceMapShape();
      
  void InitParam(       vtkImageData* in, vtkImageData* out);

  void ResizeBand();

  void InitEvolution();
  int  Iterate();
  void EndEvolution();

  // Set a curvature weight for the geodesic framework
  void SetCurvatureWeights( float* CW)
  //
  {
    curvature_weight = CW;
  }


  //
  void SetAdvectionVectorField( float* DAx, float* DAy, float* DAz)
  {
    advection_scheme = ADVECTION_UPWIND_VECTORS;
    data_attach_x = DAx;
    data_attach_y = DAy;
    data_attach_z = DAz;
    // don't call PreComputeDataAttachment() because the pointers are not NULL anymore
  }


  void GetDataAttach( float** DAx, float** DAy, float** DAz)
  {
    switch (advection_scheme) {
    case ADVECTION_UPWIND_VECTORS:
    case ADVECTION_CENTRAL_VECTORS:
      *DAx = data_attach_x;
      *DAy = data_attach_y;
      *DAz = data_attach_z;
      break;

      // In this case, just return the second order derivatives in the gradient direction (normalized)
      // and the norm of the gradient
    case ADVECTION_MORPHO:
      *DAx = secdergrad;
      *DAy = normgrad;
      *DAz = NULL;
      break;
    }
  }

  // just copy the values when processing the next iteration
  // the array provided MUST be allocated
  // or NULL for cancelling the action
  void GetCurvatureTerm(  float* data) { curvature_data = data; }
  void GetAdvectionTerm(  float* data) { advection_data = data; }
  void GetVelocityTerm(   float* data) { velocity_data  = data; }
  void GetBalloonTerm(    float* data) { balloon_data   = data; }
  void GetDistanceMap(    float* data) { distance_data  = data; }
  void GetLogCondTerm(    float* data) { logCond_data = data;}

  // Interface with TCL
  void GetCurvatureTerm(  vtkImageData* data) { curvature_data = (float*)data->GetScalarPointer(); }
  void GetAdvectionTerm(  vtkImageData* data) { advection_data = (float*)data->GetScalarPointer(); }
  void GetVelocityTerm(   vtkImageData* data) { velocity_data  = (float*)data->GetScalarPointer(); }
  void GetBalloonTerm(    vtkImageData* data) { balloon_data   = (float*)data->GetScalarPointer(); }
  void GetDistanceMap(    vtkImageData* data) { distance_data  = (float*)data->GetScalarPointer(); }
  void GetLogCondTerm(    vtkImageData* data) { logCond_data   = (float*)data->GetScalarPointer();}

  // public methods for multithreading ...
  int  SplitBand(int& first, int& last, int num, int total);
  void Evolve3D(int first_band, int last_band);

  void PrintParameters();

  // Updates the resulting image so that it contains the current result
  int UpdateResult();

  // Compute mean and standard deviation of the intensity
  // within the initial spheres or disks
  //
  //float *InitPointsStatistics();
  void InitPointsStatistics(float stats[2]);

  // Precompute the probability Lookup Table is the image
  // was rescaled to [0,255]
  void ComputeProbabilityLUT();

  // Probability of the tissue based on the image intensity
  float ExpansionMap( float I, unsigned char compute=0);
 
  // Disabled GetOutput bc it is not properly defined 
  vtkImageData* GetOutput();

protected:
  vtkImageLevelSets();
  ~vtkImageLevelSets();
  //vtkImageLevelSets(const vtkImageLevelSets&) {};
  //  void operator=(const vtkImageLevelSets&) {};
  //

  void          MakeBand( );
  void          MakeBand0( );
  void          ExtractCoords(   int p, short &x, short &y, short &z);


  void          Evolve();
  void          Evolve3D();
  void          Evolve2D();

  unsigned char CheckConvergence();
  void          CheckConvergenceNew();

  void          ExecuteInformation()
    {
      this->vtkImageToImageFilter::ExecuteInformation();
    };

  void PreComputeDataAttachment();

  void NormalizeSecDerGrad();

  //  void Execute(vtkImageData *inData, vtkImageData *outData);
  void ExecuteData( vtkDataObject *outData);

//BTX
  int   RescaleImage;
  float minu,maxu;

  // Minimal distance computed inside the structure for 
  // searching the skeleton in case of Shape-based level set
  int   ShapeMinDist; 

  int   Band;
  int   Tube;
  int   NumIters;
  int   DoMean;

  // Contour Attachment coeff
  float AdvectionCoeff;

  int   CheckFreq;
  int   ReinitFreq;
  float ConvergedThresh;
  float StepDt;
  float init_dt;
  float HistoGradThreshold;
  float coeff_curvature;

  int   DMmethod; // Distance Map algorithm
    // 0: curves
    // 1: fast marching
    // 2: Fast Signed Chamfer
    // 3: Shape based
    // 4: LogOdds 


  // for testing, create isocontour with 0.5 and -0.5 values
  int   IsoContourBin;

  int   UseCosTerm; // Use the cosinus term of Liana's paper

  int Dimension; // 2 for 2D and 3 for 3D
  int SliceNum; // slice number for 2D level set

  int tx,ty,tz,txy;
  int imsize;

  // values of the narrow Band
  float*         u[2];
  // current elt used
  int            current;

  //------------------- Narrow Band information
  int*             bnd;
  float*           bnd_initialvalues;
  unsigned char    bnd_allocated;
  unsigned char*   flag;
  unsigned char    flag_allocated;
  int              bnd_pc;
  int              bnd_maxsize;

  int              distmap_count;
  int              band_count;

  unsigned char *stored_seg;
  unsigned char stored_seg_allocated;

  // Save the intermediary distance maps
  unsigned char savedistmap;

  // Save the the second order derivatives of the gradient
  unsigned char savesecdergrad;

  unsigned char advection_scheme;

  unsigned char balloon_scheme;

  // Parameters for the evolution
  // current step
  int step;
  // reinitialize the contour ?
  int reinitcntr;

  int touched; // if the contour touched the outer narrow band

  int EvolveThreads; // number of threads for Evolve()

  unsigned char verbose;


  // Curvature Weight for the geodesic framework
  float*        curvature_weight;

  // Precomputed data attachment vector field  H*DI/|DI|
  float*        data_attach_x;
  float*        data_attach_y;
  float*        data_attach_z;

  float*        secdergrad;
  float*        normgrad;
  float         max_normgrad;
  

  float*  curvature_data;
  float*  advection_data;
  float*  velocity_data;
  float*  balloon_data;
  float*  distance_data;
  float*  logCond_data;


  // We copy input data to float, just to avoid
  // template function in .cxx
  vtkImageData *inputImage;
  int inputImage_allocated;

  vtkImageData* velocity;
  float         coeff_velocity;

  // Initialization of the balloon force
  float         balloon_coeff;
  float         balloon_value;
  vtkImageData* balloon_image;
  // Number of Gaussians for probability map
  int           NumGaussians;
  float**       Gaussians;
  float*        Probability;
  float         ProbabilityThreshold;
  float         ProbabilityHighThreshold;


  // Initialization Parameters
  vtkImageData* initImage;
  float         InitThreshold;
  enum TInitIntensity { Bright, Dark };
  TInitIntensity InitIntensity;


  // Kilian: IPMI 07
  vtkImageData* logCondIntensityImage;
  float logCondIntensityCoefficient;
  float logCondIntensityInside;
  float probCondWeightMin;

  // Number of seeding points for initialization
  int   NumInitPoints;
  int** InitPoints;

  // High and Low intensity thresholds
  int   UseLowThreshold;
  float LowThreshold;
  int   UseHighThreshold;
  float HighThreshold;


  vtkImageData *outputImage;

  // In case of shape-based levelset
  vtkImageData *SkeletonImage;
  unsigned char SkeletonImage_allocated;

  vtkLevelSetFastMarching* fm;

  vtkImageIsoContourDist*    isodist;
  vtkImageFastSignedChamfer* chamfer;
  vtkImagePropagateDist2*    shape;


  // some precomputed values
  // voxel size
  float vx,vy,vz;
  float sqxspacing,sqyspacing,sqzspacing;
  float doubxspacing,doubyspacing,doubzspacing;
  float xyspacing,yzspacing,xzspacing;

  int isotropic_voxels;

  // statistics per iteration
  double mean_curv     ;
  double mean_balloon  ;
  double mean_advection;
  double mean_vel      ;
  double mean_logCond  ;

  // Memory management
  float MemoryAllocated;

//ETX
};

#endif


