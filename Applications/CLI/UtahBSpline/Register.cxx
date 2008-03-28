
/*
 *
./Register fixed moving mask gd-step gd-tol ncpu ncpv [ncpw] p q [r] useMask alpha sigma movingOut maskOut transformOut

./Register fixed.nrrd moving.nrrd moving.nrrd 1 .01  5 5 5  3 3 3  0 1 5  movingOut.nrrd maskOut.nrrd transformout.transform
 *
 * */


#include "GradientDescent.h"

#include "NormalizedCorrelationCostFunction3D.h"
#include "NormalizedCorrelationCostFunction2D.h"

#include "itkRescaleIntensityImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDiscreteGaussianImageFilter.h"

#include <fstream>

#define DIMENSION 3
#define PRECISION double

#if DIMENSION == 3
  typedef NormalizedCorrelationCostFunction3D<PRECISION> TCostFunction;
#else
  typedef NormalizedCorrelationCostFunction3D<PRECISION> TCostFunction;
#endif

typedef TCostFunction::TParametric TParametric;
typedef TCostFunction::TControlPoint TControlPoint;
typedef TCostFunction::TControlMesh TControlMesh;

typedef TCostFunction::Image Image;
typedef TCostFunction::ImagePointer ImagePointer;
typedef Image::RegionType ImageRegion;
typedef Image::IndexType ImageIndex;
typedef Image::SizeType ImageSize;
typedef ImageIndex::IndexValueType IndexValue;

typedef itk::ImageFileReader<Image> ImageReader;
typedef ImageReader::Pointer ImageReaderPointer;

typedef itk::ImageFileWriter<Image> ImageWriter;


typedef itk::RescaleIntensityImageFilter<Image> RescaleFilter;
typedef RescaleFilter::Pointer RescaleFilterPointer;

typedef itk::DiscreteGaussianImageFilter<Image, Image> GaussianImageFilter;
typedef GaussianImageFilter::Pointer GaussianImageFilterPointer;

typedef itk::ImageFileReader<Image> ImageReader;
typedef ImageReader::Pointer ImageReaderPointer;

typedef itk::ImageFileWriter<Image> ImageWriter;
typedef ImageWriter::Pointer ImageWriterPointer;

ImageRegion findBoundingBox(ImagePointer maskImage){

  ImageIndex minIndex;
  minIndex.Fill( vnl_numeric_traits< IndexValue >::maxval );
  ImageIndex maxIndex;
  maxIndex.Fill(0);

  typedef itk::ImageRegionConstIteratorWithIndex<Image> MaskImageIterator;
  MaskImageIterator maskIt(maskImage, maskImage->GetLargestPossibleRegion());
  for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt){
    if(maskIt.Get() != 0){
      ImageIndex tmp = maskIt.GetIndex();
      for(int i = 0; i < Image::GetImageDimension(); i++){
        if(maxIndex[i] < tmp[i]){
          maxIndex[i] = tmp[i];
        }
        if(minIndex[i] > tmp[i]){
          minIndex[i] = tmp[i];
        }
      }
    }
  }
    


  ImageIndex boundsIndex;
  ImageSize boundsSize;
  for(int i = 0; i < Image::GetImageDimension(); i++){
    boundsSize[i] = (maxIndex[i]  - minIndex[i]);
    boundsIndex[i] = minIndex[i] - boundsSize[i]/2;
    boundsSize[i] *= 2;
  }

  ImageRegion maxRegion = maskImage->GetLargestPossibleRegion();
  ImageSize maxSize = maxRegion.GetSize();
  for(int i = 0; i < Image::GetImageDimension();  i++){
    if(boundsIndex[i] < 0){
      boundsIndex[i] = 0;
    }
    if(boundsIndex[i] + boundsSize[i] > maxSize[i]){
      boundsSize[i] = maxSize[i] - boundsIndex[i];
    }
  }

  ImageRegion bounds(boundsIndex, boundsSize);
  return bounds;
}


int main(int argc, char **argv){

 if(argc != 12 + 2 *DIMENSION){
   std::cout << "Usage:" << std::endl;
   std::cout << argv[0] << " fixed moving mask gd-step gd-tol ncpu ncpv [ncpw] p q [r]";
   std::cout << " useMask alpha sigma movingOut maskOut transformOut" << std::endl;
   return 0;
 } 
 try{ 

  double step = atof(argv[4]);
  double tol = atof(argv[5]);
  int argIndex = 6;
  int ncps[DIMENSION];

  int nElements = DIMENSION;
  for(int i=0; i < DIMENSION; i++){
    ncps[i] = atoi(argv[argIndex++]);
    nElements *= ncps[i];
  }
  int degree[DIMENSION];
  for(int i=0; i < DIMENSION; i++){
    degree[i] = atoi(argv[argIndex++]);
  }
  int useMask = atoi(argv[argIndex++]) != 0;
  double alpha = atof(argv[argIndex++]);
  std::cout << alpha << std::endl; 
  double sigma = atof(argv[argIndex++]);

  //Read Input Image
  ImageReaderPointer fixedImageReader = ImageReader::New();
  fixedImageReader->SetFileName( argv[1] );
  RescaleFilterPointer fixedRescaleFilter = RescaleFilter::New();
  fixedRescaleFilter->SetInput(fixedImageReader->GetOutput());
  fixedRescaleFilter->SetOutputMaximum(1);
  fixedRescaleFilter->SetOutputMinimum(0);
  fixedRescaleFilter->Update();
  GaussianImageFilterPointer gaussian1 = GaussianImageFilter::New();
  gaussian1->SetInput(fixedRescaleFilter->GetOutput());
  gaussian1->SetVariance(sigma*sigma);
  gaussian1->Update();
  ImagePointer fixedImage = gaussian1->GetOutput();



  ImageReaderPointer movingImageReader = ImageReader::New();
  movingImageReader->SetFileName( argv[2] );
  RescaleFilterPointer movingRescaleFilter = RescaleFilter::New();
  movingRescaleFilter->SetInput(movingImageReader->GetOutput());
  movingRescaleFilter->SetOutputMaximum(1);
  movingRescaleFilter->SetOutputMinimum(0);
  movingRescaleFilter->Update();
  GaussianImageFilterPointer gaussian2 = GaussianImageFilter::New();
  gaussian2->SetInput(movingRescaleFilter->GetOutput());
  gaussian2->SetVariance(sigma*sigma);
  gaussian2->Update();
  ImagePointer movingImage = gaussian2->GetOutput();

  ImageReaderPointer maskImageReader = ImageReader::New();
  maskImageReader->SetFileName( argv[3] );
  maskImageReader->Update();
  ImagePointer maskImage = maskImageReader->GetOutput();

  //Optimization
  ImageRegion region;
  if(useMask){
   region = findBoundingBox(maskImage);
  }
  else{
    region = movingImage->GetLargestPossibleRegion();
  }
  ImageIndex index = region.GetIndex();
  ImageSize size = region.GetSize();
  double *rangeIndex =  new double[DIMENSION];
  double *rangeSize =  new double[DIMENSION];
  for(int i=0; i < DIMENSION; i++){
    rangeIndex[i] = index[i];
    rangeSize[i] = size[i];
  }
  TParametric surface = TParametric::createIdentity( rangeIndex, rangeSize, ncps, degree);
 

  TCostFunction costFunction(nElements);
  costFunction.SetFixedImage(fixedImage);
  costFunction.SetMovingImage(movingImage);
  costFunction.SetMaskImage(maskImage);
  costFunction.SetRange( region );
  costFunction.SetParametric(surface);
  costFunction.SetUseMask(useMask);
  vnl_vector<double> params(nElements);
  costFunction.GetVNLParametersFromParametric(params);
  costFunction.SetAlpha(alpha);
  costFunction.Init();
  costFunction.SetVerbose(10);

  //std::cout << costFunction.get_number_of_residuals() << std::endl;
  //vnl_vector<double> r(size[0] * size[1]);
 // costFunction.f(params);
  //std::cout << r << std::endl;



/* 
  //vnl_levenberg_marquardt optimizer(costFunction);
  vnl_conjugate_gradient optimizer(costFunction);
  optimizer.set_verbose(true);
  optimizer.set_trace(true);
  optimizer.set_max_function_evals(10);
  std::cout << "ftol: " << optimizer.get_f_tolerance() << std::endl;
  std::cout << "xtol: " << optimizer.get_x_tolerance() << std::endl;
  std::cout << "gtol: " << optimizer.get_g_tolerance() << std::endl;
  std::cout << "epstol: " << optimizer.get_epsilon_function() << std::endl;
  std::cout << "max f evals: " << optimizer.get_max_function_evals() << std::endl;

  optimizer.minimize( params );

  optimizer.diagnose_outcome();
*/

  
  GradientDescent optimizer;
  optimizer.SetTolerance(tol);
  optimizer.SetStepSize(step);
  optimizer.Minimize(costFunction, params);
  optimizer.SetVerbose(10);

  //Write ouput image
  ImagePointer output = costFunction.GetTransformedImage();
  ImageWriterPointer imageWriter = ImageWriter::New();
  imageWriter->SetFileName(argv[argIndex++]);
  imageWriter->SetInput(output);
  imageWriter->Update();
  imageWriter->Write();

  ImagePointer maskOutput = costFunction.GetTransformedMaskImage();
  imageWriter = ImageWriter::New();
  imageWriter->SetFileName(argv[argIndex++]);
  imageWriter->SetInput(maskOutput);
  imageWriter->Update();
  imageWriter->Write();


  //write surface
  std::ofstream file;
  file.open(argv[argIndex++]);
  file << costFunction.GetTransformation() << std::endl;
  file.close(); 

  return 0;
 }
 catch(const char *err){
  std::cout << err << std::endl;
 }

}

