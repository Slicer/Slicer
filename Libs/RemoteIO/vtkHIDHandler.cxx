#include "itksys/Process.h"
#include "itksys/SystemTools.hxx"
#include "vtkHIDHandler.h"

vtkStandardNewMacro ( vtkHIDHandler );
vtkCxxRevisionMacro ( vtkHIDHandler, "$Revision: 1.0 $" );



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

size_t hid_write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  if (stream == NULL)
    {
    std::cerr << "write_callback: can't write, stream is null. size = " << size << std::endl;
    return -1;
    }
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

size_t hid_ProgressCallback(FILE* outputFile, double dltotal, double dlnow, double ultotal, double ulnow)
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

  /*
  if (this->LocalFile)
    {
    this->LocalFile->close();
    delete this->LocalFile;
    this->LocalFile = NULL;
    }
  this->LocalFile = new std::ofstream(destination, std::ios::binary);
*/
  this->InitTransfer( );

  
  curl_easy_setopt(this->CurlHandle, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, source);
  //  curl_easy_setopt(this->CurlHandle, CURLOPT_NOPROGRESS, false);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  // use the default curl write call back
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, NULL); // write_callback);
  this->LocalFile = fopen(destination, "wb");
  // output goes into LocalFile, must be  FILE*
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, this->LocalFile);
  //--- allow for self-signing certificate
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

  /*
  this->LocalFile->close();
  delete this->LocalFile;
  this->LocalFile = NULL;
  */
  fclose(this->LocalFile);
}


//----------------------------------------------------------------------------
void vtkHIDHandler::StageFileWrite(const char *source,
                                   const char *destination)

{
  // need to use the -k argument for signed... how to
  // do with curl_easy?

  //--- check these arguments...
  /*
  if (this->LocalFile)
    {
    this->LocalFile->close();
    delete this->LocalFile;
    this->LocalFile = NULL;
    }
  this->LocalFile = new std::ofstream(destination, std::ios::binary);
  */
  const char *hostname = this->GetHostName();
  if ( hostname == NULL )
    {
    vtkErrorMacro("StageFileWrite: null host name");
    return;    
    }

  this->LocalFile = fopen(source, "r");

  this->InitTransfer( );
  
  curl_easy_setopt(this->CurlHandle, CURLOPT_PUT, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, source);
//  curl_easy_setopt(this->CurlHandle, CURLOPT_NOPROGRESS, false);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READFUNCTION, hid_read_callback);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READDATA, this->LocalFile);
//  curl_easy_setopt(this->CurlHandle, CURLOPT_PROGRESSDATA, NULL);
  //curl_easy_setopt(this->CurlHandle, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
  //--- allow for self-signing certificate
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
  /*
  this->LocalFile->close();
  delete this->LocalFile;
  this->LocalFile = NULL;
  */
}




//--- for querying
//----------------------------------------------------------------------------
void vtkHIDHandler::QueryServer( const char *uri, const char *destination)
{
  if (uri == NULL )
    {
    vtkErrorMacro("QueryServer: uri is NULL!")
    return;
    }

  this->InitTransfer( );

  curl_easy_setopt(this->CurlHandle, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, uri);
  //  curl_easy_setopt(this->CurlHandle, CURLOPT_NOPROGRESS, false);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  // use the default curl write call back
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, NULL); // write_callback);
  this->LocalFile = fopen(destination, "wb");
  // output goes into LocalFile, must be  FILE*
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, this->LocalFile);
  //--- allow for self-signing certificate
  vtkDebugMacro("QueryServer: about to do the curl download... uri = " << uri << ", dest = " << destination);
  CURLcode retval = curl_easy_perform(this->CurlHandle);


  if (retval == CURLE_OK)
    {
    vtkDebugMacro("QueryServer: successful return from curl");
    }
  else if (retval == CURLE_BAD_FUNCTION_ARGUMENT)
    {
    vtkErrorMacro("QueryServer: bad function argument to curl, did you init CurlHandle?");
    }
  else if (retval == CURLE_OUT_OF_MEMORY)
    {
    vtkErrorMacro("QueryServer: curl ran out of memory!");
    }
  else
    {
    const char *stringError = curl_easy_strerror(retval);
    vtkErrorMacro("QueryServer: error running curl: " << stringError);
    }
  this->CloseTransfer();

  fclose(this->LocalFile);
}



