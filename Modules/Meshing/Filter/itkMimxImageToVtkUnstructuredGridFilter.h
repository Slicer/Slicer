/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxImageToVtkUnstructuredGridFilter.h,v $
Language:  C++
Date:      $Date: 2007/05/17 17:09:46 $
Version:   $Revision: 1.3 $

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

#ifndef __itkMimxImageToVtkUnstructuredGridFilter_h
#define __itkMimxImageToVtkUnstructuredGridFilter_h


#include <itkImageRegionConstIterator.h>
#include <itkAutomaticTopologyMeshSource.h>
#include <itkPointSet.h>
#include <itkLightProcessObject.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedLongArray.h>

namespace itk
{

/**
 * \class ImageToVtkUnstructuredGridFilter
 * This class generates a Hexahedron unstructured grid from an image or an
 * an image and a mask. If a mask is provided then the unstructured grid
 * will be limited to the region of the mask. It is assumed that the
 * image and the mask have the same spacing and size. If no mask
 * is provided, then the grid is generated from the entire image
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
 * image as proposed by Grosland et al.
 *
 *    Grosland NM, Brown TD
 *    A voxel-based formulation for contact finite element analysis.
 *    Comput Methods Biomech Biomed Engin. 2002 Feb;5(1):21-32
 *
 * Funding for this work was provided by NIBIB. The initial work was
 * supported by 5R21EB001501, and the conversion into the NA-MIC
 * toolkit was supported by a NCBC collaboration grant 1R01EB005973
 * 
 */
 
 
template <class TInputImage, class TMaskImage>
class ITK_EXPORT MimxImageToVtkUnstructuredGridFilter : public ProcessObject
{
public:
  /** Standard "Self" typedef. */
  typedef MimxImageToVtkUnstructuredGridFilter     Self;
  typedef ProcessObject                            Superclass;
  
  typedef SmartPointer<Self>                       Pointer;
  typedef SmartPointer<const Self>                 ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);  

  /** Run-time type information (and related methods). */
  itkTypeMacro(MimxImageToVtkUnstructuredGridFilter, ProcessObject);

  
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
  

  /** Get/Set the threshold applied to the image for mesh generation.
      This is only used if the mask is not defined */ 
  itkSetMacro( ImageThreshold, InputPixelType );
  itkGetMacro( ImageThreshold, InputPixelType);
  
  /** Get/Set the property used to define if the mesh is to have
      cell wise material properties generated. */ 
  itkSetMacro( ComputeMeshProperties, bool );
  itkGetMacro( ComputeMeshProperties, bool );
  
  void SetComputeMeshPropertiesOn( );
  void SetComputeMeshPropertiesOff( );

  /** Get/Set the method used to generate the material properties. 
      These properties can either be a conversion of the voxel
      data to a Modulus value based on a CT density assumption.
      The user can also provide a function to replace the
      default conversion based on CT data. */ 
  itkSetMacro( UseCtProperties, bool );
  itkGetMacro( UseCtProperties, bool );

  void SetUseCtPropertiesOn( );
  void SetUseCtPropertiesOff( );


  /** Get/Set the mesh size. This is essential a downsampling
      parameter for the mesh density and is uniform in all 
      directions */ 
  itkSetMacro( MeshResampleSize, unsigned long );
  itkGetMacro( MeshResampleSize, unsigned long );
  
  /** Get/Set the property to compute an alternate mesh numbering
      based on the voxel index. This can be handy to readily 
      reference back into the original image*/ 
  itkSetMacro( ComputeMeshNodeNumbering, bool );
  itkGetMacro( ComputeMeshNodeNumbering, bool );
  
  void SetComputeMeshNodeNumberingOn( );
  void SetComputeMeshNodeNumberingOff( );
  
  /** Get/Set the index offset. This is helpful when generating
      multiple meshes so that the cell and node numbering
      does not overlap */ 
  itkSetMacro( MeshIndexOffset, unsigned long );
  itkGetMacro( MeshIndexOffset, unsigned long );

  /** Get/Set the voxel count threshold. At least this many voxels
      must be on to generate a cell for this region */ 
  itkSetMacro( VoxelCountThreshold, unsigned long );
  itkGetMacro( VoxelCountThreshold, unsigned long );
  
  /** Get/Set the point precision to be used. */ 
  itkSetMacro( Precision, double );
  itkGetMacro( Precision, double );
  
  /** Set the Input Image */
  virtual void SetInput( const InputImageType * inputImage );
  const TInputImage* GetInput( ) const;
  
  /** Set the Mask Image. This will be used to limit the portion
      of the image used for mesh generation. The threshold value
      will be ignored if the mask is set. */
  virtual void SetMaskImage( const MaskImageType * maskImage );
  const TMaskImage* GetMaskImage( ) const;


  /** Allows the user to specify the function used to convert 
      voxel data into material property information. */
  void SetCellTraitFunction(double(*CellTraitFunction)(InputPixelType));
  
  /** Get the Output Unstructured Grid */
  itkGetMacro( Output, vtkUnstructuredGrid* );
  
  void Update( );

protected:
  MimxImageToVtkUnstructuredGridFilter( );
  ~MimxImageToVtkUnstructuredGridFilter( ){ };
  void PrintSelf( std::ostream& os, Indent indent ) const;

  /** Calculate Material Properties for the mesh */
  double CalculateMaterialProps( InputPixelType pixelData );

  /** Calculate Material Properties for the mesh */
  vtkIdType AddPoint( vtkPoints *nodalPoints, const vtkFloatingPointType newPoint[3], 
                      vtkUnsignedLongArray *nodeIndexArray, const InputIndexType imageIndex,
                      const InputImageSizeType imageSize);
  
  
private:
  MimxImageToVtkUnstructuredGridFilter( const Self& ); //purposely not implemented
  void operator=( const Self& ); //purposely not implemented
  
  bool                  m_ComputeMeshProperties;
  bool                  m_UseCtProperties;
  bool                  m_ComputeMeshNodeNumbering;
  unsigned long         m_MeshIndexOffset;
  unsigned long         m_MeshResampleSize;
  InputPixelType        m_ImageThreshold; 
  unsigned long         m_VoxelCountThreshold;
  double                m_Precision;
  vtkUnstructuredGrid*  m_Output;

  double ( *m_MaterialPropertyFunction )( InputPixelType );

};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkMimxImageToVtkUnstructuredGridFilter.txx"
#endif

#endif
