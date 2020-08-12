
#include "FiducialRegistrationCLP.h"

// ITK includes
#include <itkAffineTransform.h>
#include <itkImage.h>
#include <itkLandmarkBasedTransformInitializer.h>
#include <itkSimilarity3DTransform.h>
#include <itkTransformFileWriter.h>

// STD includes
#include <numeric>

namespace
{
  // Function to convert a point from std::vector to itk::Point
  itk::Point<double, 3>
    convertStdVectorToITKPoint(const std::vector<float> & vec)
  {
    itk::Point<double, 3> p;
    p[0] = vec[0];
    p[1] = vec[1];
    p[2] = vec[2];
    return p;
  }

  // Operator to compute the squared distance between two points
  class SquaredPointDistance
  {
  public:
    explicit SquaredPointDistance(const itk::Point<double, 3>& ctr)
      : m_Point(ctr)
    {
    }

    double operator()(const itk::Point<double, 3>& p)
    {
      return (p - m_Point).GetSquaredNorm();
    }

  private:
    itk::Point<double, 3> m_Point;

  };

  // Function to compute the scaling factor between two sets of points.
  // This is the symmetric form given by
  //    Berthold K. P. Horn (1987),
  //    "Closed-form solution of absolute orientation using unit quaternions,"
  //    Journal of the Optical Society of America A, 4:629-642

  double
    computeSymmetricScale(const std::vector<itk::Point<double, 3> >& fixedPoints,
    const std::vector<itk::Point<double, 3> >& movingPoints,
    const itk::Point<double, 3>& fixedcenter,
    const itk::Point<double, 3>& movingcenter)
  {
    std::vector<double> centeredFixedPoints(fixedPoints.size(), 0.0);
    std::vector<double> centeredMovingPoints(movingPoints.size(), 0.0);

    std::transform(fixedPoints.begin(), fixedPoints.end(),
      centeredFixedPoints.begin(),
      SquaredPointDistance(fixedcenter) );

    std::transform(movingPoints.begin(), movingPoints.end(),
      centeredMovingPoints.begin(),
      SquaredPointDistance(movingcenter) );

    double fixedmag = 0.0, movingmag = 0.0;
    fixedmag = std::accumulate(centeredFixedPoints.begin(),
      centeredFixedPoints.end(),
      fixedmag);

    movingmag = std::accumulate(centeredMovingPoints.begin(),
      centeredMovingPoints.end(),
      movingmag);

    return sqrt(movingmag / fixedmag);
  }

}

int main(int argc, char* argv[])
{
  PARSE_ARGS;


  double invalidRMS = -1;

    // Checking conditions.

  if( fixedLandmarks.size() <= 0 || movingLandmarks.size() <= 0 ||
    fixedLandmarks.size() != movingLandmarks.size() )
  {
    std::cerr << "Fixed and moving landmark lists must be of the same size "
      << "and contain at least one point" << std::endl;
    outputMessage = "Fixed and moving landmark lists must be of the same size and contain at least one point";
    rms = invalidRMS;
  }

  if( saveTransform == "" )
  {
    std::cerr << "An output transform must be specified" << std::endl;
    if (outputMessage == "")
    {
      outputMessage = "An output transform must be specified";
      rms = invalidRMS;
    }
  }


  if( transformType != "Translation" && fixedLandmarks.size() < 3 )
  {
    std::cerr << "At least 3 fixed landmark fiducial points must be specified "
              << "for Rigid or Similarity transforms, have " << fixedLandmarks.size()
              << std::endl;
    if (outputMessage == "")
    {
      outputMessage = "At least 3 fixed landmark fiducial points must be specified for Rigid or Similarity transforms";
      rms = invalidRMS;
    }
  }

    // Return if conditions not met.

  if ( rms == invalidRMS )
  {
    // Write out the return parameters in "name = value" form
    std::ofstream rts;
    rts.open(returnParameterFile.c_str() );
    rts << "rms = " << rms << std::endl;
    rts << "outputMessage = " << outputMessage <<std::endl;
    rts.close();

    return EXIT_SUCCESS;
  }


  // only calculate if the above conditions hold

  typedef  std::vector<itk::Point<double, 3> > PointList;

  PointList fixedPoints(fixedLandmarks.size() );
  PointList movingPoints(movingLandmarks.size() );

  // Convert both points lists to ITK points

  std::transform(fixedLandmarks.begin(), fixedLandmarks.end(),
    fixedPoints.begin(),
    convertStdVectorToITKPoint);

  std::transform(movingLandmarks.begin(), movingLandmarks.end(),
    movingPoints.begin(),
    convertStdVectorToITKPoint);

  // Our input into landmark based initialize will be of this form
  // The format for saving to slicer is defined later
  typedef itk::Similarity3DTransform<double> SimilarityTransformType;
  SimilarityTransformType::Pointer transform = SimilarityTransformType::New();
  transform->SetIdentity();
  // workaround a bug in older versions of ITK
  transform->SetScale(1.0);


  typedef itk::LandmarkBasedTransformInitializer<SimilarityTransformType,
    itk::Image<short, 3>, itk::Image<short, 3> > InitializerType;
  InitializerType::Pointer initializer = InitializerType::New();

  // This expects a VersorRigid3D.  The similarity transform works because
  // it derives from that class
  initializer->SetTransform(transform);

  initializer->SetFixedLandmarks(fixedPoints);
  initializer->SetMovingLandmarks(movingPoints);

  initializer->InitializeTransform();


    // Handle different transform types.

  if( transformType == "Translation" )
  {
    // Clear out the computed rotaitoin if we only requested translation
    itk::Versor<double> v;
    v.SetIdentity();
    transform->SetRotation(v);
  }
  else if( transformType == "Rigid" )
  {
    // do nothing
  }
  else if( transformType == "Similarity" )
  {
    // Compute the scaling factor and add that in
    itk::Point<double, 3> fixedCenter(transform->GetCenter() );
    itk::Point<double, 3> movingCenter(transform->GetCenter() + transform->GetTranslation() );

    double s = computeSymmetricScale(fixedPoints, movingPoints,
      fixedCenter, movingCenter);
    transform->SetScale(s);
  }
  else if( transformType == "Affine" )
  {
    // itk::Matrix<double, 3> a = computeAffineTransform(fixedPoints, movingPoints, fixedCenter, movingCenter);
    std::cerr << "Unsupported transform type: " << transformType << std::endl;
    // return EXIT_FAILURE;
  }
  else
  {
    std::cerr << "Unsupported transform type: " << transformType << std::endl;
    return EXIT_FAILURE;
  }


    // Convert into an affine transform for saving to Slicer.

  typedef itk::AffineTransform<double, 3> AffineTransform;
  AffineTransform::Pointer fixedToMovingT = itk::AffineTransform<double, 3>::New();

  fixedToMovingT->SetCenter( transform->GetCenter() );
  fixedToMovingT->SetMatrix( transform->GetMatrix() );
  fixedToMovingT->SetTranslation( transform->GetTranslation() );


    // Compute RMS error in the target coordinate system.

  AffineTransform::Pointer movingToFixedT = AffineTransform::New();
  fixedToMovingT->GetInverse( movingToFixedT );

  typedef InitializerType::LandmarkPointContainer LandmarkPointContainerType;

  typedef LandmarkPointContainerType::const_iterator PointsContainerConstIterator;
  PointsContainerConstIterator mitr = movingPoints.begin();
  PointsContainerConstIterator fitr = fixedPoints.begin();

  SimilarityTransformType::OutputVectorType                 errortr;
  SimilarityTransformType::OutputVectorType::RealValueType  sum;
  InitializerType::LandmarkPointType                        movingPointInFixed;
  int                                                       counter;

  sum = itk::NumericTraits< SimilarityTransformType::OutputVectorType::RealValueType >::ZeroValue();
  counter = itk::NumericTraits< int >::ZeroValue();

  while( mitr != movingPoints.end() )
  {
    movingPointInFixed = movingToFixedT->TransformPoint( *mitr );
    errortr = *fitr - movingPointInFixed;
    sum = sum + errortr.GetSquaredNorm();
    ++mitr;
    ++fitr;
    counter++;
  }

  rms = sqrt( sum / counter );


  itk::TransformFileWriter::Pointer twriter = itk::TransformFileWriter::New();
  twriter->SetInput( fixedToMovingT );
  twriter->SetFileName( saveTransform );

  twriter->Update();


  outputMessage = "Success";


    // Write out the return parameters in "name = value" form

  std::ofstream rts;
  rts.open(returnParameterFile.c_str() );
  rts << "rms = " << rms << std::endl;
  rts << "outputMessage = " << outputMessage <<std::endl;
  rts.close();

  return EXIT_SUCCESS;
}
