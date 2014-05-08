/*=========================================================================

  Program:   Diffusion Applications
  Language:  C++
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/CLI/DiffusionApplications/dwiNoiseFilter/dwiNoiseFilter.cxx $
  Date:      $Date: 2008-11-25 18:46:58 +0100 (Tue, 25 Nov 2008) $
  Version:   $Revision: 7972 $

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/

#ifdef _WIN32
// to pick up M_SQRT2 and other nice things...
#define _USE_MATH_DEFINES
#endif

// DWIUnbiasedNonLocalMeansFilter includes
#include "DWIUnbiasedNonLocalMeansFilterCLP.h"
#include "itkComputeRestrictedHistogram.h"
#include "itkOtsuStatistics.h"
#include "itkOtsuThreshold.h"
#include "itkUNLMFilter.h"

// CLI includes
#include <itkPluginUtilities.h>

// ITK includes
#include <itkMetaDataObject.h>
#include <itkImageFileWriter.h>
#include <itkNrrdImageIO.h>
#include <itkCastImageFilter.h>

#define DIMENSION 3
#define dwiPI 3.141592653589793

template <class PixelType>
int DoIt( int argc, char * argv[], PixelType )
{
  PARSE_ARGS;

  // do the typedefs

  typedef itk::VectorImage<PixelType, DIMENSION> DiffusionImageType;

  typedef itk::Image<PixelType, DIMENSION> ScalarImageType;

  typedef double                                       PixelTypeDouble;
  typedef itk::VectorImage<PixelTypeDouble, DIMENSION> DoubleDiffusionImageType;

  typedef itk::CovariantVector<double, DIMENSION> CovariantVectorType;

  std::vector<CovariantVectorType> diffusionDirections;

  typedef itk::ImageFileReader<DiffusionImageType> FileReaderType;
  typename FileReaderType::Pointer reader = FileReaderType::New();
  reader->SetFileName( inputVolume.c_str() );
  reader->Update();

  itk::MetaDataDictionary            imgMetaDictionary = reader->GetMetaDataDictionary();
  std::vector<std::string>           imgMetaKeys = imgMetaDictionary.GetKeys();
  std::string                        metaString;

  std::cout << "Number of keys = " << imgMetaKeys.size() << std::endl;

  typedef itk::MetaDataDictionary DictionaryType;
  const DictionaryType & dictionary = reader->GetMetaDataDictionary();

  typedef itk::MetaDataObject<std::string> MetaDataStringType;

  DictionaryType::ConstIterator itr = dictionary.Begin();
  DictionaryType::ConstIterator end = dictionary.End();

  double       dBValue = 1000;
  unsigned int channels = 0;
  while( itr != end )
    {
    itk::MetaDataObjectBase::Pointer entry = itr->second;
    MetaDataStringType::Pointer      entryvalue =
      dynamic_cast<MetaDataStringType *>( entry.GetPointer() );

    if( entryvalue )
      {
      ::size_t pos = itr->first.find("DWMRI_gradient");

      if( pos != std::string::npos )
        {
        std::string tagkey = itr->first;
        std::string tagvalue = entryvalue->GetMetaDataObjectValue();

        double dx[DIMENSION];
        std::sscanf(tagvalue.c_str(), "%lf %lf %lf\n", &dx[0], &dx[1], &dx[2]);
        diffusionDirections.push_back( (CovariantVectorType)(dx) );
        ++channels;
        }
      else
        {

        // try to find the b-value

        ::size_t posB = itr->first.find("DWMRI_b-value");

        if( posB != std::string::npos )
          {
          std::string tagvalue = entryvalue->GetMetaDataObjectValue();
          std::sscanf(tagvalue.c_str(), "%lf\n", &dBValue );
          }
        else
          {
          // std::cout << itr->first << " " << entryvalue->GetMetaDataObjectValue() << std::endl;
          }
        }
      }

    ++itr;
    }

  // find the first zero baseline image and use it for the noise estimation

  ::size_t iNrOfDWIs = diffusionDirections.size();
  ::size_t iFirstBaseline = std::string::npos;
  for( ::size_t iI = 0; iI < iNrOfDWIs; iI++ )
    {

    if( diffusionDirections[iI].GetNorm() == 0 )
      {
      iFirstBaseline = iI;
      std::cout << "First baseline found at index = " << iFirstBaseline << std::endl;
      break;
      }

    }

  if( iFirstBaseline == std::string::npos )
    {

    std::cout << "Did not find an explicit baseline image." << std::endl;
    std::cout << "Treating the first volume as the baseline volume." << std::endl;
    iFirstBaseline = 0;

    }

  typename ScalarImageType::SizeType indexRadiusS;
  typename ScalarImageType::SizeType indexRadiusC;
  typename ScalarImageType::SizeType indexRadiusE;

  indexRadiusS[0] = iRadiusSearch[0]; // radius along x
  indexRadiusS[1] = iRadiusSearch[1]; // radius along y
  indexRadiusS[2] = iRadiusSearch[2]; // radius along z

  indexRadiusC[0] = iRadiusComp[0]; // radius along x
  indexRadiusC[1] = iRadiusComp[1]; // radius along y
  indexRadiusC[2] = iRadiusComp[2]; // radius along z

  indexRadiusE[0] = iRadiusEstimation[0]; // radius along x
  indexRadiusE[1] = iRadiusEstimation[1]; // radius along y
  indexRadiusE[2] = iRadiusEstimation[2]; // radius along z

  // now filter the diffusion weighted images with this noise level
  // filter volume, by volume

  // filter the whole thing

  typedef itk::UNLMFilter<DiffusionImageType, DoubleDiffusionImageType> UNLMFilterType;
  typename UNLMFilterType::Pointer filter = UNLMFilterType::New();
  filter->SetInput( reader->GetOutput() );
  filter->SetRSearch( indexRadiusS );
  filter->SetRComp( indexRadiusC );
  typename UNLMFilterType::GradientType grad;
  unsigned int     nDWI = 0;
  unsigned int     nBaselines = 0;
  std::vector<int> pDWI;
  std::vector<int> pBaselines;
  for( unsigned int iI = 0; iI < channels; ++iI )
    {
    float norm = diffusionDirections[iI].GetNorm();
    if( norm > 1e-3 )
      {
      grad[0] = diffusionDirections[iI][0] / norm;
      grad[1] = diffusionDirections[iI][1] / norm;
      grad[2] = diffusionDirections[iI][2] / norm;
      filter->AddGradientDirection( grad );
      ++nDWI;
      pDWI.push_back( iI );
      }
    else
      {
      ++nBaselines;
      pBaselines.push_back( iI );
      }
    }
  filter->SetNDWI( nDWI );
  filter->SetNBaselines( nBaselines );
  unsigned int* indicator = new unsigned int[nDWI];
  for( unsigned int iI = 0; iI < nDWI; ++iI )
    {
    indicator[iI] = pDWI[iI];
    }
  filter->SetDWI( indicator );
  delete[] indicator;
  indicator = new unsigned int[nBaselines];
  for( unsigned int iI = 0; iI < nBaselines; ++iI )
    {
    indicator[iI] = pBaselines[iI];
    }
  filter->SetBaselines( indicator );
  delete[] indicator;
  if( iNumNeighbors < 1 )
    {
    iNumNeighbors = 1;
    }
  else if( iNumNeighbors > 5 )
    {
    iNumNeighbors = 5;
    }
  filter->SetNeighbours( iNumNeighbors );
// ======================================================================================================
// Noise estimation
  typedef itk::Image<float, DiffusionImageType::ImageDimension>           NoiseImageType;
  typedef itk::OtsuStatistics<DiffusionImageType, NoiseImageType>         StatsType;
  typedef typename StatsType::Pointer                                     StatsPointer;
  typedef itk::OtsuThreshold<NoiseImageType, NoiseImageType>              ThresholdType;
  typedef typename ThresholdType::Pointer                                 ThresholdPointer;
  typedef itk::ComputeRestrictedHistogram<NoiseImageType, NoiseImageType> HistogramType;
  typedef typename HistogramType::Pointer                                 HistogramPointer;
  StatsPointer     stats     = StatsType::New();
  ThresholdPointer threshold = ThresholdType::New();
  HistogramPointer histogram = HistogramType::New();
  stats->SetInput( reader->GetOutput() );
  typename StatsType::IndicatorType ind( 1 );
  ind[0] = iFirstBaseline;
  stats->SetIndicator( ind );
  stats->SetRadius( indexRadiusE );
  stats->SetChannels( channels );
  stats->SetUseNeighborhoodBaselines();
  stats->Update();
  threshold->SetMin( stats->GetMin() );
  threshold->SetMax( stats->GetMax() );
  threshold->SetW( 2.0f );
  threshold->SetBins( 2048 );
  threshold->SetInput( stats->GetOutput() );
  threshold->Update();
  double th = threshold->GetThreshold();
  histogram->SetInput( stats->GetOutput() );
  histogram->SetMin(  2.0f );
  histogram->SetMax(   th  );
  histogram->SetBins( 256 );
  histogram->Update();
  double sigma  = histogram->GetMode();
  sigma *= sqrt(2 / dwiPI);
  std::cout << "The estimated noise is: " << sigma << std::endl;
// ======================================================================================================
  filter->SetSigma( sigma );
  if( iH < 0.8 )
    {
    iH = 0.8;
    }
  else if( iH > 1.2 )
    {
    iH = 1.2;
    }
  filter->SetH( iH * sigma );
  filter->Update();

  std::cout << "number of components per pixel" << filter->GetOutput()->GetNumberOfComponentsPerPixel() << std::endl;

  // now cast it back to diffusionimagetype

  typedef itk::CastImageFilter<DoubleDiffusionImageType, DiffusionImageType> CastImageFilterType;

  typename CastImageFilterType::Pointer castImageFilter = CastImageFilterType::New();
  castImageFilter->SetInput( filter->GetOutput() );
  castImageFilter->Update();

  // let's write it out

  typename itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();

  itk::MetaDataDictionary metaDataDictionary;
  metaDataDictionary = reader->GetMetaDataDictionary();

  io->SetFileTypeToBinary();
  io->SetMetaDataDictionary( metaDataDictionary );

  typedef itk::ImageFileWriter<DiffusionImageType> WriterType;
  typename WriterType::Pointer nrrdWriter = WriterType::New();
  nrrdWriter->UseInputMetaDataDictionaryOff();
  nrrdWriter->SetInput( castImageFilter->GetOutput() );
  nrrdWriter->SetImageIO(io);
  nrrdWriter->SetFileName( outputVolume.c_str() );
  nrrdWriter->UseCompressionOn();
  try
    {
    nrrdWriter->Update();
    }
  catch( itk::ExceptionObject e )
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "success = " << EXIT_SUCCESS << std::endl;

  return EXIT_SUCCESS;

}

int main( int argc, char * argv[] )
{

  PARSE_ARGS;

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

  // try
  // {
  itk::GetImageType(inputVolume, pixelType, componentType);

  // This filter handles all types

  switch( componentType )
    {
#ifndef WIN32
    case itk::ImageIOBase::UCHAR:
      return DoIt( argc, argv, static_cast<unsigned char>(0) );
      break;
    case itk::ImageIOBase::CHAR:
      return DoIt( argc, argv, static_cast<char>(0) );
      break;
#endif
    case itk::ImageIOBase::USHORT:
      return DoIt( argc, argv, static_cast<unsigned short>(0) );
      break;
    case itk::ImageIOBase::SHORT:
      return DoIt( argc, argv, static_cast<short>(0) );
      break;
    case itk::ImageIOBase::UINT:
      return DoIt( argc, argv, static_cast<unsigned int>(0) );
      break;
    case itk::ImageIOBase::INT:
      return DoIt( argc, argv, static_cast<int>(0) );
      break;
#ifndef WIN32
    case itk::ImageIOBase::ULONG:
      return DoIt( argc, argv, static_cast<unsigned long>(0) );
      break;
    case itk::ImageIOBase::LONG:
      return DoIt( argc, argv, static_cast<long>(0) );
      break;
#endif
    case itk::ImageIOBase::FLOAT:
      std::cout << "FLOAT type not currently supported." << std::endl;
      break;
    case itk::ImageIOBase::DOUBLE:
      std::cout << "DOUBLE type not currently supported." << std::endl;
      break;
    case itk::ImageIOBase::UNKNOWNCOMPONENTTYPE:
    default:
      std::cout << "unknown component type" << std::endl;
      break;
    }

  // }

  /*catch( itk::ExceptionObject &excep)
    {
    std::cerr << argv[0] << ": exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }*/

  return EXIT_SUCCESS;
}
