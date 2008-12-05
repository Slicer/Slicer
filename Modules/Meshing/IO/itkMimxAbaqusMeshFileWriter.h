/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxAbaqusMeshFileWriter.h,v $
Language:  C++
Date:      $Date: 2007/05/17 17:09:46 $
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

#ifndef __itkMimxAbaqusMeshFileWriter_h
#define __itkMimxAbaqusMeshFileWriter_h

#include "itkLightProcessObject.h"
#include "itkMesh.h"
#include "itkCellInterface.h"
#include "itkArray.h"
#include <string>
#include <sstream> 

namespace itk
{

/**
 * \class itkMimxAbaqusMeshFileWriter
 * This class writes out Mesh in Abaqus format ".inp" file
 * which contains the Nodes, Material and Element definitions
 */

template<class TInputMesh> 
class itkMimxAbaqusMeshFileWriter : public LightProcessObject  
{
public:

  /** SmartPointer typedef support */
  typedef itkMimxAbaqusMeshFileWriter Self;
  typedef SmartPointer<Self> Pointer;

  /** Useful Types for Mesh Support */
  typedef TInputMesh MeshType;
  typedef typename MeshType::Pointer                        MeshTypePointer;
  typedef typename MeshType::CellType                       CellType;
  typedef typename MeshType::PointType                      PointType;
  typedef typename MeshType::CellPixelType                  CellPixelType;  
  typedef typename MeshType::CellsContainer::ConstIterator  CellIterator;

  typedef typename itk::VertexCell< CellType >              VertexType;
  typedef typename itk::HexahedronCell< CellType >          HexahedronType;
  typedef typename HexahedronType::PointIdIterator          PointIdIterator;
  
  typedef double          HistogramDataType;
  typedef itk::Array<HistogramDataType>  HistogramArrayType;

  typedef double          NodeNumberType;
  typedef itk::Array<NodeNumberType>  NodeNumberArrayType;

  /** Method for creation through the object factory */
  itkNewMacro( Self );

  /** Run-time type information (and related methods). */
  typedef Object Superclass;
  itkTypeMacro( AbaqusMeshFileWriter, LightProcessObject );
  
  /* SetInput and GetOutput Macros */ 
  itkSetObjectMacro ( Input, MeshType );
  
  /** Set and Get the filename  */
  itkSetStringMacro( FileName );
  itkGetStringMacro( FileName );

  itkSetStringMacro( AbaqusHeader );
  itkSetStringMacro( Title );
  itkSetMacro( PoissonRatio, float );
  
  /** Set and Get the Number Of Histogram Bins  */
  itkSetMacro( NumberOfHistogramBins, int );
  itkGetMacro( NumberOfHistogramBins, int );

  /** Set the Mesh Index Offset */
  itkSetMacro( MeshIndexOffset, unsigned long );
  itkGetMacro( MeshIndexOffset, unsigned long );

  /** Set the Bin Values  */
  itkSetMacro( binValue, HistogramArrayType );

  void SetRebinningOn( );
  void SetRebinningOff( );
  void SetHistogramBins( HistogramArrayType );

  /** Write out the Abaqus inp file */
  void Update();
  
protected:

  NodeNumberArrayType GetNodeNumbers( NodeNumberArrayType );
  void WriteElementDefinitions( HexahedronType *, std::ofstream &);

  std::string       m_FileName;
  std::string       m_Title;
  std::string       m_AbaqusHeader;
  MeshTypePointer   m_Input;
  int               m_NumberOfHistogramBins;
  HistogramArrayType m_binValue;
  bool              m_UseHistogramBins;
  float             m_PoissonRatio;
  unsigned long     m_MeshIndexOffset;

  itkMimxAbaqusMeshFileWriter( );
  virtual ~itkMimxAbaqusMeshFileWriter( );

};

} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkAbaqusMeshFileWriter.txx"
#endif

#endif
