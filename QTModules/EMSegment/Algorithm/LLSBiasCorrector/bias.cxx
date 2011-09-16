
#include "LLSBiasCorrector.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkOutputWindow.h"
#include "itkTextOutput.h"

#include <iostream>
#include <sstream>
#include <vector>

#include <math.h>
#include <stdlib.h>

#include "DynArray.h"

int main(int argc, char** argv)
{
  if (argc < 5)
  {
    std::cerr << "Bias field correction for n images with m fuzzy probabilities"
      << std::endl;
    std::cerr << argv[0] << " m n "
      << " <prob_1> ... <prob_m> <image_1> ... <image_n>" << std::endl;
  }

  // Use text output
  itk::TextOutput::Pointer textout = itk::TextOutput::New();
  itk::OutputWindow::SetInstance(textout);

  typedef itk::Image<float, 3> FloatImageType;
  typedef itk::Image<short, 3> ShortImageType;

  unsigned int numProbs = (unsigned int)atoi(argv[1]);
  unsigned int numImages = (unsigned int)atoi(argv[2]);

  // Read probabilities
  DynArray<FloatImageType::Pointer> probImages;
  for (unsigned int i = 0; i < numProbs; i++)
  {
    typedef itk::ImageFileReader<FloatImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(argv[3+i]);
    reader->Update();
    probImages.Append(reader->GetOutput());
  }

  // Read input images
  DynArray<FloatImageType::Pointer> inputImages;
  for (unsigned int i = 0; i < numImages; i++)
  {
    typedef itk::ImageFileReader<FloatImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(argv[3+numProbs+i]);
    reader->Update();
    inputImages.Append(reader->GetOutput());
  }

  //
  // Do bias correction
  //

  typedef LLSBiasCorrector<FloatImageType, FloatImageType> BiasCorrectorType;

  // Compute mask for getting samples (sum of probs > 0)
  BiasCorrectorType::MaskImageType::Pointer maskImg =
    BiasCorrectorType::MaskImageType::New();
  maskImg->SetRegions(inputImages[0]->GetLargestPossibleRegion());
  maskImg->Allocate();
  maskImg->SetSpacing(inputImages[0]->GetSpacing());
  maskImg->FillBuffer(0);

  typedef itk::ImageRegionIteratorWithIndex<BiasCorrectorType::MaskImageType>
    IteratorType;
  IteratorType it(maskImg, maskImg->GetLargestPossibleRegion());
  for (it.GoToBegin(); !it.IsAtEnd(); ++it)
  {
    FloatImageType::IndexType ind = it.GetIndex();
    double sump = 0;
    for (unsigned int i = 0; i < numProbs; i++)
      sump += probImages[i]->GetPixel(ind);
    if (sump > 0)
      it.Set(1);
  }

  BiasCorrectorType::Pointer biascorr = BiasCorrectorType::New();

  biascorr->SetClampBias(false);
  biascorr->SetSampleSpacing(4.0);
  biascorr->SetMask(maskImg);
  biascorr->SetMaxDegree(4); // Most MRI protocols do well with poly order 4
  biascorr->SetProbabilities(probImages);

  // Do correction and store results in place
  // Method params is input, output, do all voxels
  biascorr->CorrectImages(inputImages, inputImages, true);

  // Write output as short image in [0, 4096]

  for (unsigned int i = 0; i < numImages; i++)
  {
    typedef itk::RescaleIntensityImageFilter<FloatImageType, ShortImageType>
      ConverterType;

    ConverterType::Pointer converter = ConverterType::New();
    converter->SetOutputMinimum(0);
    converter->SetOutputMaximum(4096);
    converter->SetInput(inputImages[i]);
    converter->Update();

    typedef itk::ImageFileWriter<ShortImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();

    std::ostringstream oss;
    oss << "corrected_image" << i << ".mha" << std::ends;
 
    writer->SetFileName(oss.str().c_str());
    writer->SetInput(converter->GetOutput());
    writer->Update();
  }

  return 0;

}
