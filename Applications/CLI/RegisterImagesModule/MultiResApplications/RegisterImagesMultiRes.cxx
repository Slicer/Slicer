#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "RegisterImagesMultiResCLP.h"

#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkFRPROptimizer.h"
#include "itkCenteredTransformInitializer.h"
#include "itkVersor.h"
#include "itkRecursiveMultiResolutionPyramidImageFilter.h"
#include "itkImageMaskSpatialObject.h"
#include "itkAffineTransform.h"
#include "itkSlicerBoxSpatialObject.h"

// From Review
#include "itkTransformFileWriter.h"
#include "itkImageRegistrationMethod.h"
#include "itkVectorResampleImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkRegionalMinimaImageFilter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkMattesMutualInformationImageToImageMetric.h"

// My headers
#include "itkFixedRotationSimilarity3DTransform.h"
#include "itkEulerSimilarity3DTransform.h"
#include "itkSimilarity3DTransform.h"
#include "itkDecomposedAffine3DTransform.h"

#include "RegistrationMonitor.h"
#include "ImageWriters.h"
#include "DownsampleHeuristics.h"
#include "ScalingHeuristics.h"
#include "ConvertSlicerROIToRegion.h"

#include <queue>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>

#ifndef M_PI
#define M_PI  3.1415926
#endif

using namespace itk;

typedef short                             Pixel;
typedef float                             ProcessingPixel;
typedef Image<Pixel, 3>           FileImage;
typedef Image<unsigned char, 3>   MaskImage;
typedef Image<ProcessingPixel, 3> ProcessingImage;
typedef ImageFileReader<FileImage>        Reader;
typedef ImageFileReader<MaskImage>        MaskReader;

// Rotation Grid is how the optimized parameters 
typedef Image<Vector<float, 3>, 3>    RotationGrid;
typedef Image<float, 3>                   CostGrid;

namespace
{
template<typename T>
struct
pairFirstOnlyGreater
{
  bool operator()(const T & a,
                  const T & b)
{
  return a.first > b.first;
}

};


template<class T>
typename T::Pointer
createGrid(double max, double resolution)
{
  typename T::Pointer image = T::New();

  typename T::RegionType region;

  typename T::IndexType start;
  typename T::SizeType size;
  typename T::SpacingType spacing;
  typename T::PointType origin;

  typedef typename T::SizeType     SizeType;
  typedef typename SizeType::SizeValueType  SizeValueType;

  for(unsigned int i = 0; i < 3; ++i)
    {
    start[i] = 0;
    size[i] = static_cast<SizeValueType>( 2*floor(max / resolution) + 1 );
    spacing[i] = resolution;
    origin[i] = -(resolution * static_cast<double>(size[i]/2));
    }

  region.SetSize( size );
  region.SetIndex( start ); 

  image->SetRegions( region );
  image->SetSpacing( spacing );
  image->SetOrigin( origin );
  image->Allocate();

  return image;
}

Versor<double> createRotationFromEulerAngles(double x, double y, double z)
{
  Versor<double> vx, vy, vz;
  vx.SetRotationAroundX(x);
  vy.SetRotationAroundY(y);
  vz.SetRotationAroundZ(z);

  return vz * vx * vy;
}

}

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  const bool DEBUG = false;
  const bool VERYDEBUG = false;
 
  const double maxa = M_PI/8;
//  const double maxa = 0;
  const double res1 = M_PI/16;
  const double res2 = res1/2;

  Reader::Pointer freader = Reader::New();
  Reader::Pointer mreader = Reader::New();
  MaskReader::Pointer maskreader = MaskReader::New();

  freader->SetFileName(fixedImage);
  mreader->SetFileName(movingImage);

  freader->Update();
  mreader->Update();


  SpatialObject<3>::Pointer mask = NULL;
  if(fixedImageMask != "")
    {
    maskreader->SetFileName(fixedImageMask);
    maskreader->Update();
    ImageMaskSpatialObject<3>::Pointer imask =
      ImageMaskSpatialObject<3>::New();
    imask->SetImage(maskreader->GetOutput());
    mask = imask;
    }

  // If we have a bounding box mask
  if( fixedImageROI.size() == 6)
    {
    if(mask)
      {
      std::cerr << "Specifying an ROI and an image mask is currently not supported" << std::endl;
      return EXIT_FAILURE;
      }

    std::vector<double> c(3, 0.0);
    std::vector<double> r(3, 0.0);

    // the input is a 6 element vector containing the center in RAS space
    // followed by the radius in real world coordinates

    // copy out center values
    std::copy(fixedImageROI.begin(), fixedImageROI.begin() + 3,
              c.begin());
    // copy out radius values
    std::copy(fixedImageROI.begin() + 3, fixedImageROI.end(),
              r.begin());

    // create lower point
    itk::Point<double, 3> p1;
    p1[0] = -c[0] + r[0];
    p1[1] = -c[1] + r[1];
    p1[2] = c[2] + r[2];

    // create upper point
    itk::Point<double, 3> p2;
    p2[0] = -c[0] - r[0];
    p2[1] = -c[1] - r[1];
    p2[2] = c[2] - r[2];

    if(DEBUG)
      {
      std::cout << "p1: " << p1 << std::endl;
      std::cout << "p2: " << p2 << std::endl;
      }

    SlicerBoxSpatialObject<3>::Pointer bmask =
      convertPointsToBoxSpatialObject(p1, p2);

    mask = bmask;
    }
  else if (fixedImageROI.size() > 1 &&
           fixedImageROI.size() < 6)
    {
    std::cerr << "Number of parameters for ROI not as expected" << std::endl;
    return EXIT_FAILURE;
    }
  else if (fixedImageROI.size() > 6)
    {
    std::cerr << "Multiple ROIs not supported" << std::endl;
    return EXIT_FAILURE;
    }

  // Resample both images to 8x8x8
  typedef RecursiveMultiResolutionPyramidImageFilter<FileImage, ProcessingImage> ImagePyramid;

  // resolution schedule heuristic
  // 1) perform inplane downsamples until image is as close as possible to isotropic with 2x downsampling
  // 2) downsample until one image would have gone below 20

  ImagePyramid::Pointer fpyramid = ImagePyramid::New();
  typedef ImagePyramid::ScheduleType Schedule;
  fpyramid->UseShrinkImageFilterOff();
  fpyramid->SetInput(freader->GetOutput());
  scheduleImagePyramid<ImagePyramid>(fpyramid);
  fpyramid->Update();

  ImagePyramid::Pointer mpyramid = ImagePyramid::New();
  typedef ImagePyramid::ScheduleType Schedule;
  mpyramid->UseShrinkImageFilterOff();
  // hard code for now
  mpyramid->SetInput(mreader->GetOutput());
  scheduleImagePyramid<ImagePyramid>(mpyramid);
  mpyramid->Update();

  const unsigned int fnumberoflevels = fpyramid->GetNumberOfLevels();
  const unsigned int mnumberoflevels = mpyramid->GetNumberOfLevels();

  if(DEBUG)
    {
    std::cout << "fixed image schedule: " << std::endl << fpyramid->GetSchedule() << std::endl;

    // 0 is the downsampled image
    for(unsigned int i = 0;  i < fnumberoflevels; ++i)
      {
      std::cout << "pyramid[" << i << "]: " << 
        fpyramid->GetOutput(i)->GetLargestPossibleRegion().GetSize() << std::endl;
      }

    std::cout << "moving image schedule: " << std::endl <<  mpyramid->GetSchedule() << std::endl;

    // 0 is the downsampled image
    for(unsigned int i = 0;  i < mnumberoflevels; ++i)
      {
      std::cout << "pyramid[" << i << "]: " << 
        mpyramid->GetOutput(i)->GetLargestPossibleRegion().GetSize() << std::endl;
      }

    }
  
  // compute number of samples per level
  std::vector<unsigned long> numberOfVoxelsPerLevel(fnumberoflevels, 0);
  for(unsigned int i = 0; i < fnumberoflevels; ++i)
    {
    numberOfVoxelsPerLevel[i] = countInsideVoxels(fpyramid->GetOutput(i), mask);
    if(DEBUG)
      {
      std::cout << "num samples [" << i << "]: " << numberOfVoxelsPerLevel[i] << std::endl;
      }
    }
  
  typedef FixedRotationSimilarity3DTransform<double> FixedRotationTransform;
  typedef CenteredTransformInitializer< FixedRotationTransform,
    ProcessingImage,
    ProcessingImage>   TransformInitializer;

  FixedRotationTransform::Pointer initt = FixedRotationTransform::New();
  initt->SetIdentity();

  TransformInitializer::Pointer tinit = TransformInitializer::New();
  tinit->SetTransform(initt);
  tinit->SetFixedImage(fpyramid->GetOutput(0));
  tinit->SetMovingImage(mpyramid->GetOutput(0));
  tinit->MomentsOn();
  //tinit->GeometryOn();

  if(VERYDEBUG)
    {
    writeimage(fpyramid->GetOutput(0), "tmp/dfixed.nrrd");
    writeimage(mpyramid->GetOutput(0), "tmp/dmoving.nrrd");
    }

  try
    {
    tinit->InitializeTransform();
    }
  catch( itk::ExceptionObject e)
    {
    std::cerr << "Error initalizing transform" << std::endl;
    return EXIT_FAILURE;
    }

  if(DEBUG)
    std::cout << "Initialized transform: " << std::endl;
  //std::cout << initt << std::endl;

  // Generate a set of rotations
  // Parameter is n where number of samples per dimensions is 2*n+1
  // We have (2*n+1)^3 rotaions.  So this number must be very very small.
  // Setup registration framework
  
  RotationGrid::Pointer coarsegrid = 
    createGrid<RotationGrid>(maxa, res1);

  if(VERYDEBUG)
    {
    std::cout << "Built " << coarsegrid->GetLargestPossibleRegion().GetNumberOfPixels() << 
      " trial rotations" << std::endl;

    std::ofstream file("tmp/moresearch.csv");
    if(!file)
      {
    std::cerr  << "cannot open csv file for writing" << std::endl;
    return EXIT_FAILURE;
      }
    file << "metric,i,# its,t0,t1,t2,s" << std::endl;
    }

  std::vector<double> scales;

  int counter = 0;
  typedef ImageRegionIteratorWithIndex<RotationGrid> Iterator;
  Iterator it(coarsegrid,
              coarsegrid->GetLargestPossibleRegion());

  for(it.GoToBegin() ;
      !it.IsAtEnd(); ++it)
    {
    //const Versor<double> & rot = *it;
    RotationGrid::PointType p;
    coarsegrid->TransformIndexToPhysicalPoint(it.GetIndex(), p);
    
    const Versor<double> rot = createRotationFromEulerAngles(p[0], p[1], p[2]);
    
    typedef ImageRegistrationMethod<ProcessingImage, ProcessingImage> ImageRegistration;
    ImageRegistration::Pointer reg = ImageRegistration::New();
    reg->SetFixedImage(fpyramid->GetOutput(0));
    reg->SetMovingImage(mpyramid->GetOutput(0));

    typedef ImageRegistrationViewer ViewerCommandType;
    ViewerCommandType::Pointer command = ViewerCommandType::New();

    typedef LinearInterpolateImageFunction<ProcessingImage, double> Interpolator;
    Interpolator::Pointer reginterp1 = Interpolator::New();

    reg->SetInterpolator(reginterp1);

    typedef MattesMutualInformationImageToImageMetric<ProcessingImage, ProcessingImage> Metric;
//   typedef NormalizedMutualInformationHistogramImageToImageMetric<ProcessingImage, ProcessingImage> Metric;
//    typedef NormalizedCorrelationImageToImageMetric<ProcessingImage, ProcessingImage> Metric;
    Metric::Pointer metric = Metric::New();
    metric->SetNumberOfHistogramBins(256/8);
    //    Metric::HistogramSizeType hsize;
    //    hsize[0] = 256/8;
    //    hsize[1] = 256/8;
    //    metric->SetHistogramSize(hsize);
    //Contains only 16384 pixels total   
    //metric->SetUseAllPixels(true);
    // metric->SetFixedImageMask(maskreader->GetOutput());
    metric->SetNumberOfSpatialSamples(15000);
    if(mask)
      {
      metric->SetFixedImageMask(mask);
      }
    metric->SetUseExplicitPDFDerivatives(true);
//    metric->Initialize();

    reg->SetMetric(metric);

    //typedef OnePlusOneEvolutionaryOptimizer OptimizerType;
    //typedef PowellOptimizer OptimizerType;
    typedef FRPROptimizer OptimizerType;
    //typedef GradientDescentOptimizer OptimizerType;
    OptimizerType::Pointer opt = OptimizerType::New(); 
    typedef Optimizer::ScalesType       OptimizerScalesType;
    OptimizerScalesType optimizerScales( 4 );
    // should set this scale based on size of the image
    ScalingValues sv(fpyramid->GetOutput(0), initt->GetCenter());

    optimizerScales[0] = 1.0/ sv.TranslationScale;
    optimizerScales[1] = 1.0/ sv.TranslationScale;
    optimizerScales[2] = 1.0/ sv.TranslationScale;
    optimizerScales[3] = 1.0/ sv.ScalingScale;

    opt->SetScales( optimizerScales );
    opt->SetMaximize(false);
    opt->SetCatchGetValueException( true );
    opt->SetMetricWorstPossibleValue(0.0);
    //opt->AddObserver( IterationEvent(), command);

    // For FRPR
    opt->SetStepLength(stepSize);
    opt->SetStepTolerance(stepTolerance);
    opt->SetValueTolerance(metricTolerance);
    opt->SetMaximumIteration(numIterations*optimizerScales.size());
    opt->SetMaximumLineIteration(numLineIterations);
    opt->SetUseUnitLengthGradient( true );
    opt->SetToPolakRibiere();

    reg->SetOptimizer(opt);

    // Execute a simplified rotation on the downsampled images
    FixedRotationTransform::Pointer ot = FixedRotationTransform::New();
    ot->SetIdentity();
    ot->SetRotation(rot);
    ot->SetCenter(initt->GetCenter());
    ot->SetTranslation(initt->GetTranslation());
    ot->SetParameters(initt->GetParameters());

    FixedRotationTransform::Pointer nt = FixedRotationTransform::New();
    nt->SetIdentity();
    nt->SetRotation(rot);
    nt->SetCenter(initt->GetCenter());
    nt->SetTranslation(initt->GetTranslation());
    nt->SetParameters(initt->GetParameters());

    //std::cout << "translation: " << nt->GetTranslation() << std::endl;
    
    reg->SetTransform(nt);
    reg->SetInitialTransformParameters( nt->GetParameters() );

    reg->StartRegistration();
    //std::cout << "params" << reg->GetInitialTransformParameters() << std::endl;

    nt->SetParameters( reg->GetLastTransformParameters() );

    if(DEBUG)
      {
      std::cout << "Finished grid point " << counter << std::endl;
      // std::cout << "t: " << initt->GetTranslation() << std::endl;
      }

    typedef FixedRotationTransform::ParametersType ParametersType;
    ParametersType pm = nt->GetParameters();

    // file << opt->GetValue() << "," <<
    //   counter << "," <<
    //   opt->GetCurrentIteration() << "," <<
    //   pm[0] << "," <<
    //   pm[1] << "," <<
    //   pm[2] << "," <<
    //   pm[3] << std::endl;

    // std::cout << "Metric: " << opt->GetValue() << std::endl;
    // std::cout << "it: " << opt->GetCurrentIteration() << std::endl;
    // std::cout << "Rotation versor: " << rot << std::endl;
    // std::cout << "p: " << reg->GetLastTransformParameters() << std::endl;
    // std::cout << "scale: " << nt->GetScale() << std::endl;

    
    RotationGrid::PixelType t;
    t[0] = pm[0];
    t[1] = pm[1];
    t[2] = pm[2];

    it.Set(t);

    scales.push_back(pm[3]);

    if(VERYDEBUG)
      {
      std::stringstream ss;
      ss << "tmp/pre" << std::setw(3) << std::setfill('0') << counter << ".nrrd";
      writeimage(mpyramid->GetOutput(0), ot, ss.str());
      ss.clear();
      ss.str("");
      ss << "tmp/post" << std::setw(3) << std::setfill('0') << counter << ".nrrd";
      writeimage(mpyramid->GetOutput(0), nt, ss.str());
      }
    ++counter;
    }

  // Image of metric values over a fine grid
  CostGrid::Pointer costgrid =
    createGrid<CostGrid>(maxa, res2);

  // Resample the translation parameters from the coarse grid to the fine
  // grid
  typedef VectorResampleImageFilter<RotationGrid, RotationGrid> VectorResample;
  VectorResample::Pointer rgridresampler = VectorResample::New();
  rgridresampler->SetInput(coarsegrid);
  rgridresampler->SetOutputOrigin(costgrid->GetOrigin());
  rgridresampler->SetOutputSpacing(costgrid->GetSpacing());
  rgridresampler->SetOutputStartIndex(costgrid->GetLargestPossibleRegion().GetIndex() );
  rgridresampler->SetSize(costgrid->GetLargestPossibleRegion().GetSize() );
  rgridresampler->Update();

  typedef MattesMutualInformationImageToImageMetric<ProcessingImage, ProcessingImage> Metric;
  Metric::Pointer metric = Metric::New();
  metric->SetNumberOfHistogramBins(256/8);

  typedef LinearInterpolateImageFunction<ProcessingImage, double> Interpolator;
  Interpolator::Pointer reginterp = Interpolator::New();

  metric->SetInterpolator(reginterp);
  metric->SetFixedImage(fpyramid->GetOutput(0));
  metric->SetFixedImageRegion(fpyramid->GetOutput(0)->GetLargestPossibleRegion());
  metric->SetMovingImage(mpyramid->GetOutput(0));

  //    Metric::HistogramSizeType hsize;
  //    hsize[0] = 256/8;
  //    hsize[1] = 256/8;
  //    metric->SetHistogramSize(hsize);
  //Contains only 16384 pixels total   
  //metric->SetUseAllPixels(true);
  // metric->SetFixedImageMask(maskreader->GetOutput());
  metric->SetNumberOfSpatialSamples(15000);
  if(mask)
    {
    metric->SetFixedImageMask(mask);
    }
  metric->SetUseExplicitPDFDerivatives(true);

  // replace with median
  std::nth_element(scales.begin(),
                   scales.begin() + scales.size()/2,
                   scales.end());
  const double mscale = *(scales.begin() + scales.size()/2);

  if(DEBUG)
    std::cout << "Computing on fine grid" << std::endl;

  typedef ImageRegionIteratorWithIndex<CostGrid> CostIterator;
  typedef ImageRegionConstIteratorWithIndex<CostGrid> CostConstIterator;

  CostIterator cit(costgrid,
                   costgrid->GetLargestPossibleRegion());
  Iterator fit(rgridresampler->GetOutput(),
               rgridresampler->GetOutput()->GetLargestPossibleRegion());
  for(cit.GoToBegin(), fit.GoToBegin();
      !cit.IsAtEnd(); ++cit, ++fit)
    {
    FixedRotationTransform::Pointer t = FixedRotationTransform::New();
    t->SetIdentity();
    
    RotationGrid::PointType pt;
    costgrid->TransformIndexToPhysicalPoint(fit.GetIndex(), pt);
    
    const Versor<double> rot = createRotationFromEulerAngles(pt[0], pt[1], pt[2]);
    t->SetRotation(rot);
    t->SetCenter(initt->GetCenter());

    metric->SetTransform(t);
    metric->Initialize();

    FixedRotationTransform::ParametersType p(4);
    p.Fill(0.0);
    p[0] = fit.Get()[0];
    p[1] = fit.Get()[1];
    p[2] = fit.Get()[2];
    p[3] = mscale;

    // Shouldnt be necessary but just in case
    t->SetParameters(p);
    
    double m = 0.0;
    try
      {
      m = metric->GetValue(p);
      }
    catch (itk::ExceptionObject&)
      {
      // Let m = 0
      }

    cit.Set(m);
    }

  if(DEBUG)
    std::cout << "Finished fine grid" << std::endl;

  typedef itk::Image<unsigned char, 3> BooleanImage;

  typedef itk::RegionalMinimaImageFilter<CostGrid, BooleanImage> RegionalMinimaFilterType;
  RegionalMinimaFilterType::Pointer minima = RegionalMinimaFilterType::New();
  minima->SetInput(costgrid);
  minima->SetFlatIsMinima(false);
  minima->SetFullyConnected(false);
  minima->SetForegroundValue(1);
  minima->SetBackgroundValue(0);

  if(DEBUG)
    std::cout << "Computing minima" << std::endl;

  minima->Update();

  if(VERYDEBUG)
    {
    std::cout << "Finished computing minima" << std::endl;

    writeimage(costgrid, "costgrid.mha");
    writeimage(minima->GetOutput(), "minima.mha");
    }

  // Build a priority queue of best N minima to use
  typedef RotationGrid::IndexType RIndexType;
  typedef RotationGrid::PointType RPointType;

  typedef std::pair<float, RIndexType> PairType;
  std::priority_queue<PairType,
    std::vector<PairType>,
    pairFirstOnlyGreater<PairType> > pq;

  CostConstIterator ccit(costgrid,
                         costgrid->GetLargestPossibleRegion());
  typedef ImageRegionConstIteratorWithIndex<BooleanImage> BooleanConstIterator;
  BooleanConstIterator bit(minima->GetOutput(),
                           minima->GetOutput()->GetLargestPossibleRegion());

  if(DEBUG)
    std::cout << "Putting minima in priority queue" << std::endl;
  
  for(cit.GoToBegin(), bit.GoToBegin();
      !cit.IsAtEnd();
      ++cit, ++bit)
    {
    if(bit.Get())
      {
      if(DEBUG)
        {
        std::cout << "mi: " << cit.Get() << std::endl;
        }

      pq.push(std::make_pair(cit.Get(),
                             cit.GetIndex()));
      }
    }
  assert(bit.IsAtEnd());

  if(pq.size() == 0)
    {
    std::cerr << "Could not find any candidate starting positions" << std::endl;
    return EXIT_FAILURE;
    }

  // Array of candidate parameters to use for 7dof registration
  typedef EulerSimilarity3DTransform<double>                 SimilarityTransform;
  typedef SimilarityTransform::ParametersType                SimilarityParameters;

  std::vector<SimilarityParameters> initialSimilarity;

  for(unsigned int candidateRank = 0; candidateRank < 3; ++candidateRank)
    {
    if(DEBUG)
      std::cout << "i: " << candidateRank << std::endl;

    if(pq.size() == 0)
      {
      break;
      }

    PairType p = pq.top();
    pq.pop();

    if(DEBUG)
      {
      std::cout << "m: " << p.first << std::endl;
      std::cout << "ri: " << p.second << std::endl;
      }

    SimilarityParameters params(SimilarityTransform::ParametersDimension);
    // Get the rotatoin
    RPointType rotationvalue;
    costgrid->TransformIndexToPhysicalPoint(p.second, rotationvalue);

    if(DEBUG)
      std::cout << "Euler angles: " << rotationvalue << std::endl;

    params[0] = rotationvalue[0];
    params[1] = rotationvalue[1];
    params[2] = rotationvalue[2];

    if(DEBUG)
      std::cout << "Scale: " << mscale << std::endl;
    
    // Get the translation
    RotationGrid::PixelType t =
      rgridresampler->GetOutput()->GetPixel(p.second);

    params[3] = t[0];
    params[4] = t[1];
    params[5] = t[2];

    // Set the scale to the median scale
    params[6] = mscale;

    initialSimilarity.push_back(params);
    }

  float bestValue = std::numeric_limits<float>::max();
  SimilarityParameters bestParams;

  // Use level 0 then level 1 for each of the candidate transforms using
  // a similarity transform
  typedef std::vector<SimilarityParameters>::const_iterator SimilarityParametersIterator;
  counter = 0;
  for(SimilarityParametersIterator pit = 
        initialSimilarity.begin();
      pit != initialSimilarity.end();
      ++pit, ++counter)
    {
    typedef ImageRegistrationMethod<ProcessingImage, ProcessingImage> ImageRegistration;
    ImageRegistration::Pointer reg = ImageRegistration::New();
    reg->SetFixedImage(fpyramid->GetOutput(0));
    reg->SetMovingImage(mpyramid->GetOutput(0));

    typedef ImageRegistrationViewer ViewerCommandType;
    ViewerCommandType::Pointer command = ViewerCommandType::New();

    typedef LinearInterpolateImageFunction<ProcessingImage, double> Interpolator;
    Interpolator::Pointer reginterp2 = Interpolator::New();

    reg->SetInterpolator(reginterp2);

    typedef MattesMutualInformationImageToImageMetric<ProcessingImage, ProcessingImage> Metric;
//   typedef NormalizedMutualInformationHistogramImageToImageMetric<ProcessingImage, ProcessingImage> Metric;
//    typedef NormalizedCorrelationImageToImageMetric<ProcessingImage, ProcessingImage> Metric;
    Metric::Pointer metric2 = Metric::New();
    metric2->SetNumberOfHistogramBins(256/8);
    //    Metric::HistogramSizeType hsize;
    //    hsize[0] = 256/8;
    //    hsize[1] = 256/8;
    //    metric2->SetHistogramSize(hsize);
    //Contains only 16384 pixels total   
    //metric2->SetUseAllPixels(true);
    // metric2->SetFixedImageMask(maskreader->GetOutput());
    metric2->SetNumberOfSpatialSamples(15000);
    if(mask)
      {
      metric2->SetFixedImageMask(mask);
      }
    metric2->SetUseExplicitPDFDerivatives(true);
//    metric->Initialize();

    reg->SetMetric(metric2);

    //typedef OnePlusOneEvolutionaryOptimizer OptimizerType;
    //typedef PowellOptimizer OptimizerType;
    typedef FRPROptimizer OptimizerType;
    //typedef GradientDescentOptimizer OptimizerType;
    OptimizerType::Pointer opt = OptimizerType::New(); 
    typedef Optimizer::ScalesType       OptimizerScalesType;
    OptimizerScalesType optimizerScales( 7 );
    ScalingValues sv(fpyramid->GetOutput(0), initt->GetCenter());

    optimizerScales[0] = 1.0/ sv.RotationScale;
    optimizerScales[1] = 1.0/ sv.RotationScale;
    optimizerScales[2] = 1.0/ sv.RotationScale;
    optimizerScales[3] = 1.0/ sv.TranslationScale;
    optimizerScales[4] = 1.0/ sv.TranslationScale;
    optimizerScales[5] = 1.0/ sv.TranslationScale;
    optimizerScales[6] = 1.0/ sv.ScalingScale;

    opt->SetScales( optimizerScales );
    opt->SetMaximize(false);
    opt->SetCatchGetValueException( true );
    opt->SetMetricWorstPossibleValue(0.0);
    if(DEBUG)
      {
      opt->AddObserver( IterationEvent(), command);
      }

    opt->SetStepLength(stepSize);
    opt->SetStepTolerance(stepTolerance);
    opt->SetValueTolerance(metricTolerance);
    opt->SetMaximumIteration(numIterations*optimizerScales.size());
    opt->SetMaximumLineIteration(numLineIterations);
    opt->SetUseUnitLengthGradient( true );
    opt->SetToPolakRibiere();

    reg->SetOptimizer(opt);

    SimilarityTransform::Pointer ot = 
      SimilarityTransform::New();

    ot->SetIdentity();
    ot->SetCenter(initt->GetCenter());
    ot->SetParameters(*pit);

    reg->SetTransform(ot);
    reg->SetInitialTransformParameters(*pit);

    if(DEBUG)
      {
      std::cout << "======= Candidate ======" << std::endl;

      std::cout << "Initial optimized transform params" << std::endl
                << *pit << std::endl;
      }

    reg->StartRegistration();

    SimilarityParameters pm = reg->GetLastTransformParameters();

    if(DEBUG)
      {
      std::cout << "8mm optimized transform params" << std::endl
                << pm << std::endl;
      std::cout << "Metric: " << opt->GetValue() << std::endl;
      }

    // Bump up a resolution level
    if(fnumberoflevels > 1 || mnumberoflevels > 1)
      {
      reg->SetFixedImage(fpyramid->GetOutput(fnumberoflevels >= 2 ? 1 : 0));
      reg->SetMovingImage(mpyramid->GetOutput(mnumberoflevels >= 2 ? 1 : 0));

      metric->SetNumberOfHistogramBins(256/4);
      metric->SetNumberOfSpatialSamples(60000);
      if(mask)
        {
        metric->SetFixedImageMask(mask);
        }

      ScalingValues sv2(fpyramid->GetOutput(fnumberoflevels >= 2 ? 1 : 0), initt->GetCenter());

      optimizerScales[0] = 1.0/ sv2.RotationScale;
      optimizerScales[1] = 1.0/ sv2.RotationScale;
      optimizerScales[2] = 1.0/ sv2.RotationScale;
      optimizerScales[3] = 1.0/ sv2.TranslationScale;
      optimizerScales[4] = 1.0/ sv2.TranslationScale;
      optimizerScales[5] = 1.0/ sv2.TranslationScale;
      optimizerScales[6] = 1.0/ sv2.ScalingScale;

      opt->SetScales(optimizerScales);

      reg->SetInitialTransformParameters(pm);
      reg->StartRegistration();

      pm = reg->GetLastTransformParameters();

      if(DEBUG)
        {
        std::cout << "4mm optimized transform params" << std::endl
                  << pm << std::endl;
        std::cout << "Metric: " << opt->GetValue() << std::endl;
        }

      if(opt->GetValue() < bestValue)
        {
        bestValue = opt->GetValue();
        bestParams = pm;
        }
      }
    // If there is no more resolution levels just use the current value
    else
      {
      if(opt->GetValue() < bestValue)
        {
        bestValue = opt->GetValue();
        bestParams = pm;
        }
      }
    }

  // Get the best metric of the level 1 registrations
  if(DEBUG)
    {
    std::cout << "Best metric: " << bestValue << std::endl;
    std::cout << "Params: " << bestParams <<std::endl;
    }

  typedef DecomposedAffine3DTransform<double>             AffineTransform;
  
  AffineTransform::Pointer affinet = AffineTransform::New();
  affinet->SetIdentity();
  affinet->SetCenter(initt->GetCenter());

  AffineTransform::ParametersType affineparams =
    affinet->GetParameters();
  
  for(int i = 0; i < 6; ++i)
    affineparams[i] = bestParams[i];
  
  affineparams[6] = bestParams[6];
  affineparams[7] = bestParams[6];
  affineparams[8] = bestParams[6];
  affineparams[9] = 0.0;
  affineparams[10] = 0.0;
  affineparams[11] = 0.0;

  typedef ImageRegistrationMethod<ProcessingImage, ProcessingImage> ImageRegistration;
  ImageRegistration::Pointer reg = ImageRegistration::New();
  reg->SetFixedImage(fpyramid->GetOutput(fnumberoflevels  >= 3 ? 2 : fnumberoflevels - 1));
  reg->SetMovingImage(mpyramid->GetOutput(mnumberoflevels >= 3 ? 2 : mnumberoflevels - 1));

  typedef ImageRegistrationViewer ViewerCommandType;
  ViewerCommandType::Pointer command = ViewerCommandType::New();

  typedef LinearInterpolateImageFunction<ProcessingImage, double> Interpolator;
  Interpolator::Pointer reginterpf = Interpolator::New();

  reg->SetInterpolator(reginterpf);

  typedef MattesMutualInformationImageToImageMetric<ProcessingImage, ProcessingImage> Metric;
//   typedef NormalizedMutualInformationHistogramImageToImageMetric<ProcessingImage, ProcessingImage> Metric;
//    typedef NormalizedCorrelationImageToImageMetric<ProcessingImage, ProcessingImage> Metric;
  Metric::Pointer metricf = Metric::New();
  metricf->SetNumberOfHistogramBins(256/2);
  //    Metric::HistogramSizeType hsize;
  //    hsize[0] = 256/8;
  //    hsize[1] = 256/8;
  //    metric->SetHistogramSize(hsize);
  //Contains only 16384 pixels total   
  //metric->SetUseAllPixels(true);
  // metric->SetFixedImageMask(maskreader->GetOutput());
  metricf->SetNumberOfSpatialSamples(240000);
  if(mask)
    {
    metricf->SetFixedImageMask(mask);
    }
  metricf->SetUseExplicitPDFDerivatives(true);
//    metricf->Initialize();

  reg->SetMetric(metricf);

  //typedef OnePlusOneEvolutionaryOptimizer OptimizerType;
  //typedef PowellOptimizer OptimizerType;
  typedef FRPROptimizer OptimizerType;
  //typedef GradientDescentOptimizer OptimizerType;
  OptimizerType::Pointer opt = OptimizerType::New(); 
  typedef Optimizer::ScalesType       OptimizerScalesType;
  OptimizerScalesType optimizerScales( 12 );

  ScalingValues sv(fpyramid->GetOutput(fnumberoflevels >= 3 ? 2 : fnumberoflevels - 1), initt->GetCenter());

  optimizerScales[0] = 1.0/ sv.RotationScale;
  optimizerScales[1] = 1.0/ sv.RotationScale;
  optimizerScales[2] = 1.0/ sv.RotationScale;
  optimizerScales[3] = 1.0/ sv.TranslationScale;
  optimizerScales[4] = 1.0/ sv.TranslationScale;
  optimizerScales[5] = 1.0/ sv.TranslationScale;
  optimizerScales[6] = 1.0/ sv.ScalingScale;
  optimizerScales[7] = 1.0/ sv.ScalingScale;
  optimizerScales[8] = 1.0/ sv.ScalingScale;
  optimizerScales[9] = 1.0/ sv.SkewingScale;
  optimizerScales[10] = 1.0/ sv.SkewingScale;
  optimizerScales[11] = 1.0/ sv.SkewingScale;

  opt->SetScales( optimizerScales );
  opt->SetMaximize(false);
  opt->SetCatchGetValueException( true );
  opt->SetMetricWorstPossibleValue(0.0);
  if(DEBUG)
    {
    opt->AddObserver( IterationEvent(), command);
    opt->AddObserver( StartEvent(), command);
    }

  opt->SetStepLength(stepSize);
  opt->SetStepTolerance(stepTolerance);
  opt->SetValueTolerance(metricTolerance);
  opt->SetMaximumIteration(numIterations*optimizerScales.size());
  opt->SetMaximumLineIteration(numLineIterations);
  opt->SetUseUnitLengthGradient( true );
  opt->SetToPolakRibiere();

  reg->SetOptimizer(opt);
  
  reg->SetTransform(affinet);
  reg->SetInitialTransformParameters(affineparams);

  reg->StartRegistration();

  AffineTransform::ParametersType mm2p = reg->GetLastTransformParameters();

  if(DEBUG)
    {
    std::cout << " 2mm optimized params " << std::endl
              << mm2p << std::endl;
    }

  // Rerun with new params
  ScalingValues sv2(fpyramid->GetOutput(fnumberoflevels >= 4 ? 3 : fnumberoflevels - 1), initt->GetCenter());

  optimizerScales[0] = 1.0/ sv2.RotationScale;
  optimizerScales[1] = 1.0/ sv2.RotationScale;
  optimizerScales[2] = 1.0/ sv2.RotationScale;
  optimizerScales[3] = 1.0/ sv2.TranslationScale;
  optimizerScales[4] = 1.0/ sv2.TranslationScale;
  optimizerScales[5] = 1.0/ sv2.TranslationScale;
  optimizerScales[6] = 1.0/ sv2.ScalingScale;
  optimizerScales[7] = 1.0/ sv2.ScalingScale;
  optimizerScales[8] = 1.0/ sv2.ScalingScale;
  optimizerScales[9] = 1.0/ sv2.SkewingScale;
  optimizerScales[10] = 1.0/ sv2.SkewingScale;
  optimizerScales[11] = 1.0/ sv2.SkewingScale;

  opt->SetScales( optimizerScales );

  opt->SetMaximumIteration(numIterations*optimizerScales.size());
  
  metricf->SetNumberOfHistogramBins(256);
  metricf->SetNumberOfSpatialSamples(480000);
  if(mask)
    {
    metricf->SetFixedImageMask(mask);
    }

  reg->SetFixedImage(fpyramid->GetOutput(fnumberoflevels >= 4? 3 : fnumberoflevels -1));
  reg->SetMovingImage(mpyramid->GetOutput(mnumberoflevels >= 4? 3 : mnumberoflevels -1)); 
  reg->SetInitialTransformParameters(reg->GetLastTransformParameters());

  reg->StartRegistration();
    
  AffineTransform::ParametersType finalp = reg->GetLastTransformParameters();

  if(DEBUG)
    {
    std::cout << " 1mm optimized params " << std::endl
              << finalp << std::endl;
    }

  affinet->SetParameters(finalp);

  if(resampledImage != "")
    {
    writeimage(mpyramid->GetOutput(mnumberoflevels - 1), affinet, resampledImage);
    }

  if(outputTransform != "")
    {
    typedef itk::AffineTransform<double, 3> ExportAffineTransformType;
    ExportAffineTransformType::Pointer outputt = ExportAffineTransformType::New();
    outputt->SetCenter(affinet->GetCenter());
    outputt->SetMatrix(affinet->GetMatrix());
    outputt->SetTranslation(affinet->GetTranslation());
    
    TransformFileWriter::Pointer twriter = TransformFileWriter::New();
    twriter->SetInput(outputt);
    twriter->SetFileName(outputTransform);
    twriter->Update();    
    }

  return EXIT_SUCCESS;
}

