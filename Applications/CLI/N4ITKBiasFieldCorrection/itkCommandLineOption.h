/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkCommandLineOption.h,v $
  Language:  C++
  Date:      $Date: 2009/01/22 22:48:30 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkCommandLineOption_h
#define __itkCommandLineOption_h

#include "itkDataObject.h"
#include "itkObjectFactory.h"

#include "itkMacro.h"
#include "itkNumericTraits.h"

#include <list>
#include <string>
#include <deque>

namespace itk
{
/** \class CommandLineOption
    \brief Simple data structure for holding command line options.
    An option can have multiple values with each value holding 0 or more
    parameters.  E.g. suppose we were creating an image registration program
    which has several transformation model options such as 'rigid', 'affine',
    and 'deformable'.  A instance of the command line option could have a
    long name of "transformation", short name 't', and description
    "Transformation model---rigid, affine, or deformable".  The values for
    this option would be "rigid", "affine", and "deformable".  Each value
    would then hold parameters that relate to that value.  For example, a
    possible subsection of the command line would be

    " --transformation rigid[parameter1,parameter2,etc.]
      -m mutualinformation[parameter1] --optimization gradientdescent"
*/

class ITK_EXPORT CommandLineOption
: public DataObject
{
public:
    typedef CommandLineOption                        Self;
    typedef DataObject                               Superclass;
    typedef SmartPointer<Self>                       Pointer;

    itkNewMacro( Self );

    itkTypeMacro( Option, DataObject );

    typedef std::string                              ValueType;
    typedef std::deque<ValueType>                    ValueStackType;
    typedef std::deque<ValueStackType>               ParameterStackType;

    ValueStackType GetValues()
      { return this->m_Values; }

    unsigned int GetNumberOfValues()
      { return this->m_Values.size(); }

    std::string GetValue( unsigned int i = 0 )
      {
      if( i < this->m_Values.size() )
        {
        return this->m_Values[i];
        }
      else
        {
        return std::string( "" );
        }
      }

    ValueStackType GetParameters( unsigned int i = 0 )
      {
      if( i < this->m_Parameters.size() )
        {
        return this->m_Parameters[i];
        }
      else
        {
        ValueStackType empty;
        return empty;
        }
      }

    std::string GetParameter( unsigned int i, unsigned int j )
      {
      if( i < this->m_Parameters.size() && j < this->m_Parameters[i].size() )
        {
        return this->m_Parameters[i][j];
        }
      else
        {
        return std::string( "" );
        }
      }

    std::string GetParameter( unsigned int j )
      {
      return this->GetParameter( 0, j );
      }

    unsigned int GetNumberOfParameters( unsigned int i = 0 )
      {
      if( i < this->m_Parameters.size() )
        {
        return this->m_Parameters[i].size();
        }
      else
        {
        return 0;
        }
      }

    itkSetMacro( ShortName, char );
    itkGetMacro( ShortName, char );

    itkSetMacro( LongName, std::string );
    itkGetMacro( LongName, std::string );

    itkSetMacro( Description, std::string );
    itkGetMacro( Description, std::string );

    void AddValue( std::string, char, char );
    void AddValue( std::string s )
      {
      this->AddValue( s, '[', ']' );
      }

    void SetValue( unsigned int, std::string );

protected:
    CommandLineOption();
    virtual ~CommandLineOption() {};

private:
    char                                                     m_ShortName;
    std::string                                              m_LongName;
    std::string                                              m_Description;
    ValueStackType                                           m_Values;
    ParameterStackType                                       m_Parameters;
};

} // end namespace itk

#endif
