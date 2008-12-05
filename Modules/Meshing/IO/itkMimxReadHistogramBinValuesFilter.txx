/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: itkMimxReadHistogramBinValuesFilter.txx,v $
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

#ifndef __itkReadHistogramBinValuesFilter_txx
#define __itkReadHistogramBinValuesFilter_txx

#include "itkReadHistogramBinValuesFilter.h"

namespace itk
{

template <class TTraitType>
ReadHistogramBinValuesFilter<TTraitType>
::ReadHistogramBinValuesFilter( )
{
m_BinFileName = "";
m_NumberOfBins = 0;
}

template <class TTraitType>
ReadHistogramBinValuesFilter<TTraitType>
::~ReadHistogramBinValuesFilter( )
{
}

/** This method returns the number of bins
 *  specified by the user in the input file
 */

template <class TTraitType>
int
ReadHistogramBinValuesFilter<TTraitType>
::GetNumberOfBins( ) 
{
return m_NumberOfBins;
}

/** This method returns the array containing
 *  the bin values as specified by the user
 *  via an input file
 */

template <class TTraitType>
typename ReadHistogramBinValuesFilter<TTraitType>::ArrayType
ReadHistogramBinValuesFilter<TTraitType>
::GetHistogramBins( ) 
{
return m_binValue;
}

/** This method reads the ASCII file containing the
 *  bin values as specified by the user. These bin
 *  values are then set up into an array.
 */

template <class TTraitType>
void
ReadHistogramBinValuesFilter<TTraitType>
::Update( ) 
{
  if ( !(m_BinFileName.empty() && m_BinFileName == "" ))
    {
    #ifdef __sgi
      // Create the file. This is required on some older sgi's
      std::ifstream tFile(m_BinFileName.c_str( ),std::ios::in);
      tFile.close( ); 
    #endif

    std::ifstream inFile; 
    inFile.open( m_BinFileName.c_str( ), std::ios::in );
    double dump = 0.0;

    // calculate the number of bins from the number 
    // of modulus values present in the file
  
    if ( ! inFile )
      {
      ExceptionObject e(__FILE__, __LINE__);
      OStringStream msg;
      msg << static_cast<const char *>( this->GetNameOfClass( ) );
      msg << "::Update( )";
      e.SetLocation( msg.str().c_str() );
      e.SetDescription( "Error opening the specified file. Please check the given filename." );
      throw e; 
      }
    while (inFile >> dump)
      {
      m_NumberOfBins++;
      }

    // Set the size of Bin Values Array
    m_binValue.SetSize( m_NumberOfBins );               

    // Clear & set the pointer to the start of the file
    inFile.clear( );
    inFile.seekg( 0 );

    int count = 0;
    double bin = 0.0;
    
    // Iterate through the file & populate the bins array
    // with the values specified in the file

    while ( count < m_NumberOfBins )
      {
      inFile >> bin;
      m_binValue.SetElement( count, bin );
      count++;
      }

    // Checking whether there are repeated bin values
    // in the file. If not, throw exception 

    for ( int i = 0; i < m_NumberOfBins; i++ )
      {
      for ( int j = (i + 1); j < m_NumberOfBins; j++ )
        {
        if ( m_binValue.GetElement ( i ) == m_binValue.GetElement ( j ) )
          {
          // build an exception
          ExceptionObject e(__FILE__, __LINE__);
          OStringStream msg;
          msg << static_cast<const char *>( this->GetNameOfClass( ) );
          msg << "::Update( )";
          e.SetLocation( msg.str().c_str() );
          e.SetDescription( "No Two Bin Values can be the same." );
          throw e;
          }
        }
      }
    }
}

} /** end namespace itk. */

#endif
