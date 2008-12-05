/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxImageToHexahedronMeshSource.h,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
Version:   $Revision: 1.2 $

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

#ifndef __itkImageToHexahedronMeshSource_h
#define __itkImageToHexahedronMeshSource_h

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

namespace itk
{

/** \class ImageToHexahedronMeshSource
 * 
 * 
 * \par
 * Add documentation.
 *
 * 
 *
 *  */
 
 
#ifndef ITK_EXPLICIT_INSTANTIATION
//This is already explicitly defined when ITK_EXPLICIT_INSTANTIATION is used.
template class Image<unsigned short,3>;
#endif

template <class TInputImage, class TOutputMesh>
class ITK_EXPORT ImageToHexahedronMeshSource : public ImageToMeshFilter< TInputImage, TOutputMesh >
{
public:
  /** Standard "Self" typedef. */
  typedef ImageToHexahedronMeshSource         Self;
  typedef ImageToMeshFilter< TInputImage, TOutputMesh >  Superclass;
  
  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);  

  /** Run-time type information (and related methods). */
  itkTypeMacro(ImageToHexahedronMeshSource, ImageToMeshFilter);

  /** Hold on to the type information specified by the template parameters. */
  typedef TOutputMesh OutputMeshType;
  typedef typename OutputMeshType::MeshTraits   OMeshTraits;
  typedef typename OutputMeshType::PointType    OPointType;
  typedef typename OMeshTraits::PixelType       OPixelType;  

  /** Some convenient typedefs. */
  typedef typename OutputMeshType::Pointer OutputMeshPointer;
  typedef typename OutputMeshType::CellTraits CellTraits;
  typedef typename OutputMeshType::PointsContainerPointer PointsContainerPointer;
  typedef typename OutputMeshType::PointsContainer   PointsContainer;
  typedef typename OutputMeshType::CellsContainerPointer CellsContainerPointer;
  typedef typename OutputMeshType::CellsContainer   CellsContainer;
  typedef CovariantVector<double, 2>     doubleVector;
  typedef CovariantVector<int, 2>        intVector;
  
  /** Define the triangular cell types which forms the surface of the model
   * and will be used in FEM application. */
  typedef CellInterface<OPixelType, CellTraits>  TCellInterface;
  typedef TriangleCell<TCellInterface> TriCell;
  typedef typename TriCell::SelfAutoPointer TriCellAutoPointer;

  /** Input Image Type Definition. */
  typedef TInputImage InputImageType;
  typedef typename InputImageType::Pointer         InputImagePointer;
  typedef typename InputImageType::ConstPointer    InputImageConstPointer;
  typedef typename InputImageType::PixelType       InputPixelType;
  typedef typename InputImageType::SpacingType     SpacingType;
  typedef typename InputImageType::PointType       OriginType;
  typedef typename InputImageType::IndexType       IndexType;
     
  /** Type definition for the classified image index type. */
  typedef typename InputImageType::IndexType       InputImageIndexType;

  typedef ImageRegionConstIterator< InputImageType > InputImageIterator;
  
  typedef typename itk::AutomaticTopologyMeshSource< OutputMeshType >  MeshSourceType;
  typedef typename MeshSourceType::Pointer  MeshSourceTypePointer;
  
  itkGetMacro(NumberOfNodes, unsigned long);
  itkGetMacro(NumberOfCells, unsigned long);

  /** accept the input image */
  virtual void SetInput( const InputImageType * inputImage );


protected:
  ImageToHexahedronMeshSource();
  ~ImageToHexahedronMeshSource();
  void PrintSelf(std::ostream& os, Indent indent) const;

  void GenerateData();
  virtual void GenerateOutputInformation(){}; // do nothing
  
private:
  ImageToHexahedronMeshSource(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  typedef typename InputImageType::SizeType InputImageSizeType;

  void CreateMesh();
  void InitializeLUT(); // initialize the look up table before the mesh construction

  unsigned char m_LUT[256][2]; // the two lookup tables

  unsigned long m_NumberOfNodes;
  unsigned long m_NumberOfCells;
  
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkImageToHexahedronMeshSource.txx"
#endif

#endif
