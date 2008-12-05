/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxImageToHexahedronMeshFilter.h,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.7 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkImageToHexahedronMeshFilter_h
#define __itkImageToHexahedronMeshFilter_h

#include "vnl/vnl_matrix_fixed.h"
#include "itkMesh.h"
#include "itkImageToMeshFilter.h"
#include "itkVector.h"
#include "itkCellInterface.h"
#include "itkTriangleCell.h"
#include "itkCovariantVector.h"
#include "itkDefaultStaticMeshTraits.h"
#include "itkImageRegionConstIterator.h"
#include "itkAutomaticTopologyMeshSource.h"
#include "itkResampleImageFilter.h"
#include "itkPointSet.h"

namespace itk
{

/**
 * \class ImageToHexahedronMeshFilter
 * This class generates a Hexahedron mesh from an image or an
 * an image and a mask. If a mask is provided then the mesh will
 * be limited to the region of the mask. It is assumed that the
 * image and the mask have the same spacing and size. If no mask
 * is provided, then the mesh is generated from the entire image
 * above the user specified threshold. By default this threshold
 * is set to 0.
 *
 * The user can also control whether or not an alternate node
 * number is to be applied and material properties are assigned
 * to the mesh. The default material properties are assigned
 * based on a conversion of the signal intensity to a modulus 
 * of elasticity based on the work of Carter et al.
 *
 *   Carter DR, Hayes, WC, 
 *   The compressive behavior of bone as a two-phase porous structure 
 *   J of Bone Joint Surgery, 1977; 59A: 954-62
 *
 * The default node numbering is to number nodes sequentially. An 
 * alternate is to use a numbering scheme based on position in the
 * image as proposed by Grosland and Brown.
 *
 *    Grosland NM, Brown TD
 *    A voxel-based formulation for contact finite element analysis.
 *    Comput Methods Biomech Biomed Engin. 2002 Feb;5(1):21-32
 *
 *
 * \sa ImageToMeshFilter  
 */
 
 
template <class TInputImage, class TMaskImage, class TOutputMesh>
class ITK_EXPORT ImageToHexahedronMeshFilter : 
        public ImageToMeshFilter< TInputImage, TOutputMesh >
{
public:
  /** Standard "Self" typedef. */
  typedef ImageToHexahedronMeshFilter         Self;
  typedef ImageToMeshFilter< TInputImage, TOutputMesh >  Superclass;
  
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);  

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageToHexahedronMeshFilter, ImageToMeshFilter);

  
  /** Input Image typedefs. */
  typedef TInputImage InputImageType;
  typedef typename InputImageType::Pointer         InputImagePointer;
  typedef typename InputImageType::ConstPointer    InputImageConstPointer;
  typedef typename InputImageType::PixelType       InputPixelType;
  typedef typename InputImageType::SpacingType     InputSpacingType;
  typedef typename InputImageType::PointType       InputOriginType;
  typedef typename InputImageType::IndexType       InputIndexType;
  typedef typename InputImageType::IndexType       InputImageIndexType;
  typedef typename InputImageType::SizeType        InputImageSizeType;
  typedef ImageRegionConstIterator<InputImageType> InputImageIterator;
  
  /** Mask Image typedefs */
  typedef TMaskImage MaskImageType;
  typedef typename MaskImageType::Pointer          MaskImagePointer;
  typedef typename MaskImageType::ConstPointer     MaskImageConstPointer;
  typedef typename MaskImageType::PixelType        MaskPixelType;
  typedef typename MaskImageType::SpacingType      MaskSpacingType;
  typedef typename MaskImageType::PointType        MaskOriginType;
  typedef typename MaskImageType::IndexType        MaskIndexType;
  typedef typename MaskImageType::IndexType        MaskImageIndexType;
  typedef typename MaskImageType::SizeType         MaskImageSizeType;
  typedef ImageRegionConstIterator<MaskImageType>  MaskImageIterator;
  
  /** Output Mesh typedefs */  
  typedef TOutputMesh OutputMeshType;
  typedef typename OutputMeshType::MeshTraits               OutputMeshTraits;
  typedef typename OutputMeshType::PointType                OutputPointType;
  typedef typename OutputMeshTraits::PixelType              OutputPixelType; 
  typedef typename OutputMeshType::CellType                 CellType;
  typedef typename OutputMeshType::Pointer                  OutputMeshPointer;
  typedef typename OutputMeshType::CellTraits               OutputCellTraits;
  typedef typename OutputMeshType::PointsContainer          PointsContainer;
  typedef typename OutputMeshType::CellType::PointIdIterator PointIterator;
  typedef typename OutputMeshType::PointsContainerPointer 
                                                      PointsContainerPointer;
  typedef typename OutputMeshType::CellsContainerPointer 
                                                      CellsContainerPointer;
  typedef typename OutputMeshType::CellsContainer::ConstIterator CellIterator;

  typedef CovariantVector<double, 2>           doubleVector;
  typedef CovariantVector<int, 2>              intVector;
  typedef itk::PointSet<double, 3>             PointSetType;
  typedef PointSetType::PointType              PointType;
  
  /** Internal Filters Used to Generate the Mesh */ 
  typedef typename 
  itk::AutomaticTopologyMeshSource< OutputMeshType > MeshSourceType;
  typedef typename MeshSourceType::Pointer  MeshSourceTypePointer;
  
  unsigned long GetNumberOfPoints( );
  unsigned long GetNumberOfCells( );
  
  itkSetMacro( ImageThreshold, InputPixelType );
  itkGetMacro( ImageThreshold, InputPixelType);
  
  itkSetMacro( ComputeMeshProperties, bool );
  itkGetMacro( ComputeMeshProperties, bool );
  
  itkSetMacro( UseCtProperties, bool );
  itkGetMacro( UseCtProperties, bool );
  
  void SetComputeMeshPropertiesOn( );
  void SetComputeMeshPropertiesOff( );
  
  void SetUseCtPropertiesOn( );
  void SetUseCtPropertiesOff( );
  
  itkSetMacro( ComputeMeshNodeNumbering, bool );
  itkGetMacro( ComputeMeshNodeNumbering, bool );
  
  void SetComputeMeshNodeNumberingOn( );
  void SetComputeMeshNodeNumberingOff( );
  
  itkSetMacro( MeshIndexOffset, unsigned long );
  itkGetMacro( MeshIndexOffset, unsigned long );
    

  /** Set the Input */
  virtual void SetInput( const InputImageType * inputImage );
  const TInputImage* GetInput( ) const;
  
  /** Set the Mask Image */
  virtual void SetMaskImage( const MaskImageType * maskImage );
  const TMaskImage* GetMaskImage( ) const;

  /** Calculate Material Properties*/
  double CalculateMaterialProps( InputPixelType pixelData );

  void SetCellTraitFunction(double(*CellTraitFunction)(InputPixelType));

protected:
  ImageToHexahedronMeshFilter( );
  ~ImageToHexahedronMeshFilter( ){ };
  void PrintSelf( std::ostream& os, Indent indent ) const;

  void GenerateData( );
  virtual void GenerateOutputInformation( ){ }; // do nothing

  inline double fround(double n, unsigned d)
    {
    return floor(n * pow(10., static_cast<double>(d)) + .5) / 
                   pow(10., static_cast<double>(d));
    }
  
private:
  ImageToHexahedronMeshFilter( const Self& ); //purposely not implemented
  void operator=( const Self& ); //purposely not implemented
  
  bool                  m_ComputeMeshProperties;
  bool                  m_UseCtProperties;
  bool                  m_ComputeMeshNodeNumbering;
  unsigned long         m_MeshIndexOffset;
  InputPixelType        m_ImageThreshold; 
  MeshSourceTypePointer meshSource;
  double ( *m_MaterialPropertyFunction )( InputPixelType );

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkImageToHexahedronMeshFilter.txx"
#endif

#endif
