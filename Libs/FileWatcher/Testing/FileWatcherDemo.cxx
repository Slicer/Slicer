/**
  Demo app for FileWatcher. FileWatcher is a simple wrapper for the file
  modification system in Windows and Linux.

  @author James Wynn
  @date 2/25/2009

  Copyright (c) 2009 James Wynn (james@jameswynn.com)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include <FileWatcher/FileWatcher.h>
#include <iostream>
#include <stdio.h>

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


int main(int argc, char **argv)
{
  try 
    {
    // create the file watcher object
    FW::FileWatcher fileWatcher;

    // add a watch to the system
    argc--; argv++;
    while (argc)
      {
      fileWatcher.addWatch(argv[0],
                           new UpdateListener(),
                           true);
      argc--; argv++;
      }
    std::cout << "Press ^C to exit demo" << std::endl;

    // loop until a key is pressed
    while(1)
      {
      fileWatcher.update();
      }
    } 
  catch( std::exception& e ) 
    {
    fprintf(stderr, "An exception has occurred: %s\n", e.what());
    }

  return 0;
}
