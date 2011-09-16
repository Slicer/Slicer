#include "vtkHTTPHandler.h"
#include <iostream>
#include <string>
#include <vtksys/SystemTools.hxx>


int main(int argc, char** argv)
{

  bool failed = false;
  //
  // parse command line
  if (argc < 2)
  {
    std::cerr 
      << "Usage: vtkEMSegmentRemoteIOTest"  << std::endl
      <<         "remote URI"               << std::endl
      <<         "filename"                 << std::endl
      << std::endl;
    return EXIT_FAILURE;
  }

  std::string url(argv[1]);
  std::string file(argv[2]);


  // our HTTP handler
  vtkHTTPHandler* httpHandler = vtkHTTPHandler::New();
  // use the following line if the SlicerApplication is available
  //vtkHTTPHandler* httpHandler = vtkHTTPHandler::SafeDownCast(this->GetSlicerApplication()->GetMRMLScene()->FindURIHandlerByName("HTTPHandler"));

  httpHandler->SetForbidReuse(1);

  if (!failed && !httpHandler->CanHandleURI(url.c_str()))
    {
      std::cerr << "ERROR: Invalid URI specified" << std::endl;
      failed=true;
    }


  std::cout << "Download file: " << url << std::endl;
  std::cout << "into         : " << file << std::endl;
  httpHandler->StageFileRead( url.c_str(), file.c_str() );

  
  if (!failed && !vtksys::SystemTools::FileExists(file.c_str()) )
    {
      std::cerr << "ERROR: File doesn't exists" << std::endl;
      failed=true;
    }

  if (!failed && vtksys::SystemTools::FileLength(file.c_str())<1 )
    {
      std::cerr << "ERROR: File is empty" << std::endl;
      failed=true;
    }


  std::cout << "Cleanup..." << std::endl;
  httpHandler->Delete();


  if (failed)
    {
      return EXIT_FAILURE;
    }
  else
    {
      return EXIT_SUCCESS;
    }


}

