/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkLMMSEVectorImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2005/05/4 14:28:51 $
  Version:   $Revision: 1.1
=========================================================================*/
#ifndef _itkLMMSEVectorImageFilter_txx
#define _itkLMMSEVectorImageFilter_txx
#include "itkLMMSEVectorImageFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkOffset.h"
#include "vnl/vnl_math.h"

namespace itk
{

/** Constructor */
template <class TInputImage, class TOutputImage>
LMMSEVectorImageFilter<TInputImage, TOutputImage>::LMMSEVectorImageFilter()
{
  m_Radius.Fill(1);
  m_NDWI          = 0;
  m_NBaselines    = 0;
  m_DWI           = IndicatorType( 0 );
  m_Baselines     = IndicatorType( 0 );
  m_Sigma         = 20.0f;
  m_GradientList  = GradientListType(0);
  m_Neighbours    = 1;    // By default, we use the gradient by gradient behaviour
  m_NeighboursInd = NeighboursIndType(0, 0);
  m_MinimumNumberOfUsedVoxelsFiltering = 1;
  m_UseAbsoluteValue = false;
  m_KeepValue = false;
}

template <class TInputImage, class TOutputImage>
void LMMSEVectorImageFilter<TInputImage, TOutputImage>
::BeforeThreadedGenerateData( void )
{
  if( m_Neighbours > m_NDWI )
    {
    m_Neighbours = m_NDWI;
    }
  // Find the closest neighbours to each gradient direction
  if( m_NDWI != m_DWI.GetSize() || m_NBaselines != m_Baselines.GetSize() ||
      (m_NDWI < 1 && m_NBaselines < 1) || m_GradientList.size() != m_NDWI || m_Neighbours < 1 || m_Neighbours > m_NDWI )
    {
    itkExceptionMacro( << "Bad iniialisation of the filter!!! Check parameters, please" );
    }
  m_NeighboursInd = NeighboursIndType( m_NDWI, m_Neighbours );

  // Vectors to compute the distance from each gradient direction to each other gradient direction; we need to sort to
  // find the closest
  // gradient directions to each of one.
  std::vector<OrderType> distances;
  OrderType              orderElement;
  for( unsigned int g = 0; g < m_NDWI; ++g )           // For each gradient direction
    {
    distances.clear();
    for( unsigned int k = 0; k < m_NDWI; ++k )       // Compare to each other gradient direction
      {
      orderElement[0] = (double)k;
      orderElement[1] = itk::NumericTraits<double>::ZeroValue();
      for( unsigned int d = 0; d < TInputImage::ImageDimension; ++d ) // Sum of squared differences (euclidean norm)
        {
        orderElement[1] += ( m_GradientList[g][d] * m_GradientList[k][d] );
        }
      if( orderElement[1] < -1.0f || orderElement[1] > 1.0f )
        {
        orderElement[1] = 0.0f;
        }
      else
        {
        orderElement[1] = ::acos( orderElement[1] );
        }
      if( 3.141592654f - orderElement[1] < orderElement[1] )
        {
        orderElement[1] = 3.141592654f - orderElement[1];
        }
      distances.push_back( orderElement );
      }
    std::sort( distances.begin(), distances.end(), UNLM_gradientDistance_smaller );
    for( unsigned int k = 0; k < m_Neighbours; ++k )
      {
      m_NeighboursInd[g][k] = m_DWI[(unsigned int)(distances[k][0])];
      }
    }
  return;
}

/** The requested input region is larger than the corresponding output, so we need to override this method: */
template <class TInputImage, class TOutputImage>
void LMMSEVectorImageFilter<TInputImage, TOutputImage>
::GenerateInputRequestedRegion()
throw (InvalidRequestedRegionError)
{
  // Call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();

  // Get pointers to the input and output
  InputImagePointer  inputPtr  = const_cast<TInputImage *>( this->GetInput() );
  OutputImagePointer outputPtr = this->GetOutput();

  if( !inputPtr || !outputPtr )
    {
    return;
    }

  // Get a copy of the input requested region (should equal the output
  // requested region)
  InputImageRegionType inputRequestedRegion = inputPtr->GetRequestedRegion();

  // Pad the input requested region by the operator radius
  inputRequestedRegion.PadByRadius( m_Radius );

  // Crop the input requested region at the input's largest possible region
  inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion() );
  inputPtr->SetRequestedRegion( inputRequestedRegion );
  return;
}

template <class TInputImage, class TOutputImage>
void LMMSEVectorImageFilter<TInputImage, TOutputImage>
::ThreadedGenerateData( const OutputImageRegionType& outputRegionForThread,
                        ThreadIdType itkNotUsed(threadId) )
{
  // Boundary conditions for this filter; Neumann conditions are fine
  ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;
  // Iterators:
  ConstNeighborhoodIterator<InputImageType> bit;  // Iterator for the input image
  ImageRegionIterator<OutputImageType>      it;   // Iterator for the output image
  // Input and output
  InputImageConstPointer input   =  this->GetInput();
  OutputImagePointer     output  =  this->GetOutput();
  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType           faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType> bC;
  faceList = bC( input, outputRegionForThread, m_Radius );
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>::FaceListType::iterator fit;
  // Auxilair variables to compute statistics for each DWI channel:
  double*       diff                  = new double[m_NDWI + m_NBaselines];
  double*       dSecondAveragedMoment = new double[m_NDWI + m_NBaselines];
  double*       dSquaredMagnitude     = new double[m_NDWI + m_NBaselines];
  double*       dFiltered             = new double[m_NDWI + m_NBaselines];
  unsigned int* iNumberOfUsedVoxels   = new unsigned int[m_NDWI + m_NBaselines];
  double        dFourthAveragedMoment;
  unsigned int  voxelsUsedFor4Moment;

  double* bSqMag = new double[m_NBaselines];
  double* bSqAvg = new double[m_NBaselines];
  double* bRes   = new double[m_NBaselines];
  double* dSqMag = new double[m_Neighbours];
  double* dSqAvg = new double[m_Neighbours];
  double* dRes   = new double[m_Neighbours];
  for( fit = faceList.begin(); fit != faceList.end(); ++fit )  // Iterate through facets
    { // Iterators:
    bit = ConstNeighborhoodIterator<InputImageType>(  m_Radius, input, *fit  );
    it  = ImageRegionIterator<OutputImageType>(        output,     *fit      );
    unsigned int neighborhoodSize = bit.Size();
    // Boundary condition:
    bit.OverrideBoundaryCondition( &nbc );
    // ===========================================================================================================================
    // ===========================================================================================================================
    // ===========================================================================================================================
    for( bit.GoToBegin(), it.GoToBegin(); !bit.IsAtEnd(); ++bit, ++it )    // Iterate through pixels in the current
                                                                           // facet
      { // -----------------------------------------------------------------------------------------------------------------------
       // Initiallise the vectors of sample averages for the second order magnitudes, as well as compute the squared
       // value of
       // the central pixel. Initiallise the number of voxels used for the computation of sample statistics in each
       // channel:
       // For the central voxel:
      InputPixelType dMagnitude = bit.GetCenterPixel();
      for( unsigned int iJ = 0; iJ < m_NDWI + m_NBaselines; ++iJ )   // For each channel
        {
        dSquaredMagnitude[iJ] = ( (double)dMagnitude[iJ]) * ( (double)dMagnitude[iJ]);
        dSecondAveragedMoment[iJ] = 0;
        iNumberOfUsedVoxels[iJ] = 0;
        }
      // -----------------------------------------------------------------------------------------------------------------------
      // With this implementation, we only need to estimate the fourth order moment for the baseline images
      dFourthAveragedMoment = itk::NumericTraits<double>::ZeroValue();
      voxelsUsedFor4Moment = 0;
      // -----------------------------------------------------------------------------------------------------------------------
      // Compute the sample statistics as the sum over the neighbourhood:
      for( unsigned int i = 0; i < neighborhoodSize; ++i )  // For each voxel in the neighbourhood
        {
        InputPixelType currentPixelValue = bit.GetPixel( i );
        for( unsigned int iJ = 0; iJ < m_NDWI + m_NBaselines; ++iJ )  // For each channel
          {
          if( currentPixelValue[iJ] > 0 )  // exactly zero indicates an artifical value filled in by the scanner, maybe
                                           // make a flag for this test
            {
            iNumberOfUsedVoxels[iJ]++;
            dSecondAveragedMoment[iJ] += ( (double)currentPixelValue[iJ] * (double)currentPixelValue[iJ] );
            }
          }
        for( unsigned int iJ = 0; iJ < m_NBaselines; ++iJ )
          {
          if( currentPixelValue[m_Baselines[iJ]] > 0 )
            {
            double aux  = ( (double)currentPixelValue[m_Baselines[iJ]] * (double)currentPixelValue[m_Baselines[iJ]] );
            dFourthAveragedMoment += (aux * aux);
            ++voxelsUsedFor4Moment;
            }
          }
        }
      // -----------------------------------------------------------------------------------------------------------------------
      // Compute sample statistics as relative frequencies; if the number of used voxels is too low, keep the central
      // value
      for( unsigned int iJ = 0; iJ < m_NDWI + m_NBaselines; ++iJ )  // For each DWI channel
        {
        if( iNumberOfUsedVoxels[iJ] >= m_MinimumNumberOfUsedVoxelsFiltering && dMagnitude[iJ] > 0 )
          {
          dSecondAveragedMoment[iJ] /= (double)iNumberOfUsedVoxels[iJ];
          }
        else
          {
          dSecondAveragedMoment[iJ] = dSquaredMagnitude[iJ];
          }
        }
      // The fourth order moment:
      if( voxelsUsedFor4Moment >= m_MinimumNumberOfUsedVoxelsFiltering && dMagnitude[m_Baselines[0]] > 0 )
        {
        dFourthAveragedMoment /= (double)voxelsUsedFor4Moment;
        }
      else
        {
        dFourthAveragedMoment  = dSquaredMagnitude[m_Baselines[0]] * dSquaredMagnitude[m_Baselines[0]];
        }
      // -----------------------------------------------------------------------------------------------------------------------
      // At this point, we have estimates of E{M_i^2,4}, but we need to compute the corresponding estimates for A;
      // however, we
      // compute before the zero-mean input, since dSecondAveragedMoment will be overwriten:
      //   {
      for( unsigned int iJ = 0; iJ < m_NDWI + m_NBaselines; ++iJ )
        {
        diff[iJ] = dSquaredMagnitude[iJ] - dSecondAveragedMoment[iJ];
        }
      //   }
      for( unsigned int iJ = 0; iJ < m_NDWI + m_NBaselines; ++iJ )
        {
        dSecondAveragedMoment[iJ] -= 2 * m_Sigma * m_Sigma;
        if( dSecondAveragedMoment[iJ] < 100000 * std::numeric_limits<double>::epsilon() )
          {
          dSecondAveragedMoment[iJ] = 100000 * std::numeric_limits<double>::epsilon();
          }
        }
      double aux4 = itk::NumericTraits<double>::ZeroValue();
      for( unsigned int k = 0; k < m_NBaselines; ++k )
        {
        aux4 += dSecondAveragedMoment[m_Baselines[k]];
        }
      dFourthAveragedMoment -= 8 * m_Sigma * m_Sigma * ( aux4 / (double)m_NBaselines + m_Sigma * m_Sigma );
      if( dFourthAveragedMoment < 100000 * std::numeric_limits<double>::epsilon() )
        {
        dFourthAveragedMoment = 100000 * std::numeric_limits<double>::epsilon();
        }
      // -----------------------------------------------------------------------------------------------------------------------
      // Now, we have estimates of the moments of A. We have computed as well the difference M - E{M}, that has to be
      // filtered
      // with the inverse of the covariance matrix C_M2M2.
      const unsigned int MAX_ALLOWED_VAR = 1000;
      const float        CFACT1          = 5.0f;
      //    -Initiallisation:
      OutputPixelType outPixel  = dMagnitude;
      double          normal    = itk::NumericTraits<double>::ZeroValue();
      for( unsigned int k = 0; k < m_NBaselines; ++k )
        {
        normal += dSecondAveragedMoment[m_Baselines[k]];
        }
      normal  = (normal / (double)m_NBaselines);
      normal *= normal;
      //    - Background checking:
      if( normal > 100000 * std::numeric_limits<double>::epsilon() )
        {
        normal        = ( dFourthAveragedMoment - normal ) / normal;
        //    - Variability checking:
        if( normal <= 100000 * std::numeric_limits<double>::epsilon() )      // Variability is extremely slow, so it is
                                                                             // likely that an homogeneous region is
                                                                             // being filtered
          { // In this case, ||C_A2A2|| << ||C_M2M2||, so we simply use the unbiased estimate of the second order
            // moment:
          for( unsigned int iJ = 0; iJ < m_NBaselines + m_NDWI; ++iJ )
            {
            dFiltered[iJ] = dSecondAveragedMoment[iJ];
            }
          }
        else if( normal > MAX_ALLOWED_VAR )      // Variability is too high, so C_M2M2 is close to singular, and
                                                 // numerical problems may arise.
          {
          for( unsigned int iJ = 0; iJ < m_NBaselines + m_NDWI; ++iJ )
            {
            dFiltered[iJ] = dSquaredMagnitude[iJ];
            }
          }
        else
          {
          // This is the normal case, and should be the one present in the majority of the voxels of the image
          // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
          // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
          // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
          // First, filter the baseline images, all together:
          double minSqAvg = itk::NumericTraits<double>::max();
          for( unsigned int iJ = 0; iJ < m_NBaselines; ++iJ )
            {
            bSqMag[iJ] = diff[m_Baselines[iJ]];
            bSqAvg[iJ] = dSecondAveragedMoment[m_Baselines[iJ]];
            if( bSqAvg[iJ] < minSqAvg )
              {
              minSqAvg = bSqAvg[iJ];
              }
            }
          //    - Pre-whitening of the input:
          if( minSqAvg > CFACT1 * m_Sigma * m_Sigma ) // In this case, the series expansion is convergent, so we may
                                                      // perform the linear correction
            {
            this->CMMInversion( bSqMag, bSqAvg, normal, bRes, 1, m_NBaselines );
            }
          else // The serie expansion is not convergent, and the linear correction is not stable; the aproximation is
               // not accurate, but this corresponds mainly to background pixels, so it is not so important
            {
            this->ComputeInverseMatrix( bSqMag, bSqAvg, normal, bRes, m_NBaselines );
            }
          //    - Product with C_A2M2
          //          Scalar product with the vector of second order moments:
          double dp = itk::NumericTraits<double>::ZeroValue();
          for( unsigned int iJ = 0; iJ < m_NBaselines; ++iJ )
            {
            dp += bRes[iJ] * bSqAvg[iJ];
            }
          //    - Correction of the output value:
          for( unsigned int iJ = 0; iJ < m_NBaselines; ++iJ )
            {
            dFiltered[m_Baselines[iJ]] = (1.0f + normal * dp) * bSqAvg[iJ];
            }
          // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
          // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
          // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
          // Now, filter the gradient images
          unsigned int top = m_NDWI;
          if( m_Neighbours == m_NDWI )
            {
            top = 1;
            }
          for( unsigned int g = 0; g < top; ++g )
            {
            minSqAvg = itk::NumericTraits<double>::max(); // Initialise maximum
            // Generate the vector with the appropriate measures, i.e., the ones from the closest gradient directions
            for( unsigned int iJ = 0; iJ < m_Neighbours; ++iJ )
              {
              dSqMag[iJ] = diff[m_NeighboursInd[g][iJ]];
              dSqAvg[iJ] = dSecondAveragedMoment[m_NeighboursInd[g][iJ]];
              if( dSqAvg[iJ] < minSqAvg )
                {
                minSqAvg = dSqAvg[iJ];
                }
              }
            //    - Pre-whitening of the input:
            if( minSqAvg > CFACT1 * m_Sigma * m_Sigma ) // In this case, the series expansion is convergent, so we may
                                                        // perform the linear correction
              {
              this->CMMInversion( dSqMag, dSqAvg, normal, dRes, 1, m_Neighbours );
              }
            else // The serie expansion is not convergent, and the linear correction is not stable; the aproximation is
                 // not accurate, but this corresponds mainly to background pixels, so it is not so important
              {
              this->ComputeInverseMatrix( dSqMag, dSqAvg, normal, dRes, m_Neighbours );
              }
            //    - Product with C_A2M2
            //          Scalar product with the vector of second order moments:
            dp = itk::NumericTraits<double>::ZeroValue();
            for( unsigned int iJ = 0; iJ < m_Neighbours; ++iJ )
              {
              dp += dRes[iJ] * dSqAvg[iJ];
              }
            if( m_Neighbours == m_NDWI )
              {
              //    - Correction of the output value:
              for( unsigned int iJ = 0; iJ < m_Neighbours; ++iJ )
                {
                dFiltered[m_NeighboursInd[g][iJ]] = (1.0f + normal * dp) * dSqAvg[iJ];
                }
              }
            else
              {
              dFiltered[m_DWI[g]] = (1.0f + normal * dp) * dSqAvg[0];
              }
            }
          // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
          // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
          // $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
          }
        // Compute the square root of the output, and check if the result is physically consisitent:
        for( unsigned int iJ = 0; iJ < m_NBaselines + m_NDWI; ++iJ )
          {
          if( dFiltered[iJ] > 0 )
            {
            dFiltered[iJ] = sqrt( dFiltered[iJ] );
            }
          else
            {
            if( m_UseAbsoluteValue )
              {
              dFiltered[iJ] = sqrt( -dFiltered[iJ] );
              }
            else if( m_KeepValue )
              {
              dFiltered[iJ] = dMagnitude[iJ];
              }
            else
              {
              dFiltered[iJ] = 0;
              }
            }
          }
        }
      else        // In this case, the second order moment is too small; this is likely to occur in the background
        {
        for( unsigned int iJ = 0; iJ < m_NBaselines + m_NDWI; ++iJ )
          {
          if( m_UseAbsoluteValue && (double)dFiltered[iJ] < 0 )
            {
            dFiltered[iJ] = -dFiltered[iJ];
            }
          else if( m_KeepValue )
            {
            dFiltered[iJ] = dMagnitude[iJ];
            }
          else
            {
            dFiltered[iJ] = 0;
            }
          }
        }
      // -----------------------------------------------------------------------------------------------------------------------
      // Put the output in place:
      for( unsigned int iJ = 0; iJ < m_NDWI + m_NBaselines; ++iJ )
        {
        outPixel[iJ] = static_cast<ScalarType>( dFiltered[iJ] );
        }
      it.Set( outPixel );
      }
    // ===========================================================================================================================
    // ===========================================================================================================================
    // ===========================================================================================================================
    }
  // Delete previously alloctaed memory:
  delete [] diff;
  delete [] dSecondAveragedMoment;
  delete [] dSquaredMagnitude;
  delete [] iNumberOfUsedVoxels;
  delete [] dFiltered;
  delete [] bSqMag;
  delete [] bSqAvg;
  delete [] bRes;
  delete [] dSqMag;
  delete [] dSqAvg;
  delete [] dRes;
}

/** Smart approximate inversion of C_{M^2M^2} (high SNR case)*/
template <class TInputImage, class TOutput>
void LMMSEVectorImageFilter<TInputImage, TOutput>
::CMMInversion( const double* measures, const double* squaredAverages, double normal, double* whitened,
                unsigned int order,
                unsigned int K ) const
{
  // Where:
  //     measures: the squared measurements, which is, the original data (one per channel)
  //     squaredAverages: The vector containing the second order moment for each DWI channel
  //     normal: the variance of the second order moment normalised by the square of the second order moment
  //     whitened: the processed signal, which is, C_MM^(-1)*(M^2-E{M^2})
  //     order: the number of iterations, i.e., the order of Taylor series expansion
  // Auxiliar value to precompute constants:
  if( K == 1 )
    {
    double var  = m_Sigma * m_Sigma;
    double aux  = squaredAverages[0];
    aux         = normal * aux * aux + 4.0f * var * aux + 4.0f * var * var;
    whitened[0] = measures[0] / aux;
    return;
    }
  normal     = itk::NumericTraits<double>::OneValue() / normal; // For convenience
  double aux = 4.0f * m_Sigma * m_Sigma * normal;
  // The terms in the inverse matrix:
  double  Ad = aux;
  double* Ai = new double[K];
  for( unsigned int k = 0; k < K; ++k )
    {
    Ad   += squaredAverages[k];
    Ai[k] = itk::NumericTraits<double>::OneValue() / ( aux * squaredAverages[k] );
    }
  Ad     = -itk::NumericTraits<double>::OneValue() / ( aux * Ad );
  // Now, recursively process the output; initiallise w_0 = x
  for( unsigned int k = 0; k < K; ++k )
    {
    whitened[k] = measures[k];
    }
  double cum; // Auxiliar value
  aux *= (m_Sigma * m_Sigma);
  // Iterate: w_{n+1} = x - D^{-1}w_n
  for( unsigned int o = 0; o < order; ++o )        // If order=0, this loop does nothing!
    { // Compute A_d*w
    cum = itk::NumericTraits<double>::ZeroValue();  // Initiallise acumulator
    for( unsigned int k = 0; k < K; ++k )
      {
      cum += whitened[k];
      }
    cum *= Ad;
    // Compute A_i*w
    for( unsigned int k = 0; k < K; ++k )
      {
      whitened[k] = measures[k] - aux * (   Ai[k] * whitened[k] + cum   );
      }
    }
  // Now we have the truncated series of ( Id + D^(-1) )^(-1). It remains to
  // multiplicate by D^(-1):
  // Compute A_d*w
  cum = itk::NumericTraits<double>::ZeroValue(); // Initiallise acumulator
  for( unsigned int k = 0; k < K; ++k )
    {
    cum += whitened[k];
    }
  cum *= Ad;
  // Compute A_i*w + A_d*w and normalise
  for( unsigned int k = 0; k < K; ++k )
    {
    whitened[k] = (   Ai[k] * whitened[k] + cum   ) * normal;
    }
  // Delete allocated memory:
  delete[] Ai;
  return;
}

/** Matrix inversion; the general case */
template <class TInputImage, class TOutput>
bool LMMSEVectorImageFilter<TInputImage, TOutput>
::ComputeInverseMatrix( const double* measures, const double* squaredAverages, double normal, double* whitened,
                        unsigned int K ) const
{
  if( K == 1 )
    {
    double var  = m_Sigma * m_Sigma;
    double aux  = squaredAverages[0];
    aux         = normal * aux * aux + 4.0f * var * aux + 4.0f * var * var;
    whitened[0] = measures[0] / aux;
    return true;
    }
  // Compute the matrix to invert
  double* * matrix = new double *[K];
  for( unsigned int j = 0; j < K; ++j )
    {
    matrix[j]    = new double[K];
    }
  for( unsigned int j = 0; j < K; ++j )
    {
    matrix[j][j] = normal * squaredAverages[j] * squaredAverages[j] + 4 * m_Sigma * m_Sigma
      * (squaredAverages[j] + m_Sigma * m_Sigma);
    for( unsigned int k = j + 1; k < K; ++k )
      {
      matrix[j][k] = normal * squaredAverages[j] * squaredAverages[k];
      matrix[k][j] = matrix[j][k];
      }
    }
  // Compute the independent term:
  double* iterm = new double[K];
  for( unsigned int j = 0; j < K; ++j )
    {
    iterm[j] = measures[j];
    }
  // For each column col = 1 to m_Channels-1, we need to make zeros in rows from
  // col+1 to m_Channels (note that in C++ array indices are 0-based):
  for( unsigned int col = 0; col < K - 1; ++col )  // For each column
    { // We need a non-null element in the position (col,col), in order to
     // accomplish gaussian elimination:
    if( fabs(matrix[col][col]) <= 1e-10 )
      {
      // Bad luck! The element is zero. We need to add a complete row to
      // the row in position c, so that the new element in position (c,c)
      // is not null. Find the first row for which the element (row,col)
      // is non-zero:
      unsigned int row = col + 1;
      while( fabs(matrix[row][col]) <= 1e-10 && row < K )
        {
        ++row;
        }

      // If we are not able to find a row satisfying this condition, then
      // the matrix is singular, and this should not be the case; for
      // this reason, we do not perform bound checking, for efficiency. We
      // assume that row is a valid position, and then correct the input
      // and output:
      if( row == K )  // Singular matrix!!!
        {
        for( unsigned int j = 0; j < K; ++j )
          {
          delete[] matrix[j];
          }
        delete[] matrix;
        delete[] iterm;
        return false;
        }
      for( unsigned int cc = col; cc < K; ++cc )
        {
        matrix[col][cc]  += matrix[row][cc];
        }
      iterm[col] += iterm[row];
      }
    // At this point, we have a valid (col,col), element. We scale the whole
    // corresponding col-th row so that the pivoting element is simply 1:
    double scale = itk::NumericTraits<double>::OneValue() / matrix[col][col];
    for( unsigned int cc = col; cc < K; ++cc )
      {
      matrix[col][cc]  *= scale;
      }
    iterm[col] *= scale;
    // Now, we may perform gaussian elimination for each row:
    for( unsigned int row = col + 1; row < K; ++row )  // For each row
      {
      scale = matrix[row][col]; // This is the scale, since input[col][col] = 1.
      // Once again, for each column, we add the corresponding scaled
      // version of the pivoting element; however, in the input matrix,
      // values at the left of this column are assumed to be already zero:
      for( unsigned int cc = col; cc < K; ++cc ) // Only the columns from col
        {
        matrix[row][cc] -= scale * matrix[col][cc];
        }
      iterm[row] -= scale * iterm[col];
      // We have completed this row
      }
    // We have completed this column
    }
  // Now we have an upper-triangular matrix, where all diagonal elements are
  // just 1, except for the last one; Now, we may compute the output in a recursive
  // fashion:
  if( fabs(matrix[K - 1][K - 1]) <= 1e-10 )
    {
    for( unsigned int j = 0; j < K; ++j )
      {
      delete[] matrix[j];
      }
    delete[] matrix;
    delete[] iterm;
    return false;
    }
  whitened[K - 1] = iterm[K - 1] / matrix[K - 1][K - 1]; // The last one
  for( int k = K - 2; k >= 0; --k )                      // For each component
    {
    whitened[k] = iterm[k]; // Initiallise
    for( unsigned int j = k + 1; j < K; ++j )
      {
      whitened[k] -= whitened[j] * matrix[k][j];
      }
    }
  // Delete allocated memory:
  for( unsigned int j = 0; j < K; ++j )
    {
    delete[] matrix[j];
    }
  delete[] matrix;
  delete[] iterm;
  // Matrix has been inverted!!
  return true;
}

/** Standard "PrintSelf" method */
template <class TInputImage, class TOutput>
void LMMSEVectorImageFilter<TInputImage, TOutput>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Radius: "                             << m_Radius                             << std::endl;
  os << indent << "UseAbsoluteValue: "                   << m_UseAbsoluteValue                   << std::endl;
  os << indent << "KeepValue: "                          << m_KeepValue                          << std::endl;
  os << indent << "MinimumNumberOfUsedVoxelsFiltering: " << m_MinimumNumberOfUsedVoxelsFiltering << std::endl;
}

} // end namespace itk

#endif
