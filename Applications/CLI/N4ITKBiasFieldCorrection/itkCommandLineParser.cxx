/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkCommandLineParser.cxx,v $
  Language:  C++
  Date:      $Date: 2009/01/22 22:43:11 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkCommandLineParser.h"

namespace itk
{
CommandLineParser
::CommandLineParser()
{
  this->m_Options.clear();
  this->m_Command.clear();
  this->m_CommandDescription.clear();
  this->m_UnknownOptions.clear();

  this->m_LeftDelimiter = '[';
  this->m_RightDelimiter = ']';
}

void
CommandLineParser
::AddOption( OptionType::Pointer option )
{
  if( ( option->GetShortName() != '\0' ||
    !this->GetOption( option->GetShortName() ) )
    || ( !option->GetLongName().empty() ||
    !this->GetOption( option->GetLongName() ) ) )
    {
    this->m_Options.push_back( option );
    }
  else
    {
    if( option->GetShortName() != '\0' &&
      this->GetOption( option->GetShortName() ) )
      {
      itkWarningMacro( "Duplicate short option '-"
        << option->GetShortName() << "'" );
      }
    if( !( option->GetLongName().empty() ) &&
      this->GetOption( option->GetLongName() ) )
      {
        itkWarningMacro( "Duplicate long option '--"
          << option->GetLongName() << "'" );
      }
    }
}

void
CommandLineParser
::Parse( unsigned int argc, char **argv )
{
  std::vector<std::string> arguments =
    this->RegroupCommandLineArguments( argc, argv );

  unsigned int n = 0;

  this->m_Command = arguments[n++];

  while( n < arguments.size() )
    {
    std::string argument = arguments[n++];
    std::string name;

    name.clear();
    if( argument.find( "--" ) == 0 )
      {
      name = argument.substr( 2, argument.length()-1 );
      }
    else if( argument.find( "-" ) == 0 && argument.find( "--" ) > 0 )
      {
      name = argument.substr( 1, 2 );
      }

    if( !( name.empty() ) )
      {
      OptionType::Pointer option = this->GetOption( name );
      if( !option )
        {
        OptionType::Pointer unknownOption = OptionType::New();
        if( name.length() > 1 )
          {
          unknownOption->SetLongName( name );
          }
        else
          {
          unknownOption->SetShortName( name.at( 0 ) );
          }
        if( n == arguments.size() )
          {
          unknownOption->AddValue( "1",
            this->m_LeftDelimiter, this->m_RightDelimiter );
          }
        else
          {
          for( unsigned int m = n; m < arguments.size(); m++ )
            {
            std::string value = arguments[m];
            if( value.find( "-" ) != 0 )
              {
unknownOption->AddValue( value,
this->m_LeftDelimiter, this->m_RightDelimiter );
              }
            else
              {
              if( m == n )
                {
unknownOption->AddValue( "1",
this->m_LeftDelimiter, this->m_RightDelimiter );
                }
              n = m;
              break;
              }
            }
          }
        this->m_UnknownOptions.push_back( unknownOption );
        }
      else  // the option exists
        {
        if( n == arguments.size() )
          {
option->AddValue( "1",
this->m_LeftDelimiter, this->m_RightDelimiter );
          }
        else
          {
          for( unsigned int m = n; m < arguments.size(); m++ )
            {
            std::string value = arguments[m];
            if( value.find( "-" ) != 0 )
              {
option->AddValue( value,
this->m_LeftDelimiter, this->m_RightDelimiter );
              }
            else
              {
              if( m == n )
                {
option->AddValue( "1",
this->m_LeftDelimiter, this->m_RightDelimiter );
                }
              n = m;
              break;
              }
            }
          }
        }
      }
    }
}

std::vector<std::string>
CommandLineParser
::RegroupCommandLineArguments( unsigned int argc, char **argv )
{
  /**
   * Inclusion of this function allows the user to use spaces inside
   * the left and right delimiters.
   */

  std::vector<std::string> arguments;

  std::string currentArg( "" );
  bool isArgOpen = false;
  for( unsigned int n = 0; n < argc; n++ )
    {

    std::string a( argv[n] );

    if( isArgOpen )
      {
      std::size_t leftDelimiterPosition = a.find( this->m_LeftDelimiter );
      if( leftDelimiterPosition != std::string::npos )
        {
        itkExceptionMacro( "Incorrect command line specification." );
        }

      std::size_t rightDelimiterPosition = a.find( this->m_RightDelimiter );
      if( rightDelimiterPosition != std::string::npos )
        {
        if( rightDelimiterPosition < a.length()-1 )
{
itkExceptionMacro( "Incorrect command line specification." );
}
        else
{
          currentArg += a;
          arguments.push_back( currentArg );
          currentArg.clear();
          isArgOpen = false;
}
        }
      else
        {
        currentArg += a;
        }
      }
    else
      {
      std::size_t leftDelimiterPosition = a.find( this->m_LeftDelimiter );
      std::size_t rightDelimiterPosition = a.find( this->m_RightDelimiter );

      if( leftDelimiterPosition == std::string::npos )
        {
        if( rightDelimiterPosition == std::string::npos )
          {
currentArg += a;
arguments.push_back( currentArg );
currentArg.clear();
          }
        else
{
itkExceptionMacro( "Incorrect command line specification." );
}
        }
      else if( leftDelimiterPosition != std::string::npos &&
        rightDelimiterPosition != std::string::npos &&
        leftDelimiterPosition < rightDelimiterPosition )
        {
        if( rightDelimiterPosition < a.length()-1 )
{
itkExceptionMacro( "Incorrect command line specification." );
}
        currentArg += a;
arguments.push_back( currentArg );
currentArg.clear();
isArgOpen = false;
        }
      else if( rightDelimiterPosition == std::string::npos &&
        leftDelimiterPosition != std::string::npos )
        {
        currentArg += a;
        isArgOpen = true;
        }
      }
    }

  return arguments;
}

CommandLineParser::OptionType::Pointer
CommandLineParser
::GetOption( std::string name )
{
  if( name.length() == 1 )
    {
    return this->GetOption( name.at( 0 ) );
    }

  OptionListType::iterator it;
  for( it = this->m_Options.begin(); it != this->m_Options.end(); it++ )
    {
    if( name.compare( (*it)->GetLongName() ) == 0 )
      {
      return *it;
      }
    }
  return NULL;
}

CommandLineParser::OptionType::Pointer
CommandLineParser
::GetOption( char name )
{
  OptionListType::iterator it;
  for( it = this->m_Options.begin(); it != this->m_Options.end(); it++ )
    {
    if( name == (*it)->GetShortName() )
      {
      return *it;
      }
    }
  return NULL;
}

void
CommandLineParser
::PrintMenu( std::ostream& os, Indent indent ) const
{
  os << std::endl;
  os << "COMMAND: " << std::endl;
  os << indent << this->m_Command << std::endl;
  if( !this->m_CommandDescription.empty() )
    {
    os << indent << indent << this->m_CommandDescription << std::endl;
    }
  os << std::endl;
  os << "OPTIONS: " << std::endl;

  OptionListType::const_iterator it;
  for( it = this->m_Options.begin(); it != this->m_Options.end(); it++ )
    {
    os << indent;
    if( (*it)->GetShortName() != '\0' )
      {
      os << "-" << (*it)->GetShortName();
      if( !( (*it)->GetLongName() ).empty() )
        {
          os << ", " << "--" << (*it)->GetLongName() << ": " << std::endl;
        }
      else
        {
          os << ": " << std::endl;
        }
      }
    else
      {
      os << "--" << (*it)->GetLongName() << ": " << std::endl;
      }

    if( !( (*it)->GetDescription().empty() ) )
      {
      os << indent << indent << (*it)->GetDescription() << std::endl;
      }
    if( (*it)->GetValues().size() == 1 )
      {
      os << indent << indent << "<VALUES>: " << (*it)->GetValue( 0 );
      if( (*it)->GetParameters( 0 ).size() > 0 )
        {
        os << "[";
        if( (*it)->GetParameters( 0 ).size() == 1 )
          {
          os << (*it)->GetParameter( 0, 0 );
          }
        else
          {
          for( unsigned int i = 0;
            i < (*it)->GetParameters( 0 ).size()-1; i++ )
            {
            os << (*it)->GetParameter( 0, i ) << ",";
            }
          os << (*it)->GetParameter( 0, (*it)->GetParameters( 0 ).size()-1 );
          }
        os << "]";
        }
      os << std::endl;
      }
    else if( (*it)->GetValues().size() > 1 )
      {
      os << indent << indent << "<VALUES>: ";
      for( unsigned int n = 0; n < (*it)->GetValues().size()-1; n++ )
        {
        os << (*it)->GetValue( n );
        if( (*it)->GetParameters( n ).size() > 0 )
          {
          os << "[";
          if( (*it)->GetParameters( n ).size() == 1 )
            {
            os << (*it)->GetParameter( n, 0 ) << "], ";
            }
          else
            {
            for( unsigned int i = 0;
              i < (*it)->GetParameters( n ).size()-1; i++ )
              {
              os << (*it)->GetParameter( n, i ) << ",";
              }
            os << (*it)->GetParameter( n,
              (*it)->GetParameters( n ).size()-1 ) << "], ";
            }
          }
        else
          {
          os << ", ";
          }
        }

      unsigned int n = (*it)->GetValues().size()-1;

      os << (*it)->GetValue( n );
      if( (*it)->GetParameters( n ).size() > 0 )
        {
        os << "[";
        if( (*it)->GetParameters( n ).size() == 1 )
          {
          os << (*it)->GetParameter( n, 0 ) << "]";
          }
        else
          {
          for( unsigned int i = 0;
            i < (*it)->GetParameters( n ).size()-1; i++ )
            {
            os << (*it)->GetParameter( n, i ) << ",";
            }
          os << (*it)->GetParameter( n,
            (*it)->GetParameters( n ).size()-1 ) << "]";
          }
        }
      }
    os << std::endl;
    }
}

/**
 * Standard "PrintSelf" method
 */
void
CommandLineParser
::PrintSelf( std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Command: " << this->m_Command << std::endl;
  os << indent << "Options: " << std::endl;

  OptionListType::const_iterator it;
  for( it = this->m_Options.begin(); it != this->m_Options.end(); it++ )
    {
    (*it)->Print( os, indent );
    }

  if( this->m_UnknownOptions.size() )
    {
    os << indent << "Unknown Options: " << std::endl;
    OptionListType::const_iterator its;
    for( its = this->m_UnknownOptions.begin();
      its != this->m_UnknownOptions.end(); its++ )
      {
      (*its)->Print( os, indent );
      }
    }
}

} // end namespace itk
