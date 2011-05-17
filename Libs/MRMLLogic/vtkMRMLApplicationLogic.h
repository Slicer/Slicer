/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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

class VTK_MRML_LOGIC_EXPORT vtkMRMLApplicationLogic : public vtkMRMLAbstractLogic 
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
  void PropagateVolumeSelection() {this->PropagateVolumeSelection(1);}; 


  /// Fit all the volumes into their views
  void FitSliceToAll();

protected:

  vtkMRMLApplicationLogic();
  virtual ~vtkMRMLApplicationLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene *newScene);

  void SetSelectionNode(vtkMRMLSelectionNode* );
  void SetInteractionNode(vtkMRMLInteractionNode* );
private:
  
  vtkMRMLApplicationLogic(const vtkMRMLApplicationLogic&);
  void operator=(const vtkMRMLApplicationLogic&);
  
  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX

};


#endif

