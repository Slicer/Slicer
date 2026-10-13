/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __vtkMRMLOperationProgress_h
#define __vtkMRMLOperationProgress_h

#include "vtkMRMLLogicExport.h"

#include <vtkObject.h>

#include <string>

/// \brief Progress of a long operation, such as loading files.
///
/// The object that performs the operation passes this object as call data of the events that report
/// the start, progress, and end of the operation (for example, vtkMRMLFileIOManager::OperationStartEvent,
/// OperationProgressEvent, and OperationEndEvent).
/// Observers can get the progress and text to display and can request aborting of the operation.
///
/// The same object is used for all events of an operation. Work that is done while an operation is
/// in progress (for example, loading each file of a list) is part of that operation.
/// The overall progress never decreases. ProgressEvent is invoked when the progress, the progress text,
/// or the current file changes. The object contains a full state, which is useful if progress update
/// events are skipped because of limits on maximum rate of progress updates.
/// The latest state is always reported before EndEvent.
///
/// In Python, the observer must specify that the call data is a VTK object:
/// \code{.py}
/// @vtk.calldata_type(vtk.VTK_OBJECT)
/// def onProgress(caller, event, progress):
///   if progress.GetNumberOfFiles() == 0:
///     print(f"In progress: {progress.GetProgressText()}")  # the total amount of work is unknown
///   else:
///     print(f"{progress.GetProgress() * 100:.0f}% {progress.GetProgressText()}")
///
/// fileIOManager = slicer.app.applicationLogic().GetFileIOManager()
/// fileIOManager.AddObserver(fileIOManager.OperationProgressEvent, onProgress)
/// \endcode
class VTK_MRML_LOGIC_EXPORT vtkMRMLOperationProgress : public vtkObject
{
public:
  static vtkMRMLOperationProgress* New();
  vtkTypeMacro(vtkMRMLOperationProgress, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Overall progress of the operation, in the range of 0.0-1.0.
  /// It is only meaningful if the number of files is not zero.
  vtkGetMacro(Progress, double);
  vtkSetClampMacro(Progress, double, 0.0, 1.0);

  /// Short description of the operation (translated, such as "Loading data"). It does not change during the operation.
  vtkGetMacro(Title, std::string);
  vtkSetMacro(Title, std::string);

  /// Text that describes what is being done (translated, can be displayed to the user).
  vtkGetMacro(ProgressText, std::string);
  vtkSetMacro(ProgressText, std::string);

  /// Number of files that the operation processes (for example, number of files to load or nodes to export).
  /// 0 means that the total amount of work is unknown (progress cannot be estimated, for example
  /// a busy indicator can be displayed). The number of files may change while the operation is in progress
  /// (for example, from 0 to 1 when it turns out that progress of loading the file is reported).
  vtkGetMacro(NumberOfFiles, int);
  vtkSetMacro(NumberOfFiles, int);

  /// Index of the file that is currently processed (0-based).
  vtkGetMacro(CurrentFileIndex, int);
  vtkSetMacro(CurrentFileIndex, int);

  /// Progress of processing the current file, in the range of 0.0-1.0.
  /// -1 means that progress of processing the current file is unknown (for example, a busy indicator can be displayed).
  vtkGetMacro(CurrentFileProgress, double);
  vtkSetClampMacro(CurrentFileProgress, double, -1.0, 1.0);

  //@{
  /// For internal use by the operation that reports the progress. Observers must not modify these values.
  /// Part of the overall progress range that the currently processed item covers. It is smaller than the
  /// range of the current file if lists are nested (for example, if loading a file loads a list of other files).
  vtkGetMacro(ProgressRangeMinimum, double);
  vtkSetClampMacro(ProgressRangeMinimum, double, 0.0, 1.0);
  vtkGetMacro(ProgressRangeMaximum, double);
  vtkSetClampMacro(ProgressRangeMaximum, double, 0.0, 1.0);
  /// Part of the overall progress range that the current file covers.
  vtkGetMacro(CurrentFileRangeMinimum, double);
  vtkSetClampMacro(CurrentFileRangeMinimum, double, 0.0, 1.0);
  vtkGetMacro(CurrentFileRangeMaximum, double);
  vtkSetClampMacro(CurrentFileRangeMaximum, double, 0.0, 1.0);
  //@}

  /// True if the operation can be aborted (by setting AbortExecute).
  /// It is set before the operation starts and it does not change while the operation is in progress.
  vtkGetMacro(Abortable, bool);
  vtkSetMacro(Abortable, bool);
  vtkBooleanMacro(Abortable, bool);

  /// Observers can request aborting of the operation by setting this flag (similarly to vtkAlgorithm::AbortExecute).
  /// The operation stops at the next point where it can be stopped (for example, when loading of a file is completed).
  /// Only has effect if the operation is abortable.
  vtkGetMacro(AbortExecute, bool);
  vtkSetMacro(AbortExecute, bool);
  vtkBooleanMacro(AbortExecute, bool);

protected:
  vtkMRMLOperationProgress();
  ~vtkMRMLOperationProgress() override;

  double Progress{ 0.0 };
  std::string Title;
  std::string ProgressText;
  int NumberOfFiles{ 1 };
  int CurrentFileIndex{ 0 };
  double CurrentFileProgress{ -1.0 };
  double ProgressRangeMinimum{ 0.0 };
  double ProgressRangeMaximum{ 1.0 };
  double CurrentFileRangeMinimum{ 0.0 };
  double CurrentFileRangeMaximum{ 1.0 };
  bool Abortable{ false };
  bool AbortExecute{ false };

private:
  vtkMRMLOperationProgress(const vtkMRMLOperationProgress&) = delete;
  void operator=(const vtkMRMLOperationProgress&) = delete;
};

#endif
