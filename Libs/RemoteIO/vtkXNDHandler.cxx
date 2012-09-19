#include "vtkXNDHandler.h"
#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro ( vtkXNDHandler );
vtkCxxRevisionMacro ( vtkXNDHandler, "$Revision: 1.0 $" );

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
size_t xnd_write_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  if (stream == NULL)
    {
    std::cerr << "xnd_write_callback: can't write, stream is null. size = " << size << std::endl;
    return 0;
    }
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

//----------------------------------------------------------------------------
// write header
size_t xnd_writeheader_callback(void *ptr, size_t size, size_t nmemb, void *stream)
{
  if (stream == NULL)
    {
    std::cerr << "xnd_writeheader_callback: can't write, stream is null. size = " << size << std::endl;
    return 0;
    }
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

//----------------------------------------------------------------------------
size_t xnd_ProgressCallback(FILE* vtkNotUsed(outputFile), double dltotal,
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

//----------------------------------------------------------------------------

//  curl commands for doing various xnat desktop things:
//  #-check for tags
//  curl http://localhost:8081/tags
//
//  #add a tag
//  curl -X POST 'http://localhost:8081/tags?NewTag1&NewTag2'
//
//  #delete a tag
//  curl -X DELETE 'http://localhost:8081/tags?NewTag1'
//
//  #create a tagged data entry
//  curl -X POST -H "Content-Type: application/x-xnat-metadata+xml" -H "Content-Disposition: x-xnat-metadata; filename=\"demo-howto\"" -d @demo/sample-metadata/cardiac-text.xml http://localhost:8000/data
//  # returns url, which is an empty file
//  # http://localhost:8000/data/demo-sample-project/subj02/CT/demo-howto
//
//  #add some data
//  curl -T XNAT-FileServer-HOWTO.txt http://localhost:8000/data/demo-sample-project/subj02/CT/demo-howto
//
//  # can get metadata with:
//  curl http://localhost:8000/data/demo-sample-project/subj02/CT/demo-howto?part=metadata
//
//  # can search for this in the database:
//  curl http://localhost:8000/search
//
//  # can search for modalities:
//  curl http://localhost:8000/search??modality
//
//  # can search for all resources by modality:
//  curl http://localhost:8000/search?modality=CT
//
//  # can get just the resource 
//  curl 'http://localhost:8000/search?modality=CT&no-metadata'
//
//  # now you can use the URIs found using search commands 
//  # to download the actual data
//  # http://localhost:8000/data/demo-sample-project/subj02/CT/demo-howto
//
//----------------------------------------------------------------------------

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
  // read all the stuff in the file into a buffer to find size.
  // seems to work correctly as long as file is binary.
  fseek(this->LocalFile, 0, SEEK_END);
  long lSize = ftell(this->LocalFile);
  rewind(this->LocalFile);

  unsigned char *post_data = NULL;
  this->InitTransfer( );

  curl_easy_setopt(this->CurlHandle, CURLOPT_PUT, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, destination);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READFUNCTION, xnd_read_callback);
  curl_easy_setopt(this->CurlHandle, CURLOPT_READDATA, this->LocalFile);
  curl_easy_setopt(this->CurlHandle, CURLOPT_INFILESIZE, lSize);
  CURLcode retval = curl_easy_perform(this->CurlHandle);
  
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
int vtkXNDHandler::PostTag ( const char *svr, const char *label,
                             const char *temporaryResponseFileName)
{

  const char *hostname = this->GetHostName();
  if ( hostname == NULL )
    {
    vtkErrorMacro("PostTag: null host name");
    return 0;
    }
  if ( svr == NULL )
    {
    vtkErrorMacro ("PostTag: got a null server.");
    return 0;
    }
  if ( label == NULL )
    {
    vtkErrorMacro ("PostTag: got a null tag to add.");
    return 0;
    }
  if (temporaryResponseFileName == NULL)
    {
    vtkErrorMacro("PostTag: temporaryResponseFileName is null.");
    return 0;
    }

  std::stringstream ssuri;
  ssuri << svr;
  ssuri << "/tags?";
  std::string tmp = ssuri.str();
  const char *uri = tmp.c_str();  

  //--- create tag postfields
  std::stringstream ss;
  ss << label;
  std::string tmp2 = ss.str();
  const char *pf = tmp2.c_str();

  this->InitTransfer();
  
  //-- configure the curl handle
  curl_easy_setopt(this->CurlHandle, CURLOPT_POST, 1);
  curl_easy_setopt(this->CurlHandle, CURLOPT_VERBOSE, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, uri);
  curl_easy_setopt(this->CurlHandle, CURLOPT_POSTFIELDS, pf );  
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  
  // then need to set up a local file for capturing the return uri from the
  // post
  const char *responseFileName = temporaryResponseFileName;
  FILE *responseFile = fopen(responseFileName, "wb");
  if (responseFile == NULL)
    {
    vtkErrorMacro("PostTag: unable to open a local file caled " << responseFileName << " to write out to for capturing the uri");
    }
  else
    {
    // for windows
    curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, NULL); // write_callback);
    curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, responseFile);
    }
  
  CURLcode retval = curl_easy_perform(this->CurlHandle);

  if (retval == CURLE_OK)
    {
    vtkDebugMacro("PostTag: successful return from curl");
    }
  else
    {
    const char *stringError = curl_easy_strerror(retval);
    vtkErrorMacro("PostTag: error running curl: " << stringError);
    return 0;
    }

  this->CloseTransfer();

  if (responseFile)
    {
    fclose(responseFile);
    }
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkXNDHandler::PostMetadata( const char *serverPath,
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
int vtkXNDHandler::DeleteResource ( const char *uri, const char *temporaryResponseFileName )
{
  int result;
  if (uri == NULL )
    {
    vtkErrorMacro("vtkXNDHander::DeleteResource: uri is NULL!");
    return ( 0);
    }

  this->InitTransfer( );

  //--- not sure what config options we need...
  curl_easy_setopt(this->CurlHandle, CURLOPT_CUSTOMREQUEST, "DELETE");
  curl_easy_setopt(this->CurlHandle, CURLOPT_URL, uri);
  curl_easy_setopt(this->CurlHandle, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(this->CurlHandle, CURLOPT_VERBOSE, true);

  // then need to set up a local file for capturing any return xml
  const char *responseFileName = temporaryResponseFileName;
  FILE *responseFile = fopen(responseFileName, "wb");
  if (responseFile == NULL)
    {
    vtkErrorMacro("vtkXNDHandler::DeleteResource unable to open a local file caled " << responseFileName << " to write out to for capturing the uri");
    }
  else
    {
    // for windows
    curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEFUNCTION, NULL); // write_callback);
    curl_easy_setopt(this->CurlHandle, CURLOPT_WRITEDATA, responseFile);
    }
  
  CURLcode retval = curl_easy_perform(this->CurlHandle);

  if (retval == CURLE_OK)
    {
    result = 1;
    vtkDebugMacro("vtkXNDHandler::DeleteResource successful return from curl");
    }
  else
    {
    if (retval == CURLE_BAD_FUNCTION_ARGUMENT)
      {
      result = 0;
      vtkErrorMacro("vtkXNDHandler::DeleteResource bad function argument to curl, did you init CurlHandle?");
      }
    else if (retval == CURLE_OUT_OF_MEMORY)
      {
      result = 0;
      vtkErrorMacro("vtkXNDHandler::DeleteResource curl ran out of memory!");
      }
    else
      {
      result = 0;
      const char *returnString = curl_easy_strerror(retval);
      vtkErrorMacro("vtkXNDHandler::DeleteResource error running curl: " << returnString);
      }
    }
  this->CloseTransfer();
  fclose(this->LocalFile);
  if (responseFile)
    {
    fclose(responseFile);
    }

  //--- if result = 1, delete response went fine. Otherwise, problem with delete.
  return ( result );
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
