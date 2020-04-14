/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkAnisotropicSimilarityLandmarkBasedTransformInitializer.txx,v $
  Language:  C++
  Date:      $Date: 2007/08/23 13:27:53 $
  Version:   $Revision: 1.17 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef itkAnisotropicSimilarityLandmarkBasedTransformInitializer_txx
#define itkAnisotropicSimilarityLandmarkBasedTransformInitializer_txx

#include "itkAnisotropicSimilarityLandmarkBasedTransformInitializer.h"
#include "itkMatrix.h"
#include "itkSymmetricEigenAnalysis.h"

#include <math.h>

namespace itk
{

template <class TTransform, class TFixedImage, class TMovingImage>
AnisotropicSimilarityLandmarkBasedTransformInitializer<TTransform, TFixedImage, TMovingImage>
::AnisotropicSimilarityLandmarkBasedTransformInitializer() = default;

template <class TTransform, class TFixedImage, class TMovingImage>
void
AnisotropicSimilarityLandmarkBasedTransformInitializer<TTransform, TFixedImage, TMovingImage>
::InitializeTransform()
{
  // Sanity check
  if( !m_Transform )
    {
    itkExceptionMacro( "Transform has not been set" );
    return;
    }
  if( m_FixedLandmarks.size() != m_MovingLandmarks.size() )
    {
    itkExceptionMacro("Different number of fixed and moving landmarks");
    return;
    }

  const double PI = 4.0 * atan(1.0);

  // We will do an explicit typeid check here (via dynamic_cast) to check
  // the transform type. The initialization scheme will generally be different
  // based on the transform type and the dimension. As more transforms are
  // supported in future, an explicit typeid check is expected to be done here.
  // Note that the typeid is done via dynamic_cast. This means that as more transforms
  // are added in future, you will have to order your checks from the bottom
  // of the transform hierarchy, upwards.
  //
  InputTransformType                    transformType = Else;
  AnisotropicSimilarity3DTransformType *testPtr = dynamic_cast<AnisotropicSimilarity3DTransformType *>(
      this->m_Transform.GetPointer() );
  if( testPtr )
    {
    transformType = AnisotropicSimilarity3Dtransform;
    }
  else if( dynamic_cast<Rigid2DTransformType *>(this->m_Transform.GetPointer() ) )
    {
    transformType = Rigid2Dtransfrom;
    }

  // The returning value of size() must be casted, as it might not be the same type in
  // 32 and 64 bits builds.
  unsigned int numberOfLandmarks = static_cast<unsigned int>(m_FixedLandmarks.size() );

  // If images come from filters, then update those filters.
  switch( transformType )
    {
    case AnisotropicSimilarity3Dtransform:
      {
      // Sanity check
      if( FixedImageType::ImageDimension != 3 )
        {
        itkExceptionMacro(
          "Transform is AnisotropicSimilarity3DTransform and Fixed image dimension is not 3");
        return;
        }
      if( MovingImageType::ImageDimension != 3 )
        {
        itkExceptionMacro(
          "Transform is AnisotropicSimilarity3DTransform and Moving image dimension is not 3");
        return;
        }

      // --- compute the necessary transform to match the two sets of landmarks ---
      //
      //
      //    The solution is based on
      //    Berthold K. P. Horn (1987),
      //    "Closed-form solution of absolute orientation using unit quaternions,"
      //    Journal of the Optical Society of America A, 4:629-642
      //
      //
      //    Original python implementation by David G. Gobbi
      //    Readpted from the code in VTK: Hybrid/vtkLandmarkTransform
      //
      // ----------------------------------------------------------------------------

      AnisotropicSimilarity3DTransformType *transform = dynamic_cast<AnisotropicSimilarity3DTransformType *>(
          this->m_Transform.GetPointer() );

      typedef typename AnisotropicSimilarity3DTransformType::OutputVectorType VectorType;
      typedef typename AnisotropicSimilarity3DTransformType::OutputPointType  PointType;

      // Compute the centroids
      PointType fixedCentroid;
      fixedCentroid.Fill(0.0);
      PointsContainerConstIterator fixedItr = m_FixedLandmarks.begin();
      while( fixedItr != m_FixedLandmarks.end() )
        {
        fixedCentroid[0] += (*fixedItr)[0];
        fixedCentroid[1] += (*fixedItr)[1];
        fixedCentroid[2] += (*fixedItr)[2];
        ++fixedItr;
        }

      fixedCentroid[0] /= m_FixedLandmarks.size();
      fixedCentroid[1] /= m_FixedLandmarks.size();
      fixedCentroid[2] /= m_FixedLandmarks.size();

      PointsContainerConstIterator movingItr = m_MovingLandmarks.begin();
      PointType                    movingCentroid;
      movingCentroid.Fill(0.0);
      while( movingItr != m_MovingLandmarks.end() )
        {
        movingCentroid[0] += (*movingItr)[0];
        movingCentroid[1] += (*movingItr)[1];
        movingCentroid[2] += (*movingItr)[2];
        ++movingItr;
        }

      movingCentroid[0] /= m_MovingLandmarks.size();
      movingCentroid[1] /= m_MovingLandmarks.size();
      movingCentroid[2] /= m_MovingLandmarks.size();

      itkDebugMacro(<< "fixed centroid  = " <<  fixedCentroid);
      itkDebugMacro(<< "moving centroid  = " << movingCentroid);

      VectorType scale;
      double     fixedScale = 0;
      fixedItr = m_FixedLandmarks.begin();
      while( fixedItr != m_FixedLandmarks.end() )
        {
        fixedScale += ( ( (*fixedItr)[0] - fixedCentroid[0])
                        * ( (*fixedItr)[0] - fixedCentroid[0]) );
        fixedScale += ( ( (*fixedItr)[1] - fixedCentroid[1])
                        * ( (*fixedItr)[1] - fixedCentroid[1]) );
        fixedScale += ( ( (*fixedItr)[2] - fixedCentroid[2])
                        * ( (*fixedItr)[2] - fixedCentroid[2]) );
        ++fixedItr;
        }

      fixedScale = sqrt( fixedScale / (3 * numberOfLandmarks) );
      double movingScale = 0;
      movingItr = m_MovingLandmarks.begin();
      while( movingItr != m_MovingLandmarks.end() )
        {
        movingScale += ( ( (*movingItr)[0] - movingCentroid[0])
                         * ( (*movingItr)[0] - movingCentroid[0]) );
        movingScale += ( ( (*movingItr)[1] - movingCentroid[1])
                         * ( (*movingItr)[1] - movingCentroid[1]) );
        movingScale += ( ( (*movingItr)[2] - movingCentroid[2])
                         * ( (*movingItr)[2] - movingCentroid[2]) );
        ++movingItr;
        }

      movingScale = sqrt( movingScale / (3 * numberOfLandmarks) );
      scale[0] = movingScale / fixedScale;
      scale[1] = movingScale / fixedScale;
      scale[2] = movingScale / fixedScale;

      typedef typename AnisotropicSimilarity3DTransformType::VersorType VersorType;
      typedef typename AnisotropicSimilarity3DTransformType::MatrixType MatrixType;

      VersorType versor;
      transform->SetIdentity();
      transform->SetScale( scale );

      // If we have at least 3 landmarks, we can compute a rotation.
      // Otherwise the versor will be an identity versor.
      if( numberOfLandmarks >= ImageDimension )
        {
        // loop twice - first using isotropic scale, then using
        //   anisotropic scale.
        for( int loop = 0; loop < 3; loop++ )
          {
          itk::Matrix<double, ImageDimension, ImageDimension> M;

          fixedItr  = m_FixedLandmarks.begin();
          movingItr = m_MovingLandmarks.begin();

          PointType fixedCentered;
          PointType movingCentered;

          fixedCentered.Fill( 0.0 );
          movingCentered.Fill( 0.0 );

          int ii = 0;
          // Computations are relative to the Center of Rotation.
          while( movingItr != m_MovingLandmarks.end() )
            {
            for( unsigned int i = 0; i < ImageDimension; i++ )
              {
              fixedCentered[i]  = ( (*fixedItr)[i]  - fixedCentroid[i])
                * scale[i];
              movingCentered[i] = (*movingItr)[i] - movingCentroid[i];
              }
            for( unsigned int i = 0; i < ImageDimension; i++ )
              {
              for( unsigned int j = 0; j < ImageDimension; j++ )
                {
                // mmm this indices i,j may have to be reverted...
                M[i][j] += fixedCentered[i] * movingCentered[j];
                }
              }

            ++ii;
            itkDebugMacro(<< "f_" << ii << " = " << fixedCentered );
            itkDebugMacro(<< "m_" << ii << " = " << movingCentered );
            ++movingItr;
            ++fixedItr;
            }

          // -- build the 4x4 matrix N --

          itk::Matrix<double, 4, 4> N;

          // on-diagonal elements
          N[0][0] =  M[0][0] + M[1][1] + M[2][2];
          N[1][1] =  M[0][0] - M[1][1] - M[2][2];
          N[2][2] = -M[0][0] + M[1][1] - M[2][2];
          N[3][3] = -M[0][0] - M[1][1] + M[2][2];
          // off-diagonal elements
          N[0][1] = N[1][0] = M[1][2] - M[2][1];
          N[0][2] = N[2][0] = M[2][0] - M[0][2];
          N[0][3] = N[3][0] = M[0][1] - M[1][0];

          N[1][2] = N[2][1] = M[0][1] + M[1][0];
          N[1][3] = N[3][1] = M[2][0] + M[0][2];
          N[2][3] = N[3][2] = M[1][2] + M[2][1];

          itkDebugMacro( << "For Closed form solution: ");
          itkDebugMacro(<< "M matrix " << M );
          itkDebugMacro(<< "N matrix " << N );

          vnl_matrix<double> eigenVectors(4, 4);
          vnl_vector<double> eigenValues(4);

          typedef itk::SymmetricEigenAnalysis<
            itk::Matrix<double, 4, 4>,
            vnl_vector<double>,
            vnl_matrix<double> > SymmetricEigenAnalysisType;
          SymmetricEigenAnalysisType symmetricEigenSystem(4);

          symmetricEigenSystem.ComputeEigenValuesAndVectors( N, eigenValues, eigenVectors );

          itkDebugMacro( << "EigenVectors " << eigenVectors);
          itkDebugMacro( << "EigenValues " << eigenValues);

          // By default eigen values are sorted in ascending order.  therefore the maximum
          // eigen value is the one  in the fourth place = index 3. We need the eigen
          // vector associated with the maximum eigenvalue, so we take the eigenvector
          // from the last row, index=3.

          versor.Set( eigenVectors[3][1],
                      eigenVectors[3][2],
                      eigenVectors[3][3],
                      eigenVectors[3][0]  );

          transform->SetCenter(fixedCentroid);
          transform->SetRotation( versor );

          transform->SetScale( scale );

          VectorType translation = transform->GetTranslation();
          translation = movingCentroid - fixedCentroid;
          transform->SetTranslation( translation );

          fixedItr = m_FixedLandmarks.begin();
          movingItr = m_MovingLandmarks.begin();
          VectorType fixedAniScale;
          fixedAniScale.Fill(0);
          VectorType fixedAniSign;
          fixedAniSign.Fill(0);
          while( fixedItr != m_FixedLandmarks.end() )
            {
            itk::Point<double, 3> pnt;
            pnt[0] = (*fixedItr)[0];
            pnt[1] = (*fixedItr)[1];
            pnt[2] = (*fixedItr)[2];
            fixedCentered = transform->TransformPoint( pnt );
            double tf;
            for( int i = 0; i < 3; i++ )
              {
              tf = ( (fixedCentered[i] - movingCentroid[i])
                     * (fixedCentered[i] - movingCentroid[i]) );
              if( ( fixedCentered[i] - movingCentroid[i] )
                  * ( (*movingItr)[i] - movingCentroid[i] ) < 0 )
                {
                fixedAniSign[i] -= tf;
                }
              else
                {
                fixedAniSign[i] += tf;
                }
              fixedAniScale[i] += tf;
              }
            ++fixedItr;
            ++movingItr;
            }

          movingItr = m_MovingLandmarks.begin();
          VectorType movingAniScale;
          movingAniScale.Fill(0);
          while( movingItr != m_MovingLandmarks.end() )
            {
            for( int i = 0; i < 3; i++ )
              {
              movingAniScale[i] += ( ( (*movingItr)[i] - movingCentroid[i])
                                     * ( (*movingItr)[i] - movingCentroid[i]) );
              }
            ++movingItr;
            }

          double scaleMagnitude = 0;
          for( int i = 0; i < 3; i++ )
            {
            fixedAniScale[i] = sqrt( fixedAniScale[i] / numberOfLandmarks );
            movingAniScale[i] = sqrt( movingAniScale[i] / numberOfLandmarks );
            scaleMagnitude += movingAniScale[i] * movingAniScale[i];
            if( fixedAniSign[i] < 0 )
              {
              fixedAniScale[i] *= -1;
              }
            }
          scaleMagnitude = sqrt( scaleMagnitude );
          VectorType newScale;
          for( int i = 0; i < 3; i++ )
            {
            if( movingAniScale[i] < 0.005 * scaleMagnitude
                || fabs( fixedAniScale[i] ) < 0.005 * scaleMagnitude )
              {
              newScale[i] = 1;
              }
            else
              {
              newScale[i] = movingAniScale[i] / fixedAniScale[i];
              }
            }
          MatrixType newScaleMatrix;
          newScaleMatrix(0, 0) = newScale[0];
          newScaleMatrix(1, 1) = newScale[1];
          newScaleMatrix(2, 2) = newScale[2];
          MatrixType invMatrix;
          invMatrix = versor.GetMatrix().GetInverse();
          newScaleMatrix = invMatrix * newScaleMatrix;
          newScale[0] = sqrt( fabs( newScaleMatrix.GetVnlMatrix()[0][0] * newScaleMatrix.GetVnlMatrix()[0][0]
                                    + newScaleMatrix.GetVnlMatrix()[0][1] * newScaleMatrix.GetVnlMatrix()[0][1]
                                    + newScaleMatrix.GetVnlMatrix()[0][2] * newScaleMatrix.GetVnlMatrix()[0][2] ) );
          newScale[1] = sqrt( fabs( newScaleMatrix.GetVnlMatrix()[1][0] * newScaleMatrix.GetVnlMatrix()[1][0]
                                    + newScaleMatrix.GetVnlMatrix()[1][1] * newScaleMatrix.GetVnlMatrix()[1][1]
                                    + newScaleMatrix.GetVnlMatrix()[1][2] * newScaleMatrix.GetVnlMatrix()[1][2] ) );
          newScale[2] = sqrt( fabs( newScaleMatrix.GetVnlMatrix()[2][0] * newScaleMatrix.GetVnlMatrix()[2][0]
                                    + newScaleMatrix.GetVnlMatrix()[2][1] * newScaleMatrix.GetVnlMatrix()[2][1]
                                    + newScaleMatrix.GetVnlMatrix()[2][2] * newScaleMatrix.GetVnlMatrix()[2][2] ) );
          VectorType newSign;
          newSign[0] = newScaleMatrix.GetVnlMatrix()[0][0] + newScaleMatrix.GetVnlMatrix()[0][0]
            + newScaleMatrix.GetVnlMatrix()[0][1] + newScaleMatrix.GetVnlMatrix()[0][1]
            + newScaleMatrix.GetVnlMatrix()[0][2] + newScaleMatrix.GetVnlMatrix()[0][2];
          newSign[1] = newScaleMatrix.GetVnlMatrix()[1][0] + newScaleMatrix.GetVnlMatrix()[1][0]
            + newScaleMatrix.GetVnlMatrix()[1][1] + newScaleMatrix.GetVnlMatrix()[1][1]
            + newScaleMatrix.GetVnlMatrix()[1][2] + newScaleMatrix.GetVnlMatrix()[1][2];
          newSign[2] = newScaleMatrix.GetVnlMatrix()[2][0] + newScaleMatrix.GetVnlMatrix()[2][0]
            + newScaleMatrix.GetVnlMatrix()[2][1] + newScaleMatrix.GetVnlMatrix()[2][1]
            + newScaleMatrix.GetVnlMatrix()[2][2] + newScaleMatrix.GetVnlMatrix()[2][2];
          for( int i = 0; i < 3; i++ )
            {
            if( newSign[i] < 0 )
              {
              newScale[i] *= -1;
              }
            }
          scale[0] *= newScale[0];
          scale[1] *= newScale[1];
          scale[2] *= newScale[2];
          }
        }
      else
        {
        // Remember..
        // Less than 3 landmarks available. Rotation is not computed
        transform->SetCenter(fixedCentroid);
        transform->SetRotation( versor );

        VectorType translation = transform->GetTranslation();
        translation = movingCentroid - fixedCentroid;
        transform->SetTranslation( translation );
        }

      break;
      }
    case Rigid2Dtransfrom:
      {
      // Sanity check
      if( FixedImageType::ImageDimension != 2 )
        {
        itkExceptionMacro(
          "Transform is Rigid2DTransfrom and Fixed image dimension is not 2");
        return;
        }
      if( MovingImageType::ImageDimension != 2 )
        {
        itkExceptionMacro(
          "Transform is Rigid2DTransform and Moving image dimension is not 2");
        return;
        }

      Rigid2DTransformType *transform = dynamic_cast<Rigid2DTransformType *>(
          this->m_Transform.GetPointer() );

      typedef typename Rigid2DTransformType::OutputVectorType VectorType;
      typedef typename Rigid2DTransformType::OutputPointType  PointType;

      // Initialize the transform to identity
      transform->SetIdentity();

      // Compute the centroids
      PointType fixedCentroid;
      fixedCentroid.Fill(0.0);
      PointsContainerConstIterator fixedItr = m_FixedLandmarks.begin();
      while( fixedItr != m_FixedLandmarks.end() )
        {
        fixedCentroid[0] += (*fixedItr)[0];
        fixedCentroid[1] += (*fixedItr)[1];
        ++fixedItr;
        }

      fixedCentroid[0] /= m_FixedLandmarks.size();
      fixedCentroid[1] /= m_FixedLandmarks.size();

      PointsContainerConstIterator movingItr = m_MovingLandmarks.begin();
      PointType                    movingCentroid;
      movingCentroid.Fill(0.0);
      while( movingItr != m_MovingLandmarks.end() )
        {
        movingCentroid[0] += (*movingItr)[0];
        movingCentroid[1] += (*movingItr)[1];
        ++movingItr;
        }

      movingCentroid[0] /= m_MovingLandmarks.size();
      movingCentroid[1] /= m_MovingLandmarks.size();

      itkDebugMacro(<< "fixed centroid  = " <<  fixedCentroid);
      itkDebugMacro(<< "moving centroid  = " << movingCentroid);

      double rotationAngle = 0.0;

      // If we have at least 2 landmarks, we can compute a rotation.
      // Otherwise the rotation matrix will be identity.
      //
      // For the Rigid2DTransform, the least squares error will be minimized
      // by choosing the offset as the distance between the two centroids,
      // fixed centroid (after having undergone the rotation transform, that
      // we must compute) and the moving centroid.
      // The rotation angle will be given by the cross and dot products of the
      // fixed and moving landmark vectors, the vectors being computed relative
      // to the fixed and moving centroids.
      if( numberOfLandmarks >= 2 )
        {
        fixedItr  = m_FixedLandmarks.begin();
        movingItr = m_MovingLandmarks.begin();

        VectorType fixedCentered;
        VectorType movingCentered;

        fixedCentered.Fill( 0.0 );
        movingCentered.Fill( 0.0 );

        int    ii = 0;
        double s_dot   = 0;
        double s_cross = 0;
        // Computations are relative to the Center of Rotation.
        while( movingItr != m_MovingLandmarks.end() )
          {
          fixedCentered[0]  = (*fixedItr)[0]  - fixedCentroid[0];
          movingCentered[0] = (*movingItr)[0] - movingCentroid[0];
          fixedCentered[1]  = (*fixedItr)[1]  - fixedCentroid[1];
          movingCentered[1] = (*movingItr)[1] - movingCentroid[1];

          s_dot += (movingCentered[0] * fixedCentered[0])
            + (movingCentered[1] * fixedCentered[1]);
          s_cross += (movingCentered[1] * fixedCentered[0])
            - (movingCentered[0] * fixedCentered[1]);

          ++ii;
          itkDebugMacro(<< "f_" << ii << " = " << fixedCentered );
          itkDebugMacro(<< "m_" << ii << " = " << movingCentered );
          ++movingItr;
          ++fixedItr;
          }

        itkDebugMacro(<< "Dot Product of landmarks: " << s_dot << " Cross Product: " << s_cross);
        if( std::fabs(s_dot) > 0.00005 )
          {
          rotationAngle = std::atan2(s_cross, s_dot);
          }
        else
          {
          rotationAngle = -0.5 * PI;
          }
        }
      else
        {
        itkWarningMacro(<< "Less than 2 landmarks available. Rotation is not computed");
        }

      typename Rigid2DTransformType::Pointer t = Rigid2DTransformType::New();
      t->SetIdentity();
      t->SetAngle( rotationAngle );

      transform->SetCenter( fixedCentroid );
      transform->SetAngle( rotationAngle );

      VectorType translation = transform->GetTranslation();
      itkDebugMacro(<< "Initial transform translation: " << translation);
      translation = movingCentroid - fixedCentroid;
      itkDebugMacro(<< "translation computed as difference of centroids: " << translation);
      transform->SetTranslation( translation );

      break;
      }

    case Else:
      itkWarningMacro(<< "Landmark initialization using the specified input transform not implemented");
      m_Transform->SetIdentity();
      ITK_FALLTHROUGH;

    default:
      itkWarningMacro(<< "Landmark initialization using the specified input transform not implemented");
      m_Transform->SetIdentity();
      break;
    }

}

template <class TTransform, class TFixedImage, class TMovingImage>
void
AnisotropicSimilarityLandmarkBasedTransformInitializer<TTransform, TFixedImage, TMovingImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Transform   = " << std::endl;
  if( m_Transform )
    {
    os << indent << m_Transform  << std::endl;
    }
  else
    {
    os << indent << "None" << std::endl;
    }

  os << indent << "FixedImage   = " << std::endl;
  if( m_FixedImage )
    {
    os << indent << m_FixedImage  << std::endl;
    }
  else
    {
    os << indent << "None" << std::endl;
    }

  os << indent << "MovingImage   = " << std::endl;
  if( m_MovingImage )
    {
    os << indent << m_MovingImage  << std::endl;
    }
  else
    {
    os << indent << "None" << std::endl;
    }

  os << indent << "Fixed Landmarks: " << std::endl;
  PointsContainerConstIterator fitr = m_FixedLandmarks.begin();
  while( fitr != m_FixedLandmarks.end() )
    {
    os << indent << *fitr << std::endl;
    ++fitr;
    }

  os << indent << "Moving Landmarks: " << std::endl;
  PointsContainerConstIterator mitr = m_MovingLandmarks.begin();
  while( mitr != m_MovingLandmarks.end() )
    {
    os << indent << *mitr << std::endl;
    ++mitr;
    }

}

}  // namespace itk

#endif /* itkAnisotropicSimilarityLandmarkBasedTransformInitializer_txx */
