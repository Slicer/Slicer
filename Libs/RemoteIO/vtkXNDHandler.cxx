#include "itksys/Process.h"
#include "itksys/SystemTools.hxx"
#include "vtkXNDHandler.h"

vtkStandardNewMacro ( vtkXNDHandler );
vtkCxxRevisionMacro ( vtkXNDHandler, "$Revision: 1.0 $" );


size_t xnd_read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t retcode;

  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  retcode = fread(ptr, size, nmemb, stream);

  std::cout << "*** We read " << retcode << " bytes from file\n";

  return retcode;
}

size_t xnd_write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  if (stream == NULL)
    {
    std::cerr << "write_callback: can't write, stream is null. size = " << size << std::endl;
    return -1;
    }
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

size_t xnd_ProgressCallback(FILE* outputFile, double dltotal, double dlnow, double ultotal, double ulnow)
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
vtkXNDHandler::vtkXNDHandler()
{
}


//----------------------------------------------------------------------------
vtkXNDHandler::~vtkXNDHandler()
{
}


//----------------------------------------------------------------------------
void vtkXNDHandler::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}



//--- for downloading
//----------------------------------------------------------------------------
void vtkXNDHandler::StageFileRead(const char * source,
                                   const char *destination,
                                   const char *hostname)
{
  if (source == NULL || destination == NULL)
    {
    vtkErrorMacro("StageFileRead: source or dest is null!");
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
//  curl_easy_setopt ( this->CurlHandle, CURLOPT_SSL_VERIFYPEER, 0 );
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


//--- for uploading
//----------------------------------------------------------------------------
void vtkXNDHandler::StageFileWrite(const char *source,
                                   const char *destination,
                                   const char *hostname )
{
  if (source == NULL || destination == NULL)
    {
    vtkErrorMacro("StageFileWrite: source or dest is null!");
    return;
    }

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
  this->LocalFile = fopen(source, "r");

  this->InitTransfer( );
  
  curl_easy_setopt(this->CurlHandle, CURLOPT_POST, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, source);
//  curl_easy_setopt(this->CurlHandle, CURLOPT_NOPROGRESS, false);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READFUNCTION, xnd_read_callback);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READDATA, this->LocalFile);
//  curl_easy_setopt(this->CurlHandle, CURLOPT_PROGRESSDATA, NULL);
  //curl_easy_setopt(this->CurlHandle, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
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
const char *vtkXNDHandler::QueryServer( const char *uri, const char *destination)
{
  const char *returnString;
  if (uri == NULL )
    {
    returnString = "QueryServer: uri is NULL!";
    vtkErrorMacro("QueryServer: uri is NULL!");
    return ( returnString );
    }

  this->InitTransfer( );

  //--- not sure what config options we need...

  curl_easy_setopt(this->CurlHandle, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, uri);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, NULL); // write_callback);
  this->LocalFile = fopen(destination, "w");
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, this->LocalFile);
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
      returnString = "Bad function arguement to cURL.";
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


//----------------------------------------------------------------------------
const char* vtkXNDHandler::GetXMLDeclaration()
{
  const char *returnString = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
  return  (returnString);
}


//----------------------------------------------------------------------------
const char* vtkXNDHandler::GetNameSpace()
{
  const char *returnString = "xmlns=\"http://nrg.wustl.edu/xe\"";
  return (returnString);
}
