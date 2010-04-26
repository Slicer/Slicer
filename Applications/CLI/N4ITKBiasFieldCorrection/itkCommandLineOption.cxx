/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkCommandLineOption.cxx,v $
  Language:  C++
  Date:      $Date: 2009/01/22 22:48:30 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkCommandLineOption.h"

namespace itk
{

CommandLineOption
::CommandLineOption() : m_ShortName( '\0' ),
                        m_LongName( "" ),
                        m_Description( "" )
{
  this->m_Values.clear();
}

void
CommandLineOption
::AddValue( std::string value, char leftDelimiter, char rightDelimiter )
{
  std::string::size_type leftDelimiterPos = value.find( leftDelimiter );
  std::string::size_type rightDelimiterPos = value.find( rightDelimiter );

  if( leftDelimiterPos == std::string::npos ||
    rightDelimiterPos == std::string::npos )
    {
  this->m_Values.push_front( value );

    ValueStackType parameters;
    this->m_Parameters.push_front( parameters );
    }
  else
    {
    ValueStackType parameters;

    this->m_Values.push_front( value.substr( 0, leftDelimiterPos ) );

    std::string::size_type leftPos = leftDelimiterPos;
    std::string::size_type rightPos = value.find( ',', leftPos+1 );
    while( rightPos != std::string::npos )
      {
      parameters.push_back( value.substr( leftPos+1, rightPos - leftPos - 1 ) );
      leftPos = rightPos;
      rightPos = value.find( ',', leftPos+1 );
      }
    rightPos = rightDelimiterPos;
    parameters.push_back( value.substr( leftPos+1, rightPos - leftPos - 1 ) );

    this->m_Parameters.push_front( parameters );
    }

  this->Modified();
}

void
CommandLineOption
::SetValue( unsigned int i, std::string value )
{
this->m_Values[i] = value;
this->Modified();
}

} // end namespace itk
