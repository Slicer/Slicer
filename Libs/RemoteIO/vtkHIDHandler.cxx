#include "vtkHIDHandler.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro ( vtkHIDHandler );
vtkCxxRevisionMacro ( vtkHIDHandler, "$Revision: 1.0 $" );

//----------------------------------------------------------------------------
size_t hid_read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t retcode;

  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  retcode = fread(ptr, size, nmemb, stream);

  std::cout << "*** We read " << retcode << " bytes from file\n";

  return retcode;
}

//----------------------------------------------------------------------------
size_t hid_write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  if (stream == NULL)
    {
    std::cerr << "write_callback: can't write, stream is null. size = " << size << std::endl;
    return 0;
    }
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

//----------------------------------------------------------------------------
size_t hid_ProgressCallback(FILE* vtkNotUsed(outputFile), double dltotal,
                            double dlnow, double ultotal, double ulnow)
{
  if(ultotal == 0)
    {
    if(dltotal > 0)
      {
      std::cout << "<filter-progress>" << dlnow/dltotal
                << "</filter-progress>" << std::endl;
      }
    }
  else
    {
    std::cout << ulnow*100/ultotal << "%" << std::endl;
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkHIDHandler::vtkHIDHandler()
{
}

//----------------------------------------------------------------------------
vtkHIDHandler::~vtkHIDHandler()
{
}

//----------------------------------------------------------------------------
void vtkHIDHandler::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}

//----------------------------------------------------------------------------
void vtkHIDHandler::StageFileRead(const char * source,
                                  const char *destination)
{
  if (source == NULL || destination == NULL)
    {
    vtkErrorMacro("StageFileRead: source or dest is null!");
    return;
    }
  const char *hostname = this->GetHostName();
  if ( hostname == NULL )
    {
    vtkErrorMacro("StageFileWrite: null host name");
    return;    
    }

  this->InitTransfer( );

  curl_easy_setopt(this->CurlHandle, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, source);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, NULL); 
  this->LocalFile = fopen(destination, "wb");
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, this->LocalFile);
  curl_easy_setopt ( this->CurlHandle, CURLOPT_SSL_VERIFYPEER, 0 );
    
  vtkDebugMacro("StageFileRead: about to do the curl download... source = " << source << ", dest = " << destination);
  CURLcode retval = curl_easy_perform(this->CurlHandle);

  if (retval == CURLE_OK)
    {
    vtkDebugMacro("StageFileRead: successful return from curl");
    }
  else if (retval == CURLE_BAD_FUNCTION_ARGUMENT)
    {
    vtkErrorMacro("StageFileRead: bad function argument to curl, did you init CurlHandle?");
    }
  else if (retval == CURLE_OUT_OF_MEMORY)
    {
    vtkErrorMacro("StageFileRead: curl ran out of memory!");
    }
  else
    {
    const char *stringError = curl_easy_strerror(retval);
    vtkErrorMacro("StageFileRead: error running curl: " << stringError);
    }
  this->CloseTransfer();
  fclose(this->LocalFile);
}

//----------------------------------------------------------------------------
void vtkHIDHandler::StageFileWrite(const char *source,
                                   const char *vtkNotUsed(destination))
{
  // need to use the -k argument for signed... how to
  // do with curl_easy?

  const char *hostname = this->GetHostName();
  if ( hostname == NULL )
    {
    vtkErrorMacro("StageFileWrite: null host name");
    return;    
    }

  this->LocalFile = fopen(source, "r");

  this->InitTransfer( );
  
  curl_easy_setopt(this->CurlHandle, CURLOPT_POST, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, source);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READFUNCTION, hid_read_callback);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READDATA, this->LocalFile);
  curl_easy_setopt ( this->CurlHandle, CURLOPT_SSL_VERIFYPEER, 0 );

  CURLcode retval = curl_easy_perform(this->CurlHandle);

   if (retval == CURLE_OK)
    {
    vtkDebugMacro("StageFileWrite: successful return from curl");
    }
   else
    {
    const char *stringError = curl_easy_strerror(retval);
    vtkErrorMacro("StageFileWrite: error running curl: " << stringError);
    }
   
  this->CloseTransfer();
  
  fclose(this->LocalFile);
}

//--- for querying
//----------------------------------------------------------------------------
const char* vtkHIDHandler::QueryServer( const char *uri, const char *destination)
{
  const char *returnString;
  if (uri == NULL )
    {
    returnString = "QueryServer: uri is NULL!";
    vtkErrorMacro("QueryServer: uri is NULL!");
    return ( returnString );
    }

  this->InitTransfer( );

  curl_easy_setopt(this->CurlHandle, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, uri);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, NULL); // write_callback);
  this->LocalFile = fopen(destination, "w");
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, this->LocalFile);
  curl_easy_setopt ( this->CurlHandle, CURLOPT_SSL_VERIFYPEER, 0 );
  curl_easy_setopt ( this->CurlHandle, CURLOPT_SSL_VERIFYHOST, 0 );
  curl_easy_setopt ( this->CurlHandle, CURLOPT_VERBOSE, 1 );

  vtkDebugMacro("QueryServer: about to do the curl download... uri = " << uri << ", dest = " << destination);
  CURLcode retval = curl_easy_perform(this->CurlHandle);

  if (retval == CURLE_OK)
    {
    returnString = "OK";
    vtkDebugMacro("QueryServer: successful return from curl");
    }
  else
    {
    if (retval == CURLE_BAD_FUNCTION_ARGUMENT)
      {
      returnString = "Bad function argument to cURL.";
      vtkErrorMacro("QueryServer: bad function argument to curl, did you init CurlHandle?");
      }
    else if (retval == CURLE_OUT_OF_MEMORY)
      {
      returnString = "Transfer library (cURL) ran out of memory.";
      vtkErrorMacro("QueryServer: curl ran out of memory!");
      }
    else
      {
      returnString = curl_easy_strerror(retval);
      vtkErrorMacro("QueryServer: error running curl: " << returnString);
      }
    }
  this->CloseTransfer();
  fclose(this->LocalFile);
  return ( returnString );
}
