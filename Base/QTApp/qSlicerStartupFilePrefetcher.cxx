/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "qSlicerStartupFilePrefetcher.h"

// Qt includes
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QSet>

// CTK includes
#include <ctkUtils.h>

// Slicer includes
#include "qSlicerCoreApplication.h"    // For the static path helpers used before there is an application
#include "vtkSlicerConfigure.h"        // For SLICER_REVISION_SPECIFIC_USER_SETTINGS_FILEBASENAME
#include "vtkSlicerVersionConfigure.h" // For Slicer_REVISION

// STD includes
#include <atomic>
#include <thread>
#include <vector>

#if defined(_WIN32)
# ifndef NOMINMAX
#  define NOMINMAX
# endif
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
// Select the PSAPI implementation exported by kernel32, so that linking against
// psapi.lib is not required.
# ifndef PSAPI_VERSION
#  define PSAPI_VERSION 2
# endif
# include <windows.h>
# include <psapi.h>
#else
# include <fcntl.h>  // for open()
# include <unistd.h> // for read(), close(), readlink()
# if defined(Q_OS_MACOS)
#  include <mach-o/dyld.h> // for _dyld_image_count(), _NSGetExecutablePath()
#  include <stdlib.h>      // for realpath()
# else
// dl_iterate_phdr() is a GNU extension, which needs _GNU_SOURCE to be declared. Every C++
// compiler on a glibc system defines that itself, so there is nothing to define here.
#  include <link.h>
# endif
#endif

//----------------------------------------------------------------------------
// Everything the prefetch does that the operating system does differently: finding the
// running executable, asking the loader what is mapped, reading a file so that it is
// cached, comparing two paths, and which libraries are not worth touching. The mechanism
// built on top of these is the same everywhere and lives below, outside any #ifdef.
//----------------------------------------------------------------------------
namespace
{

/// Path in the form the platform's file API takes, so that a worker thread does not
/// convert or allocate per library.
#if defined(_WIN32)
using NativePathString = std::wstring;
#else
using NativePathString = std::string;
#endif

//----------------------------------------------------------------------------
NativePathString toNativePathString(const QString& path)
{
#if defined(_WIN32)
  return QDir::toNativeSeparators(path).toStdWString();
#else
  const QByteArray encoded = QFile::encodeName(path);
  return std::string(encoded.constData(), encoded.size());
#endif
}

//----------------------------------------------------------------------------
/// Full path of the running executable, with forward slashes. Read from the operating
/// system rather than from argv[0], which is not available in WinMain() and need not be
/// a usable path anywhere.
QString applicationFilePath()
{
#if defined(_WIN32)
  std::vector<wchar_t> buffer(32768);
  const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
  if (length == 0 || length >= buffer.size())
  {
    return QString();
  }
  return QDir::fromNativeSeparators(QString::fromWCharArray(buffer.data(), length));
#elif defined(Q_OS_MACOS)
  // The size is an in-out parameter: on failure it is set to what would have been needed,
  // so a second call with the enlarged buffer succeeds.
  std::vector<char> buffer(1024, 0);
  uint32_t size = static_cast<uint32_t>(buffer.size());
  if (_NSGetExecutablePath(buffer.data(), &size) != 0)
  {
    buffer.assign(size + 1, 0);
    if (_NSGetExecutablePath(buffer.data(), &size) != 0)
    {
      return QString();
    }
  }
  // What _NSGetExecutablePath returns may contain symbolic links and relative components,
  // and the recorded list is compared against paths reported by the loader, which are
  // resolved. realpath() allocates the result when given no buffer.
  char* resolved = realpath(buffer.data(), nullptr);
  if (!resolved)
  {
    return QFile::decodeName(buffer.data());
  }
  const QString path = QFile::decodeName(resolved);
  free(resolved);
  return path;
#else
  // Resolved by the kernel, so it is absolute and free of symbolic links. Its length is
  // not reported in advance, hence growing the buffer until it fits.
  std::vector<char> buffer(1024);
  while (true)
  {
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size());
    if (length < 0)
    {
      return QString();
    }
    if (static_cast<size_t>(length) < buffer.size())
    {
      return QFile::decodeName(QByteArray(buffer.data(), static_cast<int>(length)));
    }
    buffer.resize(buffer.size() * 2);
  }
#endif
}

//----------------------------------------------------------------------------
/// Reads a file from beginning to end so that the operating system has it cached, and any
/// on-access scanner has scanned it, by the time the loader asks for it. The contents are
/// discarded into the caller's buffer, which is reused for every file.
///
/// Nothing is mapped and no module is created, so no imports are resolved, no relocations
/// are applied, no initializer runs, and the library does not join the module list.
///
/// Returns the number of bytes read, or -1 if the file could not be opened, which happens
/// to a library that has been removed or replaced since the list was recorded.
qint64 readWholeFile(const NativePathString& path, std::vector<char>& buffer)
{
#if defined(_WIN32)
  // Shared for reading, writing and deletion, so that a real load of the same library, or
  // a build writing over it, is never blocked or refused because of this.
  HANDLE file = CreateFileW(
    path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);
  if (file == INVALID_HANDLE_VALUE)
  {
    return -1;
  }
  qint64 readByteCount = 0;
  while (true)
  {
    DWORD returnedByteCount = 0;
    if (!ReadFile(file, buffer.data(), static_cast<DWORD>(buffer.size()), &returnedByteCount, nullptr) //
        || returnedByteCount == 0)
    {
      // End of file, or a read error that leaves this library simply not anticipated.
      break;
    }
    readByteCount += returnedByteCount;
  }
  CloseHandle(file);
  return readByteCount;
#else
  // O_CLOEXEC because this races with whatever the application may be about to launch.
  // Opening for reading only never blocks another process from writing the file.
  const int file = open(path.c_str(), O_RDONLY | O_CLOEXEC);
  if (file < 0)
  {
    return -1;
  }
  qint64 readByteCount = 0;
  while (true)
  {
    const ssize_t returnedByteCount = read(file, buffer.data(), buffer.size());
    if (returnedByteCount <= 0)
    {
      // End of file, or a read error that leaves this library simply not anticipated.
      // EINTR is not retried: a signal arriving here costs one library, not correctness.
      break;
    }
    readByteCount += returnedByteCount;
  }
  close(file);
  return readByteCount;
#endif
}

//----------------------------------------------------------------------------
/// Normalized form used to compare paths coming from the loader with paths read back from
/// the recorded list. Case is folded only where the file system ignores it: doing so on a
/// case-sensitive file system would make two different libraries compare equal.
QString normalizedPath(const QString& path)
{
#if defined(_WIN32) || defined(Q_OS_MACOS)
  return QDir::fromNativeSeparators(path).toLower();
#else
  return QDir::fromNativeSeparators(path);
#endif
}

//----------------------------------------------------------------------------
/// Directories whose libraries are not worth prefetching, as normalized prefixes ending
/// in a separator. Empty where there is no such directory.
QStringList systemLibraryPrefixes()
{
#if defined(_WIN32)
  // Operating system libraries are signed and their scan verdict is cached system-wide,
  // so scanning them costs time without saving any.
  wchar_t buffer[MAX_PATH + 1] = { 0 };
  const UINT length = GetWindowsDirectoryW(buffer, MAX_PATH);
  if (length == 0 || length > MAX_PATH)
  {
    return QStringList();
  }
  return QStringList(normalizedPath(QString::fromWCharArray(buffer, length)) + "/");
#elif defined(Q_OS_MACOS)
  // Since macOS 11 the system libraries live in the dyld shared cache and no longer exist
  // as files, so the loader reports paths that cannot be opened. Listing them would spend
  // an open() per library only to count a failure.
  return QStringList{ "/usr/lib/", "/system/" };
#else
  // Nothing: a distribution build legitimately loads Qt plugins and module libraries from
  // /usr/lib, and there is no scanning to avoid, so the only reason to skip a library
  // would be that it is likely cached already, which is not knowable from its path.
  return QStringList();
#endif
}

//----------------------------------------------------------------------------
/// Whether the prefetch runs when SLICER_STARTUP_FILE_PREFETCH says nothing.
///
/// On Windows it recovers the on-access virus scan that the loader would otherwise
/// serialize, which is worth seconds on a cold start. Elsewhere there is no such scan to
/// recover and what is left is the file cache: on macOS that measured 0.39 s of a 7.30 s
/// startup, small but consistent, and on Linux it depends on the storage. Both are worth
/// measuring on a given installation before they are spent on every startup.
#if defined(_WIN32)
const bool PrefetchEnabledByDefault = true;
#else
const bool PrefetchEnabledByDefault = false;
#endif

} // end of anonymous namespace

//----------------------------------------------------------------------------
QStringList qSlicerStartupFilePrefetcher::loadedLibraryPaths()
{
  QStringList paths;
#if defined(_WIN32)
  std::vector<HMODULE> modules(1024);
  DWORD bytesNeeded = 0;
  while (true)
  {
    const DWORD bytesAvailable = static_cast<DWORD>(modules.size() * sizeof(HMODULE));
    if (!EnumProcessModules(GetCurrentProcess(), modules.data(), bytesAvailable, &bytesNeeded))
    {
      return paths;
    }
    if (bytesNeeded <= bytesAvailable)
    {
      modules.resize(bytesNeeded / sizeof(HMODULE));
      break;
    }
    // More modules were loaded than the buffer could hold. Grow it and ask again,
    // with some headroom so that libraries loaded in the meantime do not force
    // another round.
    modules.resize(bytesNeeded / sizeof(HMODULE) + 64);
  }

  // Long paths are not exceptional in a build tree, so do not settle for MAX_PATH:
  // GetModuleFileNameEx silently truncates, and a truncated path recorded here would
  // be prefetched as a file that does not exist.
  std::vector<wchar_t> buffer(32768);
  for (HMODULE module : modules)
  {
    const DWORD length = GetModuleFileNameExW(GetCurrentProcess(), module, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length > 0 && length < static_cast<DWORD>(buffer.size()))
    {
      paths << QString::fromWCharArray(buffer.data(), length);
    }
  }
#elif defined(Q_OS_MACOS)
  // Not safe against a library being loaded from another thread, which is why this is
  // only ever called from the thread that runs start() and finish().
  const uint32_t imageCount = _dyld_image_count();
  for (uint32_t imageIndex = 0; imageIndex < imageCount; ++imageIndex)
  {
    const char* name = _dyld_get_image_name(imageIndex);
    // An image built into the dyld shared cache has a path that no file answers to; those
    // are dropped later, by the same prefix list that keeps them out of the queue.
    if (name && name[0] == '/')
    {
      paths << QFile::decodeName(name);
    }
  }
#else
  // dl_iterate_phdr holds the loader lock while it walks the list, so the list cannot
  // change underneath it.
  dl_iterate_phdr(
    [](struct dl_phdr_info* info, size_t, void* data) -> int
    {
      // The main executable is reported with an empty name, and the vDSO with a name that
      // is not a path ("linux-vdso.so.1"). Neither is a file to prefetch.
      if (info->dlpi_name && info->dlpi_name[0] == '/')
      {
        *static_cast<QStringList*>(data) << QFile::decodeName(info->dlpi_name);
      }
      return 0;
    },
    &paths);
#endif
  return paths;
}

namespace
{

//----------------------------------------------------------------------------
/// Upper bound on the number of workers chosen automatically, which
/// SLICER_STARTUP_FILE_PREFETCH_THREADS overrides in either direction.
///
/// On Windows the workers spend nearly all their time blocked in the anti-malware filter,
/// so the threads cost almost no CPU of their own, but the scanning service they wake up
/// does use a core per outstanding request. Beyond 8 the measured speedup flattens out
/// (8.1x at 8 threads, 8.4x at 16), so there is nothing to gain from competing harder
/// with the thread that is actually starting the application. Where there is no scanner
/// the workers are waiting on the storage device instead, and the same bound keeps them
/// from taking the queue away from the loader's own reads.
const unsigned int MaximumWorkerCount = 8;

//----------------------------------------------------------------------------
class FilePrefetchState
{
public:
  /// Libraries to prefetch, in the order they were recorded during the previous run.
  /// That order is whatever the loader reported, which is not documented to be load
  /// order; the workers share one queue, so it only affects which library happens to be
  /// read first.
  std::vector<NativePathString> Paths;
  /// Index of the next entry of Paths to hand out to a worker.
  std::atomic<size_t> NextIndex{ 0 };
  std::vector<std::thread> Workers;

  /// Libraries already loaded when start() was called, lower-cased. These were mapped
  /// by the loader before any application code ran, so they cannot be anticipated and
  /// must be left out of the recorded list.
  QSet<QString> InitiallyLoaded;

  /// Normalized paths of everything queued for prefetching. Compared against what the
  /// application ends up loading, to report how much of this startup the previous run
  /// managed to predict.
  QSet<QString> Queued;

  bool Started = false;

  /// Started by start(), and read afterwards from the workers as well, which is safe
  /// because nothing restarts it once there is another thread to read it. Every time in
  /// the report is taken from it, so the whole report shares one origin: the moment the
  /// prefetch began, which is the process entry point.
  QElapsedTimer Timer;

  /// Everything the report is made of except the fields the workers keep changing.
  /// Written only from the thread that calls start() and finish(), which is the same
  /// one, so it needs no synchronization of its own.
  qSlicerStartupFilePrefetcher::Report Report;

  /// Updated from the worker threads, and read by report() from whichever thread asks.
  std::atomic<size_t> PrefetchedCount{ 0 };
  std::atomic<size_t> PrefetchedBytes{ 0 };
  std::atomic<size_t> FailedCount{ 0 };
  std::atomic<unsigned int> ActiveWorkers{ 0 };
  /// Set by the last worker to run out of work; stays negative while any is still going.
  std::atomic<double> WorkersFinishedTimeMs{ -1.0 };
};

//----------------------------------------------------------------------------
/// Returns the process-wide state.
///
/// The object is intentionally leaked: if the application exits before finish() runs,
/// destroying a still joinable std::thread would call std::terminate(). Letting the
/// state outlive the process instead lets the operating system tear the workers down.
FilePrefetchState* filePrefetchState()
{
  static FilePrefetchState* state = new FilePrefetchState;
  return state;
}

//----------------------------------------------------------------------------
/// Records something the report cannot express as a number. Only called from the thread
/// that drives start() and finish().
void addNote(FilePrefetchState* state, const QString& note)
{
  state->Report.Notes << note;
}

//----------------------------------------------------------------------------
/// Milliseconds from the start of the prefetch, which is what every time in the report is
/// measured from. Zero until start() has started the clock.
double elapsedMs(FilePrefetchState* state)
{
  return state->Timer.isValid() ? state->Timer.nsecsElapsed() / 1e6 : 0.0;
}

//----------------------------------------------------------------------------
/// Renders a time recorded in a report, which is negative when the moment it stands for
/// has not been reached.
QString timeString(double milliseconds)
{
  if (milliseconds < 0.0)
  {
    return QString("n/a");
  }
  return QString("%1 ms").arg(milliseconds, 0, 'f', 0);
}

//----------------------------------------------------------------------------
/// Size of the buffer each worker reads into. The reads are only there to make the
/// operating system, and any scanner watching it, look at the file, so nothing is kept
/// and one buffer per worker is reused for every library.
const size_t ReadBufferSize = 1024 * 1024;

//----------------------------------------------------------------------------
/// Prefetches one library and records what that cost, or that it could not be done.
///
/// The whole file is read rather than a prefix of it: a scanner reads it whole as soon as
/// it is touched at all, so stopping early saves nothing (measured: reading the first
/// megabyte of each library touches 46 MB instead of 62.7 MB and costs the same).
///
/// Failures are counted rather than reported: a library that has been removed or
/// replaced since the list was recorded is simply not anticipated.
void prefetchLibrary(const NativePathString& path, FilePrefetchState* state, std::vector<char>& buffer)
{
  const qint64 readByteCount = readWholeFile(path, buffer);
  if (readByteCount < 0)
  {
    state->FailedCount.fetch_add(1, std::memory_order_relaxed);
    return;
  }
  state->PrefetchedCount.fetch_add(1, std::memory_order_relaxed);
  state->PrefetchedBytes.fetch_add(static_cast<size_t>(readByteCount), std::memory_order_relaxed);
}

//----------------------------------------------------------------------------
void prefetchWorker(FilePrefetchState* state)
{
  // Allocated once per worker rather than per library, so that the prefetch is not
  // measuring its own allocations.
  std::vector<char> buffer(ReadBufferSize);

  while (true)
  {
    const size_t index = state->NextIndex.fetch_add(1, std::memory_order_relaxed);
    if (index >= state->Paths.size())
    {
      break;
    }
    prefetchLibrary(state->Paths[index], state, buffer);
  }

  if (state->ActiveWorkers.fetch_sub(1, std::memory_order_acq_rel) != 1)
  {
    // Not the last worker; the one that finishes last stamps the completion time for
    // all of them.
    return;
  }
  state->WorkersFinishedTimeMs.store(elapsedMs(state), std::memory_order_release);
}

//----------------------------------------------------------------------------
/// Whether a library is one of those systemLibraryPrefixes() rules out.
bool isSystemLibrary(const QString& normalizedLibraryPath, const QStringList& normalizedPrefixes)
{
  for (const QString& prefix : normalizedPrefixes)
  {
    if (normalizedLibraryPath.startsWith(prefix))
    {
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
/// Value of an environment variable, trimmed, or a null string when it is not set.
///
/// qEnvironmentVariable() rather than QCoreApplication or QProcessEnvironment, because
/// this runs at the process entry point, before there is an application to ask.
QString environmentValue(const char* name)
{
  return qEnvironmentVariable(name).trimmed();
}

//----------------------------------------------------------------------------
/// Whether the prefetch may run, from SLICER_STARTUP_FILE_PREFETCH: "1" turns it on, "0"
/// turns it off, and leaving it unset means whatever suits the platform. Any other value
/// is reported and treated as if the variable was not set, so that a misspelled value
/// cannot silently mean the opposite of what was intended.
bool filePrefetchEnabled(FilePrefetchState* state)
{
  const QString requested = environmentValue("SLICER_STARTUP_FILE_PREFETCH");
  if (requested.isEmpty())
  {
    return PrefetchEnabledByDefault;
  }
  if (requested == "1")
  {
    return true;
  }
  if (requested == "0")
  {
    return false;
  }
  addNote(state, QString("SLICER_STARTUP_FILE_PREFETCH is \"%1\", which is neither 1 nor 0; using the default for this platform instead.").arg(requested));
  return PrefetchEnabledByDefault;
}

//----------------------------------------------------------------------------
/// Writes the list of libraries this startup needed, for the next run to replay, and
/// records in the report how it went.
void writeLibraryList(FilePrefetchState* state, const QStringList& librariesToRecord)
{
  if (state->Report.CacheFilePath.isEmpty())
  {
    return;
  }
  const QString cacheDirPath = QFileInfo(state->Report.CacheFilePath).absolutePath();
  if (!QDir().mkpath(cacheDirPath))
  {
    // An installation the user cannot write to, most likely. Prefetching still works on
    // every run that finds a list, it just cannot be refreshed here.
    addNote(state, QString("Cannot create %1, the library list was not updated.").arg(cacheDirPath));
    return;
  }
  if (librariesToRecord.isEmpty())
  {
    // Nothing was loaded between start() and here, which means start() ran too late to
    // be useful. Keep the existing list rather than replacing it with an empty one.
    addNote(state, QString("No library was loaded during startup, the existing library list was kept."));
    return;
  }

  // QSaveFile writes to a temporary file and renames it into place, so an interrupted
  // run cannot leave a truncated list behind.
  QSaveFile cacheFile(state->Report.CacheFilePath);
  if (!cacheFile.open(QIODevice::WriteOnly))
  {
    addNote(state, QString("Cannot write %1, the library list was not updated.").arg(state->Report.CacheFilePath));
    return;
  }
  // Written as UTF-8 rather than through QTextStream, whose default encoding differs
  // between Qt versions, so that paths containing non-ASCII characters survive.
  QString contents;
  contents += QString("# Shared libraries loaded while %1 was starting up.\n").arg(state->Report.ApplicationName);
  contents += "# Read on background threads during the next run, so that the work the operating\n";
  contents += "# system does when a library is first touched (on Windows, scanning it for\n";
  contents += "# malware) happens in parallel instead of blocking the loader.\n";
  contents += QString("# Paths inside the application directory are relative to %1.\n").arg(state->Report.ApplicationHome);
  contents += "# Generated automatically; safe to delete.\n";
  for (const QString& path : librariesToRecord)
  {
    contents += path + "\n";
  }
  const QByteArray encodedContents = contents.toUtf8();
  if (cacheFile.write(encodedContents) != encodedContents.size())
  {
    cacheFile.cancelWriting();
    addNote(state, QString("Failed to write %1, the library list was not updated.").arg(state->Report.CacheFilePath));
    return;
  }
  if (!cacheFile.commit())
  {
    // commit() flushes and renames the temporary file into place, so it can still fail
    // after every write succeeded, for example when the disk is full.
    addNote(state, QString("Failed to save %1, the library list was not updated.").arg(state->Report.CacheFilePath));
    return;
  }
  state->Report.RecordedLibraryCount = librariesToRecord.count();
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
void qSlicerStartupFilePrefetcher::start()
{
  FilePrefetchState* state = filePrefetchState();
  if (state->Started)
  {
    return;
  }
  if (!filePrefetchEnabled(state))
  {
    addNote(state,
            environmentValue("SLICER_STARTUP_FILE_PREFETCH") == "0" //
              ? QString("Turned off by setting the SLICER_STARTUP_FILE_PREFETCH environment variable to 0.")
              : QString("Off by default on this platform; set the SLICER_STARTUP_FILE_PREFETCH environment variable to 1 to measure what it would save here."));
    return;
  }
  state->Started = true;
  state->Report.Enabled = true;
  // First, so that everything the prefetch does is inside what it reports.
  state->Timer.start();

  const QString executablePath = applicationFilePath();
  state->Report.ApplicationName = qSlicerCoreApplication::applicationNameFromExecutablePath(executablePath);
  // Where this executable lives, not where SLICER_HOME points: the variable is set by the
  // launcher, so trusting it would make the recorded list depend on how the application
  // happened to be started.
  state->Report.ApplicationHome = qSlicerCoreApplication::applicationHomeDirectoryFromExecutablePath(executablePath);
  state->Report.CacheFilePath = qSlicerStartupFilePrefetcher::cacheFilePath();

  // Remember what the loader mapped before the application got a chance to run, so
  // that finish() only records the libraries loaded from this point on.
  for (const QString& path : qSlicerStartupFilePrefetcher::loadedLibraryPaths())
  {
    state->InitiallyLoaded.insert(normalizedPath(path));
  }
  state->Report.InitiallyLoadedLibraryCount = state->InitiallyLoaded.count();

  if (state->Report.CacheFilePath.isEmpty())
  {
    addNote(state, QString("No library list location could be determined from %1, nothing was prefetched.").arg(executablePath));
    return;
  }
  QFile cacheFile(state->Report.CacheFilePath);
  if (!cacheFile.open(QIODevice::ReadOnly))
  {
    // Nothing recorded yet: this is the first run after installation, or the first
    // since the list was deleted. It will be written when this startup completes.
    addNote(state, QString("No library list at %1 yet, nothing to prefetch this run.").arg(state->Report.CacheFilePath));
    return;
  }
  const QStringList systemPrefixes = systemLibraryPrefixes();
  while (!cacheFile.atEnd())
  {
    // The list is UTF-8 encoded, so it is decoded explicitly rather than through
    // QTextStream, whose default encoding differs between Qt versions.
    const QString line = QString::fromUtf8(cacheFile.readLine()).trimmed();
    if (line.isEmpty() || line.startsWith('#'))
    {
      continue;
    }
    ++state->Report.ListedLibraryCount;
    // Entries inside the application directory are stored relative to it; the rest are
    // absolute. This is the same convention qSlicerCoreApplication::toSlicerHomeAbsolutePath()
    // applies to settings such as Modules/AdditionalPaths, and it resolves both forms.
    const QString path = ctk::absolutePathFromInternal(line, state->Report.ApplicationHome);
    const QString normalized = normalizedPath(path);
    if (isSystemLibrary(normalized, systemPrefixes))
    {
      ++state->Report.SystemLibraryCount;
      continue;
    }
    if (state->InitiallyLoaded.contains(normalized))
    {
      ++state->Report.AlreadyLoadedLibraryCount;
      continue;
    }
    state->Queued.insert(normalized);
    state->Paths.push_back(toNativePathString(path));
  }
  cacheFile.close();
  state->Report.QueuedLibraryCount = static_cast<int>(state->Paths.size());

  if (state->Paths.empty())
  {
    addNote(state, QString("Every listed library was already loaded, nothing was prefetched."));
    return;
  }

  // An explicit SLICER_STARTUP_FILE_PREFETCH_THREADS is taken at its word, including
  // above MaximumWorkerCount, because the reason to set it is to try a pool size the
  // automatic choice would not have picked.
  unsigned int workerCount = 0;
  const QString requestedWorkerCount = environmentValue("SLICER_STARTUP_FILE_PREFETCH_THREADS");
  if (!requestedWorkerCount.isEmpty())
  {
    bool isNumber = false;
    const int requested = requestedWorkerCount.toInt(&isNumber);
    if (isNumber && requested > 0)
    {
      workerCount = static_cast<unsigned int>(requested);
    }
    else
    {
      addNote(state, QString("SLICER_STARTUP_FILE_PREFETCH_THREADS is \"%1\", which is not a number of threads; choosing one instead.").arg(requestedWorkerCount));
    }
  }
  if (workerCount == 0)
  {
    workerCount = std::thread::hardware_concurrency();
    if (workerCount == 0)
    {
      workerCount = 2;
    }
    workerCount = qBound(2u, workerCount, MaximumWorkerCount);
  }
  // More workers than libraries would leave the extra ones with nothing to do.
  workerCount = qMin(workerCount, static_cast<unsigned int>(state->Paths.size()));

  // Published before the first worker starts, so that the last one to finish can
  // recognize itself and stamp the completion time.
  state->ActiveWorkers.store(workerCount, std::memory_order_release);
  state->Workers.reserve(workerCount);
  for (unsigned int i = 0; i < workerCount; ++i)
  {
    state->Workers.emplace_back(prefetchWorker, state);
  }
  state->Report.WorkerCount = static_cast<int>(workerCount);
  state->Report.WorkersStartedTimeMs = elapsedMs(state);
}

//----------------------------------------------------------------------------
void qSlicerStartupFilePrefetcher::finish()
{
  FilePrefetchState* state = filePrefetchState();
  if (!state->Started)
  {
    return;
  }
  state->Report.FinishStartTimeMs = elapsedMs(state);

  // The workers are detached rather than joined: whatever is left in the list is only
  // needed by libraries loaded on demand after startup, so finishing it is still
  // worthwhile, and waiting for it here would stall the user interface instead. The
  // state they run on is never destroyed, so outliving this call is safe.
  for (std::thread& worker : state->Workers)
  {
    if (worker.joinable())
    {
      worker.detach();
    }
  }
  state->Workers.clear();

  const QStringList systemPrefixes = systemLibraryPrefixes();
  QStringList librariesToRecord;
  for (const QString& path : qSlicerStartupFilePrefetcher::loadedLibraryPaths())
  {
    const QString normalized = normalizedPath(path);
    if (state->InitiallyLoaded.contains(normalized) || isSystemLibrary(normalized, systemPrefixes))
    {
      continue;
    }
    if (state->Queued.contains(normalized))
    {
      ++state->Report.PredictedLibraryCount;
    }
    // Libraries inside the application directory are recorded relative to it, so that
    // the list keeps working when the tree is moved or renamed. Libraries outside it
    // (Qt, and the other build trees in a development build) stay absolute.
    librariesToRecord << ctk::internalPathFromAbsolute(path, state->Report.ApplicationHome);
  }
  state->Report.StartupLibraryCount = librariesToRecord.count();

  writeLibraryList(state, librariesToRecord);

  state->Report.FinishEndTimeMs = elapsedMs(state);
}

//----------------------------------------------------------------------------
qSlicerStartupFilePrefetcher::Report qSlicerStartupFilePrefetcher::report()
{
  FilePrefetchState* state = filePrefetchState();
  qSlicerStartupFilePrefetcher::Report report = state->Report;
  // The workers outlive finish(), so these are read now rather than copied along with
  // the rest, and a report taken later can legitimately show more than an earlier one.
  report.PrefetchedLibraryCount = static_cast<int>(state->PrefetchedCount.load(std::memory_order_relaxed));
  report.FailedLibraryCount = static_cast<int>(state->FailedCount.load(std::memory_order_relaxed));
  report.PrefetchedByteCount = static_cast<qint64>(state->PrefetchedBytes.load(std::memory_order_relaxed));
  report.WorkersFinishedTimeMs = state->WorkersFinishedTimeMs.load(std::memory_order_acquire);
  return report;
}

//----------------------------------------------------------------------------
QString qSlicerStartupFilePrefetcher::cacheFilePath()
{
  const QString executablePath = applicationFilePath();
  const QString name = qSlicerCoreApplication::applicationNameFromExecutablePath(executablePath);
  if (name.isEmpty())
  {
    return QString();
  }

  const QString home = qSlicerCoreApplication::applicationHomeDirectoryFromExecutablePath(executablePath);
  if (home.isEmpty())
  {
    return QString();
  }

  // The list goes where the revision-specific settings go, which is the application's
  // existing answer to where this installation may write, and it is asked of
  // qSlicerCoreApplication rather than worked out again here so that the two cannot
  // drift apart. The static form of the question is the one that can be asked this
  // early: start() runs before there is an application to ask.
  const QString directory = qSlicerCoreApplication::revisionUserSettingsDirectory(home, name);

  // Named like the revision-specific settings file beside it, so that it is recognizable
  // as belonging to the same installation, and so that two revisions sharing one user
  // profile do not replay each other's lists.
  return QDir(directory).filePath(QString("%1%2-%3-StartupFilePrefetch.txt") //
                                    .arg(name)
                                    .arg(SLICER_REVISION_SPECIFIC_USER_SETTINGS_FILEBASENAME)
                                    .arg(Slicer_REVISION));
}

//----------------------------------------------------------------------------
QStringList qSlicerStartupFilePrefetcher::reportLines()
{
  const qSlicerStartupFilePrefetcher::Report report = qSlicerStartupFilePrefetcher::report();

  QStringList lines;
  lines << QString("Startup library prefetching report");
  if (!report.Enabled)
  {
    if (report.Notes.isEmpty())
    {
      lines << QString("  Did not run.");
    }
    for (const QString& note : report.Notes)
    {
      lines << "  " + note;
    }
    return lines;
  }

  lines << QString("  Application: %1 in %2").arg(report.ApplicationName, report.ApplicationHome);
  lines << QString("  Library list: %1").arg(report.CacheFilePath.isEmpty() ? QString("none") : report.CacheFilePath);
  lines << QString("  Loaded before the application started: %1 libraries").arg(report.InitiallyLoadedLibraryCount);
  lines << QString("  Listed for this run: %1 libraries (%2 queued, %3 already loaded, %4 in a system directory)")
             .arg(report.ListedLibraryCount)
             .arg(report.QueuedLibraryCount)
             .arg(report.AlreadyLoadedLibraryCount)
             .arg(report.SystemLibraryCount);
  lines << QString("  Prefetched: %1 of %2 libraries (%3 MB, %4 failed) on %5 threads")
             .arg(report.PrefetchedLibraryCount)
             .arg(report.QueuedLibraryCount)
             .arg(static_cast<double>(report.PrefetchedByteCount) / 1e6, 0, 'f', 1)
             .arg(report.FailedLibraryCount)
             .arg(report.WorkerCount);
  // How much of this startup the previous run managed to predict. A low percentage means
  // the prefetch was working from a stale or unrelated list and cannot have helped,
  // however quickly it ran.
  lines << QString("  Needed by this startup: %1 libraries, %2 of them prefetched (%3%)")
             .arg(report.StartupLibraryCount)
             .arg(report.PredictedLibraryCount)
             .arg(report.StartupLibraryCount > 0 ? (100 * report.PredictedLibraryCount / report.StartupLibraryCount) : 0);
  lines << QString("  Recorded for the next run: %1 libraries").arg(report.RecordedLibraryCount);
  // Said once here, because the line below is measured from the same origin: the moment
  // the prefetch started, which is the process entry point.
  lines << QString("  Measured from the start of the prefetch: workers started at %1, workers finished at %2")
             .arg(timeString(report.WorkersStartedTimeMs), timeString(report.WorkersFinishedTimeMs));
  if (report.WorkersFinishedTimeMs < 0.0 && report.WorkerCount > 0)
  {
    lines << QString("  Some libraries are still being prefetched in the background.");
  }
  // A report taken before finish() has neither, so the duration is only meaningful once
  // both ends have been stamped.
  const double recordingDurationMs = (report.FinishStartTimeMs < 0.0 || report.FinishEndTimeMs < 0.0) //
                                       ? -1.0
                                       : report.FinishEndTimeMs - report.FinishStartTimeMs;
  lines << QString("  Startup completed at %1, recording the library list took %2") //
             .arg(timeString(report.FinishStartTimeMs), timeString(recordingDurationMs));
  for (const QString& note : report.Notes)
  {
    lines << "  " + note;
  }
  return lines;
}
