#include "vtkPichonFastMarching.h"
#include "itkImageFileReader.h"
#include "vtkImageCast.h"
#include "itkImageFileWriter.h"
#include "vtkNRRDReader.h"
#include "TestingMacros.h"
#include "itkImageToVTKImageFilter.h"
#include "itkVTKImageToImageFilter.h"
#include "vtkImageThreshold.h"
#include "itkImageDuplicator.h"

typedef itk::Image<short, 3> ImageType;
typedef itk::ImageToVTKImageFilter<ImageType> ITK2VTKType;
typedef itk::VTKImageToImageFilter<ImageType> VTK2ITKType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::ImageFileWriter<ImageType> WriterType;
typedef itk::ImageDuplicator<ImageType> DupType;
 
int vtkPichonFastMarchingTest(int argc, char * argv[] ){
  vtkSmartPointer<vtkPichonFastMarching> filter = 
    vtkSmartPointer<vtkPichonFastMarching>::New();

  EXERCISE_BASIC_OBJECT_METHODS(filter);

  if(argc == 1)
    return EXIT_SUCCESS;

  // otherwise, the arguments are expected to be:
  // nrrd_image <x y z coordinates of the initial seed in IJK> target_volume
  
  int seed[3];
  int volume;
  double *spacing;
  int *dim;
  double range[2];

  char *inputName = argv[1];
  seed[0] = atoi(argv[2]);
  seed[1] = atoi(argv[3]);
  seed[2] = atoi(argv[4]);
  volume = atoi(argv[5]);
  char *outputName = argv[6];

 
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputName);
  reader->Update();

  VTK2ITKType::Pointer vtk2itk = VTK2ITKType::New();

  ImageType::Pointer inputImageITK = reader->GetOutput();

  vtkNRRDReader* inputReader = vtkNRRDReader::New();
  vtkImageData* outputImage = vtkImageData::New();
  vtkImageData* inputImage;
  inputReader->SetFileName(inputName);
  inputReader->Update();

  inputImage = inputReader->GetOutput();
  outputImage->SetExtent(inputImage->GetExtent());
  
  dim = inputImage->GetWholeExtent();
  inputImage->GetScalarRange(&range[0]);
  spacing = inputImage->GetSpacing();

  vtkImageThreshold* thresh = vtkImageThreshold::New();
  thresh->SetInput(inputImage);
  thresh->SetReplaceIn(1);
  thresh->SetReplaceOut(1);
  thresh->SetInValue(0);
  thresh->SetOutValue(0);
  thresh->SetOutputScalarTypeToShort();
  thresh->SetOutput(outputImage);
  thresh->Update();
  thresh->Delete();

  filter->SetInput(inputImage);
  filter->SetOutput(outputImage);
  filter->init(dim[1]+1, dim[3]+1, dim[5]+1, range[1]-range[0], 
    spacing[0], spacing[1], spacing[2]);

  filter->Modified();
  filter->Update();

  filter->setActiveLabel(1);
  filter->initNewExpansion();

  filter->setNPointsEvolution(10000);
  filter->setActiveLabel(1);
  filter->addSeedIJK(seed[0], seed[1], seed[2]);
  filter->Modified();
  filter->SetOutput(outputImage);
  filter->Update();

  filter->show(1.);
  filter->Modified();
  filter->Update();

  vtk2itk->SetInput(outputImage);
  vtk2itk->Update();

  DupType::Pointer dup = DupType::New();
  dup->SetInputImage(vtk2itk->GetOutput());
  dup->Update();

  ImageType::Pointer outputImageITK = dup->GetOutput();
  outputImageITK->DisconnectPipeline();
  outputImageITK->SetSpacing(inputImageITK->GetSpacing());
  outputImageITK->SetDirection(inputImageITK->GetDirection());
  outputImageITK->SetOrigin(inputImageITK->GetOrigin());

  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputName);
  writer->SetInput(outputImageITK);
  writer->Update();

  filter->unInit();
  inputReader->Delete();
  outputImage->Delete();

  return EXIT_SUCCESS;
}
