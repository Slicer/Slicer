/*=========================================================================

  Program:   Diffusion Applications
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#ifndef itkDiffusionTensor3DRead_txx
#define itkDiffusionTensor3DRead_txx

#include "itkDiffusionTensor3DRead.h"

namespace itk
{

template <class TData>
DiffusionTensor3DRead<TData>
::DiffusionTensor3DRead()
{
  m_MeasurementFrame.SetIdentity();
  m_NumberOfThreads = 0;
  m_HasMeasurementFrame = false;
}

template <class TData>
typename DiffusionTensor3DRead<TData>::DiffusionImagePointer
DiffusionTensor3DRead<TData>
::GetOutput() const
{
  return m_Reader->GetOutput();
}

template <class TData>
typename DiffusionTensor3DRead<TData>::DictionaryType
DiffusionTensor3DRead<TData>
::GetMetaDataDictionary() const
{
  return m_Reader->GetMetaDataDictionary();
}

template <class TData>
int
DiffusionTensor3DRead<TData>
::Update( const char* input )
{
  try
    {
    m_Reader = FileReaderType::New();
    m_Reader->SetFileName( input );
    m_Reader->SetNumberOfThreads(m_NumberOfThreads);
    m_Reader->Update();
    const DictionaryType &        dictionary = m_Reader->GetMetaDataDictionary();
    DictionaryType::ConstIterator itr = dictionary.Begin();
    DictionaryType::ConstIterator end = dictionary.End();
    while( itr != end )
      {
      itk::MetaDataObjectBase::Pointer  entry = itr->second;
      MetaDataDoubleVectorType::Pointer entryvalue1
        = dynamic_cast<MetaDataDoubleVectorType *>( entry.GetPointer() );
      if( entryvalue1 )
        {
        int pos = itr->first.find( "NRRD_measurement frame" );
        if( pos != -1 )
          {
          DoubleVectorType tagvalue = entryvalue1->GetMetaDataObjectValue();
          for( int i = 0; i < 3; i++ )
            {
            for( int j = 0; j < 3; j++ )
              {
              m_MeasurementFrame[i][j] = tagvalue.at( j ).at( i );
              }
            }
          m_HasMeasurementFrame = true;
          }
        }
      // get the space orientation
      MetaDataStringType::Pointer entryvalue2
        = dynamic_cast<MetaDataStringType *>( entry.GetPointer() );
      if( entryvalue2 )
        {
        int pos = itr->first.find( "NRRD_space" );
        if( pos != -1 )
          {
          std::string tagvalue = entryvalue2->GetMetaDataObjectValue();
          }
        }
      ++itr;
      }

    return 0;
    }
  catch( itk::ExceptionObject & excep )
    {
    std::cerr << "DiffusionTensor3DRead::Load:  exception caught!" << std::endl;
    std::cerr << excep << std::endl;
    return -1;
    }
  return 0;
}

} // end namespace itk

#endif
