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

#ifndef __qSlicerStartupFilePrefetcher_h
#define __qSlicerStartupFilePrefetcher_h

// Qt includes
#include <QString>
#include <QStringList>

#include "qSlicerBaseQTAppExport.h"

/// \brief Overlaps the work the operating system does when the application's shared
/// libraries are first touched.
///
/// On Windows, the first time a shared library is mapped after it has been created or
/// modified (which includes the first launch after an installation, and the first after
/// a reboot, because the scan verdict cache does not survive one), the anti-malware
/// filter driver scans the whole file before the mapping completes. The scan is CPU
/// bound and dwarfs the cost of reading the file.
///
/// The operating system loader maps libraries one at a time, on the thread that needs
/// them, so those scans are serialized even though the scanning service is perfectly
/// happy to run them in parallel.
///
/// This class recovers that parallelism. On a small pool of background threads it reads
/// each library the application is about to need from beginning to end and throws the
/// contents away. Nothing is mapped and no module is created, so no imports are
/// resolved, no relocations applied, no initializer run, and the library does not join
/// the module list. The only lasting effect is that the operating system has read the
/// file, and whatever it does on first sight of one -- on Windows, scanning it -- is
/// done, so the real load no longer waits for it.
///
/// Measured over 8 cold startups of an installed Slicer, each on a freshly copied tree
/// so that every file was new to the scanner, anticipating the 278 libraries (62.7 MB)
/// that startup goes on to load takes 1.8 seconds of background work and saves about
/// 12 seconds: startup drops from 25.1 to 12.8 seconds, of which module registration,
/// instantiation and loading drop from 11.2 to 4.1 seconds. What remains is mostly out
/// of reach: 229 libraries are mapped by the loader before main() runs, and no list can
/// anticipate those.
///
/// On macOS, measured over 20 copies of an installed Slicer, each launched once before a
/// reboot so that the checks of a first launch were already paid, and once after it, ten
/// with the prefetch on and ten with it off: startup drops from 7.30 to 6.92 seconds, and
/// the two groups do not overlap. All of the saving is after main() -- module
/// registration is 154 ms shorter, instantiation 119 ms and application initialization
/// 93 ms -- while the phase before the entry point is 22 ms longer, the background reads
/// competing for a cache that is already warm for what the loader maps. The first launch
/// of a freshly copied tree spends about 10 seconds before main() on signature checks,
/// but that is paid once per file and survives a reboot, so a later run has nothing there
/// left to recover.
///
/// The whole file is read rather than a prefix of it, and reading is used rather than
/// mapping the file as a data file, because neither alternative measured any better.
/// Reading the first megabyte of each library touches 46 MB instead of 62.7 MB and costs
/// the same, because the scanner reads the whole file as soon as it is touched at all.
/// What is being bought is a scan verdict, not a warm file cache, so how much is asked
/// for barely matters.
///
/// Which libraries to prefetch is not guessed from the directory layout: start() replays
/// the list recorded by finish() during the previous run. That covers loadable modules,
/// Python extension modules, Qt plugins and extensions wherever they are installed, and
/// never touches a library the application does not actually use. The first run after
/// installation has nothing to replay and is not accelerated.
///
/// What the prefetch did is recorded in a Report as it goes, and can be printed once
/// startup is complete with the --report-startup-timing command-line option.
///
/// The mechanism is the same on every platform, but what it recovers is not, so it only
/// runs by default where it is known to pay:
/// - Windows: the serialized virus scan described above. Enabled by default.
/// - macOS: the file cache. The signature and quarantine checks of a first launch are
///   paid once per file and survive a reboot, so unlike the Windows scan they are not a
///   recurring cold-start cost and only the cache is left to recover: 0.39 s of a 7.30 s
///   startup on the installation measured above. Off by default.
/// - Linux: the file cache only, so it is worth something on a cold cache, a network
///   file system or a spinning disk, and nothing on a warm one, unless an on-access
///   scanner is deployed. Off by default for the same reason.
///
/// SLICER_STARTUP_FILE_PREFETCH overrides that per run: 1 turns it on, 0 turns it off.
/// SLICER_STARTUP_FILE_PREFETCH_THREADS sets the size of the pool instead of it being
/// chosen from the number of cores. The class is left out of the build entirely when
/// Slicer_BUILD_STARTUP_FILE_PREFETCH is off.
///
/// Typical usage, from the application entry point:
/// \code
/// int main(int argc, char* argv[])
/// {
///   qSlicerStartupFilePrefetcher::start(); // first statement of main()/WinMain()
///   ...
///   qSlicerStartupFilePrefetcher::finish(); // once startup is complete
/// }
/// \endcode
class Q_SLICER_BASE_QTAPP_EXPORT qSlicerStartupFilePrefetcher
{
public:
  /// \brief What the prefetch did during this startup.
  ///
  /// Filled in as the prefetch progresses and returned by report().
  /// All times are milliseconds from the moment the prefetch started, which is the process
  /// entry point, so that they can be compared directly with each other; a time that has
  /// not been reached is -1. What the loader did before that point is not the prefetch's to
  /// measure, and the startup timing report gives it as its first phase.
  ///
  /// The counts are what makes a report interpretable. Prefetching can be fast and still
  /// useless (a stale list, so PredictedLibraryCount is low), or slow and yet a large win
  /// (cold scans, so the loader was spared that time). Neither can be told from a duration
  /// alone.
  ///
  /// This is plain data with no behavior, so that it can be copied and inspected freely;
  /// reportLines() turns it into readable text.
  struct Report
  {
    /// Whether the prefetch ran at all. False when SLICER_STARTUP_FILE_PREFETCH turned it
    /// off, when it is off by default on this platform, or when start() was never called.
    /// Notes says which.
    bool Enabled = false;

    /// Application the list belongs to, and the directory it was installed or built into,
    /// as resolved by start() from the running executable.
    QString ApplicationName;
    QString ApplicationHome;
    /// The list start() replayed and finish() rewrites. \sa cacheFilePath()
    QString CacheFilePath;

    /// Libraries the loader had already mapped when start() ran. They cannot be
    /// anticipated by any list, so they bound what prefetching can ever cover.
    int InitiallyLoadedLibraryCount = 0;

    /// Entries read from the recorded list, and how they were dispatched: those already
    /// mapped and those in the Windows directory are skipped, the rest are queued.
    int ListedLibraryCount = 0;
    int AlreadyLoadedLibraryCount = 0;
    int SystemLibraryCount = 0;
    int QueuedLibraryCount = 0;

    /// Background threads started to work through the queue, chosen from the number of
    /// cores unless SLICER_STARTUP_FILE_PREFETCH_THREADS asked for a particular number.
    int WorkerCount = 0;

    /// What the workers had done by the time the report was taken. The workers outlive
    /// finish(), so these keep growing after startup is complete.
    int PrefetchedLibraryCount = 0;
    /// Entries that could not be opened, usually because the library has been removed or
    /// renamed since the list was recorded.
    int FailedLibraryCount = 0;
    qint64 PrefetchedByteCount = 0;

    /// Libraries loaded between start() and finish(), that is, the ones this startup
    /// actually needed, and how many of them had been prefetched. The ratio of the two is
    /// how much of this startup the previous run managed to predict; a low ratio means the
    /// list was stale or unrelated and the prefetch cannot have helped much.
    int StartupLibraryCount = 0;
    int PredictedLibraryCount = 0;
    /// Entries written back to the list for the next run, 0 if it could not be written.
    int RecordedLibraryCount = 0;

    /// When the workers were started, which is also what reading the list cost the startup
    /// thread, the two being separated by nothing else.
    double WorkersStartedTimeMs = -1.0;
    /// When the last worker ran out of queued libraries, or -1 if some are still going.
    double WorkersFinishedTimeMs = -1.0;
    /// When finish() began and ended. Their distance is what recording the list cost.
    double FinishStartTimeMs = -1.0;
    double FinishEndTimeMs = -1.0;

    /// Anything worth saying that is not a number: why the prefetch was skipped, a list
    /// that could not be read or written, and so on. Empty when everything went as
    /// expected.
    QStringList Notes;
  };

  /// Start reading, on background threads, the shared libraries that the previous run
  /// of the application loaded after this point. Returns immediately; the work
  /// continues in the background.
  ///
  /// Also records which shared libraries are already loaded, so that finish() can tell
  /// the ones the loader mapped before the application got a chance to run (which are
  /// therefore not worth anticipating) from the ones loaded during startup.
  ///
  /// Meant to be the first statement of main() or WinMain(): every library mapped
  /// between here and finish() is one the next run can anticipate, so anything done
  /// before this call is coverage given away. It takes no argument and depends on
  /// neither QCoreApplication nor qSlicerCoreApplication, neither of which exists that
  /// early; the application name and directory are derived from the running executable.
  static void start();

  /// Record, for the next run, the shared libraries that were loaded since start() was
  /// called, and release the background threads. Returns immediately: reads that have
  /// not been issued yet are left to complete on their own, because they still benefit
  /// the libraries the application loads on demand later on.
  ///
  /// Call once startup is complete. Calling it without a preceding start() does nothing.
  static void finish();

  /// Snapshot of what the prefetch has done so far. Meaningful at any point, but meant to
  /// be taken after finish(), when everything but the tail of the background work is
  /// accounted for.
  static Report report();

  /// report() rendered as one line per fact, ready to be logged. The lines carry no
  /// timestamp or severity of their own, so that whoever prints them decides that.
  static QStringList reportLines();

  /// Path of the file that holds the list of shared libraries recorded by finish().
  ///
  /// It sits next to the revision-specific settings file, named after it, because that
  /// is the application's existing answer to where this installation may write:
  /// the application's own directory when Slicer_STORE_SETTINGS_IN_APPLICATION_HOME_DIR
  /// is on, and the user profile when it is off, which is the configuration meant for
  /// installations whose directory is read-only. The path is worked out from the running
  /// executable rather than asked of
  /// qSlicerCoreApplication::slicerRevisionUserSettingsFilePath(), which resolves to the
  /// same place, because start() runs before there is an application to ask.
  ///
  /// Keying the name on the revision keeps two installations that share one user profile
  /// from replaying each other's lists. Wherever the file ends up, the libraries inside
  /// the application directory are recorded relative to it, following the same convention
  /// as qSlicerCoreApplication::toSlicerHomeRelativePath(), so the list survives the tree
  /// being moved or renamed.
  ///
  /// Returns an empty string if the application directory cannot be determined.
  static QString cacheFilePath();

  /// Full paths of the shared libraries currently loaded in this process, in no
  /// particular order. Returns an empty list on platforms other than Windows.
  static QStringList loadedLibraryPaths();

private:
  qSlicerStartupFilePrefetcher() = delete;
  ~qSlicerStartupFilePrefetcher() = delete;
};

#endif
