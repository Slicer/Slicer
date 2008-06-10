#ifndef _itkWarpJacobianDeterminantFilter_txx
#define _itkWarpJacobianDeterminantFilter_txx

#include "itkWarpJacobianDeterminantFilter.h"

#include "itkNeighborhoodAlgorithm.h"
#include "itkImageRegionIterator.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkProgressReporter.h"

#include "vnl/vnl_matrix.h"
#include "vnl/vnl_det.h"
#include "vnl/vnl_math.h"
#include "vnl/algo/vnl_qr.h"

#include <vnl/algo/vnl_determinant.h>

//#include <boost/numeric/ublas/operation.hpp>
//#include <boost/numeric/ublas/lu.hpp>
//#include <qrfactorization.hpp>

namespace itk
{

template <typename TInputImage, typename TOutputImage>
WarpJacobianDeterminantFilter<TInputImage, TOutputImage>
::WarpJacobianDeterminantFilter()
{
  unsigned int i;
  m_UseImageSpacing = true;
  m_RequestedNumberOfThreads = this->GetNumberOfThreads();
  for (i = 0; i < ImageDimension; i++)
    {
    m_NeighborhoodRadius[i] = 1; // radius of neighborhood we will use
    m_DerivativeWeights[i] = 1.0;
    }
}
template <typename TInputImage, typename TOutputImage>
void
WarpJacobianDeterminantFilter<TInputImage, TOutputImage>
::SetDerivativeWeights(double data[])
{
  m_UseImageSpacing = false;

  for (unsigned int i = 0; i < ImageDimension; ++i)
    {
    if (m_DerivativeWeights[i] != data[i])
      {
      this->Modified();
      m_DerivativeWeights[i] = data[i];
      }
    }
}

template <typename TInputImage, typename TOutputImage>
void 
WarpJacobianDeterminantFilter<TInputImage, TOutputImage>
::SetUseImageSpacing(bool f)
{
  if (m_UseImageSpacing == f)
    {
    return;
    }

  // Only reset the weights if they were previously set to the image spacing,
  // otherwise, the user may have provided their own weightings.
  if (f == false && m_UseImageSpacing == true)
    {
    for (unsigned int i = 0; i < ImageDimension; ++i)
      {
      m_DerivativeWeights[i] = 1.0;
      }
    }

  m_UseImageSpacing = f;
}
  
template <typename TInputImage, typename TOutputImage>
void 
WarpJacobianDeterminantFilter<TInputImage, TOutputImage>
::GenerateInputRequestedRegion() throw(InvalidRequestedRegionError)
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  
  // get pointers to the input and output
  InputImagePointer  inputPtr = 
    const_cast< InputImageType * >( this->GetInput());
  OutputImagePointer outputPtr = this->GetOutput();
  
  if ( !inputPtr || !outputPtr )
    {
    return;
    }

  // get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion = inputPtr->GetRequestedRegion();

  // pad the input requested region by the operator radius
  inputRequestedRegion.PadByRadius( m_NeighborhoodRadius );

  // crop the input requested region at the input's largest possible region
  if ( inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()) )
    {
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    return;
    }
  else
    {
    // Couldn't crop the region (requested region is outside the largest
    // possible region).  Throw an exception.

    // store what we tried to request (prior to trying to crop)
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    
    // build an exception
    InvalidRequestedRegionError e(__FILE__, __LINE__);
    e.SetLocation(ITK_LOCATION);
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(inputPtr);
    throw e;
    }
}

template<typename TInputImage, typename TOutputImage>
void
WarpJacobianDeterminantFilter<TInputImage, TOutputImage>
::BeforeThreadedGenerateData()
{
  Superclass::BeforeThreadedGenerateData();

  // Set the weights on the derivatives.
  // Are we using image spacing in the calculations?  If so we must update now
  // in case our input image has changed.
  if (m_UseImageSpacing == true)
    {

    for (unsigned int i = 0; i < ImageDimension; i++)
      {
      if (this->GetInput()->GetSpacing()[i] <= 0.0)
        {
        itkExceptionMacro(<< "Image spacing in dimension " << i << " is zero.");
        }
      m_DerivativeWeights[i] = 1.0 / static_cast<double>(this->GetInput()->GetSpacing()[i]);
      }
    }  
}

template<typename TInputImage, typename TOutputImage>
void
WarpJacobianDeterminantFilter<TInputImage, TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread,
                       int threadId)
{

  ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;
  ConstNeighborhoodIteratorType bit;
  ImageRegionIterator<TOutputImage> it;
  
  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::
    FaceListType faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  faceList = bC(this->GetInput(), outputRegionForThread, m_NeighborhoodRadius);

  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::
    FaceListType::iterator fit;
  fit = faceList.begin();

  // Support progress methods/callbacks
  ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  // Process each of the data set faces.  The iterator is reinitialized on each
  // face so that it can determine whether or not to check for boundary
  // conditions.
  for (fit=faceList.begin(); fit != faceList.end(); ++fit)
    { 
    bit = ConstNeighborhoodIteratorType(m_NeighborhoodRadius,
                                        this->GetInput(),
                                        *fit);
    it = ImageRegionIterator<TOutputImage>(this->GetOutput(), *fit);
    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();

    while ( ! bit.IsAtEnd() )
      {
      it.Set( this->EvaluateAtNeighborhood(bit) );
      ++bit;
      ++it;
      progress.CompletedPixel();
      }
    }
}


template <typename TInputImage, typename TOutputImage>
typename WarpJacobianDeterminantFilter<TInputImage, TOutputImage>::OutputPixelType
WarpJacobianDeterminantFilter<TInputImage, TOutputImage>
::EvaluateAtNeighborhood(ConstNeighborhoodIteratorType &it) const
{
   const bool useQR = false;

   if ( !useQR )
   {
      // Simple method using field derivatives
      
      unsigned int i, j;
      vnl_matrix_fixed<double,ImageDimension,VectorDimension> J;

      //const InputPixelType centerpix = it.GetCenterPixel();
      InputPixelType next, prev;

      double weight;
   
      for (i = 0; i < ImageDimension; ++i)
      {
         next = it.GetNext(i);
         prev = it.GetPrevious(i);

         weight = 0.5*m_DerivativeWeights[i];
      
         for (j = 0; j < VectorDimension; ++j)
         {
            J[i][j]=weight*(static_cast<double>(next[j])-static_cast<double>(prev[j]));
            //J[i][j]=m_DerivativeWeights[i]*((double)(next[j])-(double)(centerpix[j]));
         }
      
         // add one on the diagonal to consider the warping and not only the deformation field
         J[i][i] += 1.0;
      }
   
      return static_cast<OutputPixelType>(vnl_det(J));
   }


   // Method using a QR factorization over a neighborhood
   typedef typename ConstNeighborhoodIteratorType::OffsetType OffsetType;
   typedef typename ConstNeighborhoodIteratorType::NeighborhoodType     NeighborhoodType;
   typedef typename NeighborhoodType::ConstIterator IteratorType;

   const NeighborhoodType neighborhood = it.GetNeighborhood();
   IteratorType it2=neighborhood.Begin();

   const InputPixelType center_pix = neighborhood.GetCenterValue();
   InputPixelType diff_pix;

   unsigned int idx = 0, dim;
   const unsigned int center_idx = neighborhood.GetCenterNeighborhoodIndex();
   const unsigned int end_idx = neighborhood.Size();
   
   OffsetType offset;

   typedef vnl_matrix<double> MatrixType;
   MatrixType Pmat(end_idx-1,ImageDimension);
   MatrixType Diffmat(end_idx-1,ImageDimension);

   for ( ; idx<center_idx; ++idx, ++it2 )
   {
      offset = neighborhood.GetOffset(idx);

      diff_pix = *it2 - center_pix;

      for (dim=0; dim<ImageDimension; ++dim)
      {
         Pmat(idx,dim) = offset[dim];
         Diffmat(idx,dim) = m_DerivativeWeights[dim]*diff_pix[dim];
      }
   }
   
   ++idx;
   ++it2;

   for ( ; idx<end_idx; ++idx, ++it2 )
   {
      offset = neighborhood.GetOffset(idx);

      diff_pix = *it2 - center_pix;

      for (dim=0; dim<ImageDimension; ++dim)
      {
         Pmat(idx-1,dim) = offset[dim];
         Diffmat(idx-1,dim) = m_DerivativeWeights[dim]*diff_pix[dim];
      }
   }

   /*
   typedef boost::numeric::ublas::vector< double > VectorType;
   VectorType diag_elements(ImageDimension);

   boost::numeric::ublas::qr_factorize(Pmat,diag_elements);
   MatrixType JminusId(ImageDimension,ImageDimension);
   boost::numeric::ublas::qr_solve(Pmat,diag_elements,Diffmat,JminusId);
   */

   vnl_qr<double> qr_solver (Pmat);
   MatrixType JminusId = qr_solver.solve(Diffmat);

   for (dim=0; dim<ImageDimension; ++dim)
     JminusId(dim,dim) += 1.0;
   
   double det = vnl_determinant<double> (JminusId);
   return det;
}



template <typename TInputImage, typename TOutputImage>
void
WarpJacobianDeterminantFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  unsigned int i;
  Superclass::PrintSelf(os,indent);
  os << indent << "m_UseImageSpacing = "          << m_UseImageSpacing
     << std::endl;
  os << indent << "m_RequestedNumberOfThreads = " << m_RequestedNumberOfThreads
     << std::endl;
  os << indent << "m_DerivativeWeights = ";
  for (i = 0; i < ImageDimension; i++)
    { os << m_DerivativeWeights[i] << " "; }
  os << std::endl;
  os << indent << "m_NeighborhoodRadius = "          << m_NeighborhoodRadius
     << std::endl;
}
  
} // end namespace itk

#endif
