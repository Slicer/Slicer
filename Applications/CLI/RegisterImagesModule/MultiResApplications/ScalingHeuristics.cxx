#include "ScalingHeuristics.h"

ScalingValues::ScalingValues(const itk::ImageBase<3>* image, 
                             const itk::Point<double, 3> centerOfRotation)
{
  typedef itk::ImageBase<3>            ImageBaseType;
  typedef ImageBaseType::SpacingType   SpacingType;
  
  SpacingType spacing = image->GetSpacing();
  double vs = spacing.GetVnlVector().min_value();

  // Find corner points of the image
  typedef ImageBaseType::RegionType    RegionType;
  RegionType region = image->GetLargestPossibleRegion();

  typedef RegionType::IndexType        IndexType;
  typedef RegionType::SizeType         SizeType;

  IndexType ind1 = region.GetIndex();
  IndexType ind2 = region.GetIndex();
  SizeType  size = region.GetSize();

  for(unsigned int i = 0; i < 3; ++i)
    {
    ind2[i] += size[i];
    }

  typedef ImageBaseType::PointType     PointType;
  PointType p1, p2;
  image->TransformIndexToPhysicalPoint(ind1, p1);
  image->TransformIndexToPhysicalPoint(ind2, p2);

  // Find the closest point in the bounding box from
  // center of Rotation
  
  double d[3];
  for(unsigned int i = 0; i < 3; ++i)
    {
    PointType boundingPoint = centerOfRotation;
    if(fabs(centerOfRotation[i] - p1[i]) < 
       fabs(centerOfRotation[i] - p2[i]))
      {
      boundingPoint[i] = p1[i];
      }
    else
      {
      boundingPoint[i] = p2[i];
      }
    d[i] = boundingPoint.EuclideanDistanceTo(centerOfRotation);

    }  

  const double mind  = std::min(std::min(d[0], d[1]), d[2]);

  // Now that the bounding point has been found
  // compute the distance
  
  // Heuristic here 2/3 distance from center of rotation to 
  // nearest edge
  const double R = .666667 * mind;

  this->TranslationScale = vs;
  this->RotationScale = 2 * std::atan(vs / (2*R));
  this->ScalingScale = (vs / R);
  this->SkewingScale = this->ScalingScale;
}
