/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxRebinHexahedronMeshTraitDataInPlaceFilter.txx,v $
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

#ifndef __itRebinHexahedronMeshTraitDataInPlaceFilter_txx
#define __itRebinHexahedronMeshTraitDataInPlaceFilter_txx

#include "itkRebinHexahedronMeshTraitDataInPlaceFilter.h"

namespace itk
{

template <class TInputMesh, class TOutputMesh>
RebinHexahedronMeshTraitDataInPlaceFilter<TInputMesh, TOutputMesh>
::RebinHexahedronMeshTraitDataInPlaceFilter( )
{
  m_ComputeMaxBin = true;
  m_ComputeMinBin = true;
  m_NumberOfHistogramBins = 0;
  m_BinLowerBound = 0.0;
  m_BinUpperBound = 0.0;
  m_HistogramBins.SetSize( 0 );
}

template <class TInputMesh, class TOutputMesh>
RebinHexahedronMeshTraitDataInPlaceFilter<TInputMesh, TOutputMesh>
::~RebinHexahedronMeshTraitDataInPlaceFilter( )
{
}

template <class TInputMesh, class TOutputMesh>
void 
RebinHexahedronMeshTraitDataInPlaceFilter<TInputMesh, TOutputMesh>
::SetHistogramBins( ArrayType histogramBins )
{
  m_HistogramBins = histogramBins;
}

template <class TInputMesh, class TOutputMesh>
typename RebinHexahedronMeshTraitDataInPlaceFilter<TInputMesh, 
                                                    TOutputMesh>::ArrayType
RebinHexahedronMeshTraitDataInPlaceFilter<TInputMesh, TOutputMesh>
::GetHistogramBins( )
{
  return m_HistogramBins;
}

/** This method computes the bin ranges and bin values for the material
 *  property and assigns a material property value to each voxel
 *  depending upon its material property value computed from its Houndsfield
 *  number.
 */

template <class TInputMesh, class TOutputMesh>
void
RebinHexahedronMeshTraitDataInPlaceFilter<TInputMesh, TOutputMesh>
::Update( ) 
{
  
  if ( m_HistogramBins.size( ) != 0 )
    {
    m_BinUpperBound = m_HistogramBins.GetElement( m_NumberOfHistogramBins-1 );
    m_BinLowerBound = m_HistogramBins.GetElement( 0 );
    }
  else
    {
    ComputeHistogramBins( );
    }

  SetModulusValues( );
}

/** This method computes the Bins of modulus values. These bins can
 *  be computed either from the user specified upper & lower bounds
 *  or from the modulus values computed from the CT values.
 */

template <class TInputMesh, class TOutputMesh>
void
RebinHexahedronMeshTraitDataInPlaceFilter<TInputMesh, TOutputMesh>
::ComputeHistogramBins( ) 
{
  CellIterator cellIterator;
  CellIterator cellEnd;
  CellPixelType matProp;
  CellPixelType maxMatProp = -1E30;
  CellPixelType minMatProp = 1E30;
  
  cellIterator = m_Input->GetCells( )->Begin( );
  cellEnd      = m_Input->GetCells( )->End( );

  while ( cellIterator != cellEnd ) 
    {
    CellType * cell = cellIterator.Value( );
    switch( cell->GetType( ) )
      {
      case CellType::HEXAHEDRON_CELL:
        HexahedronType * hexahedron = dynamic_cast<HexahedronType *>( cell );

        m_Input->GetCellData( cellIterator.Index( ), &matProp );

        // If the user doesnt specify the upper bound of the
        //  modulus value, then set it based on the CT value
        if ( m_ComputeMaxBin ) 
          {
          if ( matProp > maxMatProp )
            {
            maxMatProp = matProp;
            }
          }
        
        // If the user doesnt specify the lower bound of the 
        // modulus value, then set it based on the CT value
        if ( m_ComputeMinBin )
        {
          if ( matProp < minMatProp )
            {
            minMatProp = matProp;
            }
        }
        break;
      }
    ++cellIterator;
    }

  if ( m_ComputeMinBin )
    {
    m_BinLowerBound = minMatProp;
    }

  if ( m_ComputeMaxBin )
    {
    m_BinUpperBound = maxMatProp;
    }

  // Checking whether maximum value entered by the user is less than
  // minimum value. If m_BinUpperBound < m_BinLowerBound, throw exception

  if ( m_BinUpperBound < m_BinLowerBound )
    {
    // build an exception
    ExceptionObject e(__FILE__, __LINE__);
    OStringStream msg;
    msg << static_cast<const char *>(this->GetNameOfClass());
    msg << "::ComputeHistogramBins( )";
    e.SetLocation( msg.str().c_str() );
    e.SetDescription
    ("The maximum value specified is less than the minimum value.");
    throw e;
    }

  // If Number of Bins < 2, throw exception

  if ( m_NumberOfHistogramBins < 2 )
    {
    // build an exception
    ExceptionObject e(__FILE__, __LINE__);
    OStringStream msg;
    msg << static_cast<const char *>(this->GetNameOfClass());
    msg << "::ComputeHistogramBins()";
    e.SetLocation( msg.str().c_str() );
    e.SetDescription
    ("The number of bins must be greater than 1.");
    throw e;
    }

  //Calculating and setting the bin ranges and bin values
  m_HistogramBins.SetSize( m_NumberOfHistogramBins );

  float binSize = ( m_BinUpperBound - m_BinLowerBound ) /
                   static_cast<float>( m_NumberOfHistogramBins - 1);

  for ( int i = 0; i < m_NumberOfHistogramBins; i++ )
    {
    m_HistogramBins.SetElement( i, m_BinLowerBound + 
                           static_cast<float> ( i )* binSize );
    }
}            



/** This method sets the modulus values  
 *  to each voxel from one of the bins
 */

template <class TInputMesh, class TOutputMesh>
void
RebinHexahedronMeshTraitDataInPlaceFilter<TInputMesh, TOutputMesh>
::SetModulusValues( ) 
{
  float binSize = ( m_BinUpperBound - m_BinLowerBound ) /
                    static_cast<float>( m_NumberOfHistogramBins - 1);

  CellPixelType matProp;
  CellPixelType maxBinValue = static_cast<CellPixelType>( 
                            m_HistogramBins.GetElement(m_NumberOfHistogramBins-1));
  CellPixelType minBinValue = static_cast<CellPixelType>(
                            m_HistogramBins.GetElement(0));
  CellIterator cellIterator;
  CellIterator cellEnd;
  cellIterator = m_Input->GetCells( )->Begin( );
  cellEnd      = m_Input->GetCells( )->End( );
  while( cellIterator != cellEnd ) 
    {
    CellType * cell = cellIterator.Value( );
    switch( cell->GetType( ) )
      {
      case CellType::HEXAHEDRON_CELL:
        HexahedronType * hexahedron = dynamic_cast<HexahedronType *>( cell );
          {
          m_Input->GetCellData( cellIterator.Index( ), &matProp );

          // If the modulus value of the voxel is less than the 
          // minimum value specified by the user, assign the voxel
          // the minimum modulus value of the bins
          if ( matProp < minBinValue )
            {
            m_Input->SetCellData( cellIterator.Index(), minBinValue );
            break;
            }

          // If the modulus value of the voxel is greater than the 
          // maximum value specified by the user, assign the voxel
          // the maximum modulus value of the bins
          if ( matProp > maxBinValue )
            {
            m_Input->SetCellData( cellIterator.Index(), maxBinValue );
            break;
            }

          // Assign every voxel a modulus value from one of the bins

          for ( int k = 0; k < m_NumberOfHistogramBins; k++ )
            {
            if ( matProp == m_HistogramBins.GetElement(k) )
              {
              m_Input->SetCellData( cellIterator.Index( ), 
                    static_cast<CellPixelType> ( m_HistogramBins.GetElement(k) ));
              break;
              }
            else if ( fabs( matProp - m_HistogramBins.GetElement(k) ) < binSize )
              {
              m_Input->SetCellData( cellIterator.Index( ), 
                    static_cast<CellPixelType> ( m_HistogramBins.GetElement(k) ));
              break;
              }
            }
          break; 
          }
      }                   
    ++cellIterator;
    }
}
} /** end namespace itk. */

#endif
