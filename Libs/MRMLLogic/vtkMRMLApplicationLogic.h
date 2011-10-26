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

  /// 
  /// Apply the active volumes in the SelectionNode to the slice composite nodes
  /// Perform the default behavior related to selecting a volume
  /// (in this case, making it the background for all SliceCompositeNodes)
  void PropagateVolumeSelection(int fit);
  void PropagateVolumeSelection() {this->PropagateVolumeSelection(1);}


  /// Fit all the volumes into their views
  void FitSliceToAll();

  /// Save the scene into a self contained directory and zip it up. Use the
  /// tempDir to stage the scene. If screenShot is not null, use it as the
  /// screen shot for a scene view
  /// Returns the final zip file name on success, null on failure.
  const char * Zip(const char *zipFileName, const char *tempDir, vtkImageData *screenShot = NULL);
  
  /// Save the scene into a self contained directory, sdbDir
  /// If screenShot is not null, use it as the screen shot for a scene view
  /// Returns the path to the self contained directory, null on failure
  const char *SaveSceneToSlicerDataBundleDirectory(const char *sdbDir, vtkImageData *screenShot = NULL);
  
  /// Save directory into a zip file. If sdbDir does not exist, pass that to
  /// SaveSceneToSlicerDataBundleDirectory along with the screenShot.
  /// If zipFileName is null, use the base of the mrml scene's URL
  /// Returns the final zip file name on success, null on failure.
  //const char * ZipSlicerDataBundleDirectory(const char *zipFileName, const char *sdbDir, vtkImageData *screenShot = NULL);

  /// Load any default parameter sets into the specified scene
  /// Returns the total number of loaded parameter sets
  static int LoadDefaultParameterSets(vtkMRMLScene * scene,
                                      const std::vector<std::string>& directories);
  
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

