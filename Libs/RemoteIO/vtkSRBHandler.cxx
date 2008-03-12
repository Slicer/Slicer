#include "vtkSRBHandler.h"
#include "itksys/Process.h"
#include "itksys/SystemTools.hxx"

vtkStandardNewMacro ( vtkSRBHandler );
vtkCxxRevisionMacro ( vtkSRBHandler, "$Revision: 1.0 $" );

/*------------------------------------------------------------------------------
vtkSRBHandler* vtkSRBHandler::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSRBHandler");
  if(ret)
    {
    return (vtkSRBHandler*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSRBHandler;
}
*/


//----------------------------------------------------------------------------
vtkSRBHandler::vtkSRBHandler()
{
}


//----------------------------------------------------------------------------
vtkSRBHandler::~vtkSRBHandler()
{
}


//----------------------------------------------------------------------------
void vtkSRBHandler::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}



//----------------------------------------------------------------------------
int vtkSRBHandler::CanHandleURI ( const char *uri )
{
  //--- What's the best way to determine whether this handler
  //--- speaks the correct protocol?
  //--- first guess is to look at the prefix up till the colon.

  int index;
  std::string uriString (uri);
  std::string prefix;

  //--- get all characters up to (and not including) the '://'
  if ( ( index = uriString.find ( "://", 0 ) ) != std::string::npos )
    {
    prefix = uriString.substr ( 0, index );
    //--- check to see if any bracketed characters are in
    //--- this part of the string.
    if ( (index = prefix.find ( "]:", 0 ) ) != std::string::npos )
      {
      //--- if so, strip off the leading bracketed characters in case
      //--- we adopt the gwe "[filename.ext]:" prefix.
      prefix = prefix.substr ( index+2 );
      }
    if ( prefix == "srb" )
      {
      vtkDebugMacro("vtkSRBHandler: CanHandleURI: can handle this file: " << uriString.c_str());
      return (1);
      }
    }
  else
    {
    vtkDebugMacro ( "vtkSRBHandler::CanHandleURI: unrecognized uri format: " << uriString.c_str() );
    }
  return ( 0 );
}



//----------------------------------------------------------------------------
void vtkSRBHandler::InitTransfer( )
{
  std::string cmd = std::string( "Sinit");

  int retval = system(cmd.c_str());
  if (retval != 0)
    {
    vtkErrorMacro("InitTransfer: error when running command '" << cmd.c_str() << "', returned = " << retval << "\n\tDid you set up your SCommands? See http://xwiki.nbirn.net:8080/xwiki/bin/view/BIRN%2Dsoftware/SRB+3_4_2+Clients");
    
    }
}

//----------------------------------------------------------------------------
int vtkSRBHandler::CloseTransfer( )
{
  std::string cmd = std::string("Sexit");
  
  int retval = system(cmd.c_str());
  if (retval != 0)
    {
    vtkErrorMacro("CloseTransfer: error when running command '" << cmd.c_str() << "', returned = " << retval);
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}


//----------------------------------------------------------------------------
void vtkSRBHandler::StageFileRead(const char * source, const char * destination)
{
   if (source == NULL)
    {
    vtkErrorMacro("StageFileWrite: source file name is null");
    return;
    }
  if (destination == NULL)
    {
    vtkErrorMacro("StageFileWrite: destination file name is null");
    return;
    }
  
  this->InitTransfer( );

  std::string cmd = std::string("Sget ");

  // strip off the srb:// at the front of source
  int index;
  std::string sourceString (source);
  std::string prefix;
  if ( ( index = sourceString.find ( "srb://", 0 ) ) != std::string::npos )
    {
    sourceString = sourceString.substr ( 6 );
    }
  vtkDebugMacro("StageFileRead: using source value " << sourceString.c_str());
  cmd += sourceString;
    
  // add the destination
  cmd += std::string(" ") + std::string(destination);
  
  // execute the command
  vtkDebugMacro("StageFileRead: calling command: " << cmd.c_str());
  int retval = system(cmd.c_str());
  if (retval != 0)
    {
    vtkErrorMacro("StageFileRead: error when running command '" << cmd.c_str() << "', return value = " << retval);
    }
  this->CloseTransfer();

}


//----------------------------------------------------------------------------
void vtkSRBHandler::StageFileWrite(const char * source, const char * destination)
{
  if (source == NULL)
    {
    vtkErrorMacro("StageFileWrite: source file name is null");
    return;
    }
  if (destination == NULL)
    {
    vtkErrorMacro("StageFileWrite: destination file name is null");
    return;
    }

  this->InitTransfer();
  
  std::string cmd = std::string("Sput ");

  // add the local file
  cmd += std::string(source);
  
  // strip off the srb:// at the front of destination
  int index;
  std::string destinationString (destination);
  std::string prefix;
  if ( ( index = destinationString.find ( "srb://", 0 ) ) != std::string::npos )
    {
    destinationString = destinationString.substr ( 6 );
    }
  vtkDebugMacro("StageFileWrite: using destination value " << destinationString.c_str());
  cmd += std::string(" ") + destinationString;

  // execute the command
  vtkDebugMacro("StageFileWrite: calling command: " << cmd.c_str());
  int retval = system(cmd.c_str());
  if (retval != 0)
    {
    vtkErrorMacro("StageFileWrite: error when running command '" << cmd.c_str() << "', return value = " << retval);
    }
  this->CloseTransfer();
}
