
#include "LLSBiasCorrector.h"
//#include "DiffusionBasedBiasCorrector.h"

#include "itkDiscreteGaussianImageFilter.h"
#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkOutputWindow.h"
#include "itkTextOutput.h"

using namespace itk;

#include <iostream>
#include <sstream>

#include <math.h>
#include <stdlib.h>

int main(int argc, char** argv)
{

  // Use text output
  itk::TextOutput::Pointer textout = itk::TextOutput::New();
  itk::OutputWindow::SetInstance(textout);

  srand(39280694);

  typedef Image<float, 3> FloatImageType;
  typedef Image<unsigned short, 3> UShortImageType;

  typedef LLSBiasCorrector<FloatImageType, FloatImageType> BiasCorrectorType;

/*
  typedef DiffusionBasedBiasCorrector<FloatImageType, FloatImageType>
    BiasCorrectorType2;
*/

  typedef itk::RescaleIntensityImageFilter<FloatImageType, UShortImageType>
    ConverterType;

  typedef ImageFileWriter<UShortImageType> WriterType;

  // Generate images
  FloatImageType::IndexType index;

  FloatImageType::RegionType region;
  FloatImageType::SizeType size;

  size[0] = 128;
  size[1] = 128;
  size[2] = 128;
  region.SetSize(size);

  FloatImageType::SpacingType spacing;
  spacing[0] = 1.0;
  spacing[1] = 1.0;
  spacing[2] = 1.0;

  unsigned int numChannels = 2;

  // Image data (cube)
  std::cout << "Generate test image..." << std::endl;

  DynArray<FloatImageType::Pointer> images;
  DynArray<FloatImageType::Pointer> corrImages;

  for (int i = 0; i < numChannels; i++)
  {
    double fg = rand() / (double)RAND_MAX * 500;
    double bg = rand() / (double)RAND_MAX * 50;

    FloatImageType::Pointer img = FloatImageType::New();

    img->SetRegions(region);
    img->SetSpacing(spacing);
    img->Allocate();

    FloatImageType::Pointer corrImg = FloatImageType::New();

    corrImg->SetRegions(region);
    corrImg->SetSpacing(spacing);
    corrImg->Allocate();

    for (index[0] = 0; index[0] < size[0]; index[0]++)
      for (index[1] = 0; index[1] < size[1]; index[1]++)
        for (index[2] = 0; index[2] < size[2]; index[2]++)
        {
          if ((index[0] >= 32 && index[0] <= 96)
            &&
            (index[1] >= 32 && index[1] <= 96)
            &&
            (index[2] >= 32 && index[2] <= 96))
            img->SetPixel(index, fg);
          else
            img->SetPixel(index, bg);
        }

   images.Append(img);
   corrImages.Append(corrImg);
  }

  // Foreground probability
  std::cout << "Generating probabilities..." << std::endl;
  FloatImageType::Pointer fgprob = FloatImageType::New();

  fgprob->SetRegions(region);
  fgprob->Allocate();

  fgprob->SetSpacing(spacing);

  fgprob->FillBuffer(0);

  for (index[0] = 0; index[0] < size[0]; index[0]++)
    for (index[1] = 0; index[1] < size[1]; index[1]++)
      for (index[2] = 0; index[2] < size[2]; index[2]++)
      {
        if (index[0] < 32 || index[0] > 96)
          continue;
        if (index[1] < 32 || index[1] > 96)
          continue;
        if (index[2] < 32 || index[2] > 96)
          continue;
        fgprob->SetPixel(index, 1.0);
      }

  // Background probabilities
  FloatImageType::Pointer bgprob = FloatImageType::New();

  bgprob->SetRegions(region);
  bgprob->Allocate();

  bgprob->SetSpacing(spacing);

  bgprob->FillBuffer(0);

  for (index[0] = 0; index[0] < size[0]; index[0]++)
    for (index[1] = 0; index[1] < size[1]; index[1]++)
      for (index[2] = 0; index[2] < size[2]; index[2]++)
      {
        if ((index[0] >= 32 && index[0] <= 96)
          &&
          (index[1] >= 32 && index[1] <= 96)
          &&
          (index[2] >= 32 && index[2] <= 96))
          continue;
        bgprob->SetPixel(index, 1.0);
      }

  BiasCorrectorType::MaskImageType::Pointer maskImg = 
    BiasCorrectorType::MaskImageType::New();
  maskImg->SetRegions(region);
  maskImg->Allocate();
  maskImg->SetSpacing(spacing);
  maskImg->FillBuffer(1);

  // Blur probabilities
  std::cout << "Blurring probabilities..." << std::endl;
  typedef itk::DiscreteGaussianImageFilter<FloatImageType, FloatImageType>
    BlurType;

  BlurType::Pointer blur1 = BlurType::New();
  blur1->SetInput(fgprob);
  blur1->SetVariance(0.25);
  blur1->Update();

  BlurType::Pointer blur2 = BlurType::New();
  blur2->SetInput(bgprob);
  blur2->SetVariance(0.25);
  blur2->Update();

  // Generate and apply bias field
  std::cout << "Applying bias..." << std::endl;

  unsigned int maxdegree = 4;
  unsigned int numcoeffs = (maxdegree+1)*(maxdegree+2)*(maxdegree+3)/6;
  //unsigned int numcoeffs = 10;

  double* coeffs = new double[numcoeffs];

  for (unsigned int ichan = 0; ichan < numChannels; ichan++)
  {
    FloatImageType::Pointer img = images[ichan];

    for (unsigned int i = 0; i < numcoeffs; i++)
      coeffs[i] = 0.0;

    for (unsigned int i = 0; i < numcoeffs; i++)
      coeffs[i] = 0.5 * rand() / (double)RAND_MAX;

    std::cout << "Actual bias coeffs = " << std::endl;
    for (unsigned int i = 0; i < numcoeffs; i++)
      std::cout << coeffs[i] << " ";
    std::cout << std::endl;

    double xc, yc, zc;
    double xmid = 64;
    double ymid = 64;
    double zmid = 64;

    for (index[0] = 0; index[0] < size[0]; index[0]++)
      for (index[1] = 0; index[1] < size[1]; index[1]++)
        for (index[2] = 0; index[2] < size[2]; index[2]++)
        {
          double logbias = 0;

          unsigned int c = 0;

          for (int order = 0; order <= maxdegree; order++) {
            for (int xorder = 0; xorder <= order; xorder++) {
              for (int yorder = 0; yorder <= (order-xorder); yorder++) {

                int zorder = order - xorder - yorder;

                xc = (index[0] - xmid) / xmid;
                yc = (index[1] - ymid) / ymid;
                zc = (index[2] - zmid) / zmid;

                double poly =
                  (double)(pow(xc,xorder) * pow(yc,yorder) * pow(zc,zorder));

                logbias += coeffs[c] * poly;

                c++;

              }
            }
          }

          double bias = exp(logbias);

/*
#define TEST_BIAS_CLAMP_MIN 0.25
#define TEST_BIAS_CLAMP_MAX 4.0

          if (bias < TEST_BIAS_CLAMP_MIN)
            bias = TEST_BIAS_CLAMP_MIN;
          if (bias > TEST_BIAS_CLAMP_MAX)
            bias = TEST_BIAS_CLAMP_MAX;
*/

          img->SetPixel(index, img->GetPixel(index)*bias);

        }

  } // for ichan


  // Write image
  std::cout << "Write bias-ed image..." << std::endl;

  ConverterType::Pointer converter = ConverterType::New();
  converter->SetOutputMinimum(0);
  converter->SetOutputMaximum(65355);

  WriterType::Pointer writer = WriterType::New();

  for (unsigned int ichan = 0; ichan < numChannels; ichan++)
  {
    converter->SetInput(images[ichan]);
    converter->Update();

    std::ostringstream oss;
    oss << "bias" << ichan << ".nrrd" << std::ends;
 
    writer->SetFileName(oss.str().c_str());
    writer->SetInput(converter->GetOutput());

    writer->Update();
  }

  converter->SetInput(blur1->GetOutput());
  converter->Update();
  writer->SetFileName("p0.nrrd");
  writer->Update();

  converter->SetInput(blur2->GetOutput());
  converter->Update();
  writer->SetFileName("p1.nrrd");
  writer->Update();

  // Setup bias correction
  DynArray<FloatImageType::Pointer> probs;
  probs.Append(blur1->GetOutput());
  probs.Append(blur2->GetOutput());

  std::cout << "Create bias corrector..." << std::endl;
  BiasCorrectorType::Pointer biascorr = BiasCorrectorType::New();

  biascorr->DebugOn();
  biascorr->SetClampBias(false);
  biascorr->SetSampleSpacing(2.0);
  biascorr->SetMask(maskImg);
  biascorr->SetMaxDegree(maxdegree);
  biascorr->SetProbabilities(probs);

  std::cout << "Bias correction... (max degree = ";
  std::cout << biascorr->GetMaxDegree() << ")" << std::endl;

  biascorr->CorrectImages(images, corrImages, true);

  // Write image
  std::cout << "Write corrected image..." << std::endl;

  ConverterType::Pointer converter_corr = ConverterType::New();
  converter_corr->SetOutputMinimum(0);
  converter_corr->SetOutputMaximum(65355);

  for (unsigned int ichan = 0; ichan < numChannels; ichan++)
  {
    converter_corr->SetInput(corrImages[ichan]);
    converter_corr->Update();

    std::ostringstream oss;
    oss << "corr" << ichan << ".nrrd" << std::ends;
 
    writer->SetFileName(oss.str().c_str());
    writer->SetInput(converter_corr->GetOutput());
    writer->Update();
  }

/*
  std::cout << "Create bias corrector 2..." << std::endl;
  BiasCorrectorType2::Pointer biascorr2 = BiasCorrectorType2::New();

  biascorr2->DebugOn();
  biascorr2->SetProbabilities(probs);
  biascorr2->SetMultiplicative();

  biascorr2->Correct(img, corrImg);

  converter_corr->SetInput(0);
  converter_corr->SetInput(corrImg);
  converter_corr->SetOutputMinimum(0);
  converter_corr->SetOutputMaximum(65355);
  converter_corr->Update();
 
  writer->SetFileName("corr2.nrrd");
  writer->SetInput(converter_corr->GetOutput());
  writer->Update();
*/

  return 0;

}
