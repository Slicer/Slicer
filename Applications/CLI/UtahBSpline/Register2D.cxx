#include "Common2D.h"
#include "GradientDescent.h"

#include "itkRescaleIntensityImageFilter.h"
#include "itkImageRegionConstIteratorWithIndex.h"


typedef itk::ImageFileReader<Image> ImageReader;
typedef ImageReader::Pointer ImageReaderPointer;

typedef itk::ImageFileWriter<Image> ImageWriter;
typedef ImageWriter::Pointer ImageWriterPointer;

typedef itk::RescaleIntensityImageFilter<Image> RescaleFilter;
typedef RescaleFilter::Pointer RescaleFilterPointer;


int main(int argc, char **argv){

 if(argc != 15){
   std::cout << "Usage:" << std::endl;
   std::cout << argv[0] << " fixed moving mask gd-step gd-tol ncpu ncpv p q";
   std::cout << " useMask alpha movingOut maskOut transformOut" << std::endl;
   return 0;
 } 
 try{ 
  //Read Input Image
  ImageReaderPointer fixedImageReader = ImageReader::New();
  fixedImageReader->SetFileName( argv[1] );
  RescaleFilterPointer fixedRescaleFilter = RescaleFilter::New();
  fixedRescaleFilter->SetInput(fixedImageReader->GetOutput());
  fixedRescaleFilter->SetOutputMaximum(1);
  fixedRescaleFilter->SetOutputMinimum(0);
  fixedRescaleFilter->Update();
  ImagePointer fixedImage = fixedRescaleFilter->GetOutput();



  ImageReaderPointer movingImageReader = ImageReader::New();
  movingImageReader->SetFileName( argv[2] );
  RescaleFilterPointer movingRescaleFilter = RescaleFilter::New();
  movingRescaleFilter->SetInput(movingImageReader->GetOutput());
  movingRescaleFilter->SetOutputMaximum(1);
  movingRescaleFilter->SetOutputMinimum(0);
  movingRescaleFilter->Update();
  ImagePointer movingImage = movingRescaleFilter->GetOutput();

  ImageReaderPointer maskImageReader = ImageReader::New();
  maskImageReader->SetFileName( argv[3] );
  maskImageReader->Update();
  ImagePointer maskImage = maskImageReader->GetOutput();

  double step = atof(argv[4]);
  double tol = atof(argv[5]);
  int ncps[2];
  ncps[0] = atoi(argv[6]);
  ncps[1] = atoi(argv[7]);
  int degree[2];
  degree[0] = atoi(argv[8]);
  degree[1] = atoi(argv[9]);
  int useMask = atoi(argv[10]) != 0;
  double alpha = atof(argv[11]);

 
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
  double *rangeIndex =  new double[2];
  double *rangeSize =  new double[2];
  for(int i=0; i < 2; i++){
    rangeIndex[i] = index[i];
    rangeSize[i] = size[i];
  }
  TParametric surface = TParametric::createIdentity( rangeIndex, rangeSize, ncps, degree);
 

  TCostFunction costFunction(ncps[0] * ncps[1] * 2);
  costFunction.SetFixedImage(fixedImage);
  costFunction.SetMovingImage(movingImage);
  costFunction.SetMaskImage(maskImage);
  costFunction.SetRange( region );
  costFunction.SetSurface(surface);
  costFunction.SetUseMask(useMask);
  vnl_vector<double> params(ncps[0]*ncps[1]*2);
  costFunction.SurfaceToVNLParameters(params);
  costFunction.SetAlpha(alpha);
  costFunction.Init();

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

  //Write ouput image
  ImagePointer output = costFunction.GetTransformedImage();
  ImageWriterPointer imageWriter = ImageWriter::New();
  imageWriter->SetFileName(argv[12]);
  imageWriter->SetInput(output);
  imageWriter->Update();
  imageWriter->Write();

  ImagePointer maskOutput = costFunction.GetTransformedMaskImage();
  imageWriter = ImageWriter::New();
  imageWriter->SetFileName(argv[13]);
  imageWriter->SetInput(maskOutput);
  imageWriter->Update();
  imageWriter->Write();


  //write surface
  std::ofstream file;
  file.open(argv[14]);
  file << costFunction.GetTransformation() << std::endl;
  file.close(); 

  return 0;
 }
 catch(const char *err){
  std::cout << err << std::endl;
 }

}

