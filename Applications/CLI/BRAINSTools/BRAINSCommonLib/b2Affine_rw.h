#ifndef __B2AFFINE_RW_H__
#define __B2AFFINE_RW_H__

#include <itkMatrix.h>
#include <itkVector.h>
#include <itkAffineTransform.h>
#include <vnl/vnl_matrix.h>
#include <vnl/vnl_inverse.h>
#include <vnl/vnl_matrix_fixed.h>
#include <vnl/vnl_vector.h>
#include <vcl_cmath.h>
#include <stdio.h>

#include "iplUtils/iplUtils.h"
#include "iplUtils/b2Version.h"
#include "iplUtils/iplSystem.h"
#include "iplHeader/iplHeader.h"
#include <string>
#include "iplHeader/iplHeaderTypes.h"
#include "iplFilterBase/iplFilterBase.h"
#include "iplUtils/iplUtils.h"

#include "InsightToolkit/Utilities/itksys/SystemTools.hxx"
#include "itkImage.h"
#include "itkSpatialOrientation.h"
#include "itkSpatialOrientationAdapter.h"
#include "itkResampleImageFilter.h"
#include "CrossOverAffineSystem.h"
#include "ConvertToRigidAffine.h"

// TODO:  This entire class should be made into an ITK IO reader.

template <typename TImageType>
class B2AffineTransform
  {
public:
  typedef typename CrossOverAffineSystem<double,
    3>::AffineTransformType TransformType;
  typedef typename TransformType::Pointer
  TransformPointer;
  typedef typename TImageType::RegionType::SizeType
  SizeType;
  typedef typename TImageType::SpacingType
  SpacingType;

  B2AffineTransform()
    {
    // as a default to indicate failure, ReadB2AffineFile will return an IsNull
    // Pointer.
    m_AffineTransform = TransformType::New();
    }

  int Read (const std::string xfrmFile);

  // HACK:  Need corresponding Set functions, and then the write command needs
  // to be pulled
  // into the class with the folowing interface:
  // void WriteB2AffineFile(const std::string xfrmFile);
  int Write (const std::string & xfrmFile,
    const std::string & PatientId,
    const std::string & StudyId);

  int Write (const std::string & xfrmFile)
  {
    const std::string dummy("UNKNOWN");

    return Write(xfrmFile, dummy, dummy);
  }

  TransformPointer GetAffineTransformPointer()
  {
    return m_AffineTransform;
  }

  void SetAffineTransformPointer(TransformPointer & xfrm)
  {
    m_AffineTransform = xfrm;
  }

  // Check to make sure that input image
  // and transform characteristics match, but then this needs to be templated.
  bool CheckMovingImageCharacteristics(
    typename TImageType::Pointer & ImageToDeform) const
  {
    if ( ( ImageToDeform->GetLargestPossibleRegion().GetSize() !=
           m_MovingImageSize )
        || ( ImageToDeform->GetSpacing() != m_MovingImageSpacing )
         )
      {
      return false;
      }
    return true;
  }

  SizeType GetFixedImageSize()
  {
    return m_FixedImageSize;
  }

  SpacingType GetFixedImageSpacing()
  {
    return m_FixedImageSpacing;
  }

  // Brains2 XFRM always assume origin is 0,0,0
  typename TImageType::PointType GetFixedImageOrigin()
  {
    typename TImageType::PointType tempOrigin;
    tempOrigin[0] = tempOrigin[1] = tempOrigin[2] = 0;
    return tempOrigin;
  }

  // Brains2 XFRM always assume direction is coronal;
  typename TImageType::DirectionType GetFixedImageDirection()
  {
    typename itk::SpatialOrientationAdapter::DirectionType CORdir
      = itk::SpatialOrientationAdapter().ToDirectionCosines(
      itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP);
    return CORdir;
  }

  void SetFixedImageSize(SizeType & size)
  {
    m_FixedImageSize = size;
  }

  void SetFixedImageSpacing(SpacingType & spacing)
  {
    m_FixedImageSpacing = spacing;
  }

  void SetFixedAttributesFromImage(typename TImageType::Pointer & fixedImage)
  {
    SizeType size = fixedImage->GetLargestPossibleRegion().GetSize();

    this->SetFixedImageSize(size);
    SpacingType spacing = fixedImage->GetSpacing();
    this->SetFixedImageSpacing(spacing);
  }

  SizeType GetMovingImageSize()
  {
    return m_MovingImageSize;
  }

  SpacingType GetMovingImageSpacing()
  {
    return m_MovingImageSpacing;
  }

  void SetMovingImageSize(SizeType & size)
  {
    m_MovingImageSize = size;
  }

  void SetMovingImageSpacing(SpacingType & spacing)
  {
    m_MovingImageSpacing = spacing;
  }

  void SetMovingAttributesFromImage(typename TImageType::Pointer & movingImage)
  {
    SizeType size = movingImage->GetLargestPossibleRegion().GetSize();

    this->SetMovingImageSize(size);
    SpacingType spacing = movingImage->GetSpacing();
    this->SetMovingImageSpacing(spacing);
  }

  typename TImageType::Pointer
  ApplyTransform(typename TImageType::Pointer & ImageToDeform)
  {
    const bool ValidInput = this->CheckMovingImageCharacteristics(ImageToDeform);

    if ( !ValidInput )
      {
      return TImageType::New(); // Return a null pointer
      }
    typedef typename itk::ResampleImageFilter<TImageType,
      TImageType> ResampleFilterType;
    typename ResampleFilterType::Pointer resample = ResampleFilterType::New();
    resample->SetTransform(m_AffineTransform);
    resample->SetInput(ImageToDeform);
    resample->SetSize( this->GetFixedImageSize() );
    resample->SetOutputOrigin( this->GetFixedImageOrigin() );
    resample->SetOutputSpacing( this->GetFixedImageSpacing() );
    resample->SetOutputDirection( this->GetFixedImageDirection() );

    typename TImageType::PixelType p(
      static_cast<typename TImageType::PixelType>( 0 ) );
    resample->SetDefaultPixelValue(p);
    resample->Update();
    return resample->GetOutput();
  }

private:
  TransformPointer m_AffineTransform;
  // TODO:  The following should be metadata on the m_AffineTransform object.
  SizeType    m_FixedImageSize;
  SpacingType m_FixedImageSpacing;
  SizeType    m_MovingImageSize;
  SpacingType m_MovingImageSpacing;
  };

// HACK:  The following function should be put into the previous class, and all
// code should be modified to reflect the simplified interface.
template <typename TImageType>
int
B2AffineTransform<TImageType>::
  Write (const std::string & xfrmFile,
  const std::string & PatientId,
  const std::string & StudyId)
{
  FILE *fp = fopen (xfrmFile.c_str(), "wb");

  if ( fp == NULL )
    {
    std::cerr
   <<
    "Failed to open b2 xfrm file for writing -- is this a file or directory protection issue?  "
   << __FILE__ << __LINE__ << std::endl;
    return -1;
    }

  iplTransformHeader transformHdr;  /* Brains2 Transform Header */
  transformHdr.program = "MattesMutualInformation";
    {
    std::string verstr = B2_VERSION;            /* Full path to the Transform
                                                  file */
    std::string::size_type ptr = verstr.rfind( ':');
    if ( ptr == std::string::npos )
      {
      ptr = 0;
      }
    else
      {
      ptr += 2;
      }
    transformHdr.version = verstr.substr(ptr);
    }
  transformHdr.patId = PatientId;
  transformHdr.scanId = StudyId;
    {
    std::string transformPath; /* Full path to the Transform file */
    ipl_realPath (xfrmFile, transformPath);
    transformHdr.filename = transformPath;
    }
  transformHdr.creator = itksys::SystemTools::GetEnv("USER");
    {
    std::string dateString;
    ipl_getDate (dateString);
    transformHdr.date = dateString;
    }
    {
    std::string xfrmName;
    STATUS      check = ipl_pathTail (xfrmFile, xfrmName, '/');
    if ( check == IPL_SUCCESS )
      {
      transformHdr.name = xfrmName;
      }
    else
      {
      transformHdr.name = xfrmFile;
      }
    }
  transformHdr.type = IPL_AFFINE_TRANSFORM;
  transformHdr.numDims = 3;
  for ( unsigned i = 0; i < 3; i++ )
    {
    transformHdr.standardDims[i] = this->m_FixedImageSize[i];
    transformHdr.standardRes[i] = this->m_FixedImageSpacing[i];
    transformHdr.resliceDims[i] = this->m_MovingImageSize[i];
    transformHdr.resliceRes[i] = this->m_MovingImageSpacing[i];
    }

  STATUS check = transformHdr.WriteHeader (fp);
  if ( check == IPL_FAILURE )
    {
    std::cerr << "Failed to write b2 transform header " << __FILE__
              << __LINE__ << std::endl;
    fclose (fp);
    return -1;
    }

  // Transfer to intermediate 4x4 matrix so that it can be checked easily in
  // octave or matlab
  typename CrossOverAffineSystem<double, 3> ::VnlTransformMatrixType44 Q_B2(0.0);
  AssignRigid::AssignConvertedTransform( Q_B2, m_AffineTransform );
  std::cout << "=======================Q_B2 output\n" << Q_B2;

  // Transfer values into output block.
  F64 affineTransform[16];
  for ( int i = 0, k = 0; i < 4; i++ )
    {
    for ( int j = 0; j < 4; j++, k++ )
      {
      affineTransform[k] = Q_B2.get(i, j);   // NOT Transposing...
      }
    }

  const S32 nItems = fwrite (affineTransform, sizeof( F64 ), 16, fp);
  if ( nItems != 16 )
    {
    std::cerr << "Failed to write b2 transform data block " << __FILE__
              << __LINE__ << std::endl;
    fclose (fp);
    return -1;
    }

  fclose (fp);
  return 0;
}

template <typename TImageType>
int B2AffineTransform<TImageType>::
  Read (std::string xfrmFile)
{
  FILE *transformFilePointer = fopen (xfrmFile.c_str(), "rb");

  if ( transformFilePointer == NULL )
    {
    std::cerr
   <<
    "Failed to open b2 xfrm file for reading -- does your file exist as named?  "
   << __FILE__ << __LINE__ << std::endl;
    return -1;
    }

  iplTransformHeader transformHdr;  /* Brains2 Transform Header */
  STATUS             check = transformHdr.ReadHeader (transformFilePointer);
  if ( check == IPL_FAILURE )
    {
    std::cerr << "Failed to read b2 xfrm header " << __FILE__ << __LINE__
              << std::endl;
    fclose (transformFilePointer);
    return -1;
    }
  if ( transformHdr.numDims != 3 )
    {
    std::cerr
   << "While reading b2 xfrm file - TRANSFORM_NUM_DIMS do not equal 3 "
   << __FILE__ << __LINE__ << std::endl;
    fclose (transformFilePointer);
    return -1;
    }
  for ( int i = 0; i < 3; i++ )
    {
    m_FixedImageSize[i] = transformHdr.standardDims[i];
    m_FixedImageSpacing[i] = transformHdr.standardRes[i];
    m_MovingImageSize[i] = transformHdr.resliceDims[i];
    m_MovingImageSpacing[i] = transformHdr.resliceRes[i];
    }

  BOOLEAN swapFlag = IPL_FALSE;
  if ( transformHdr.byteOrder != getMachineEndianess () )
    {
    swapFlag = IPL_TRUE;
    }

  switch ( transformHdr.type )
    {
    case IPL_AFFINE_TRANSFORM:
        {
        F64       affineTransform[16];
        const S32 nItems = fread (affineTransform,
          sizeof( F64 ),
          16,
          transformFilePointer);
        if ( nItems != 16 )
          {
          std::cerr
       <<
          "While reading b2 xfrm file - IPL_AFFINE_TRANSFORM file did not contain 16 F64s after the header. "
       << __FILE__ << __LINE__ << std::endl;
          fclose (transformFilePointer);
          return -1;
          }

        if ( swapFlag == IPL_TRUE )
          {
          for ( S32 i = 0; i < 16; i++ )
            {
            EightByteSwap ( &( affineTransform[i] ) );
            }
          }

        // Transfer values from b2xfrm Affine data block.
        // Transfer to intermediate 4x4 matrix so that it can be
        // checked easily in octave or matlab

        CrossOverAffineSystem<double, 3> ::VnlTransformMatrixType44 Q_B2(0.0);
        for ( int i = 0, k = 0; i < 4; i++ )
          {
          for ( int j = 0; j < 4; j++, k++ )
            {
            Q_B2.put(i, j, affineTransform[k]); // NOT Transposing...
            }
          }

        std::cout << "=======================Q_B2 input\n" << Q_B2;
        AssignRigid::AssignConvertedTransform( m_AffineTransform, Q_B2 );
        }
        break;

    default:
      std::cerr
   <<
      "While reading b2 xfrm file - transform type was not IPL_AFFINE_TRANSFORM "
   << __FILE__ << __LINE__ << std::endl;
      fclose (transformFilePointer);
      return -1;
    }

  fclose (transformFilePointer);
  return 0;
}

#endif
