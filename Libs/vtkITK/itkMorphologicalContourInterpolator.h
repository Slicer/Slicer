/*=========================================================================
*
*  Copyright Insight Software Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/
#ifndef itkMorphologicalContourInterpolator_h
#define itkMorphologicalContourInterpolator_h

#include "itkBinaryThresholdImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkImageToImageFilter.h"
#include "itksys/hash_map.hxx"

namespace itk
{
/** \class MorphologicalContourInterpolator
 *
 *  \brief Interpolates contours between slices. Based on a paper by Albu et al.
 *
 *  \par Inputs and Outputs
 *  This is an image-to-image filter. The dimensionality is 3D or higher.
 *  Input contains an image with some slices segmented, usually manually.
 *  The output has all in-between slices also segmented.
 *
 *  \par Parameters
 *  Slices are detected at positions where a pixel exists with same labeled
 *  neighbors in slice and only clear (0) neighbors perpendicular to the slice.
 *  If default behaviour is unwanted, contour indices can be set
 *  by enabling UseCustomSlicePositions and calling SetLabeledSliceIndices.
 *
 *  Filter can be restricted to run along only one axis, and/or to interpolate
 *  just one label.
 *
 *  Since optimal alignment between slices would require exhaustive search,
 *  the default is to use heuristic (breadth first search starting from centroid).
 *
 *  There is also an alternative algorithm based on distance transform approach.
 *  It is slightly faster, but it can jump across a twisty shape (not geodesic).
 *
 *  Reference:
 *  Albu AB, Beugeling T, Laurendeau D. "A morphology-based approach for
 *  interslice interpolation of anatomical slices from volumetric images."
 *  IEEE Trans Biomed Eng. 2008 Aug;55(8):2022-38. DOI:10.1109/TBME.2008.921158
 *
 *  Acknowledgement:
 *  This work is supported by NIH grant R01 EB014346, "Continued development
 *  and maintenance of the ITK-SNAP 3D image segmentation software."
 *
 * \ingroup MorphologicalContourInterpolation
 */
template< typename TImage >
class MorphologicalContourInterpolator:
  public ImageToImageFilter< TImage, TImage >
{
  template< typename T >
  friend class MorphologicalContourInterpolatorParallelInvoker;

public:
  ITK_DISALLOW_COPY_AND_ASSIGN(MorphologicalContourInterpolator);

  /** Standard class type alias. */
  using Self = MorphologicalContourInterpolator;
  using Superclass = ImageToImageFilter< TImage, TImage >;
  using Pointer = SmartPointer< Self >;
  using SliceType = Image< typename TImage::PixelType, TImage::ImageDimension - 1 >;

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  itkTypeMacro( MorphologicalContourInterpolator, ImageToImageFilter );

  /** Interpolate only this label. Interpolates all labels if set to 0 (default). */
  itkSetMacro( Label, typename TImage::PixelType );

  /** Which label is interpolated. 0 means all labels (default). */
  itkGetMacro( Label, typename TImage::PixelType );

  /** Which label is interpolated. 0 means all labels (default). */
  itkGetConstMacro( Label, typename TImage::PixelType );

  /** Interpolate only along this axis. Interpolates along all axes if set to -1 (default). */
  itkSetMacro( Axis, int );

  /** Axis of interpolation. -1 means interpolation along all axes (default). */
  itkGetMacro( Axis, int );

  /** Axis of interpolation. -1 means interpolation along all axes (default). */
  itkGetConstMacro( Axis, int );

  /** Heuristic alignment of regions for interpolation is faster than optimal alignment.
  *   Default is heuristic. */
  itkSetMacro( HeuristicAlignment, bool );

  /** Heuristic alignment of regions for interpolation is faster than optimal alignment.
  *   Default is heuristic. */
  itkGetMacro( HeuristicAlignment, bool );

  /** Heuristic alignment of regions for interpolation is faster than optimal alignment.
  *   Default is heuristic. */
  itkGetConstMacro( HeuristicAlignment, bool );

  /** Using distance transform instead of repeated dilations to calculate
  *   the median contour is slightly faster, but produces lower quality interpolations.
  *   Default is OFF (that is, use repeated dilations). */
  itkSetMacro( UseDistanceTransform, bool );

  /** Using distance transform instead of repeated dilations to calculate
  *   the median contour is slightly faster, but produces lower quality interpolations.
  *   Default is OFF (that is, use repeated dilations). */
  itkGetMacro( UseDistanceTransform, bool );

  /** Using distance transform instead of repeated dilations to calculate
  *   the median contour is slightly faster, but produces lower quality interpolations.
  *   Default is OFF (that is, use repeated dilations). */
  itkGetConstMacro( UseDistanceTransform, bool );

  /** Use custom slice positions (not slice auto-detection).
  *   SetLabeledSliceIndices has to be called prior to Update(). */
  itkSetMacro( UseCustomSlicePositions, bool );

  /** Use custom slice positions (not slice auto-detection).
  *   SetLabeledSliceIndices has to be called prior to Update(). */
  itkGetMacro( UseCustomSlicePositions, bool );

  /** Use custom slice positions (not slice auto-detection).
  *   SetLabeledSliceIndices has to be called prior to Update(). */
  itkGetConstMacro( UseCustomSlicePositions, bool );

  /** Use ball instead of default cross structuring element for repeated dilations. */
  void
  SetUseBallStructuringElement( bool useBall )
  {
    if ( useBall != m_UseBallStructuringElement )
      {
      m_UseBallStructuringElement = useBall;
      m_ConnectedComponents->SetFullyConnected( useBall );
      this->Modified();
      }
  }

  /** Use ball instead of default cross structuring element for repeated dilations. */
  itkGetMacro( UseBallStructuringElement, bool );

  /** Use ball instead of default cross structuring element for repeated dilations. */
  itkGetConstMacro( UseBallStructuringElement, bool );

  /** If there is a pixel whose all 4-way neighbors belong the the same label
  except along one axis, and along that axis its neighbors are 0 (background),
  then that axis should be interpolated along. Interpolation is possible
  along more than one axis. Updates LabeledSliceIndices.*/
  void
  DetermineSliceOrientations();

  /** An std::set of slice indices which need to be interpolated. */
  using SliceSetType = std::set< typename TImage::IndexValueType >;

  /** Clears all custom slice positions. */
  void
  ClearLabeledSliceIndices()
  {
    m_LabeledSlices.clear();
    m_LabeledSlices.resize( TImage::ImageDimension );
    this->Modified();
  }

  /** If default slice detection is not wanted, slice indices
  *   between which interpolation is done can be set using this method. */
  void
  SetLabeledSliceIndices( unsigned int axis,
    typename TImage::PixelType label,
    const std::vector< typename TImage::IndexValueType >& indices )
  {
    SliceSetType sliceSet;
    sliceSet.insert( indices.begin(), indices.end() );
    m_LabeledSlices[axis][label] = sliceSet;
    this->Modified();
  }

  /** If default slice detection is not wanted, slice indices
  *   between which interpolation is done can be set using this method. */
  void
  SetLabeledSliceIndices( unsigned int axis, typename TImage::PixelType label, const SliceSetType& indices )
  {
    m_LabeledSlices[axis][label] = indices;
    this->Modified();
  }

  /** Slice indices between which interpolation is done. */
  SliceSetType
  GetLabeledSliceIndices( unsigned int axis, typename TImage::PixelType label )
  {
    return m_LabeledSlices[axis][label];
  }

  // each label gets a set of slices in which it is present
  using LabeledSlicesType = itksys::hash_map< typename TImage::PixelType, SliceSetType >;
  using SliceIndicesType = std::vector< LabeledSlicesType >;

  /** Slice indices between which interpolation is done. */
  SliceIndicesType
  GetLabeledSliceIndices()
  {
    return m_LabeledSlices;
  }

protected:
  MorphologicalContourInterpolator();
  ~MorphologicalContourInterpolator() override = default;
  typename TImage::PixelType m_Label;
  int                        m_Axis{ -1 };
  bool                       m_HeuristicAlignment{ true };
  bool                       m_UseDistanceTransform{ true };
  bool                       m_UseBallStructuringElement{ false };
  bool                       m_UseCustomSlicePositions{ false };
  IdentifierType             m_MinAlignIters; // minimum number of iterations in align method
  IdentifierType             m_MaxAlignIters; // maximum number of iterations in align method
  IdentifierType             m_ThreadCount;   // for thread local instances
  SliceIndicesType           m_LabeledSlices; // one for each axis

  /** Derived image type alias. */
  using BoolImageType = Image< bool, TImage::ImageDimension >;
  using FloatSliceType = Image< float, TImage::ImageDimension - 1 >;
  using BoolSliceType = Image< bool, TImage::ImageDimension - 1 >;

  /** Are these two slices equal? */
  bool
  ImagesEqual( typename BoolSliceType::Pointer& a, typename BoolSliceType::Pointer& b );

  /** Does the real work. */
  void
  GenerateData() override;

  /** Determines correspondances between two slices and calls apropriate methods. */
  void
  InterpolateBetweenTwo( int axis,
    TImage* out,
    typename TImage::PixelType label,
    typename TImage::IndexValueType i,
    typename TImage::IndexValueType j,
    typename SliceType::Pointer& iconn,
    typename SliceType::Pointer& jconn );

  /** If interpolation is done along more than one axis,
  the interpolations are merged using a modified "or" rule:
  -if all interpolated images have 0 for a given pixel, the output is 0
  -if just one image has a non-zero label, then that label is chosen
  -if more than one image has a non-zero label, last written label is chosen */
  void
  InterpolateAlong( int axis, TImage* out, float startProgress, float endProgress);

  /** Slice i has a region, slice j does not */
  void
  Extrapolate( int axis,
    TImage* out,
    typename TImage::PixelType label,
    typename TImage::IndexValueType i,
    typename TImage::IndexValueType j,
    typename SliceType::Pointer& iConn,
    typename TImage::PixelType iRegionId );

  /** Creates a signed distance field image. */
  typename FloatSliceType::Pointer
  MaurerDM( typename BoolSliceType::Pointer& inImage );

  /** A sequence of conditional dilations starting with begin and reaching end.
  begin and end must cover the same region (size and index the same) */
  std::vector< typename BoolSliceType::Pointer >
  GenerateDilationSequence( typename BoolSliceType::Pointer& begin,
    typename BoolSliceType::Pointer& end );

  /** Finds an interpolating mask for these two aligned masks */
  typename BoolSliceType::Pointer
  FindMedianImageDilations( typename BoolSliceType::Pointer& intersection,
    typename BoolSliceType::Pointer& iMask,
    typename BoolSliceType::Pointer& jMask );

  /** Finds an interpolating mask for these two aligned masks */
  typename BoolSliceType::Pointer
  FindMedianImageDistances( typename BoolSliceType::Pointer& intersection,
    typename BoolSliceType::Pointer& iMask,
    typename BoolSliceType::Pointer& jMask );

  /** Build transition sequence and pick the median */
  void
  Interpolate1to1( int axis,
    TImage* out,
    typename TImage::PixelType label,
    typename TImage::IndexValueType i,
    typename TImage::IndexValueType j,
    typename SliceType::Pointer& iConn,
    typename TImage::PixelType iRegionId,
    typename SliceType::Pointer& jConn,
    typename TImage::PixelType jRegionId,
    const typename SliceType::IndexType& translation,
    bool recursive );

  using PixelList = std::vector< typename TImage::PixelType >;

  /** Splits the bigger region and does N 1-to-1 interpolations */
  void
  Interpolate1toN( int axis,
    TImage* out,
    typename TImage::PixelType label,
    typename TImage::IndexValueType i,
    typename TImage::IndexValueType j,
    typename SliceType::Pointer& iConn,
    typename TImage::PixelType iRegionId,
    typename SliceType::Pointer& jConn,
    const PixelList& jRegionIds,
    const typename SliceType::IndexType& translation );

  /** Crates a translated copy of part of the image which fits in the newRegion. */
  typename SliceType::Pointer
  TranslateImage( typename SliceType::Pointer& image,
    const typename SliceType::IndexType& translation,
    typename SliceType::RegionType newRegion );

  /** The returns cardingal of the symmetric distance between images.
  The images must cover the same region. */
  IdentifierType
  CardinalSymmetricDifference( typename BoolSliceType::Pointer& shape1, typename BoolSliceType::Pointer& shape2 );

  /** Copied from ImageSource and changed to allocate a cleared buffer. */
  void
  AllocateOutputs() override;

  /** Returns the centroid of given regions */
  typename SliceType::IndexType
  Centroid( typename SliceType::Pointer& conn, const PixelList& regionIds );

  /** Calculates maximum intersection region for both slices given a translation.
  Both inputs are modified so that jRegion is a translated version of iRegion. */
  void
  IntersectionRegions( const typename SliceType::IndexType& translation,
    typename SliceType::RegionType& iRegion,
    typename SliceType::RegionType& jRegion );

  /** Returns number of intersecting pixels */
  IdentifierType
  Intersection( typename SliceType::Pointer& iConn,
    typename TImage::PixelType iRegionId,
    typename SliceType::Pointer& jConn,
    const PixelList& jRegionIds,
    const typename SliceType::IndexType& translation );

  /** How much j needs to be translated to best align with i */
  typename SliceType::IndexType
  Align( typename SliceType::Pointer& iConn,
    typename TImage::PixelType iRegionId,
    typename SliceType::Pointer& jConn,
    const PixelList& jRegionIds );

  using BoundingBoxesType = itksys::hash_map< typename TImage::PixelType, typename TImage::RegionType >;
  BoundingBoxesType m_BoundingBoxes; // bounding box for each label

  /** Calculates a bounding box of non-zero pixels. */
  typename SliceType::RegionType
  BoundingBox( itk::SmartPointer< SliceType > image );

  /** Expands a region to incorporate the provided index.
  *   Assumes both a valid region and a valid index.
  *   It can be invoked with 2D or 3D region, hence the additional template parameter. */
  template< typename T2 >
  void
  ExpandRegion( typename T2::RegionType& region, const typename T2::IndexType& index );

  /** Connected components of a specified region. */
  typename SliceType::Pointer
  RegionedConnectedComponents( const typename TImage::RegionType& region,
    typename TImage::PixelType label,
    IdentifierType& objectCount );

  /** Seed and mask must cover the same region (size and index the same). */
  typename BoolSliceType::Pointer
  Dilate1( typename BoolSliceType::Pointer& seed, typename BoolSliceType::Pointer& mask );

  using RoiType = ExtractImageFilter< TImage, SliceType >;
  typename RoiType::Pointer m_RoI;

  using BinarizerType = BinaryThresholdImageFilter< SliceType, BoolSliceType >;
  typename BinarizerType::Pointer m_Binarizer;

  using ConnectedComponentsType = ConnectedComponentImageFilter< BoolSliceType, SliceType >;
  typename ConnectedComponentsType::Pointer m_ConnectedComponents;
};
} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMorphologicalContourInterpolator.hxx"
#endif

#endif // itkMorphologicalContourInterpolator_h
