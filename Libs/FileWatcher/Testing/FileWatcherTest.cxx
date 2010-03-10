#include <FileWatcher/FileWatcher.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <itksys/SystemTools.hxx>

#include <cstdlib>

/// Processes a file action
class UpdateListener : public FW::FileWatchListener
{
public:
  UpdateListener() {}
  void handleFileAction(FW::WatchID watchid,
                        const FW::String& dir,
                        const FW::String& filename,
                        FW::Action action)
  {
    std::string eventDescription;
    switch (action)
      {
      case FW::Actions::Add:
        eventDescription = "File Added";
        break;
      case FW::Actions::Delete:
        eventDescription = "File Deleted";
        break;
      case FW::Actions::Modified:
        eventDescription = "File Modified";
        break;
      default:
        eventDescription = "Unknown Action";
      }
    std::cout << "DIR (" 
              << dir + ") FILE (" + filename + ") has event "
              << eventDescription << std::endl;
  }
};

//
// Test the FileWatcher
//
int main(int argc, char **argv)
{
  // create the file watcher object
  FW::FileWatcher fileWatcher;
  FW::WatchID watchID ;
  try 
    {
    // add a watch to the system
    watchID = fileWatcher.addWatch(argv[1],
                                   new UpdateListener(),
                                   true);
    fileWatcher.update();
    }
  catch( std::exception& e ) 
    {
    std::cerr << "An unexpected exception has occurred: \n"
              << e.what() << std::endl;
    return EXIT_FAILURE;
    }

  // Create a file
  std::string fileName = std::string(argv[1]) + "test1.txt";
  std::ofstream ofile(fileName.c_str(),
                      std::ios::out);
  ofile << "Test" << std::endl;

  std::cout << "********** Should see " << fileName << " added" << std::endl;
  ofile.close();
  fileWatcher.update();

  ofile.open(fileName.c_str());
  ofile << "Test again" << std::endl;
  ofile.close();

  std::cout << "********** Should see " << fileName << " modified" << std::endl;
  fileWatcher.update();
  // Cleanup
  std::cout << "********** Should see " << fileName << " deleted" << std::endl;
  itksys::SystemTools::RemoveFile(fileName.c_str());
  fileWatcher.update();

  fileWatcher.removeWatch(watchID);
  return EXIT_SUCCESS;
}
