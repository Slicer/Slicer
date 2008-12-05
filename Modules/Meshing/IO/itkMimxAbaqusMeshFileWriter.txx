/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxAbaqusMeshFileWriter.txx,v $
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

#ifndef __itkAbaqusMeshFileWriter_txx
#define __itkAbaqusMeshFileWriter_txx

#include "itkAbaqusMeshFileWriter.h"

namespace itk
{

template <class TInputMesh>
AbaqusMeshFileWriter<TInputMesh>
::AbaqusMeshFileWriter( )
{
  m_FileName = "";
  m_Title = "";
  m_UseHistogramBins = false;
  m_MeshIndexOffset = 0;
}

template <class TInputMesh>
AbaqusMeshFileWriter<TInputMesh>
::~AbaqusMeshFileWriter( )
{
}

/** This method sets the Bins as a
 *  member variable of the class
 */

template <class TInputMesh>
void
AbaqusMeshFileWriter<TInputMesh>
::SetHistogramBins( HistogramArrayType histogramBins )
{
  m_binValue = histogramBins;
}

template <class TInputMesh>
void
AbaqusMeshFileWriter<TInputMesh>
::SetRebinningOff( )
{
  m_UseHistogramBins = false;
}

template <class TInputMesh>
void
AbaqusMeshFileWriter<TInputMesh>
::SetRebinningOn( )
{
  m_UseHistogramBins = true;
}

/** This method writes out the Node & Element
 *  Definitions alongwith the Material Property
 *  Definitions in the form of Abaqus .inp file.
 */

template <class TInputMesh>
void 
AbaqusMeshFileWriter<TInputMesh>
::Update( )
{  
  #ifdef __sgi
         // Create the file. This is required on some older sgi's
    std::ofstream tFile(m_FileName.c_str( ),std::ios::out);
    tFile.close( );           

  #endif
 
  std::ofstream out;  
  out.open( m_FileName.c_str( ), std::ios::out );

  out << m_AbaqusHeader << std::endl;

/* Writing out the Node Definitions */

  out << "**" << "\n";
  out << "** Node Definitions **" << "\n"; 
  out << "*NODE" << "\n"; 
  out << "**" << std::endl;

  CellIterator cellIterator = m_Input->GetCells( )->Begin( );
  CellIterator cellEnd      = m_Input->GetCells( )->End( );
  
  while( cellIterator != cellEnd ) 
    {
    CellType * cell = cellIterator.Value( );
    switch( cell->GetType( ) )
      {
      case CellType::VERTEX_CELL:
        {
        VertexType * vertex = dynamic_cast<VertexType *>( cell );
        PointType point;
        unsigned long pointIndex;
        NodeNumberArrayType nodeNumberArray;
        unsigned int numberOfPoints = vertex->GetNumberOfPoints( );

        nodeNumberArray.SetSize( numberOfPoints );
        nodeNumberArray.SetElement( 0, vertex->GetPointId( ) );
        
        NodeNumberArrayType pointIndexArray = GetNodeNumbers( nodeNumberArray );
        pointIndex = pointIndexArray.GetElement( 0 );
        m_Input->GetPoint( vertex->GetPointId( ), &point );

        out << pointIndex+1 << ", " << point[0] << ", " ;
        out << point[1] << ", " << point[2] << std::endl;
        break;
        }
      }
      ++cellIterator;
    }  
  
  CellPixelType matProp;

  /* Writing out the Element Definitions */

   out << "**" << "\n";
   out << "** Element Definitions **" << "\n";
   out << "**" << std::endl;
   if ( m_UseHistogramBins )
    {
    int j = 1 + m_MeshIndexOffset;
    for ( int i = 0; i < m_NumberOfHistogramBins; i++ )
      {
      out << "*ELEMENT, TYPE=C3D8, "<< "ELSET=Mat" << ( i + 1 +  m_MeshIndexOffset) << "\n";
      cellIterator = m_Input->GetCells( )->Begin( );
      while( cellIterator != cellEnd ) 
        {
        CellType * cell = cellIterator.Value( );
        switch( cell->GetType( ) )
          {
          case CellType::HEXAHEDRON_CELL:
            {
            HexahedronType * hexahedron = 
                                      dynamic_cast<HexahedronType *>( cell );
            m_Input->GetCellData( cellIterator.Index( ), &matProp );
            if ( m_binValue.GetElement( i ) == matProp )
              {
              out << j << ", " ;
              WriteElementDefinitions( hexahedron , out );
              out << std::endl;
              j++;
              break;
              }
            }
          }
        ++cellIterator;
        }
      }
    }
   else
     {   
     int j = 1 + m_MeshIndexOffset;
     cellIterator = m_Input->GetCells( )->Begin( );
     while( cellIterator != cellEnd ) 
       {
       CellType * cell = cellIterator.Value( );
       switch( cell->GetType( ) )
         {
         case CellType::HEXAHEDRON_CELL:
          {
          HexahedronType * hexahedron = 
                                    dynamic_cast<HexahedronType *>( cell );
          out << "*ELEMENT, TYPE=C3D8, "<< "ELSET=Mat" << j << "\n";
          out << j << ", " ;
          WriteElementDefinitions( hexahedron , out );
          out << std::endl;
          j++;
          break;
          }
         }
       ++cellIterator;
       }
   
     out << "**" << "\n**\n**\n";
     out << "** Material Properties Definitions **" << "\n**\n**\n";
     out << "**" << std::endl;

     int k = 1 + m_MeshIndexOffset;
     cellIterator = m_Input->GetCells( )->Begin( );
     while( cellIterator != cellEnd ) 
       {
       CellType * cell = cellIterator.Value( );
       switch( cell->GetType( ) )
         {
         case CellType::HEXAHEDRON_CELL:
          {
          HexahedronType * hexahedron = 
                                    dynamic_cast<HexahedronType *>( cell );
          m_Input->GetCellData( cellIterator.Index( ), &matProp );
          out << "*SOLID SECTION, " << "ELSET=Mat" << k;
          out << ", MATERIAL=Mat" << k << "\n";
          out << "*MATERIAL, NAME = " << "Mat" << k << "\n";
          out << "*ELASTIC" << "\n";
          out << matProp << ", " <<  m_PoissonRatio << std::endl;
          k++;
          break;
          }
         }
       ++cellIterator;
       }
     }

   /* Writing out Material Property Definitions */

  if ( m_UseHistogramBins )
    {
    out << "**" << "\n";
    out << "** Material Properties Definitions **" << "\n";
    out << "**" << std::endl;
    for ( int k = 0; k < m_NumberOfHistogramBins; k++ )
      {
      matProp = m_binValue.GetElement( k );
      if ( matProp != -1)
        {
        out << "*SOLID SECTION, "<< "ELSET=Mat" << (k + 1 + m_MeshIndexOffset);
        out << ", MATERIAL=Mat" << ( k + 1 + m_MeshIndexOffset) << "\n";
        out << "*MATERIAL, NAME = " << "Mat" << ( k + 1 + m_MeshIndexOffset) << "\n";
        out << "*ELASTIC" << "\n";
        out << matProp << "," << m_PoissonRatio << std::endl;
        }
      }
    }
   out.close( );
}

/** This method takes the vertex ID/s & returns the
 *  node numbers associated with the vertex/vertices
 */

template <class TInputMesh>
typename AbaqusMeshFileWriter<TInputMesh>::NodeNumberArrayType
AbaqusMeshFileWriter<TInputMesh>
::GetNodeNumbers( NodeNumberArrayType nodeNumberArray)
{
  unsigned long pointIndex;
  
  int size = nodeNumberArray.GetSize( );
  NodeNumberArrayType pointIndexArray;
  pointIndexArray.SetSize( size );

  for ( int count = 0; count < size; count ++)
    {
    m_Input->GetPointData( nodeNumberArray.GetElement( count ), &pointIndex);
    pointIndexArray.SetElement( count, pointIndex );
    }
  
  return pointIndexArray;;
}

template <class TInputMesh>
void
AbaqusMeshFileWriter<TInputMesh>
::WriteElementDefinitions( HexahedronType *hexahedron, std::ofstream &out)
{
  int pointCount = 0;
  NodeNumberArrayType nodeNumberArray;
  unsigned int numberOfPoints = hexahedron->GetNumberOfPoints( );
  nodeNumberArray.SetSize( numberOfPoints );
  
  PointIdIterator pointIterator = hexahedron->PointIdsBegin( );
  PointIdIterator endPoint = hexahedron->PointIdsEnd( );
  while ( pointIterator != endPoint ) 
    {
    nodeNumberArray.SetElement( pointCount, pointIterator[0] );
    ++pointIterator;
    pointCount++;
    }
  NodeNumberArrayType pointIndexArray = GetNodeNumbers( nodeNumberArray );
  for ( int count = 0; count < numberOfPoints; count ++ )
    {
    if ( count == 0)
      {
      out << ( pointIndexArray.GetElement( count ) + 1);
      }
    else
      {
      out << ", " << ( pointIndexArray.GetElement( count ) + 1);
      }
    }
}

} /** end namespace itk. */

#endif
