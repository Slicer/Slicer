#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

// Includes
#include <slicerlibcurl/slicerlibcurl.h>
#include <itksys/SystemTools.hxx>
#include <itksys/Process.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "DownloadSlicerTutorialsCLP.h"

// our file stream
std::ofstream* outputFile = NULL;
CURL* curl = NULL;

// Description:
// This function writes the downloaded data in a buffered manner
size_t outputFunction(char *buffer, size_t size, size_t nitems, void *userp)
  {
  outputFile->write(buffer,size*nitems);
  size *= nitems;
  return size;
  }

// Description:
// This function gives us some feedback on how our download is going.
int progress_callback(std::ofstream* outputFile, double dltotal, double dlnow,
                      double ultotal, double ulnow)
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

// Description:
// This function wraps curl functionality to download a specified URL to a specified dir
void download(char * source, const char * destination)
  {
  outputFile = new std::ofstream(destination, std::ios::binary);
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
  curl_easy_setopt(curl, CURLOPT_URL, source);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, true);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, outputFunction);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
  curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, NULL);
  curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
  curl_easy_perform(curl);

  outputFile->close();
  delete outputFile;
  outputFile = NULL;
  }

// Description:
// This function does a fairly decent job of brute-forcing a browser open.
bool openInBrowser(char * url)
  {
  #if WIN32
    ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
  #else
    std::vector<const char*> browsers;
    browsers.push_back("gnome-open");
    browsers.push_back("exo-open");
    browsers.push_back("kfmclient openURL");
    browsers.push_back("gnome-moz-remote --newwin");
    browsers.push_back("x-www-browser");
    browsers.push_back("firefox");
    browsers.push_back("mozilla-firefox");
    browsers.push_back("mozilla");
    browsers.push_back("konqueror");
    browsers.push_back("opera");
    browsers.push_back("galeon");
    browsers.push_back("epiphany");
    browsers.push_back("amaya");
    browsers.push_back("dillo");
    int i;
    for(i = 0; i < browsers.size(); i++)
      {
      int success = -1;
      itksysProcess* gp = itksysProcess_New();
      std::vector<const char*> cmd;
      cmd.push_back(browsers[i]);
      cmd.push_back(url);
      cmd.push_back(0);
      itksysProcess_SetCommand(gp,&*cmd.begin());
      itksysProcess_Execute(gp);
      success = itksysProcess_GetExitValue(gp);
      std::cout << success << std::endl;
      if(success == 1)
        {
        return true;
        }
      }
    return false;
  #endif

  return true;
  }

// Desciption:
// This is executed at runtime. The PARSE_ARGS statement references the
// GenerateCLP structure.
int main( int argc, char * argv[] )
  {
  PARSE_ARGS;  
 
  curl_global_init(CURL_GLOBAL_ALL); 
  curl = curl_easy_init();
  
  // Long string of ifs ahead...
  if(nlm)
    {
    openInBrowser(
      "http://www.insight-journal.org/midas/view_community.php?communityid=15");
    }
  if(namic)
    {
    openInBrowser(
      "http://insight-journal.org/midas/view_community.php?communityid=17");
    }
  if(dl01)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading EM Segment Tutorial</filter-name></filter-start>" << std::endl;
    download( "http://insight-journal.org/midas/download_bitstream.php?bitstreamid=3805", folder.append( "/EMSegmentTutorial.tgz").c_str());  
    std::cout << "<filter-end><filter-name>Downloading EM Segment Tutorial</filter-name></filter-end>" << std::endl;
    } 
  if(dl02)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading Data Loading and Visualization Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_item.php?itemid=1163&name=Data_Loading_and_Visualization&zip=1",folder.append( "/Data_Loading_and_Visualization.zip").c_str());  
    std::cout << "<filter-end><filter-name>Downloading Data Loading and Visualization Tutorial</filter-name></filter-end>" << std::endl;
    }
  if(dl03)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading Manual Segmentation Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_item.php?itemid=1164&name=Manual_Segmentation&zip=1",folder.append( "/Manual_Segmentation.zip").c_str());
    std::cout << "<filter-end><filter-name>Downloading Manual Segmentation Tutorial</filter-name></filter-end>" << std::endl;
    }
  if(dl04)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading Level-Set Segmentation Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_item.php?itemid=1165&name=Level-Set_Segmentation&zip=1",folder.append( "/Level-Set_Segmentation.zip").c_str());
    std::cout << "<filter-end><filter-name>Downloading Level-Set Segmentation Tutorial</filter-name></filter-end>" << std::endl;
    }
  if(dl05)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading Diffusion Tensor Imaging Analysis Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_item.php?itemid=1166&name=Diffusion_Tensor_Imaging_Analysis&zip=1",folder.append( "/Diffusion_Tensor_Imaging_Analysis.zip").c_str());
    std::cout << "<filter-end><filter-name>Downloading Diffusion Tensor Imaging Analysis Tutorial</filter-name></filter-end>" << std::endl;
    }
  if(dl06)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading Functional Magnetic Resonance Imaging Analysis Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_item.php?itemid=1167&name=Functional_Magnetic_Resonance_Imaging_Analysis&zip=1",folder.append( "/Functional_Magnetic_Resonance_Imaging_Analysis.zip").c_str());  
    std::cout << "<filter-end><filter-name>Downloading Functional Magnetic Resonance Imaging Analysis Tutorial</filter-name></filter-end>" << std::endl;
    }
  if(dl07)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading FreeSurfer Reader Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_item.php?itemid=1168&name=FreeSurfer_Reader&zip=1",folder.append( "/FreeSurfer_Reader.zip").c_str());  
    std::cout << "<filter-end><filter-name>Downloading FreeSurfer Reader Tutorial</filter-name></filter-end>" << std::endl;
    }
  if(dl08)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading Data Saving Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_item.php?itemid=1169&name=Data_Saving&zip=1",folder.append( "/Data_Saving.zip").c_str());
    std::cout << "<filter-end><filter-name>Downloading Data Saving Tutorial</filter-name></filter-end>" << std::endl;
    }
  if(dl09)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading Nrrd File Format Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_item.php?itemid=1170&name=Nrrd_File_Format&zip=1",folder.append( "/Nrrd_File_Format.zip").c_str());
    std::cout << "<filter-end><filter-name>Downloading Nrrd File Format Tutorial</filter-name></filter-end>" << std::endl;
    }
  if(dl10)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading Nrrd to Dicom Conversion Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_item.php?itemid=1171&name=Nrrd_to_Dicom_Conversion&zip=1",folder.append("/Nrrd_to_Dicom_Conversion.zip").c_str());  
    std::cout << "<filter-end><filter-name>Downloading Nrrd to Dicom Conversion Tutorial</filter-name></filter-end>" << std::endl;
    }
  if(dl11)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading Automatic Brain Segmentation Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_bitstream.php?bitstreamid=3770",folder.append("/SlicerAdvancedTraining_EMBrainAtlasClassifier_V1.0.ppt").c_str());  
    std::cout << "<filter-end><filter-name>Downloading Automatic Brain Segmentation Tutorial</filter-name></filter-end>" << std::endl;
    }
  if(dl12)
    {
    std::string folder = dir.substr(0);
    std::cout << "<filter-start><filter-name>Downloading Registration Tutorial</filter-name></filter-start>" << std::endl;
    download("http://insight-journal.org/midas/download_item.php?itemid=1173&name=Registration&zip=1",folder.append("/Registration.zip").c_str());  
    std::cout << "<filter-end><filter-name>Downloading Registration Tutorial</filter-name></filter-end>" << std::endl;
    }

  curl_easy_cleanup(curl);
  return EXIT_SUCCESS;
  }
