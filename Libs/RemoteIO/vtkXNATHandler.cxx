#include "itksys/Process.h"
#include "itksys/SystemTools.hxx"
#include "vtkXNATHandler.h"

vtkStandardNewMacro ( vtkXNATHandler );
vtkCxxRevisionMacro ( vtkXNATHandler, "$Revision: 1.0 $" );

/*------------------------------------------------------------------------------
vtkXNATHandler* vtkXNATHandler::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkXNATHandler");
  if(ret)
    {
    return (vtkXNATHandler*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkXNATHandler;
}
*/


//----------------------------------------------------------------------------
vtkXNATHandler::vtkXNATHandler()
{
}


//----------------------------------------------------------------------------
vtkXNATHandler::~vtkXNATHandler()
{
}


//----------------------------------------------------------------------------
void vtkXNATHandler::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}



//----------------------------------------------------------------------------
int vtkXNATHandler::CanHandleURI ( const char *uri )
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
    if ( prefix == "xnat" )
      {
      vtkDebugMacro("vtkXNATHandler: CanHandleURI: can handle this file: " << uriString.c_str());
      return (1);
      }
    }
  else
    {
    vtkDebugMacro ( "vtkXNATHandler::CanHandleURI: unrecognized uri format: " << uriString.c_str() );
    }
  return ( 0 );
}




//----------------------------------------------------------------------------
void vtkXNATHandler::StageFileRead(const char * source,
                                   const char * destination,
                                   const char *username,
                                   const char *password,
                                   const char *hostname)
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
  if ( username == NULL )
    {
    vtkErrorMacro("StageFileWrite: username is null");
    return;
    }
  if ( password == NULL )
    {
    vtkErrorMacro("StageFileWrite: password is null");
    return;
    }
  if ( hostname == NULL )
    {
    vtkErrorMacro("StageFileWrite: hostname is null");
    return;    
    }
  
  //--- example command:
  //--- ArcGet -host www.xnathost.org -u user -p password -s session1 
  //---
  std::string cmd = std::string("ArcGet ");
  //--- build the command
  cmd += " -host ";
  cmd += hostname;
  cmd += " -u ";
  cmd += username;
  cmd += " -p ";
  cmd += password;
  

  // strip off the xnat:// at the front of source
  int index;
  std::string quote ("\"");
  std::string sourceString (source);
  std::string prefix;
  if ( ( index = sourceString.find ( "xnat://", 0 ) ) != std::string::npos )
    {
    sourceString = sourceString.substr ( 6 );
    }
  vtkDebugMacro("StageFileRead: using source value " << sourceString.c_str());
  sourceString = quote + sourceString + quote;

  //--- assume that the source string contains a session ID... 
  cmd += " -s ";
  cmd += sourceString;
    
  // add the destination
  cmd += " -o ";
  cmd += destination;
  
  // execute the command
  vtkDebugMacro("StageFileRead: calling command: " << cmd.c_str());
  int retval = system(cmd.c_str());
  if (retval != 0)
    {
    vtkErrorMacro("StageFileRead: error when running command '" << cmd.c_str() << "', return value = " << retval);
    }

  // TODO: we should use the retval to set a flag for the GUI
}


//----------------------------------------------------------------------------
void vtkXNATHandler::StageFileWrite(const char * source, const char * destination)
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
  // use StoreXAR?
}
