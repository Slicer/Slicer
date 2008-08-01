/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEncodedTransformFileWriter.cxx,v $
  Language:  C++
  Date:      $Date: 2007-08-09 15:15:39 $
  Version:   $Revision: 1.13 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkEncodedTransformFileWriter_cxx
#define __itkEncodedTransformFileWriter_cxx

#include "itkEncodedTransformFileWriter.h"

#include <itksys/Base64.h>

namespace itk
{

bool WriteEncodedDoubleVector( std::ostream& out,
                               vnl_vector<double>& VectorBuffer )
{
  unsigned char* srcBuffer = reinterpret_cast< unsigned char* >( &VectorBuffer[0] );
  unsigned long srcLength = VectorBuffer.size() * sizeof(double);

  out << srcLength << " ";

  unsigned long compressedLength = compressBound( srcLength );
  std::vector<unsigned char> compressedBuf( compressedLength );
  if( compress( &compressedBuf[0], &compressedLength,
                srcBuffer, srcLength ) != Z_OK )
    {
    return false;
    }

  out << compressedLength << " ";

  // src*1.5 estimate for the output buffer is from the documentation
  // of Base64_Encode.
  std::vector<unsigned char> base64Buf( compressedLength*3/2+4 ); // * 1.5
  unsigned long base64Len =
    itksysBase64_Encode( &compressedBuf[0], compressedLength,
                         &base64Buf[0], 1 );

  std::copy( base64Buf.begin(), base64Buf.begin()+base64Len,
             std::ostream_iterator<unsigned char>( out ) );

  return true;
}



EncodedTransformFileWriter
::EncodedTransformFileWriter()
{
}

EncodedTransformFileWriter
::~EncodedTransformFileWriter()
{
}

/** Set the input transform and reinitialize the list of transforms */
void EncodedTransformFileWriter::SetInput(const TransformType* transform)
{
  m_TransformList.clear();
  m_TransformList.push_back(transform);
}

/** Add a transform to be written */
void EncodedTransformFileWriter::AddTransform(const TransformType* transform)
{
  m_TransformList.push_back(transform);
}

/** Update the writer */
void EncodedTransformFileWriter
::Update()
{  
  std::list<const TransformType *>::iterator it = m_TransformList.begin();
  vnl_vector<double> TempArray;
#ifdef __sgi
  // Create the file. This is required on some older sgi's
  if (GetAppendMode())
    {
    std::ofstream tFile(m_FileName.c_str(),std::ios::out | std::ios::app);
    tFile.close();   
    }
  else
    {
    std::ofstream tFile(m_FileName.c_str(),std::ios::out);
    tFile.close(); 
    }
#endif
  std::ofstream out;
  if (GetAppendMode())
    {
    out.open(m_FileName.c_str(), std::ios::out | std::ios::app); 
    }
  else
    {
    out.open(m_FileName.c_str(), std::ios::out);
    }
  out << "#Insight Transform File V1.0" << std::endl;
  int count = 0;
  while(it != m_TransformList.end())
    {
    out << "# Transform " << count << std::endl;
    out << "Transform: " << (*it)->GetTransformTypeAsString() << std::endl;

    TempArray = (*it)->GetParameters();
    if( (*it)->GetEncodeParametersOnWrite() )
      {
      out << "ParametersAreEncoded: 1\n";
      out << "Parameters: ";
      if ( !WriteEncodedDoubleVector( out, TempArray ) )
        {
        itkExceptionMacro( "Could not encode parameters for writing" );
        }
      out << std::endl;
      }
    else
      {
      out << "Parameters: " << TempArray << std::endl;
      }
    TempArray = (*it)->GetFixedParameters();
    out << "FixedParameters: " << TempArray << std::endl;
    it++;
    count++;
    }
  out.close();
}


} // namespace itk

#endif
