#ifndef __vtkPluginFilterWatcher_h
#define __vtkPluginFilterWatcher_h

// ModuleDescriptionParser includes
#include <ModuleProcessInformation.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkAlgorithm.h>

#include "vtkSlicerBaseCLIExport.h"

// STD includes
#include <string>

/** \class vtkPluginFilterWatcher
 * \brief Simple mechanism for monitoring the pipeline events of a filter and reporting these events to std::cout
 *
 * vtkPluginFilterWatcher provides a simple mechanism for monitoring the
 * execution of filter.  vtkPluginFilterWatcher is a stack-based object
 * which takes a pointer to a vtkAlgorithm at constructor
 * time. vtkPluginFilterWatcher creates a series of commands that are
 * registered as observers to the specified vtkAlgorithm. The events
 * monitored are:
 *
 *      StartEvent
 *      EndEvent
 *      ProgressEvent
 *
 * The callbacks routines registered for these events emit a simple
 * message to std::cout.
 *
 * Example of use:
 *
 * typedef vtk::BinaryThresholdImageFilter<ImageType> FilterType;
 * FilterType::Pointer thresholdFilter = FilterType::New();
 *
 * vtkPluginFilterWatcher watcher(thresholdFilter, "Threshold");
 *
 * The second argument to the constructor to vtkPluginFilterWatcher is an
 * optional string that is prepended to the event messages. This
 * allows the user to associate the emitted messages to a particular
 * filter/variable.
 *
 *
 * \todo Allow any stream object to be used for the output (not just std::cout)
 *
 */

class vtkPluginWatcherStart;
class vtkPluginWatcherEnd;
class vtkPluginWatcherProgress;

class VTK_SLICER_BASE_CLI_EXPORT vtkPluginFilterWatcher
{
public:
  /** Constructor. Takes a vtkAlgorithm to monitor and an optional
   * comment string that is prepended to each event message. */
  vtkPluginFilterWatcher(vtkAlgorithm* o,
                         const char *comment="",
                         ModuleProcessInformation *inf=nullptr,
                         double fraction = 1.0,
                         double start = 0.0);

  /** Destructor. */
  virtual ~vtkPluginFilterWatcher();

  /** Method to get the name of the class be monitored by this
   *  vtkPluginFilterWatcher */
  const char *GetNameOfClass ()
    {
      return (Process ? Process->GetClassName() : "None");
    }

  /** Methods to control the verbosity of the messages. Quiet
   * reporting limits the information emitted at a ProgressEvent: only filter
   * start and filter end are emitted. */
  void QuietOn() { this->SetQuiet(true);};
  void QuietOff() { this->SetQuiet(false);};

  /** Methods to access member data */
  /** Get a pointer to the process object being watched. */
  vtkAlgorithm *GetProcess () {return Process;};

  /** Set/Get the steps completed. */
  void SetSteps(int val) {Steps=val;};
  int GetSteps() {return Steps;};

  /** Get the start and fraction values. */
  double GetStart() {return this->Start;};
  double GetFraction() {return this->Fraction;};

  /** Set/Get the quiet mode boolean. If false, verbose progress is
    * reported. */
  void SetQuiet(bool val);
  bool GetQuiet() {return Quiet;};

  /** Get the comment for the watcher. */
  std::string GetComment() {return Comment;};

  /** Get the pointer for the process information. */
  ModuleProcessInformation *GetProcessInformation() {return this->ProcessInformation;};


protected:

private:
  int Steps;
  bool Quiet;
  std::string Comment;
  vtkAlgorithm *Process;

  ModuleProcessInformation *ProcessInformation;
  double Fraction;
  double Start;

  vtkPluginWatcherStart *StartFilterCommand;
  vtkPluginWatcherEnd *EndFilterCommand;
  vtkPluginWatcherProgress *ProgressFilterCommand;

  unsigned long StartTag;
  unsigned long EndTag;
  unsigned long ProgressTag;
};

#endif
