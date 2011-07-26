#include "vtkFetchMIWebServicesClientXND.h"
#include "vtkXNDHandler.h"
#include <sstream>



//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIWebServicesClientXND );
vtkCxxRevisionMacro ( vtkFetchMIWebServicesClientXND, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIWebServicesClientXND::vtkFetchMIWebServicesClientXND ( )
{
}


//---------------------------------------------------------------------------
vtkFetchMIWebServicesClientXND::~vtkFetchMIWebServicesClientXND ( )
{
}

//---------------------------------------------------------------------------
void vtkFetchMIWebServicesClientXND::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
}



//---------------------------------------------------------------------------
int vtkFetchMIWebServicesClientXND::QueryServerForTags ( const char *responseFileName )
{
  vtkXNDHandler *h = vtkXNDHandler::SafeDownCast ( this->GetURIHandler() );
  if ( h == NULL )
    {
    vtkErrorMacro ( "QueryServerForTags: No handler set on Client.");
    return 0;
    }
  if  ( h->GetHostName() == NULL )
    {
    vtkErrorMacro ( "Download: No host name set on URIHandler." );
    return 0;
    }
  const char *hostname = h->GetHostName();
  std::stringstream q;
  q << hostname;
  q << "/tags";
  std::string query = q.str();
  const char *errorString = h->QueryServer (query.c_str(), responseFileName );
  if ( !strcmp (errorString, "OK"))
    {
    return 1;
    }
  return 0;
}



//---------------------------------------------------------------------------
 int vtkFetchMIWebServicesClientXND::QueryServerForTagValues ( const char *att, const char *responseFilename)
{
  vtkXNDHandler *h = vtkXNDHandler::SafeDownCast ( this->GetURIHandler() );
  if ( h == NULL )
    {
    vtkErrorMacro ( "QueryServerForTagValues: No handler set on Client.");
    return 0;
    }
  if  ( h->GetHostName() == NULL )
    {
    vtkErrorMacro ( "QueryServerForTagValues: No host name set on URIHandler." );
    return 0;
    }
  const char *hostname = h->GetHostName();
  std::stringstream q;
  q << hostname;
  q << "/search??";
  q << att;
  std::string query = q.str();
  const char *errorString = h->QueryServer ( query.c_str(), responseFilename );
  if ( !strcmp ( errorString, "OK" ) )
    {
    return 1;
    }
  return 0;
}



//---------------------------------------------------------------------------
void vtkFetchMIWebServicesClientXND::Download ( const char *src, const char *dest )
{
  vtkXNDHandler *h = vtkXNDHandler::SafeDownCast ( this->GetURIHandler() );
  if ( h == NULL )
    {
    vtkErrorMacro ( "Download: No handler set on Client.");
    return;
    }
  if  ( h->GetHostName() == NULL )
    {
    vtkErrorMacro ( "Download: No host name set on URIHandler." );
    return;
    }
  if ( src == NULL || !(strcmp (src, "" )))
    {
    vtkErrorMacro ( "Download: Got null source URI." );
    return;
    }
  if ( dest == NULL || !(strcmp (dest, "" )))
    {
    vtkErrorMacro ( "Download: Got null destination URI." );
    return;
    }
  h->StageFileRead ( src, dest );

}



//---------------------------------------------------------------------------
int vtkFetchMIWebServicesClientXND::QueryServerForResources ( vtkTagTable *table, const char *responseFileName )
{
  if ( table == NULL )
    {
    vtkErrorMacro ( "QueryServerForResources: got NULL TagTable" );
    return 0;
    }
  if ( responseFileName == NULL )
    {
    vtkErrorMacro ( "QueryServerForResources: got NULL responseFileName." );
    return 0;
    }
  if ( this->GetURIHandler() == NULL )
    {
    vtkErrorMacro ( "QueryServerForResources: Got NULL URIHandler." );
    return 0;
    }
  vtkXNDHandler *h = vtkXNDHandler::SafeDownCast ( this->GetURIHandler() );
  if (h ==  NULL )
    {
    vtkErrorMacro ( "QueryServerForResources: Unable to safe downcast the handler.");
    return 0;
    }
  if  ( this->GetURIHandler()->GetHostName() == NULL )
    {
    vtkErrorMacro ( "QueryServerForResources: No host name set on URIHandler." );
    return 0;
    }

  vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( table );

  std::string att;
  std::string val;
  std::stringstream q;
  std::string query;

  //--- Build query
  q << h->GetHostName() << "/search?";

  //--- TODO: is there a better way to filter out weird tags?
  //--- Append all selected XND tags that are reasonably
  //--- formed. That means they have both an attribute
  //--- and a value, and both are not "", "NULL" or "no values found"
  //--- etc.
  int num = t->GetNumberOfTags();

  for ( int i =0; i<num; i++)
    {
    att = t->GetTagAttribute(i);
    val = t->GetTagValue(i);
    if ( t->IsTagSelected(att.c_str()))
      {
      //--- do some checking...
      if ( att.c_str() != NULL && val.c_str() != NULL )
        {
        if ( (strcmp(att.c_str(), "NULL")) &&
             (strcmp(att.c_str(), "null")) &&
             (strcmp(att.c_str(), "none")) &&
             (strcmp(att.c_str(), "None")) &&
             (strcmp(att.c_str(), "" )))
          {
          if ( (strcmp(val.c_str(), "NULL")) &&
               (strcmp(val.c_str(), "null")) &&
               (strcmp(val.c_str(), "no values found")) &&
               (strcmp(val.c_str(), "No values found")) &&
               (strcmp(val.c_str(), "" )))
            {
            q << att;
            q << "=";
            q << val;
            q << "&";
            }
          }
        }
      }
    }
  //---
  //--- TODO: trim off the last ampersand in the query string!!!
  //---
  query = q.str();
  const char *errorString = h->QueryServer ( query.c_str(), responseFileName );
  if ( !strcmp(errorString, "OK" ))
    {
    return 1;
    }
  return 0;
}



//---------------------------------------------------------------------------
int vtkFetchMIWebServicesClientXND::AddTagToServer ( const char *att,
                                                     const char *vtkNotUsed(val),
                                                     const char *responseFileName )
{
  // assume val is null for XND.
  if ( this->GetURIHandler() == NULL )
    {
    vtkErrorMacro ( "AddTagToServer: Got NULL URIHandler." );
    return 0;
    }
  vtkXNDHandler *h = vtkXNDHandler::SafeDownCast ( this->GetURIHandler() );
  if (h ==  NULL )
    {
    vtkErrorMacro ( "AddTagToServer: Unable to safe downcast the handler.");
    return 0;
    }
  if  ( this->GetURIHandler()->GetHostName() == NULL )
    {
    vtkErrorMacro ( "AddTagToServer: No host name set on URIHandler." );
    return 0;
    }
  if ( att == NULL || !(strcmp( att, "" )) )
    {
    vtkErrorMacro ( "AddTagToServer: Got NULL tag name." );
    return 0;
    }
  if ( responseFileName == NULL || !(strcmp(responseFileName, "" )) )
    {
    vtkErrorMacro ( "AddTagToServer: Got NULL responseFileName." );
    return 0;
    }

  //--- do the post
  int retval = h->PostTag ( h->GetHostName(), att, responseFileName );

  //--- return 1 if successful, 0 if not
  return (retval );
}

  

//---------------------------------------------------------------------------
int vtkFetchMIWebServicesClientXND::DeleteResourceFromServer ( const char *uri,
                                                          const char *responseFileName)
{
  if ( uri == NULL )
    {
    vtkErrorMacro ( "DeleteResourceFromServer: got NULL uri." );
    return 0;
    }
  if ( responseFileName == NULL )
    {
    vtkErrorMacro ( "DeleteResourceFromServers: got NULL responseFileName." );
    return 0;
    }
  if ( this->GetURIHandler() == NULL )
    {
    vtkErrorMacro ( "DeleteResourceFromServers: Got NULL URIHandler." );
    return 0;
    }
  vtkXNDHandler *h = vtkXNDHandler::SafeDownCast ( this->GetURIHandler() );
  if (h ==  NULL )
    {
    vtkErrorMacro ( "DeleteResourceFromServers: Unable to safe downcast the handler.");
    return 0;
    }
  if  ( this->GetURIHandler()->GetHostName() == NULL )
    {
    vtkErrorMacro ( "DeleteResourceFromServers: No host name set on URIHandler." );
    return 0;
    }

  int retval = h->DeleteResource ( uri, responseFileName );

  //--- returns 1 if successful, 0 if not.
  return ( retval );
}



//---------------------------------------------------------------------------
int vtkFetchMIWebServicesClientXND::CreateNewURIForResource( const char *resourceName, 
                                                        const char *headerFileName,
                                                        const char *uploadFileName,
                                                        const char *responseFileName )
{
  if ( this->URIHandler == NULL )
    {
    vtkErrorMacro ( "CreateNewURIForResource: Got NULL URIHandler." );
    return 0;
    }
  vtkXNDHandler *h = vtkXNDHandler::SafeDownCast ( this->GetURIHandler() );
  if (h ==  NULL )
    {
    vtkErrorMacro ( "AddTagToServer: Unable to safe downcast the handler.");
    return 0;
    }
  if ( h->GetHostName() == NULL || !(strcmp(h->GetHostName(), "" )) )
    {
    vtkErrorMacro ( "CreateNewURIForResource: Got NULL Hostname on URIHandler." );
    return 0;
    }
  if ( resourceName == NULL || !(strcmp(resourceName, "" )))
    {
    vtkErrorMacro ( "CreateNewURIForResource: Got NULL resourceName." );
    return 0;
    }
  if ( responseFileName == NULL || !(strcmp(responseFileName, "" )))
    {
    vtkErrorMacro ( "CreateNewURIForResource: Got NULL responseFileName." );
    return 0;
    }
  if ( headerFileName == NULL || !(strcmp(headerFileName, "" )))
    {
    vtkErrorMacro ( "CreateNewURIForResource: Got NULL headerFileName." );
    return 0;
    }
  if ( uploadFileName == NULL || !(strcmp(uploadFileName, "" )))
    {
    vtkErrorMacro ( "CreateNewURIForResource: Got NULL uploadFileName." );
    return 0;
    }

  // set the post destination for data being described
  std::stringstream ss;
  ss << h->GetHostName();
  ss << "/data";
  std::string s = ss.str();
  int retval = h->PostMetadata ( s.c_str(),
                                      headerFileName,
                                      resourceName,
                                      uploadFileName,
                                      responseFileName );
  //--- return 1 if successful, 0 if not
  return (retval );
}
  






