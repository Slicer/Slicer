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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLApplicationLogic_h
#define __vtkMRMLApplicationLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

#include "vtkMRMLLogicWin32Header.h"

class vtkMRMLColorLogic;
class vtkMRMLSliceNode;
class vtkMRMLSliceLogic;
class vtkMRMLModelHierarchyLogic;
class vtkMRMLSelectionNode;
class vtkMRMLInteractionNode;
class vtkImageData;

class VTK_MRML_LOGIC_EXPORT vtkMRMLApplicationLogic
  : public vtkMRMLAbstractLogic
{
public:
  
  static vtkMRMLApplicationLogic *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLApplicationLogic, vtkMRMLAbstractLogic);

  /// Get current Selection node
  vtkMRMLSelectionNode * GetSelectionNode()const;

  /// Get current Interaction node
  vtkMRMLInteractionNode * GetInteractionNode()const;

  /// All the slice logics in the application
  void SetSliceLogics(vtkCollection* sliceLogics);
  vtkCollection* GetSliceLogics()const;
  vtkMRMLSliceLogic* GetSliceLogic(vtkMRMLSliceNode* sliceNode) const;
  vtkMRMLSliceLogic* GetSliceLogicByLayoutName(const char* layoutName) const;

  /// Get ModelHierarchyLogic
  vtkMRMLModelHierarchyLogic* GetModelHierarchyLogic() const;

  /// Set/Get color logic.
  /// The application typically sets a custom color logic (i.e.
  /// vtkSlicerColorLogic) that contains default color nodes.
  /// By default, a vtkMRMLColorLogic is instantiated.
  void SetColorLogic(vtkMRMLColorLogic* newColorLogic);
  vtkMRMLColorLogic* GetColorLogic()const;

  /// 
  /// Apply the active volumes in the SelectionNode to the slice composite nodes
  /// Perform the default behavior related to selecting a volume
  /// (in this case, making it the background for all SliceCompositeNodes)
  void PropagateVolumeSelection(int fit);
  void PropagateVolumeSelection() {this->PropagateVolumeSelection(1);}


  /// Fit all the volumes into their views
  void FitSliceToAll();

  /// zip the directory into a zip file
  /// Returns success or failure.
  bool Zip(const char *zipFileName, const char *directoryToZip);

  /// unzip the zip file to the current working directory
  /// Returns success or failure.
  bool Unzip(const char *zipFileName, const char *destinationDirectory);

  /// Convert reserved characters into percent notation to avoid issues with filenames
  /// containing things that might be mistaken, for example, for
  /// windows drive letters.  Used internally by SaveSceneToSlicerDataBundleDirectory.
  /// This is not a general purpose implementation; it preseves commonly used
  /// characters for filenames but avoids known issue like slashes or colons.
  /// Ideally a version from vtksys
  /// or similar should be used, but nothing seems to be available.
  /// http://en.wikipedia.org/wiki/Percent-encoding
  /// See http://na-mic.org/Bug/view.php?id=2605
  std::string PercentEncode(std::string s);
  
  /// Save the scene into a self contained directory, sdbDir
  /// If screenShot is not null, use it as the screen shot for a scene view
  /// Returns false if the save failed
  bool SaveSceneToSlicerDataBundleDirectory(const char *sdbDir, vtkImageData *screenShot = NULL);

  /// Open the file into a temp directory and load the scene file 
  /// inside.  Note that the first mrml file found in the extracted 
  /// directory will be used.
  bool OpenSlicerDataBundle(const char *sdbFilePath, const char *temporaryDirectory);

  /// Unpack the file into a temp directory and return the scene file 
  /// inside.  Note that the first mrml file found in the extracted 
  /// directory will be used.
  std::string UnpackSlicerDataBundle(const char *sdbFilePath, const char *temporaryDirectory);
  
  /// Load any default parameter sets into the specified scene
  /// Returns the total number of loaded parameter sets
  static int LoadDefaultParameterSets(vtkMRMLScene * scene,
                                      const std::vector<std::string>& directories);

  /// List of custom events fired by the class.
  enum Events{
    RequestInvokeEvent = vtkCommand::UserEvent + 1
  };
  /// Structure passed as calldata pointer in the RequestEvent invoked event.
  struct InvokeRequest{
    InvokeRequest();
    /// 100ms by default.
    unsigned int Delay;
    /// the caller to call InvokeEvent() on.
    vtkObject* Caller;
    /// The event id of InvokeEvent. ModifiedEvent by default.
    unsigned long EventID;
    /// Optional call data. 0 by default.
    void* CallData;
  };
  /// Conveniently calls an InvokeEvent on an object with a delay.
  void InvokeEventWithDelay(unsigned int delayInMs,
                            vtkObject* caller,
                            unsigned long eventID = vtkCommand::ModifiedEvent,
                            void* callData = 0);

protected:

  vtkMRMLApplicationLogic();
  virtual ~vtkMRMLApplicationLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene *newScene);

  void SetSelectionNode(vtkMRMLSelectionNode* );
  void SetInteractionNode(vtkMRMLInteractionNode* );


private:
  
  vtkMRMLApplicationLogic(const vtkMRMLApplicationLogic&);
  void operator=(const vtkMRMLApplicationLogic&);
  
  class vtkInternal;
  vtkInternal* Internal;

};


#endif

