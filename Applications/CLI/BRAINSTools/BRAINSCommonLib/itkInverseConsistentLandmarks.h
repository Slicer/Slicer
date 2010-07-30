/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile$
 *  Language:  C++
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/
// This file is for defining landmarks and what to do with them.

#ifndef __itkInverseConsistentLandmarks_h
#define __itkInverseConsistentLandmarks_h

#include "itkImage.h"

namespace itk
{
template< typename PointStorageType, unsigned int VDimension = 3 >
class InverseConsistentLandmarkPoint:public itk::Point< PointStorageType,
                                                        VDimension >
{
public:
  typedef InverseConsistentLandmarkPoint             Self;
  typedef itk::Point< PointStorageType, VDimension > Superclass;

  InverseConsistentLandmarkPoint(void)
  {
    m_FWeighting = 0.0;
    ( *this )[0] = ( *this )[1] = ( *this )[2] = 0.0;
  }

  InverseConsistentLandmarkPoint(const PointStorageType x,
                                 const PointStorageType y,
                                 const PointStorageType z,
                                 const PointStorageType t,
                                 const PointStorageType weighting):
    m_FWeighting(weighting)
  {
    ( *this )[0] = x; ( *this )[1] = y; ( *this )[2] = z;
    m_T = t;
  }

  InverseConsistentLandmarkPoint(const InverseConsistentLandmarkPoint & rhs):
    Superclass(rhs), m_FWeighting(rhs.m_FWeighting)
  {}

  PointStorageType GetT() const
  {
    return m_T;
  }

  void SetT(const PointStorageType val)
  {
    m_T = val;
  }

  PointStorageType GetWeighting(void) const
  {
    return m_FWeighting;
  }

  void SetWeighting(const PointStorageType val)
  {
    m_FWeighting = val;
  }

  virtual ~InverseConsistentLandmarkPoint(void)
  {}
private:
  PointStorageType m_T;
  PointStorageType m_FWeighting;
};

template< typename PointStorageType, class PointSetType >
class InverseConsistentLandmarks:
  public std::map< std::string, InverseConsistentLandmarkPoint< PointStorageType > >
{
public:
  typedef InverseConsistentLandmarks                         Self;
  typedef InverseConsistentLandmarkPoint< PointStorageType > PointType;

  typedef std::map< std::string, PointType > Superclass;
  typedef SmartPointer< Self >               Pointer;
  typedef SmartPointer< const Self >         ConstPointer;

  typedef typename PointSetType::Pointer PointSetTypePointer;

  typedef enum {
    GEC_LANDMARKS = 0, INTELLX_LANDMARKS, ANALYZE_LANDMARKS,
    IPL_LANDMARKS, IPL_TALAIRACH_BOUNDS_LANDMARKS,
    IPL_CEREBELLAR_BOUNDS_LANDMARKS,
    UNKNOWN_LANDMARKS
    }
  Landmark_File_Format;
  InverseConsistentLandmarks(void);
  InverseConsistentLandmarks(const int XDim, const int YDim,
                             const int ZDim, const int TDim = 0);
  virtual ~InverseConsistentLandmarks(void)
  {}

  InverseConsistentLandmarks & operator=(const InverseConsistentLandmarks & rhs);

  typedef typename itk::Image< unsigned char, 3 > ImageType;
  PointSetTypePointer GetPointSet(typename ImageType::PointType Origin)
  {
    typedef typename PointSetType::PointIdentifier PointIdentifierType;
    typedef typename PointSetType::PointType       PointSetPointType;
    typedef typename Self::const_iterator          LocalConstIterator;

    PointSetTypePointer pointSet = PointSetType::New();
    PointIdentifierType PointID =
      itk::NumericTraits< PointIdentifierType >::Zero;

    LocalConstIterator it = this->begin();
    LocalConstIterator itend = this->end();
    while ( it != itend )
      {
      const PointType   cur = ( *it ).second;
      PointSetPointType psPoint;

      for ( unsigned int i = 0; i < 3; i++ )
        {
        psPoint[i] = ( cur[i] * ImageRes[i] ) - Origin[i];
        }
      pointSet->SetPoint(PointID, psPoint);
      it++;
      PointID++;
      }
    return pointSet;
  }

  bool ReadPointTypes(const std::string lmrkfilename);

  bool ReadPointTypes(const std::string lmrkfilenamee,
                      const int XDim,
                      const int YDim,
                      const int ZDim,
                      const int TDim = 0);

  bool WritePointTypes(const std::string lmrkfilename,
                       const Landmark_File_Format lmkff = GEC_LANDMARKS);

  bool PrintPointTypes(void) const;

  Landmark_File_Format QueryLandmarkFile(const std::string lmrkfilename);

  /**
    * Function for reading a landmark file in GEC landmark format
    * \param The landmark filename
    * \return true if successful, false if failed
    */
  bool ReadGECPointTypes(const std::string lmrkfilename);

  /**
    * Shortcut for calling
    * gec::InverseConsistentLandmarks::ReadGECPointTypes(lmrkfilename);
    * gec::InverseConsistentLandmarks::rescale(XDim,YDim,ZDim,TDim);
    * \param The landmark filename
    * \param XDim the XDimension of the image for the landmarks to be rescaled
    *to
    * \param YDim the YDimension of the image for the landmarks to be rescaled
    *to
    * \param ZDim the ZDimension of the image for the landmarks to be rescaled
    *to
    * \param TDim the TDimension of the image for the landmarks to be rescaled
    *to
    * \return true if successful, false if failed
    */
  bool ReadGECPointTypes(const std::string lmrkfilename,
                         const int XDim,
                         const int YDim,
                         const int ZDim,
                         const int TDim = 0);

  /**
    * Behavior is flaky at best, because lmks file does not specify image
    *dimensions
    * If this is used, call set[XYZT]Dim immediatly after this call.
    */
  bool ReadIntellXPointTypes(const std::string lmrkfilename);

  /**
    * Reads in a IntellXLandmark File, and uses the given dimensions for the
    *image dimensions, NO RESCALING OF LANDMARKS
    * \param The landmark filename
    * \param XDim The value for XImage Dim
    * \param YDim The value for YImage Dim
    * \param ZDim The value for ZImage Dim
    * \param TDim The value for TImage Dim
    * \return true if successful, false if failed
    */
  bool ReadIntellXPointTypes(const std::string lmrkfilename,
                             const int XDim,
                             const int YDim,
                             const int ZDim,
                             const int TDim = 0);

  /**
    * Behavior is flaky at best, because lmks file does not specify image
    *dimensions
    * If this is used, call set[XYZT]Dim immediatly after this call.
    */
  bool ReadAnalyzePointTypes(const std::string lmrkfilename);

  /**
    * Reads in an Analyze Landmark File, and uses the given dimensions for the
    *image dimensions, NO RESCALING OF LANDMARKS
    * \param The landmark filename
    * \param XDim The value for XImage Dim
    * \param YDim The value for YImage Dim
    * \param ZDim The value for ZImage Dim
    * \param TDim The value for TImage Dim
    * \return true if successful, false if failed
    */
  bool ReadAnalyzePointTypes(const std::string lmrkfilename,
                             const int XDim,
                             const int YDim,
                             const int ZDim,
                             const int TDim = 0);

  /**
    * Function for reading a landmark file in IPL landmark format
    * \param The landmark filename
    * \return true if successful, false if failed
    */
  bool ReadIPLPointTypes(const std::string lmrkfilename);

  /**
    * Shortcut for calling
    * gec::InverseConsistentLandmarks::ReadIPLPointTypes(lmrkfilename);
    * gec::InverseConsistentLandmarks::rescale(XDim,YDim,ZDim,TDim);
    * \param The landmark filename
    * \param XDim the XDimension of the image for the landmarks to be rescaled
    *to
    * \param YDim the YDimension of the image for the landmarks to be rescaled
    *to
    * \param ZDim the ZDimension of the image for the landmarks to be rescaled
    *to
    * \param TDim the TDimension of the image for the landmarks to be rescaled
    *to
    * \return true if successful, false if failed
    */
  bool ReadIPLPointTypes(const std::string lmrkfilename,
                         const int XDim,
                         const int YDim,
                         const int ZDim,
                         const int TDim = 0);

  /**
    * Function for reading a landmark file in IPL talairach bounds format
    * \param lmrkfilename The landmark filename
    * \return true if successful, false if failed
    */
  bool ReadIPLTalairachPointTypes(const std::string lmrkfilename);

  /**
    * Shortcut for calling
    * gec::InverseConsistentLandmarks::ReadIPLTalairachPointTypes(lmrkfilename);
    * gec::InverseConsistentLandmarks::rescale(XDim,YDim,ZDim,TDim);
    * \param The landmark filename
    * \param XDim the XDimension of the image for the landmarks to be rescaled
    *to
    * \param YDim the YDimension of the image for the landmarks to be rescaled
    *to
    * \param ZDim the ZDimension of the image for the landmarks to be rescaled
    *to
    * \param TDim the TDimension of the image for the landmarks to be rescaled
    *to
    * \return true if successful, false if failed
    */
  bool ReadIPLTalairachPointTypes(const std::string lmrkfilename,
                                  const int XDim,
                                  const int YDim,
                                  const int ZDim,
                                  const int TDim = 0);

  /**
    * Function for reading a landmark file in IPL talairach bounds format
    * \param The landmark filename
    * \return true if successful, false if failed
    */
  bool ReadIPLCerebellarPointTypes(const std::string lmrkfilename);

  /**
    * Shortcut for calling
    *
    *gec::InverseConsistentLandmarks::ReadIPLCerebellarPointTypes(lmrkfilename);
    * gec::InverseConsistentLandmarks::rescale(XDim,YDim,ZDim,TDim);
    * \param The landmark filename
    * \param XDim the XDimension of the image for the landmarks to be rescaled
    *to
    * \param YDim the YDimension of the image for the landmarks to be rescaled
    *to
    * \param ZDim the ZDimension of the image for the landmarks to be rescaled
    *to
    * \param TDim the TDimension of the image for the landmarks to be rescaled
    *to
    * \return true if successful, false if failed
    */
  bool ReadIPLCerebellarPointTypes(const std::string lmrkfilename,
                                   const int XDim,
                                   const int YDim,
                                   const int ZDim,
                                   const int TDim = 0);

  bool WriteGECPointTypes(const std::string lmrkfilename) const;

  bool WriteIntellXPointTypes(const std::string lmrkfilename) const;

  bool WriteAnalyzePointTypes(const std::string lmrkfilename) const;

  bool ConcatLandmarks(InverseConsistentLandmarks & newlmks);

  bool ConcatLandmarks(const std::string lmrkfilename);

  bool ConcatLandmarks(const std::string lmrkfilename,
                       const int XDim,
                       const int YDim,
                       const int ZDim,
                       const int TDim = 0);

  bool RemoveClosePoints(const PointStorageType distance = 0.0);

  bool RemoveUnmatchedPoints(InverseConsistentLandmarks & tempmap1);

  /**
    * If image was 256x256, then getXDim should return 255, which is the maxXDim
    *available
    * \return MaxXDim()
    */
  inline unsigned short getXDim(void) const
  {
    return ImageDims[0];
  }

  inline unsigned short getYDim(void) const
  {
    return ImageDims[1];
  }

  inline unsigned short getZDim(void) const
  {
    return ImageDims[2];
  }

  inline unsigned short getTDim(void) const
  {
    return ImageDims[3];
  }

  /**
    * If the image is 256x256, then this should be set to 255 because that is
    *the max pixel number of the image.
    */
  inline void setXDim(const unsigned short newx)
  {
    assert(newx > 0); ImageDims[0] = newx;
  }

  inline void setYDim(const unsigned short newy)
  {
    assert(newy > 0); ImageDims[1] = newy;
  }

  inline void setZDim(const unsigned short newz)
  {
    assert(newz > 0); ImageDims[2] = newz;
  }

  inline void setTDim(const unsigned short newt)
  {
    assert(newt > 0); ImageDims[3] = newt;
  }

  /**
    * This rescales the landmarks to fit a new coordinate system.
    * \param newx new ImageDims[0]
    * \param newy new ImageDims[1]
    * \param newz new ImageDims[2]
    * \param newt new ImageDims[3]
    * \return  true if successful
    */
  bool rescale(const int newx, const int newy, const int newz, const int newt);

  void AddExtendedPointTypes3D_OnN(const InverseConsistentLandmarks & input,
                                   const int nx, const int ny, const int nz);

  void AddExtendedPointTypes3D_UnitCube(
    const InverseConsistentLandmarks & input);

private:
  bool process_bnd_point(const std::string & CurrentLandmarkName,
                         const char *buffer,
                         const unsigned short local_ImageDims[4], const float local_ImageRes[4],
                         PointType & ModifiedPoint);

  unsigned short ImageDims[4];
  float          ImageRes[4];
};
}
#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkInverseConsistentLandmarks.txx"
#endif

#endif
