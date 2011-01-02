/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkEncodedTransformFileReader.cxx,v $
  Language:  C++
  Date:      $Date: 2007-08-09 15:15:39 $
  Version:   $Revision: 1.24 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkEncodedTransformFileReader_cxx
#define __itkEncodedTransformFileReader_cxx

#include "itkEncodedTransformFileReader.h"
#include "itkTransformBase.h"
#include "itkTransformFactory.h"
#include "itkDisplacementFieldTransform.h"

#include <itksys/ios/sstream>
#include <itksys/Base64.h>

namespace itk
{

namespace {
std::string trim(std::string const& source, char const* delims = " \t\r\n") {
  std::string result(source);
  std::string::size_type index = result.find_last_not_of(delims);
  if(index != std::string::npos)
    {
    result.erase(++index);
    }

  index = result.find_first_not_of(delims);
  if(index != std::string::npos)
    {
    result.erase(0, index);
    }
  else
    {
    result.erase();
    }
  return result;
}


bool ReadEncodedDoubleVector( std::istream& parse,
                              vnl_vector<double>& VectorBuffer )
{
  unsigned long uncompressedSize;
  unsigned long compressedSize;
  std::string encodedString;
  parse >> uncompressedSize >> compressedSize;
  parse >> encodedString;

  // The decoded buffer should contain a sequence of doubles, and
  // hence the length of the sequence must be consistent with that.
  if( ( uncompressedSize % sizeof(double) ) != 0 )
    {
    return false;
    }

  // Step 1: remove Base64 encoding
  std::vector<unsigned char> compressedBuf( compressedSize );
  unsigned long actualLength =
    itksysBase64_Decode( reinterpret_cast<const unsigned char*>(encodedString.c_str()),
                         compressedSize,
                         &compressedBuf[0],
                         0 );
  if( actualLength != compressedSize )
    {
    return false;
    }

  // Step 2: uncompress the bit-stream
  VectorBuffer.set_size( uncompressedSize/sizeof(double) );
  unsigned long destLen = uncompressedSize;
  if( uncompress( reinterpret_cast<unsigned char*>(VectorBuffer.data_block()),
                  &destLen,
                  &compressedBuf[0],
                  compressedSize ) != Z_OK )
    {
    return false;
    }

  return true;
}
}

/** Constructor */
EncodedTransformFileReader
::EncodedTransformFileReader()
{
  TransformFactory< DisplacementFieldTransform<double,3> >::RegisterTransform();
}

/** Destructor */
EncodedTransformFileReader
::~EncodedTransformFileReader()
{
}

/** Update the Reader */
void EncodedTransformFileReader
::Update()
{  
  TransformPointer transform;
  std::ifstream in;
  in.open ( m_FileName.c_str(), std::ios::in | std::ios::binary );
  if( in.fail() )
    {
    in.close();
    itkExceptionMacro ( "The file could not be opened for read access "
                        << std::endl << "Filename: \"" << m_FileName << "\"" );
    }

  std::ostringstream InData;

  // in.get ( InData );
  std::filebuf *pbuf;
  pbuf=in.rdbuf();

  // get file size using buffer's members
  int size=pbuf->pubseekoff (0,std::ios::end,std::ios::in);
  pbuf->pubseekpos (0,std::ios::in);

  // allocate memory to contain file data
  char* buffer=new char[size+1];

  // get file data  
  pbuf->sgetn (buffer,size); 
  buffer[size]='\0';
  itkDebugMacro ( "Read file transform Data" );
  InData << buffer;

  delete[] buffer;
  std::string data = InData.str();
  in.close();

  // Read line by line
  vnl_vector<double> VectorBuffer;
  std::string::size_type position = 0;
  
  Array<double> TmpParameterArray;
  Array<double> TmpFixedParameterArray;
  TmpParameterArray.clear();
  TmpFixedParameterArray.clear();
  bool haveFixedParameters = false;
  bool haveParameters = false;
  bool parametersAreEncoded = false;
 
  TransformListType* transformList = Superclass::GetTransformList();
  while ( position < data.size() )
    {
    // Find the next string
    std::string::size_type end = data.find ( "\n", position );
    std::string line = trim ( data.substr ( position, end - position ) );
    position = end+1;
    itkDebugMacro ("Found line: \"" << line << "\"" );

    if ( line.length() == 0 )
      {
      continue;
      }
    if ( line[0] == '#' || std::string::npos == line.find_first_not_of ( " \t" ) )
      {
      // Skip lines beginning with #, or blank lines
      continue;
      }

    // Get the name
    end = line.find ( ":" );
    if ( end == std::string::npos )
      {
      // Throw an error
      itkExceptionMacro ( "Tags must be delimited by :" );
      }
    std::string Name = trim ( line.substr ( 0, end ) );
    std::string Value = trim ( line.substr ( end + 1, line.length() ) );
    // Push back 
    itkDebugMacro ( "Name: \"" << Name << "\"" );
    itkDebugMacro ( "Value: \"" << Value << "\"" );
    itksys_ios::istringstream parse ( Value );
    VectorBuffer.clear();
    if ( Name == "Transform" )
      {
      // Instantiate the transform
      itkDebugMacro ( "About to call ObjectFactory" );
      LightObject::Pointer i;
      i = ObjectFactoryBase::CreateInstance ( Value.c_str() );
      itkDebugMacro ( "After call ObjectFactory");
      TransformType* ptr = dynamic_cast<TransformBase*> ( i.GetPointer() );
      if ( ptr == NULL )
        {
        std::ostringstream msg;
        msg << "Could not create an instance of " << Value << std::endl
            << "The usual cause of this error is not registering the "
            << "transform with TransformFactory" << std::endl;
        msg << "Currently registered Transforms: " << std::endl;
        std::list<std::string> names = TransformFactoryBase::GetFactory()->GetClassOverrideWithNames();
        std::list<std::string>::iterator it;
        for ( it = names.begin(); it != names.end(); it++ )
          {
          msg << "\t\"" << *it << "\"" << std::endl;
          }
        itkExceptionMacro ( << msg.str() );
        return;
        }
      transform = ptr;
      //m_TransformList.push_back ( transform );
      transformList->push_back( transform );
      }
    else if ( Name == "Parameters" || Name == "FixedParameters" ||
              Name == "ParametersAreEncoded" )
      {
      VectorBuffer.clear();

      if ( Name == "ParametersAreEncoded" )
        {
        if ( haveParameters )
          {
          itkExceptionMacro ( "ParametersAreEncoded must be specified before the Parameters" );
          }
        parse >> parametersAreEncoded;
        }
      else if ( Name == "Parameters" )
        {
        // Read them
        if( parametersAreEncoded )
          {
          if ( ! ReadEncodedDoubleVector( parse, VectorBuffer ) )
            {
            itkExceptionMacro( "Failed to parse Parameter vector" );
            }
          }
        else
          {
          parse >> VectorBuffer;
          }
        itkDebugMacro ( "Parsed: " << VectorBuffer );
        TmpParameterArray = VectorBuffer;
        itkDebugMacro ( "Setting Parameters: " << TmpParameterArray );
        if ( haveFixedParameters )
          {
          transform->SetFixedParameters ( TmpFixedParameterArray );
          itkDebugMacro ( "Set Transform Fixed Parameters" );
          transform->SetParametersByValue ( TmpParameterArray );
          itkDebugMacro ( "Set Transform Parameters" );
          TmpParameterArray.clear();
          TmpFixedParameterArray.clear(); 
          haveFixedParameters = false;
          haveParameters = false;
          parametersAreEncoded = false;
          }
        else
          {
          haveParameters = true;
          }   
        }
      else if ( Name == "FixedParameters" )
        {
        // Read them
        parse >> VectorBuffer;
        TmpFixedParameterArray = VectorBuffer;
        itkDebugMacro ( "Setting Fixed Parameters: " << TmpFixedParameterArray );
        if ( !transform )
          {
          itkExceptionMacro ( "Please set the transform before parameters or fixed parameters" );
          }
        if ( haveParameters )
          {
          transform->SetFixedParameters ( TmpFixedParameterArray );
          itkDebugMacro ( "Set Transform Fixed Parameters" );
          transform->SetParametersByValue ( TmpParameterArray );
          itkDebugMacro ( "Set Transform Parameters" );
          TmpParameterArray.clear();
          TmpFixedParameterArray.clear(); 
          haveFixedParameters = false;
          haveParameters = false;
          parametersAreEncoded = false;
          }
        else
          {
          haveFixedParameters = true;
          }
        }
      }
    }
}

} // namespace itk

#endif
