/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxRebinHexahedronMeshTraitDataInPlaceFilter.h,v $
Language:  C++
Date:      $Date: 2007/05/10 16:32:38 $
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

#ifndef __itkRebinHexahedronMeshTraitDataInPlaceFilter_h
#define __itkRebinHexahedronMeshTraitDataInPlaceFilter_h

#include "vnl/vnl_matrix_fixed.h"
#include "itkMesh.h"
#include "itkMeshToMeshFilter.h"
#include "itkVector.h"
#include "itkCellInterface.h"
#include "itkTriangleCell.h"
#include "itkDefaultStaticMeshTraits.h"
#include "itkAutomaticTopologyMeshSource.h"
#include "itkPointSet.h"
#include "itkArray.h"
#include "itkExceptionObject.h"
#include <string>
#include <sstream> 
#include <exception> 

namespace itk
{

/**
 * \class RebinHexahedronMeshTraitDataInPlaceFilter
 * This class rebins the modulus values and assigns every voxel a
 * modulus value from the bin. The number of bins is specified by the
 * user. The user also has an option to specify the upper and lower
 * bounds of the bins.
 */

template <class TInputMesh, class TOutputMesh>
class ITK_EXPORT RebinHexahedronMeshTraitDataInPlaceFilter : 
                            public MeshToMeshFilter< TInputMesh, TOutputMesh >
{
public:
  /** Standard "Self" typedef. */
  typedef RebinHexahedronMeshTraitDataInPlaceFilter         Self;
  typedef MeshToMeshFilter< TInputMesh, TOutputMesh >  Superclass;

  typedef SmartPointer<Self>  Pointer;
  typedef SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(RebinHexahedronMeshTraitDataInPlaceFilter, MeshToMeshFilter);

  /** Input Mesh typedefs */ 
  typedef TInputMesh InputMeshType;
  typedef typename InputMeshType::Pointer       InputMeshTypePointer;
  typedef typename InputMeshType::CellType      CellType;
  typedef typename InputMeshType::PointType     PointType;
  typedef typename InputMeshType::CellPixelType CellPixelType;  
  typedef typename InputMeshType::CellsContainer::ConstIterator  CellIterator;

  typedef typename InputMeshType::Pointer         MeshPointer;
  typedef typename InputMeshType::CellTraits      CellTraits;
  typedef typename InputMeshType::PointsContainerPointer 
                                                  PointsContainerPointer;
  typedef typename InputMeshType::PointsContainer PointsContainer;
  typedef typename InputMeshType::CellsContainerPointer 
                                                  CellsContainerPointer;
  typedef typename InputMeshType::CellType::PointIdIterator  PointIterator;
  typedef typename itk::VertexCell< CellType >               VertexType;
  typedef typename itk::HexahedronCell< CellType >           HexahedronType;
  typedef typename HexahedronType::PointIdIterator           PointIdIterator;

  typedef double                 TraitType;
  typedef itk::Array<TraitType>  ArrayType;

  /* SetInput and GetOutput Macros */ 
  itkSetObjectMacro ( Input, InputMeshType );
  itkGetObjectMacro ( Input, InputMeshType );

  itkSetMacro( NumberOfHistogramBins, int );
  itkGetMacro( NumberOfHistogramBins, int );

  itkSetMacro( BinLowerBound, TraitType );
  itkGetMacro( BinLowerBound, TraitType );

  itkSetMacro( BinUpperBound, TraitType );
  itkGetMacro( BinUpperBound, TraitType );

  itkSetMacro( ComputeMaxBin, bool );
  itkGetMacro( ComputeMaxBin, bool );

  itkSetMacro( ComputeMinBin, bool );
  itkGetMacro( ComputeMinBin, bool );

  void Update( );
  void SetHistogramBins ( ArrayType );
  ArrayType GetHistogramBins( );

protected:

  RebinHexahedronMeshTraitDataInPlaceFilter( );
  virtual ~RebinHexahedronMeshTraitDataInPlaceFilter( );
  virtual void GenerateOutputInformation( ){ }; // do nothing
  void ComputeHistogramBins( );
  void SetModulusValues( );

private:
  RebinHexahedronMeshTraitDataInPlaceFilter(const Self&); //purposely not
                                                          //implemented    
  void operator = ( const Self& ); //purposely not implemented
  
  InputMeshTypePointer   m_Input;
  bool                   m_ComputeMaxBin;
  bool                   m_ComputeMinBin;
  int                            m_NumberOfHistogramBins;
  TraitType              m_BinLowerBound;;
  TraitType              m_BinUpperBound;
  ArrayType              m_HistogramBins;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkRebinHexahedronMeshTraitDataInPlaceFilter.txx"
#endif

#endif
