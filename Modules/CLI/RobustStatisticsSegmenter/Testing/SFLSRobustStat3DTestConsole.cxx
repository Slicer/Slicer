

#include "SFLSRobustStatSegmentor3DLabelMap_single.h"

// ITK includes
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

// ITK includes
#include <itkConfigure.h>
#include <itkFactoryRegistration.h>


#include "labelMapPreprocessor.h"

template <typename TPixel>
itk::Image<short, 3>::Pointer
getFinalMask(typename itk::Image<TPixel, 3>::Pointer img, unsigned char l, TPixel thod = 0);

int main(int argc, char* * argv)
{
  itk::itkFactoryRegistration();

  if( argc != 7 )
    {
    std::cerr << "Parameters: inputImage labelImageName outputImage expectedVolume intensityHomo[0~1] lambda[0~1]\n";
    exit(-1);
    }

  std::string originalImageFileName(argv[1]);
  std::string labelImageFileName(argv[2]);
  std::string segmentedImageFileName(argv[3]);
  double      expectedVolume = atof(argv[4]);
  double      intensityHomogeneity = atof(argv[5]);
  double      curvatureWeight = atof(argv[6]);

  double maxRunningTime = 10000;
  short  labelValue = 1;

  typedef short                                         PixelType;
  typedef CSFLSRobustStatSegmentor3DLabelMap<PixelType> SFLSRobustStatSegmentor3DLabelMap_c;

  // read input image
  typedef SFLSRobustStatSegmentor3DLabelMap_c::TImage Image_t;

  typedef itk::ImageFileReader<Image_t> ImageReaderType;
  ImageReaderType::Pointer reader = ImageReaderType::New();
  reader->SetFileName(originalImageFileName.c_str() );
  Image_t::Pointer img;

  try
    {
    reader->Update();
    img = reader->GetOutput();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    raise(SIGABRT);
    }

  // read input label image
  typedef SFLSRobustStatSegmentor3DLabelMap_c::TLabelImage LabelImage_t;

  typedef itk::ImageFileReader<LabelImage_t> LabelImageReader_t;
  LabelImageReader_t::Pointer readerLabel = LabelImageReader_t::New();
  readerLabel->SetFileName(labelImageFileName.c_str() );
  LabelImage_t::Pointer labelImg;

  try
    {
    readerLabel->Update();
    labelImg = readerLabel->GetOutput();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    raise(SIGABRT);
    }

  // preprocess label map (labelImg, the naming is confusing.....)
  LabelImage_t::Pointer newLabelMap = preprocessLabelMap<LabelImage_t::PixelType>(labelImg, labelValue);

  // do seg
  SFLSRobustStatSegmentor3DLabelMap_c seg;
  seg.setImage(img);

  seg.setNumIter(10000); // a large enough number, s.t. will not be stopped by this criteria.
  seg.setMaxVolume(expectedVolume);
  seg.setInputLabelImage(newLabelMap);

  seg.setMaxRunningTime(maxRunningTime);

  seg.setIntensityHomogeneity(intensityHomogeneity);
  seg.setCurvatureWeight(curvatureWeight / 1.5);

  seg.doSegmenation();

  typedef itk::Image<short, 3> MaskImageType;

  MaskImageType::Pointer finalMask = getFinalMask<float>(seg.mp_phi, labelValue, 2.0);
  finalMask->CopyInformation(img);

  typedef itk::ImageFileWriter<MaskImageType> WriterType;
  WriterType::Pointer outputWriter = WriterType::New();
  outputWriter->SetFileName(segmentedImageFileName.c_str() );
  outputWriter->SetInput(finalMask);
  outputWriter->Update();

  try
    {
    outputWriter->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cout << "ExceptionObject caught !" << std::endl;
    std::cout << err << std::endl;
    raise(SIGABRT);
    }

  return EXIT_SUCCESS;
}

template <typename TPixel>
itk::Image<short, 3>::Pointer
getFinalMask(typename itk::Image<TPixel, 3>::Pointer img, unsigned char l, TPixel thod)
{
  typedef itk::Image<short, 3> MaskType;

  MaskType::SizeType size = img->GetLargestPossibleRegion().GetSize();

  long nx = size[0];
  long ny = size[1];
  long nz = size[2];

  MaskType::Pointer   mask = MaskType::New();
  MaskType::IndexType start = {{0, 0, 0}};

  MaskType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );

  mask->SetRegions( region );

  mask->SetSpacing(img->GetSpacing() );
  mask->SetOrigin(img->GetOrigin() );

  mask->Allocate();
  mask->FillBuffer(0);
  for( long ix = 0; ix < nx; ++ix )
    {
    for( long iy = 0; iy < ny; ++iy )
      {
      for( long iz = 0; iz < nz; ++iz )
        {
        MaskType::IndexType idx = {{ix, iy, iz}};
        TPixel              v = img->GetPixel(idx);

        mask->SetPixel(idx, v <= thod ? l : 0);
        }
      }
    }

  return mask;
}
