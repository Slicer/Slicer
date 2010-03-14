/**
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

  James Wynn james@jameswynn.com
*/

#include <FileWatcher/FileWatcherOSX.h>
#include <FileWatcher/FileWatcher.h>

#if FILEWATCHER_PLATFORM == FILEWATCHER_PLATFORM_KQUEUE

#include <sys/event.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sstream>
#include <iostream>

namespace FW
{
  
#define MAX_CHANGE_EVENT_SIZE 2000
  
typedef struct kevent KEvent;
  
struct EntryStruct
{
  EntryStruct(const char* filename, time_t mtime = 0)
    : mFilename(filename), mModifiedTime(mtime)
  {
  }
  ~EntryStruct()
  {
    delete[] mFilename;
  }
  const char* mFilename;
  time_t mModifiedTime;
};
  
int comparator(const void* ke1, const void* ke2)
{
  /*KEvent* kevent1 = (KEvent*) ke1;
    KEvent* kevent2 = (KEvent*) ke2;
    
    EntryStruct* event1 = (EntryStruct*)kevent1->udata;
    EntryStruct* event2 = (EntryStruct*)kevent2->udata;
    return strcmp(event1->mFilename, event2->mFilename);
    */
  return strcmp(((EntryStruct*)(((KEvent*)(ke1))->udata))->mFilename,
                ((EntryStruct*)(((KEvent*)(ke2))->udata))->mFilename);
}
  
struct WatchStruct
{
  WatchID mWatchID;
  String mDirName;
  FileWatchListener* mListener;
    
  // index 0 is always the directory
  KEvent mChangeList[MAX_CHANGE_EVENT_SIZE];
  size_t mChangeListCount;
    
  WatchStruct(WatchID watchid,
              const String& dirname,
              FileWatchListener* listener)
    : mWatchID(watchid), mDirName(dirname), mListener(listener)
  {
    mChangeListCount = 0;
    addAll();
  }
    
  void addFile(const String& name, bool imitEvents = true)
  {
    // create entry
    struct stat attrib;
    stat(name.c_str(), &attrib);
      
    int fd = open(name.c_str(), O_RDONLY);

    if(fd == -1)
      {
      std::ostringstream message;
      message << "In " << __FILE__ << " at line " << __LINE__ << ": ";
      std::cout << "Warning: " << message.str() << std::endl;
      return;
//      throw FileNotFoundException(name, message.str());
      }
      
    ++mChangeListCount;
      
    char* namecopy = new char[name.length() + 1];
    strncpy(namecopy, name.c_str(), name.length());
    namecopy[name.length()] = 0;
    EntryStruct* entry = new EntryStruct(namecopy, attrib.st_mtime);
      
    // set the event data at the end of the list
    EV_SET(&mChangeList[mChangeListCount], fd, EVFILT_VNODE,
           EV_ADD | EV_ENABLE | EV_ONESHOT,
           NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_ATTRIB,
           0, (void*)entry);
      
    // qsort
    qsort(mChangeList + 1, mChangeListCount, sizeof(KEvent), comparator);
      
    // handle action
    if(imitEvents)
      handleAction(name, FW::Actions::Add);
  }
    
  void removeFile(const String& name, bool imitEvents = true)
  {
    // bsearch
    KEvent target;
    EntryStruct tempEntry(name.c_str(), 0);
    target.udata = &tempEntry;
    KEvent* ke = (KEvent*)bsearch(&target,
                                  &mChangeList,
                                  mChangeListCount + 1,
                                  sizeof(KEvent),
                                  comparator);
    if(!ke)
      {
      std::ostringstream message;
      message << "In " << __FILE__ << " at line " << __LINE__ << ": ";
      throw FileNotFoundException(name, message.str());
      }

    tempEntry.mFilename = 0;
      
    // delete
    close(ke->ident);
    delete((EntryStruct*)ke->udata);
    memset(ke, 0, sizeof(KEvent));
      
    // move end to current
    memcpy(ke, &mChangeList[mChangeListCount], sizeof(KEvent));
    memset(&mChangeList[mChangeListCount], 0, sizeof(KEvent));
    --mChangeListCount;
      
    // qsort
    qsort(mChangeList + 1, mChangeListCount, sizeof(KEvent), comparator);
      
    // handle action
    if(imitEvents)
      handleAction(name, FW::Actions::Delete);
  }
    
  // called when the directory is actually changed
  // means a file has been added or removed
  // rescans the watched directory adding/removing files and sending notices
  void rescan()
  {
    // if new file, call addFile
    // if missing file, call removeFile
    // if timestamp modified, call handleAction(filename, ACTION_MODIFIED);
    DIR* dir = opendir(mDirName.c_str());
    if(!dir)
      return;
      
    struct dirent* dentry;
    KEvent* ke = &mChangeList[1];
    EntryStruct* entry = 0;
    struct stat attrib;      
      
    while((dentry = readdir(dir)) != NULL)
      {
      String fname = mDirName + "/" + dentry->d_name;
      stat(fname.c_str(), &attrib);
      if(!S_ISREG(attrib.st_mode))
        continue;
        
      if(ke <= &mChangeList[mChangeListCount])
        {
        entry = (EntryStruct*)ke->udata;
        int result = strcmp(entry->mFilename, fname.c_str());
        //fprintf(stderr, "[%s cmp %s]\n", entry->mFilename, fname.c_str());
        if(result == 0)
          {
          stat(entry->mFilename, &attrib);
          time_t timestamp = attrib.st_mtime;
            
          if(entry->mModifiedTime != timestamp)
            {
            entry->mModifiedTime = timestamp;
            handleAction(entry->mFilename, FW::Actions::Modified);
            }
          ke++;
          }
        else if(result < 0)
          {
          // f1 was deleted
          removeFile(entry->mFilename);
          ke++;
          }
        else
          {
          // f2 was created
          addFile(fname);
          ke++;
          }
        }
      else
        {
        // just add
        addFile(fname);
        ke++;
        }
      }//end while
      
    closedir(dir);
  };
    
  void handleAction(const String& filename, Action action)
  {
    mListener->handleFileAction(mWatchID, mDirName, filename, action);
  }
    
  void addAll()
  {
    // add base dir
    int fd = open(mDirName.c_str(), O_RDONLY);
    EV_SET(&mChangeList[0], fd, EVFILT_VNODE,
           EV_ADD | EV_ENABLE | EV_ONESHOT,
           NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_ATTRIB,
           0, 0);
      
    // scan directory and call addFile(name, false) on each file
    DIR* dir = opendir(mDirName.c_str());
    if(!dir)
      {
      std::ostringstream message;
      message << "In " << __FILE__ << " at line " << __LINE__ << ": ";
      throw FileNotFoundException(mDirName, message.str());
      }
      
    struct dirent* entry;
    struct stat attrib;
    while((entry = readdir(dir)) != NULL)
      {
      String fname = (mDirName + "/" + String(entry->d_name));
      stat(fname.c_str(), &attrib);
      if(S_ISREG(attrib.st_mode))
        addFile(fname, false);
      //else
      //  fprintf(stderr, "NOT ADDED: %s (%d)\n", fname.c_str(), attrib.st_mode);

      }//end while
      
    closedir(dir);
  }
    
  void removeAll()
  {
    KEvent* ke = NULL;
      
    // go through list removing each file and sending an event
    for(unsigned int i = 0; i < mChangeListCount; ++i)
      {
      ke = &mChangeList[i];
      //handleAction(name, Action::Delete);
      EntryStruct* entry = (EntryStruct*)ke->udata;
        
      handleAction(entry->mFilename, FW::Actions::Delete);
        
      // delete
      close(ke->ident);
      delete((EntryStruct*)ke->udata);
      }
  }
};
  
void FileWatcherOSX::update()
{
  int nev = 0;
  struct kevent event;
    
  WatchMap::iterator iter = mWatches.begin();
  WatchMap::iterator end = mWatches.end();
  for(; iter != end; ++iter)
    {
    WatchStruct* watch = iter->second;
      
    while((nev = kevent(mDescriptor,
                        (KEvent*)&(watch->mChangeList),
                        watch->mChangeListCount + 1,
                        &event,
                        1,
                        &mTimeOut)) != 0)
      {
      if(nev == -1)
        perror("kevent");
      else
        {
        EntryStruct* entry = 0;
        if((entry = (EntryStruct*)event.udata) != 0)
          {
          //fprintf(stderr, "File: %s -- ", (char*)entry->mFilename);
            
          if(event.fflags & NOTE_DELETE)
            {
            //fprintf(stderr, "File deleted\n");
            //watch->handleAction(entry->mFilename, Action::Delete);
            watch->removeFile(entry->mFilename);
            }
          if(event.fflags & NOTE_EXTEND || 
             event.fflags & NOTE_WRITE ||
             event.fflags & NOTE_ATTRIB)
            {
            //fprintf(stderr, "modified\n");
            //watch->rescan();
            struct stat attrib;
            stat(entry->mFilename, &attrib);
            entry->mModifiedTime = attrib.st_mtime;
            watch->handleAction(entry->mFilename, FW::Actions::Modified);
            }
          }
        else
          {
          //fprintf(stderr, "Dir: %s -- rescanning\n", watch->mDirName.c_str());
          watch->rescan();
          }
        }
      }
    }
}
  
//--------
FileWatcherOSX::FileWatcherOSX()
{
  mDescriptor = kqueue();
  mTimeOut.tv_sec = 0;
  mTimeOut.tv_nsec = 0;
}

//--------
FileWatcherOSX::~FileWatcherOSX()
{
  WatchMap::iterator iter = mWatches.begin();
  WatchMap::iterator end = mWatches.end();
  for(; iter != end; ++iter)
    {
    delete iter->second;
    }
  mWatches.clear();
    
  close(mDescriptor);
}

//--------
WatchID FileWatcherOSX::addWatch(const String& directory,
                                 FileWatchListener* watcher,
                                 bool recursive)
{
/*    int fd = open(directory.c_str(), O_RDONLY);
    if(fd == -1)
      perror("open");
        
    EV_SET(&change, fd, EVFILT_VNODE,
         EV_ADD | EV_ENABLE | EV_ONESHOT,
         NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_ATTRIB,
         0, (void*)"testing");
*/
    
  WatchStruct* watch = new WatchStruct(++mLastWatchID, directory, watcher);
  mWatches.insert(std::make_pair(mLastWatchID, watch));
  return mLastWatchID;
}

//--------
void FileWatcherOSX::removeWatch(const String& directory)
{
  WatchMap::iterator iter = mWatches.begin();
  WatchMap::iterator end = mWatches.end();
  for(; iter != end; ++iter)
    {
    if(directory == iter->second->mDirName)
      {
      removeWatch(iter->first);
      return;
      }
    }
}

//--------
void FileWatcherOSX::removeWatch(WatchID watchid)
{
  WatchMap::iterator iter = mWatches.find(watchid);

  if(iter == mWatches.end())
    return;

  WatchStruct* watch = iter->second;
  mWatches.erase(iter);
  
  //inotify_rm_watch(mFD, watchid);
    
  delete watch;
  watch = 0;
}
  
//--------
void FileWatcherOSX::handleAction(WatchStruct* watch,
                                  const String& filename,
                                  unsigned long action)
{
}

};//namespace FW

#endif//FILEWATCHER_PLATFORM_KQUEUE
