#ifndef __AirAFFINE_RW_H__
#define __AirAFFINE_RW_H__

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

#ifndef __RPW_AIR_H__
#define __RPW_AIR_H__
#include <AIR.h>
#endif
#include "Air16_rw.h"

template <typename TImageType>
class AirAffineTransform
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

  AirAffineTransform()
    {
    // as a default to indicate failure, ReadAirAffineFile will return an IsNull
    // Pointer.
    m_AffineTransform = TransformType::New();
    }

  int Read (const std::string xfrmFile);

  // HACK:  Need corresponding Set functions, and then the write command needs
  // to be pulled
  // into the class with the folowing interface:
  // void WriteAirAffineFile(const std::string xfrmFile);
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
  // REFACTOR:  In general, Air is probably agnostic about orientation.
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
  SizeType         m_FixedImageSize;
  SpacingType      m_FixedImageSpacing;
  SizeType         m_MovingImageSize;
  SpacingType      m_MovingImageSpacing;
  };

// HACK:  The following function should be put into the previous class, and all
// code should be modified to reflect the simplified interface.
template <typename TImageType>
int
AirAffineTransform<TImageType>::
  Write (const std::string & AirFile,
  const std::string & /* PatientId */,
  const std::string & /* StudyId */)
{
  FILE *fp = fopen (AirFile.c_str(), "wb");

  if ( fp == NULL )
    {
    std::cerr
   <<
    "Failed to open air file for writing -- is this a file or directory protection issue?  "
   << __FILE__ << __LINE__ << std::endl;
    return -1;
    }

  // Re-useable
  struct AIR_Air16 airBlk;
  strncpy (airBlk.s_file, "<fixed input file name here>", 127);
  strncpy (airBlk.r_file, "<moving input file name here>", 127);
  strncpy (airBlk.comment, "NoCommentProvided", 127);
  strncpy (airBlk.reserved, "", 115);
  airBlk.s.bits = 8; // Just assume unsigned char data.
  airBlk.s.x_dim = this->m_FixedImageSize[0];
  airBlk.s.y_dim = this->m_FixedImageSize[1];
  airBlk.s.z_dim = this->m_FixedImageSize[2];
  airBlk.s.x_size = this->m_FixedImageSpacing[0];
  airBlk.s.y_size = this->m_FixedImageSpacing[1];
  airBlk.s.z_size = this->m_FixedImageSpacing[2];
  airBlk.r.bits = 8; // Just assume unsigned char data.
  airBlk.r.x_dim = this->m_MovingImageSize[0];
  airBlk.r.y_dim = this->m_MovingImageSize[1];
  airBlk.r.z_dim = this->m_MovingImageSize[2];
  airBlk.r.x_size = this->m_MovingImageSpacing[0];
  airBlk.r.y_size = this->m_MovingImageSpacing[1];
  airBlk.r.z_size = this->m_MovingImageSpacing[2];
  airBlk.s_hash = 0;
  airBlk.r_hash = 0;
  airBlk.s_volume = 0;
  airBlk.r_volume = 0;

  // Transfer to intermediate 4x4 matrix so that it can be checked easily in
  // octave or matlab
  typename CrossOverAffineSystem<double, 3> ::VnlTransformMatrixType44 Q_AIR(
    0.0);
  AssignRigid::AssignConvertedTransform( Q_AIR, m_AffineTransform );
  std::cout << "=======================Q_AIR output\n" << Q_AIR;

  // Transfer values into air structures.
  for ( int i = 0; i < 4; i++ )
    {
    for ( int j = 0; j < 4; j++ )
      {
      airBlk.e[j][i] = Q_AIR.get(i, j);   // NOTE:  Need to transpose the "e"
                                          // air matrix
      }
    }

#if 0
  double pixel_size_s = airBlk.s.x_size;
  if ( airBlk.s.y_size < pixel_size_s )
    {
    pixel_size_s = airBlk.s.y_size;
    }

  if ( airBlk.s.z_size < pixel_size_s )
    {
    pixel_size_s = airBlk.s.z_size;
    }

  for ( int i = 0; i < 4; i++ )
    {
    airBlk.e[0][i] /= (F64)(airBlk.s.x_size / pixel_size_s);
    airBlk.e[1][i] /= (F64)(airBlk.s.y_size / pixel_size_s);
    airBlk.e[2][i] /= (F64)(airBlk.s.z_size / pixel_size_s);
    }
#endif

  // All ipl .air16 files need to be BigEndian.
  //
  if ( getMachineEndianess () != IPL_BIG_ENDIAN )
    {
    swapAir16(&airBlk);
    }

  fwrite(&airBlk, sizeof( struct AIR_Air16 ), 1, fp);
  fclose(fp);
  return 0;
}

template <typename TImageType>
int AirAffineTransform<TImageType>::
  Read (std::string xfrmFile)
{
  FILE *transformFilePointer = fopen (xfrmFile.c_str(), "rb");

  if ( transformFilePointer == NULL )
    {
    std::cerr
   << "Failed to open Air file for reading -- does your file exist as named?  "
   << __FILE__ << __LINE__ << std::endl;
    return -1;
    }
  struct AIR_Air16 airBlk;
  if ( fread(&airBlk, sizeof( struct AIR_Air16 ), 1, transformFilePointer) == 1 )
    {
    fclose(transformFilePointer);
    }
  else
    {
    std::cerr
   <<
    "Failed to read a whole air transform from Air file -- was your file really an Air file?  "
   << __FILE__ << __LINE__ << std::endl;
    return -1;
    }

  // Need to implement byteswapping code here.
  const double F64_UINT_MAX = UINT_MAX;
  if ( airBlk.s.bits > vcl_sqrt(F64_UINT_MAX) )  // copied this test from
                                                 // scanair source code.
    {
    swapAir16(&airBlk);
    }
  if ( ( airBlk.s.x_dim <= 0 ) || ( airBlk.s.x_dim > 2048 ) )
    {
    swapAir16(&airBlk);
    fprintf(
      stderr,
      "Apparently, the s.bits field was garbage, following the brains2 heuristic on the range of x_dim.\n");
    }

  this->m_FixedImageSize[0] = airBlk.s.x_dim;
  this->m_FixedImageSize[1] = airBlk.s.y_dim;
  this->m_FixedImageSize[2] = airBlk.s.z_dim;
  this->m_FixedImageSpacing[0] = airBlk.s.x_size;
  this->m_FixedImageSpacing[1] = airBlk.s.y_size;
  this->m_FixedImageSpacing[2] = airBlk.s.z_size;
  this->m_MovingImageSize[0] = airBlk.r.x_dim;
  this->m_MovingImageSize[1] = airBlk.r.y_dim;
  this->m_MovingImageSize[2] = airBlk.r.z_dim;
  this->m_MovingImageSpacing[0] = airBlk.r.x_size;
  this->m_MovingImageSpacing[1] = airBlk.r.y_size;
  this->m_MovingImageSpacing[2] = airBlk.r.z_size;

#if 0
  double pixel_size_s = airBlk.s.x_size;
  if ( airBlk.s.y_size < pixel_size_s )
    {
    pixel_size_s = airBlk.s.y_size;
    }

  if ( airBlk.s.z_size < pixel_size_s )
    {
    pixel_size_s = airBlk.s.z_size;
    }

  for ( int i = 0; i < 4; i++ )
    {
    airBlk.e[0][i] *= ( airBlk.s.x_size / pixel_size_s );
    airBlk.e[1][i] *= ( airBlk.s.y_size / pixel_size_s );
    airBlk.e[2][i] *= ( airBlk.s.z_size / pixel_size_s );
    }
#endif

  // Transfer values from air structures.
  // Transfer to intermediate 4x4 matrix so that it can be checked easily in
  // octave or matlab

  CrossOverAffineSystem<double, 3> ::VnlTransformMatrixType44 Q_AIR(0.0);
  for ( int j = 0; j < 4; j++ )
    {
    for ( int i = 0; i < 4; i++ )
      {
      Q_AIR.put(i, j, airBlk.e[j][i]);   // NOTE:  Need to transpose the "e"
                                         // matrix
      }
    }

  std::cout << "=======================Q_AIR input\n" << Q_AIR;
  AssignRigid::AssignConvertedTransform( m_AffineTransform, Q_AIR );

  return 0;
}

#endif
