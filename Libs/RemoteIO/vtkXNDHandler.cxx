#include "itksys/Process.h"
#include "itksys/SystemTools.hxx"
#include "vtkXNDHandler.h"
#include <iterator>
#include <string>
#include <iostream>
#include <fstream>

vtkStandardNewMacro ( vtkXNDHandler );
vtkCxxRevisionMacro ( vtkXNDHandler, "$Revision: 1.0 $" );


size_t xnd_read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t retcode;

  /* in real-world cases, this would probably get this data differently
     as this fread() stuff is exactly what the library already would do
     by default internally */
  retcode = fread(ptr, size, nmemb, stream);

  std::cout << "*** vtkXNDHandler: We read " << retcode << " bytes from file\n";

  return retcode;
}

size_t xnd_write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  if (stream == NULL)
    {
    std::cerr << "xnd_write_callback: can't write, stream is null. size = " << size << std::endl;
    return -1;
    }
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

// write header
size_t xnd_writeheader_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  if (stream == NULL)
    {
    std::cerr << "xnd_writeheader_callback: can't write, stream is null. size = " << size << std::endl;
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
  this->HostName = NULL;
}


//----------------------------------------------------------------------------
vtkXNDHandler::~vtkXNDHandler()
{
  if ( this->HostName )
    {
    delete [] this->HostName;
    this->HostName = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkXNDHandler::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf ( os, indent );
}



//--- for downloading
//----------------------------------------------------------------------------
void vtkXNDHandler::StageFileRead(const char * source,
                                  const char *destination )
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
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, NULL); // write_callback);
  this->LocalFile = fopen(destination, "wb");
  curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, this->LocalFile);
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


//--- for uploading
//----------------------------------------------------------------------------
void vtkXNDHandler::StageFileWrite(const char *source,
                                   const char *destination )

{
  if (source == NULL || destination == NULL)
    {
    vtkErrorMacro("StageFileWrite: source or dest is null!");
    return;
    }

  const char *hostname = this->GetHostName();
  if ( hostname == NULL )
    {
    vtkErrorMacro("StageFileWrite: null host name");
    return;    
    }

  // Open file
  this->LocalFile = fopen(source, "rb");
  if (this->LocalFile == NULL)
    {
    vtkErrorMacro("StageFileWrite: unable to open file " << source );
    return;
    }
  // read all the stuff in the file into a buffer
  fseek(this->LocalFile, 0, SEEK_END);
  long lSize = ftell(this->LocalFile);
  rewind(this->LocalFile);


  unsigned char *post_data = NULL;
/*
  post_data = (unsigned char*)malloc(sizeof(unsigned char)*lSize);
  // read into buffer and close

  if (post_data == NULL)
    {
    vtkErrorMacro("StageFileWrite unable to allocate a buffer to read from source file, size = " << lSize);
    }
  else
    {
    size_t result = fread(post_data, 1, lSize*sizeof(unsigned char), this->LocalFile);
    if (result != lSize)
      {
      vtkErrorMacro("StageFileWrite: error reading contents of the sourcefile" << source <<", read " << result << " instead of " << lSize);
      }    
    }
*/
  this->InitTransfer( );
//-- set content type and use chunked transfer encoding.
//  std::string header1 = "Content-Type: application/octet-stream";
//  struct curl_slist *cl = NULL;
//  cl = curl_slist_append(cl, header1.c_str());
//  cl = curl_slist_append(cl, "Transfer-Encoding: chunked");
//  curl_easy_setopt(this->CurlHandle, CURLOPT_POST, 1);

  curl_easy_setopt(this->CurlHandle, CURLOPT_PUT, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, destination);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READFUNCTION, xnd_read_callback);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READDATA, this->LocalFile);
  curl_easy_setopt(this->CurlHandle, CURLOPT_INFILESIZE, lSize);
  CURLcode retval = curl_easy_perform(this->CurlHandle);
  
/*
  curl_easy_setopt(this->CurlHandle, CURLOPT_PUT, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_POSTFIELDSIZE, lSize);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, destination);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_POSTFIELDS, post_data);
  CURLcode retval = curl_easy_perform(this->CurlHandle);
*/
   if (retval == CURLE_OK)
    {
    vtkDebugMacro("StageFileWrite: successful return from curl");
    }
   else
    {
    const char *stringError = curl_easy_strerror(retval);
    vtkErrorMacro("StageFileWrite: error running curl: " << stringError);
    vtkErrorMacro("\t\tsource = " << source << ", destination = " << destination);
    }
   
  this->CloseTransfer();
  if (post_data)
    {
    free(post_data);
    }
}




//----------------------------------------------------------------------------
int vtkXNDHandler::PostMetadataTest ( const char *serverPath,
                                      const char *headerFileName,
                                      const char *dataFileName,
                                      const char *metaDataFileName,
                                      const char *temporaryResponseFileName )
{
  
  // serverPath will contain $srv/data, for instance http://localhost:8081/data
  // metaDataFileName is a filename of a file that has metadata in it.
  // temporaryResponseFileName is the name of the file into which
  if ( serverPath == NULL )
    {
    vtkErrorMacro("PostMetadata: serverPath is null.");
    return 0;
    }
  if (metaDataFileName == NULL)
    {
    vtkErrorMacro("PostMetadata: metaDataFileName is null.");
    return 0;
    }
  if (temporaryResponseFileName == NULL)
    {
    vtkErrorMacro("PostMetadata: temporaryResponseFileName is null.");
    return 0;
    }
  if (headerFileName == NULL )
    {
    vtkErrorMacro ("PostMetadataTest: null header filename");
    return 0;
    }
 const char *hostname = this->GetHostName();
  if ( hostname == NULL )
    {
    vtkErrorMacro("PostMetadata: null host name");
    return 0;
    }

  //--- tried this foen with both 'r' and 'rb'
  this->LocalFile = fopen(metaDataFileName, "r");
  if (this->LocalFile == NULL)
    {
    vtkErrorMacro("PostMetadata: unable to open meta data file " << metaDataFileName);
    return 0;
    }
  

  this->InitTransfer();

  //-- add header
  std::string dataFileNameString = std::string(dataFileName);
  std::string header1 = "Content-Type: application/x-xnat-metadata+xml";
  std::string header2 = "Content-Disposition: x-xnat-metadata; filename=\"" + dataFileNameString + "\"";
  struct curl_slist *cl = NULL;
  cl = curl_slist_append(cl, header1.c_str());
  cl = curl_slist_append(cl, header2.c_str());
  cl = curl_slist_append(cl, "Transfer-Encoding: chunked");
  
  //-- configure the curl handle
  curl_easy_setopt(this->CurlHandle, CURLOPT_VERBOSE, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_HEADERFUNCTION, NULL);
  curl_easy_setopt(this->CurlHandle, CURLOPT_HTTPHEADER, cl);
  curl_easy_setopt(this->CurlHandle, CURLOPT_POST, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_HEADER, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, serverPath);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READFUNCTION, xnd_read_callback);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READDATA, this->LocalFile);
  curl_easy_setopt(this->CurlHandle, CURLOPT_POSTFIELDS, NULL );
  
  // then need to set up a local file for capturing the return uri from the
  // post
  const char *returnURIFileName = temporaryResponseFileName;
  FILE *returnURIFile = fopen(returnURIFileName, "wb");
  if (returnURIFile == NULL)
    {
    vtkErrorMacro("PostMetadata: unable to open a local file caled " << returnURIFileName << " to write out to for capturing the uri");
    }
  else
    {
    // for windows
    curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, NULL); // write_callback);
    curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, returnURIFile);
    }

  CURLcode retval = curl_easy_perform(this->CurlHandle);

  if (retval == CURLE_OK)
    {
    vtkDebugMacro("PostMetadata: successful return from curl");
    }
   else
    {
    const char *stringError = curl_easy_strerror(retval);
    vtkErrorMacro("PostMetadata: error running curl: " << stringError);
    }

  curl_slist_free_all(cl);
  this->CloseTransfer();


  if (this->LocalFile)
    {
    fclose(this->LocalFile);
    }
  if (returnURIFile)
    {
    fclose(returnURIFile);
    }

  return 1;
}


//----------------------------------------------------------------------------
const char *vtkXNDHandler::PostMetadata ( const char *serverPath,
                                          const char *metaDataFileName,
                                          const char *dataFileName,
                                          const char *temporaryResponseFileName)
{
  const char *response = NULL;
  
  // serverPath will contain $srv/data, for instance http://localhost:8081/data
  // metaDataFileName is a filename of a file that has metadata in it.
  // dataFileName is the name of the file for which we are uploading metadata
  // temporaryResponseFileName is the name of the file into which
  // the server response to the POST is sent. This file is parsed for error or uri.
  if ( serverPath == NULL )
    {
    vtkErrorMacro("PostMetadata: serverPath is null.");
    return response;
    }
  if (metaDataFileName == NULL)
    {
    vtkErrorMacro("PostMetadata: metaDataFileName is null.");
    return response;
    }
  if (dataFileName == NULL)
    {
    vtkErrorMacro("PostMetadata: dataFileName is null.");
    return response;
    }
  if (temporaryResponseFileName == NULL)
    {
    vtkErrorMacro("PostMetadata: temporaryResponseFileName is null.");
    return response;
    }


  
  const char *hostname = this->GetHostName();
  if ( hostname == NULL )
    {
    vtkErrorMacro("PostMetadata: null host name");
    return response;
    }

  this->LocalFile = fopen(metaDataFileName, "r");
  if (this->LocalFile == NULL)
    {
    vtkErrorMacro("PostMetadata: unable to open meta data file " << metaDataFileName);
    return response;
    }

  
  // read all the stuff in the file into a buffer
  fseek(this->LocalFile, 0, SEEK_END);
  long lSize = ftell(this->LocalFile);
  rewind(this->LocalFile);

  // allocate memory
  char *post_data = NULL;
  post_data = (char*)malloc(sizeof(char)*lSize);
  if (post_data == NULL)
    {
    vtkErrorMacro("PostMetadata: unable to allocate a buffer to read from meta data file, size = " << lSize);
    }
  else
    {
    size_t result = fread(post_data, 1, lSize, this->LocalFile);
    if (result != lSize)
      {
      vtkErrorMacro("PostMetadata: error reading contents of the metadatafile " << metaDataFileName <<", read " << result << " instead of " << lSize);
      }    
    }
  this->InitTransfer();


  std::string dataFileNameString = std::string(dataFileName);
  std::string header1 = "Content-Type: application/x-xnat-metadata+xml";
  std::string header2 = "Content-Disposition: x-xnat-metadata; filename=\"" + dataFileNameString + "\"";
  vtkDebugMacro("header1= '" << header1.c_str() << "', header2= '" << header2.c_str() << "'");
  
  curl_easy_setopt(this->CurlHandle, CURLOPT_HEADERFUNCTION, NULL);
  struct curl_slist *cl = NULL;
  // append the list of headers to the curl list
  cl = curl_slist_append(cl, header1.c_str());
  cl = curl_slist_append(cl, header2.c_str());
  curl_easy_setopt(this->CurlHandle, CURLOPT_HTTPHEADER, cl);

  curl_easy_setopt(this->CurlHandle, CURLOPT_POST, 1);
  
  // use the headers
  curl_easy_setopt(this->CurlHandle, CURLOPT_HEADER, true);
  
  // set the uri, will usually be $SERVER/data
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, serverPath);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  if (post_data)
    {
    curl_easy_setopt(this->CurlHandle, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(this->CurlHandle, CURLOPT_POSTFIELDSIZE, strlen(post_data));
    }
  else
    {
    // use this to read the local file
    curl_easy_setopt(this->CurlHandle, CURLOPT_READFUNCTION, xnd_read_callback);
    curl_easy_setopt(this->CurlHandle, CURLOPT_READDATA, this->LocalFile);
    }
  
  // then need to set up a local file for capturing the return uri from the
  // post
  const char *returnURIFileName = temporaryResponseFileName;
  FILE *returnURIFile = fopen(returnURIFileName, "w");
  if (returnURIFile == NULL)
    {
    vtkErrorMacro("PostMetadata: unable to open a local file caled " << returnURIFileName << " to write out to for capturing the uri");
    }
  else
    {
    // for windows
    curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, NULL); // write_callback);
    curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, returnURIFile);
    }
//# post metadata for file
//uri=`eval curl $DOCDATA1$file$DOCDATA2 $SERVER/data`
//echo "$uri"

  CURLcode retval = curl_easy_perform(this->CurlHandle);

  if (retval == CURLE_OK)
    {
    vtkDebugMacro("PostMetadata: successful return from curl");
    }
   else
    {
    const char *stringError = curl_easy_strerror(retval);
    vtkErrorMacro("PostMetadata: error running curl: " << stringError);
    }

  curl_slist_free_all(cl);
  
  this->CloseTransfer();

  if (this->LocalFile)
    {
    fclose(this->LocalFile);
    }
  if (post_data)
    {
    free(post_data);
    }
  if (returnURIFile)
    {
    fclose(returnURIFile);
    }
  // now read from it
  if (retval == CURLE_OK)
    {
    vtkDebugMacro("PostMetadata: trying to parse the uri out from " << returnURIFileName);
    returnURIFile = fopen(returnURIFileName, "r");
    if (returnURIFile == NULL)
      {
      vtkErrorMacro("PostMetadata: unable to open " << returnURIFileName << " to parse out the uri");
      }
    else
      {
      // read everything from it
      fseek(returnURIFile, 0, SEEK_END);
      long lSize = ftell(returnURIFile);
      rewind(returnURIFile);
      // allocate the return body with an extra character, as we're going to
      // add the null character at the end to help with parsing
      char *returnURIBody = (char*)malloc(sizeof(char)*(lSize+1));
      if (returnURIBody == NULL)
        {
        vtkErrorMacro("PostMetadata: unable to allocate buffer for contents of returned uri file, size = " << lSize);
        }
      else
        {
        size_t result = fread(returnURIBody, 1, lSize, returnURIFile);
        if (result != lSize)
          {
          vtkErrorMacro("PostMetadata: error reading contents of the returned uri file " << returnURIFileName <<", read " << result << " instead of " << lSize);
          fclose (returnURIFile);
          return (response);
          }
        // add the null character at the end since it's in utf8 format and
        // we're doing some magic here (don't use string.find, it won't work)
         returnURIBody[lSize] = '\0';
        // get the first instance of the uri
        const char *http = "http://";
        char *httpPointer1 = strstr(returnURIBody, http);
        // find the second uri, past the first one
        char *httpPointer2 = strstr(httpPointer1+7, http);
        if (httpPointer2 != NULL)
          {
          vtkDebugMacro("PostMetadata: strstr found a string pointer = '" << httpPointer2 << "'");
          std::string responseString = std::string(httpPointer2);
          response = responseString.c_str();          
          }
        else
          {
          vtkErrorMacro ("PostMetadata: can't find a (second) uri in the response file " << returnURIFileName);
          }
        }
      free(returnURIBody);
      }
    }
  else
    {
    vtkErrorMacro("PostMetadata: posting failed, not trying to return the uri");
    }

  if (returnURIFile)
    {
    fclose(returnURIFile);
    }
  // return a uri as a string.
  vtkWarningMacro("PostMetaData: returning response string '" << response << "'");
  return response;
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
