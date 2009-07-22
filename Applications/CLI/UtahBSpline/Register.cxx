#define USE_ORIENTED
#define PRECISION 
typedef float Precision;

#include <math.h>
#include "Config.h"

#include "ImageIO.h"


#include "GradientDescent.h"
#include "NormalizedCorrelationCostFunction3D.h"
#include "NormalizedCorrelationCostFunction2D.h"

//itk includes
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkResampleImageFilter.h"

#include <fstream>

//Command line parsing
#include "CmdLine.h"

//Normalized correlation cost for BSplines warps
#if DIMENSION == 3
  typedef NormalizedCorrelationCostFunction3D<Precision, Image> TCostFunction;
#else
  typedef NormalizedCorrelationCostFunction2D<Precision, Image> TCostFunction;
#endif

//Parametric surface - i.e. BSpline - for the wwarp
typedef TCostFunction::TParametric TParametric;
//Controlpoint of bspline
typedef TCostFunction::TControlPoint TControlPoint;
//Controlmesh of bspline
typedef TCostFunction::TControlMesh TControlMesh;


//itk typedefs
typedef itk::RescaleIntensityImageFilter<Image> RescaleFilter;
typedef RescaleFilter::Pointer RescaleFilterPointer;

typedef itk::CastImageFilter<Image, Image> CastFilter;
typedef CastFilter::Pointer CastFilterPointer;

typedef itk::SmoothingRecursiveGaussianImageFilter<Image, Image> GaussianImageFilter;
typedef GaussianImageFilter::Pointer GaussianImageFilterPointer;

//typedef itk::DiscreteGaussianImageFilter<Image, Image> GaussianImageFilter;
//typedef GaussianImageFilter::Pointer GaussianImageFilterPointer;

typedef itk::ResampleImageFilter<Image, Image> ResampleFilter;
typedef ResampleFilter::Pointer ResampleFilterPointer;
    
typedef Image::PointType ImagePoint;



//Create image pyramid
ImagePointer *downsample(ImagePointer im, int nres, Precision sigma){
  ImagePointer *pyramid = new ImagePointer[nres];
  pyramid[0] = im; 
  //Downsample
  for(int i=1; i<nres; i++){
    GaussianImageFilterPointer smooth = GaussianImageFilter::New();
    smooth->SetSigma(sigma);
    smooth->SetInput(pyramid[i-1]);
    smooth->Update();

    ResampleFilterPointer downsample = ResampleFilter::New();
    downsample->SetInput(smooth->GetOutput());
                
    ImageSize size = pyramid[i-1]->GetLargestPossibleRegion().GetSize();
    ImageSpacing spacing = pyramid[i-1]->GetSpacing();
    for(unsigned int n = 0; n < size.GetSizeDimension(); n++){
      size[n] = size[n]/2;
      spacing[n] = spacing[n]*2;
    }
    downsample->UseReferenceImageOff();
    downsample->SetOutputOrigin(pyramid[i-1]->GetOrigin()); 
    downsample->SetOutputDirection(pyramid[i-1]->GetDirection());
    downsample->SetOutputSpacing(spacing); 
    downsample->SetSize(size);

    downsample->Update();
    pyramid[i] = downsample->GetOutput();    
  }
  return pyramid;
};

//Compute region for registration
ImageRegion computeRegion(ImagePointer image){  
  
  //Find bounding box
  Converter conv(image);
  ImageRegion region = conv.getBounds();

  //Add 10 pixels to region
  ImageRegion largest = image->GetLargestPossibleRegion();
  ImageIndex lIndex = largest.GetIndex();
  ImageSize lSize = largest.GetSize();

  ImageIndex rIndex = region.GetIndex();
  ImageSize rSize = region.GetSize();
  for(int i=0; i<DIMENSION; i++){
    rIndex[i] -= 10;
    if(lIndex[i] > rIndex[i]){
      rIndex[i] = lIndex[i];
    }
    rSize[i] += 10;
    if(rSize[i] > lSize[i]){
      rSize[i] = lSize[i];
    }
  }
  return region;
}




//Compute bspline registration for between tow images (moving to fixed) and
//saves deformed moving image and the transformation to be used with
//BSPlineTransfrom to deform any other images with the same transformation.
int main(int argc, char **argv){
  
  
  //------ Command line parsin  
  TCLAP::CmdLine cmd("BSpline image registration", ' ', "1");

  TCLAP::ValueArg<std::string> fixedArg("f","fixed","fixed image", true, "",
      "filename");
  cmd.add(fixedArg);
  
  TCLAP::ValueArg<std::string> movingArg("m","moving","moving image", true, "",
      "filename");
  cmd.add(movingArg);

  TCLAP::ValueArg<std::string> maskArg("k","mask","mask image", true, "",
      "filename");
  cmd.add(maskArg);
  
  TCLAP::ValueArg<std::string> warpedArg("w","warped","warped output image", false, "",
      "filename");
  cmd.add(warpedArg);
  
  TCLAP::ValueArg<std::string> deformArg("d","deformation",
      "deformation vector field output image", false, "", 
      "filename");
  cmd.add(deformArg);

  TCLAP::ValueArg<Precision> alphaArg("a","alpha",
      "regularization: 0 - no regularization, 1 - only regularization. Penalizes controlpoint distance", 
      false, (Precision)0.2, 
      "regularization");
  cmd.add(alphaArg);

  TCLAP::ValueArg<Precision> sigmaArg("s","sigma",
      "Intial gaussian blurring", 
      false, 0,  "smooth images for regsitration");
  cmd.add(sigmaArg);
  
  
  TCLAP::ValueArg<unsigned int> n1Arg("","n1",
      "number of control points in dimension 1", 
      false, 10,  "number of control points");
  cmd.add(n1Arg);

  TCLAP::ValueArg<unsigned int> n2Arg("","n2",
      "number of control points in dimension 2", 
      false, 10,  "number of control points");
  cmd.add(n2Arg);

#if DIMENSION == 3
  TCLAP::ValueArg<unsigned int> n3Arg("","n3",
      "number of control points in dimension 3", 
      false, 10,  "number of control points");
  cmd.add(n3Arg);
#endif


  TCLAP::ValueArg<unsigned int> d1Arg("","d1",
      "degree in dimension 1", 
      false, 2,  "degree");
  cmd.add(d1Arg);  
  
  
  TCLAP::ValueArg<unsigned int> resolutionArg("","resolutions",
      "number of resolutions", 
      false, 1,  "resolutions");
  cmd.add(resolutionArg);

  TCLAP::ValueArg<unsigned int> d2Arg("","d2",
      "degree in dimension 2", 
      false, 2,  "degree");
  cmd.add(d2Arg);

#if DIMENSION == 3
  TCLAP::ValueArg<unsigned int> d3Arg("","d3",
      "degree in dimension 3", 
      false, 2,  "degree");
  cmd.add(d3Arg);
#endif

  TCLAP::ValueArg<double> stepArg("","gdstep",
      "gradient descent step size", 
      false, 1,  "step size");
  cmd.add(stepArg);

  TCLAP::ValueArg<double> tolArg("","gdtol",
      "gradient descent stopping tolerance", 
      false, 0.011,  "gdtol");
  cmd.add(tolArg); 
  
  TCLAP::ValueArg<bool> fixedBoundaryArg("","fixedBoundary",
      "Fix BSpline boundary", 
      false, true,  "True/False");
  cmd.add(fixedBoundaryArg);

  try{
    cmd.parse( argc, argv );
  } 
  catch (TCLAP::ArgException &e){ 
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; 
    return -1;
  }




  //Get Input args
  double step = stepArg.getValue();
  double tol = tolArg.getValue();
  int ncps[DIMENSION];
  ncps[0] = n1Arg.getValue();
  ncps[1] = n2Arg.getValue();
#if DIMENSION == 3
  ncps[2] = n3Arg.getValue();
#endif
  int nElements = DIMENSION;
  for(int i=0; i < DIMENSION; i++){
    nElements *= ncps[i];
  }

  int degree[DIMENSION];  
  degree[0] = d1Arg.getValue();
  degree[1] = d2Arg.getValue();
#if DIMENSION == 3
  degree[2] = d3Arg.getValue();
#endif

  double alpha = alphaArg.getValue();
  double sigma = sigmaArg.getValue();


  //----- End command line parsing




  //--- Fixed image
  ImagePointer fixedInImage = ImageIO<Image>::readImage(fixedArg.getValue()); 
  //Scale image from 0 - 1
  RescaleFilterPointer fixedRescaleFilter = RescaleFilter::New();
  fixedRescaleFilter->SetInput(fixedInImage);
  fixedRescaleFilter->SetOutputMaximum(1);
  fixedRescaleFilter->SetOutputMinimum(0);
  fixedRescaleFilter->Update();
  ImagePointer  fixedImage = fixedRescaleFilter->GetOutput();


  //--- Moving image
  ImagePointer movingInImage = ImageIO<Image>::readImage(movingArg.getValue());
  //Scale image from 0 - 1
  RescaleFilterPointer movingRescaleFilter = RescaleFilter::New();
  movingRescaleFilter->SetInput(movingInImage);
  movingRescaleFilter->SetOutputMaximum(1);
  movingRescaleFilter->SetOutputMinimum(0);
  movingRescaleFilter->Update();
  
  ImagePointer movingImage = movingRescaleFilter->GetOutput();

  ImagePointer mask = NULL;
  if(maskArg.getValue().length() != 0){
    mask = ImageIO<Image>::readImage(maskArg.getValue());
  }

  ImageRegion region;
  if(mask.IsNotNull()){
    region = computeRegion(mask);
  }
  else{
    region = computeRegion(fixedImage);
  }


  //--- Setup normalized correlation cost function
  TCostFunction costFunction(nElements);
  costFunction.SetFixedImage(fixedImage);
  costFunction.SetMovingImage(movingImage);
  costFunction.SetMaskImage(mask);
  costFunction.SetRange( region );
  costFunction.SetUseMask(false);
  costFunction.SetFixedBoundary(fixedBoundaryArg.getValue());
  costFunction.ComputePhysicalRange(region);

  //Create Identity transfrom
  ImagePoint rangeStart = costFunction.GetStart();
  ImagePoint rangeSize = costFunction.GetSize();
  std::cout << rangeStart << std::endl;
  std::cout << rangeSize << std::endl;
  
  Precision rStart[DIMENSION];
  Precision rSize[DIMENSION];
  Precision maxSize = 0;
  for(int i=0; i<DIMENSION; i++){
    rStart[i] = rangeStart[i];
    rSize[i] = rangeSize[i];
    if(maxSize < fabs(rSize[i])){
      maxSize = fabs(rSize[i]);
    }
  }   
  
  costFunction.SetAlpha(alpha/maxSize);

  TParametric surface = TParametric::createIdentity( rStart, rSize, ncps, degree);
  costFunction.SetParametric(surface);

  costFunction.Init();
  costFunction.SetVerbose(10);


 

  //----- Do multiresolution optimization
  int nres = resolutionArg.getValue();
  ImagePointer *fixedPyramid = downsample(fixedImage, nres, sigma);
  ImagePointer *maskPyramid = NULL;
  if(mask.IsNotNull()){
    maskPyramid = downsample(mask, nres, sigma);
  }
  ImagePointer *movingPyramid = downsample(movingImage, nres, sigma);


  for(int i=nres-1; i>=0; i--){
  
    GradientDescent optimizer;  
    optimizer.SetStepSize(step*maxSize * pow(2, i) );

    //Set lowerresolution images
    costFunction.SetFixedImage(fixedPyramid[i]);
    if(mask.IsNotNull()){
      costFunction.SetMaskImage(maskPyramid[i]); 
      region = computeRegion(maskPyramid[i]);
    }else{
      costFunction.SetMaskImage(fixedPyramid[i]);
      region = computeRegion(fixedPyramid[i]);
    }
    costFunction.SetMovingImage(movingPyramid[i]);
    costFunction.SetRange(region);
    
    std::cout << fixedPyramid[i]->GetLargestPossibleRegion() << std::endl;
    std::cout << fixedPyramid[i]->GetSpacing() << std::endl;
    //Set up optimizer
    optimizer.SetTolerance(tol / pow(10, i) );
    std::cout << "Tol: " << tol / pow(10, i) << std::endl;

    optimizer.SetVerbose(10);

    //--- minimize cost function 

    //intial parameters
    vnl_vector<double> params(nElements);
    costFunction.GetVNLParametersFromParametric(params);

    optimizer.Minimize(costFunction, params);
  }


  //Write ouput image
  TCostFunction::TImageTransformation &transform = costFunction.GetTransformation();
  region = computeRegion(fixedImage); 
  transform.SetRange(region);    
  
  CastFilterPointer castFilter = CastFilter::New();
  castFilter->SetInput(fixedImage);
  castFilter->Update();
  ImagePointer output = castFilter->GetOutput(); 
  transform.Transform(output, movingInImage);
  ImageIO<Image>::saveImage(output, warpedArg.getValue());

  //save bspline 
  std::ofstream file;
  file.open(deformArg.getValue().c_str());
  file << costFunction.GetTransformation() << std::endl;
  file.close(); 

  return 0;

}

