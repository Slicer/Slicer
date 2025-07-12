
#include "FiducialRegistrationCLP.h"

// ITK includes
#include <itkAffineTransform.h>
#include <itkImage.h>
#include <itkLandmarkBasedTransformInitializer.h>
#include <itkSimilarity3DTransform.h>
#include <itkTransformFileWriter.h>
#include <itkVersorRigid3DTransform.h>

// STD includes
#include <numeric>

namespace
{
// Function to convert a point from std::vector to itk::Point
itk::Point<double, 3> convertStdVectorToITKPoint(const std::vector<float>& vec)
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

  double operator()(const itk::Point<double, 3>& p) { return (p - m_Point).GetSquaredNorm(); }

private:
  itk::Point<double, 3> m_Point;
};

} // namespace

int main(int argc, char* argv[])
{
  PARSE_ARGS;

  const double INVALID_RMS = -1;

  // Checking conditions.

  if (fixedLandmarks.size() <= 0 || movingLandmarks.size() <= 0 || fixedLandmarks.size() != movingLandmarks.size())
  {
    std::cerr << "Fixed and moving landmark lists must be of the same size "
              << "and contain at least one point" << std::endl;
    outputMessage = "Fixed and moving landmark lists must be of the same size and contain at least one point";
    rms = INVALID_RMS;
  }

  if (saveTransform == "")
  {
    std::cerr << "An output transform must be specified" << std::endl;
    if (outputMessage == "")
    {
      outputMessage = "An output transform must be specified";
      rms = INVALID_RMS;
    }
  }

  if (transformType != "Translation" && fixedLandmarks.size() < 3)
  {
    std::cerr << "At least 3 fixed landmark fiducial points must be specified "
              << "for Rigid or Similarity transforms, have " << fixedLandmarks.size() << std::endl;
    if (outputMessage == "")
    {
      outputMessage = "At least 3 fixed landmark fiducial points must be specified for Rigid or Similarity transforms";
      rms = INVALID_RMS;
    }
  }

  // Return if conditions not met.

  if (rms == INVALID_RMS)
  {
    // Write out the return parameters in "name = value" form
    std::ofstream rts;
    rts.open(returnParameterFile.c_str());
    rts << "rms = " << rms << std::endl;
    rts << "outputMessage = " << outputMessage << std::endl;
    rts.close();

    return EXIT_SUCCESS;
  }

  // Prerequisites are fulfilled.

  typedef std::vector<itk::Point<double, 3>> PointList;

  PointList fixedPoints(fixedLandmarks.size());
  PointList movingPoints(movingLandmarks.size());

  // Convert both points lists to ITK points

  std::transform(fixedLandmarks.begin(), fixedLandmarks.end(), fixedPoints.begin(), convertStdVectorToITKPoint);

  std::transform(movingLandmarks.begin(), movingLandmarks.end(), movingPoints.begin(), convertStdVectorToITKPoint);

  typedef itk::AffineTransform<double, 3> AffineTransform;
  AffineTransform::Pointer fixedToMovingT = itk::AffineTransform<double, 3>::New();

  if (transformType == "Translation" || transformType == "Rigid")
  {
    typedef itk::VersorRigid3DTransform<double> TransformType;
    TransformType::Pointer transform = TransformType::New();
    transform->SetIdentity();

    typedef itk::LandmarkBasedTransformInitializer<TransformType, itk::Image<short, 3>, itk::Image<short, 3>>
      InitializerType;
    InitializerType::Pointer initializer = InitializerType::New();
    initializer->SetTransform(transform);
    initializer->SetFixedLandmarks(fixedPoints);
    initializer->SetMovingLandmarks(movingPoints);
    initializer->InitializeTransform();

    if (transformType == "Translation")
    {
      // Clear out the computed rotation if we only requested translation
      itk::Versor<double> v;
      v.SetIdentity();
      transform->SetRotation(v);
    }

    // Convert into an affine transform for saving to Slicer.
    fixedToMovingT->SetCenter(transform->GetCenter());
    fixedToMovingT->SetMatrix(transform->GetMatrix());
    fixedToMovingT->SetTranslation(transform->GetTranslation());
  }
  else if (transformType == "Similarity")
  {
    typedef itk::Similarity3DTransform<double> TransformType;
    TransformType::Pointer transform = TransformType::New();
    transform->SetIdentity();

    typedef itk::LandmarkBasedTransformInitializer<TransformType, itk::Image<short, 3>, itk::Image<short, 3>>
      InitializerType;
    InitializerType::Pointer initializer = InitializerType::New();
    initializer->SetTransform(transform);
    initializer->SetFixedLandmarks(fixedPoints);
    initializer->SetMovingLandmarks(movingPoints);
    initializer->InitializeTransform();

    // Convert into an affine transform for saving to Slicer.
    fixedToMovingT->SetCenter(transform->GetCenter());
    fixedToMovingT->SetMatrix(transform->GetMatrix());
    fixedToMovingT->SetTranslation(transform->GetTranslation());
  }
  else if (transformType == "Affine")
  {
    typedef itk::AffineTransform<double> TransformType;
    TransformType::Pointer transform = TransformType::New();
    transform->SetIdentity();

    typedef itk::LandmarkBasedTransformInitializer<TransformType, itk::Image<short, 3>, itk::Image<short, 3>>
      InitializerType;
    InitializerType::Pointer initializer = InitializerType::New();
    initializer->SetTransform(transform);
    initializer->SetFixedLandmarks(fixedPoints);
    initializer->SetMovingLandmarks(movingPoints);
    initializer->InitializeTransform();

    // Convert into an affine transform for saving to Slicer.
    fixedToMovingT->SetCenter(transform->GetCenter());
    fixedToMovingT->SetMatrix(transform->GetMatrix());
    fixedToMovingT->SetTranslation(transform->GetTranslation());
  }
  else
  {
    std::cerr << "Unsupported transform type: " << transformType << std::endl;
    return EXIT_FAILURE;
  }

  // Compute RMS error in the target coordinate system.

  AffineTransform::Pointer movingToFixedT = AffineTransform::New();
  fixedToMovingT->GetInverse(movingToFixedT);

  typedef PointList LandmarkPointContainerType;
  typedef LandmarkPointContainerType::const_iterator PointsContainerConstIterator;
  PointsContainerConstIterator mitr = movingPoints.begin();
  PointsContainerConstIterator fitr = fixedPoints.begin();

  AffineTransform::OutputVectorType::RealValueType sum = itk::NumericTraits<double>::ZeroValue();
  AffineTransform::OutputVectorType errortr;
  AffineTransform::OutputPointType movingPointInFixed;
  int counter = itk::NumericTraits<int>::ZeroValue();
  while (mitr != movingPoints.end())
  {
    movingPointInFixed = movingToFixedT->TransformPoint(*mitr);
    errortr = *fitr - movingPointInFixed;
    sum = sum + errortr.GetSquaredNorm();
    ++mitr;
    ++fitr;
    counter++;
  }

  rms = sqrt(sum / counter);

  itk::TransformFileWriter::Pointer twriter = itk::TransformFileWriter::New();
  twriter->SetInput(fixedToMovingT);
  twriter->SetFileName(saveTransform);

  twriter->Update();

  outputMessage = "Success";

  // Write out the return parameters in "name = value" form

  std::ofstream rts;
  rts.open(returnParameterFile.c_str());
  rts << "rms = " << rms << std::endl;
  rts << "outputMessage = " << outputMessage << std::endl;
  rts.close();

  return EXIT_SUCCESS;
}
