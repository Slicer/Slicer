#include "vtkHTTPHandler.h"


vtkStandardNewMacro ( vtkHTTPHandler );
vtkCxxRevisionMacro ( vtkHTTPHandler, "$Revision: 1.0 $" );

//----------------------------------------------------------------------------
vtkHTTPHandler::vtkHTTPHandler()
{
  this->CurlHandle = NULL;
}


//----------------------------------------------------------------------------
vtkHTTPHandler::~vtkHTTPHandler()
{
  this->CurlHandle = NULL;
}


//----------------------------------------------------------------------------
void vtkHTTPHandler::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}



//----------------------------------------------------------------------------
int vtkHTTPHandler::CanHandleURI ( const char *uri )
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
    //--- check to see if any bracketed characters are in this part of the string.
    prefix = uriString.substr ( 0, index );
    if ( (index = prefix.find ( "]:", 0 ) ) != std::string::npos )
      {
      //--- if so, strip off the bracketed characters in case
      //--- we adopt the gwe "[filename.ext]:" prefix.
      prefix = prefix.substr ( 0, index+1 );
      }
    if ( prefix == "http" )
      {
      return (1);
      }
    }
  else
    {
    vtkDebugMacro ( "vtkHTTPHandler::CanHandleURI: unrecognized uri format: " << uri );
    }
  return ( 0 );
}



//----------------------------------------------------------------------------
void vtkHTTPHandler::InitTransfer( )
{
  curl_global_init(CURL_GLOBAL_ALL); 
  this->CurlHandle = curl_easy_init();
}

//----------------------------------------------------------------------------
void vtkHTTPHandler::CloseTransfer( )
{
  curl_easy_cleanup(this->CurlHandle);
  return EXIT_SUCCESS;      
}


//----------------------------------------------------------------------------
void vtkHTTPHandler::StageFileRead(char * source, const char * destination)
{
  outputFile = new std::ofstream(destination, std::ios::binary);
  curl_easy_setopt(this->CurlHandle, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, source);
  curl_easy_setopt(this->CurlHandle, CURLOPT_NOPROGRESS, false);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, BufferedWrite);
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, NULL);
  curl_easy_setopt(this->CurlHandle, CURLOPT_PROGRESSDATA, NULL);
  curl_easy_setopt(this->CurlHandle, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
  curl_easy_perform(this->CurlHandle);

  this->LocalFile->close();
  delete this->LocalFile;
  this->LocalFile = NULL;
}


//----------------------------------------------------------------------------
void vtkHTTPHandler::StageFileWrite(char * source, const char * destination)
{
  //--- check these arguments...
  outputFile = new std::ofstream(destination, std::ios::binary);
  curl_easy_setopt(this->CurlHandle, CURLOPT_HTTPPUT, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, source);
  curl_easy_setopt(this->CurlHandle, CURLOPT_NOPROGRESS, false);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READFUNCTION, BufferedRead);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READDATA, NULL);
  curl_easy_setopt(this->CurlHandle, CURLOPT_PROGRESSDATA, NULL);
  curl_easy_setopt(this->CurlHandle, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
  curl_easy_perform(this->CurlHandle);

  this->LocalFile->close();
  delete this->LocalFile;
  this->LocalFile = NULL;

}
