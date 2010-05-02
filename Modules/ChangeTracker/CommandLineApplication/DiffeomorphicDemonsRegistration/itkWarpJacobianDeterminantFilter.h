#ifndef __itkWarpJacobianDeterminantFilter_h
#define __itkWarpJacobianDeterminantFilter_h

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodIterator.h"
#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "itkVector.h"

namespace itk
{
/** \class WarpJacobianDeterminantFilter
 *
 * \brief Computes a scalar image from a vector image (e.g., deformation field)
 * input, where each output scalar at each pixel is the Jacobian determinant
 * of the warping at that location.
 * 
 * \par Overview
 * This filter is based on itkVectorGradientMagnitudeImageFilter and supports
 * the m_DerivativeWeights weights for partial derivatives.
 *
 * Note that the determinant of a zero vector field is also zero, whereas
 * the Jacobian determinant of the corresponding identity warp transformation
 * is 1.0.
 *
 * \par Template Parameters (Input and Output)
 * This filter has one required template parameter which defines the input
 * image type.  The pixel type of the input image is assumed to be a vector
 * The scalar type of the vector components must be castable to floating point.
 *
 * The other template parameter is the output image type.  This parameter
 * will be automatically constructed from the first parameter, so
 * it is not necessary (or advisable) to set this parameter explicitly.  Given
 * an M-channel input image with dimensionality N, the output image will be of type
 * itk::Image<float, N> unless specified otherwise.
 *
 * \par Filter Parameters 
 * The method SetUseImageSpacingOn will cause derivatives in the image to be
 * scaled (inversely) with the pixel size of the input image, effectively
 * taking derivatives in world coordinates (versus isotropic image
 * space). SetUseImageSpacingOff turns this functionality off.  Default is
 * UseImageSpacingOn.  The parameter UseImageSpacing can
 * be set directly with the method SetUseImageSpacing(bool).
 * 
 * Weights can be applied to the derivatives directly using the
 * SetDerivativeWeights method.  Note that if UseImageSpacing is set to TRUE
 * (ON), then these weights will be overridden by weights derived from the
 * image spacing when the filter is updated.  The argument to this method is a
 * C array of TRealValue type.
 *
 * \par Constraints
 * The vector dimension of the input image values must be equal 
 * to the image dimensions, which is trivially true for a deformation field
 * that maps an n-dimensional space onto itself. 

 * Currently, dimensions up to and including 4 are supported. This
 * limitation comes from the presence of vnl_det() functions for matrices of
 * dimension up to 4x4.
 *
 * \ingroup GradientFilters
 *
 * \sa Image
 * \sa Neighborhood
 * \sa NeighborhoodOperator
 * \sa NeighborhoodIterator
 *
 *
 *
 * 
 * \author Tom Vercauteren, INRIA & Mauna Kea Technologies
 */
template < typename TInputImage,
           typename TOutputImage = Image <
              float,::itk::GetImageDimension<TInputImage>::ImageDimension >
>
class ITK_EXPORT WarpJacobianDeterminantFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef WarpJacobianDeterminantFilter Self;
  typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer<Self> Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods) */
  itkTypeMacro(WarpJacobianDeterminantFilter, ImageToImageFilter);
  
  /** Extract some information from the image types.  Dimensionality
   * of the two images is assumed to be the same. */
  typedef typename TOutputImage::PixelType OutputPixelType;
  typedef typename TInputImage::PixelType  InputPixelType;

  /** Image typedef support */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;
  typedef typename InputImageType::Pointer  InputImagePointer;
  typedef typename OutputImageType::Pointer OutputImagePointer;
  
  /** The dimensionality of the input and output images. */
  itkStaticConstMacro(ImageDimension, unsigned int,
                      TOutputImage::ImageDimension);
  
  /** Length of the vector pixel type of the input image. */
  itkStaticConstMacro(VectorDimension, unsigned int,
                      InputPixelType::Dimension);

  /** Type of the iterator that will be used to move through the image.  Also
      the type which will be passed to the evaluate function */
  typedef ConstNeighborhoodIterator<InputImageType> ConstNeighborhoodIteratorType;
  typedef typename ConstNeighborhoodIteratorType::RadiusType RadiusType;  
  
  /** Superclass typedefs. */
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

  /** WarpJacobianDeterminantFilter needs a larger input requested
   * region than the output requested region (larger by the kernel
   * size to calculate derivatives).  As such, 
   * WarpJacobianDeterminantFilter needs to provide an 
   * implementation for GenerateInputRequestedRegion() in order to inform the
   * pipeline execution model.
   *
   * \sa ImageToImageFilter::GenerateInputRequestedRegion() */
  virtual void GenerateInputRequestedRegion() throw(InvalidRequestedRegionError);

  /** Set the derivative weights according to the spacing of the input image
      (1/spacing). Use this option if you want to calculate the Jacobian
      determinant in the space in which the data was acquired.*/
  void SetUseImageSpacingOn()
  { this->SetUseImageSpacing(true); }

  /** Reset the derivative weights to ignore image spacing.  Use this option if
      you want to calculate the Jacobian determinant in the image space.  
      Default is ImageSpacingOn. */
  void SetUseImageSpacingOff()
  { this->SetUseImageSpacing(false); }

  /** Set/Get whether or not the filter will use the spacing of the input
      image in its calculations */
  void SetUseImageSpacing(bool);                         
  itkGetMacro(UseImageSpacing, bool);

  /** Directly Set/Get the array of weights used in the gradient calculations.
      Note that calling UseImageSpacingOn will clobber these values.*/
  void SetDerivativeWeights(double data[]);
  itkGetVectorMacro(DerivativeWeights, const double, itk::GetImageDimension<TInputImage>::ImageDimension);

protected:
  WarpJacobianDeterminantFilter();
  virtual ~WarpJacobianDeterminantFilter() {}

  /** Do any necessary casting/copying of the input data.  Input pixel types
     whose value types are not real number types must be cast to real number
     types.*/
  void BeforeThreadedGenerateData ();

  /** WarpJacobianDeterminantFilter can be implemented as a
   * multithreaded filter (we're only using vnl_det(), which is trivially
   * thread safe).  Therefore, this implementation provides a
   * ThreadedGenerateData() routine which is called for each
   * processing thread. The output image data is allocated
   * automatically by the superclass prior to calling
   * ThreadedGenerateData().  ThreadedGenerateData can only write to
   * the portion of the output image specified by the parameter
   * "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData() */
  void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                            int threadId );

  void PrintSelf(std::ostream& os, Indent indent) const;

  /** Get/Set the neighborhood radius used for gradient computation */
  itkGetConstReferenceMacro( NeighborhoodRadius, RadiusType );
  itkSetMacro( NeighborhoodRadius, RadiusType );
  
  OutputPixelType EvaluateAtNeighborhood
     (ConstNeighborhoodIteratorType &it) const;

  /** The weights used to scale partial derivatives during processing */
  double m_DerivativeWeights[itk::GetImageDimension<TInputImage>::ImageDimension];

private:
  bool m_UseImageSpacing;
  int m_RequestedNumberOfThreads;
  
  WarpJacobianDeterminantFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  RadiusType    m_NeighborhoodRadius;  
};
  
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkWarpJacobianDeterminantFilter.txx"
#endif

#endif
