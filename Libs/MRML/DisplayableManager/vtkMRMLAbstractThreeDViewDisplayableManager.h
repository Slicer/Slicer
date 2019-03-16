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

#ifndef __vtkMRMLAbstractThreeDViewDisplayableManager_h
#define __vtkMRMLAbstractThreeDViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkMRMLDisplayableManagerExport.h"

class vtkMRMLViewNode;

/// \brief Superclass for displayable manager classes.
///
/// A displayable manager class is responsible to represent a
/// MRMLDisplayable node in a renderer.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractThreeDViewDisplayableManager :
    public vtkMRMLAbstractDisplayableManager
{
public:

  static vtkMRMLAbstractThreeDViewDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkMRMLAbstractThreeDViewDisplayableManager, vtkMRMLAbstractDisplayableManager);

  /// Get MRML ViewNode
  vtkMRMLViewNode * GetMRMLViewNode();


  /// Find display node managed by the displayable manager at a specified world RAS position.
  /// \return Non-zero in case a node is found at the position, 0 otherwise
  virtual int Pick3D(double vtkNotUsed(ras)[3]) { return 0; }

  /// Get the MRML ID of the picked node, returns empty string if no pick
  virtual const char* GetPickedNodeID() { return nullptr; }

protected:

  vtkMRMLAbstractThreeDViewDisplayableManager();
  ~vtkMRMLAbstractThreeDViewDisplayableManager() override;

  void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Could be overloaded in DisplayableManager subclass
  virtual void OnMRMLViewNodeModifiedEvent(){}

  virtual void PassThroughInteractorStyleEvent(int eventid);

private:

  vtkMRMLAbstractThreeDViewDisplayableManager(const vtkMRMLAbstractThreeDViewDisplayableManager&) = delete;
  void operator=(const vtkMRMLAbstractThreeDViewDisplayableManager&) = delete;
};

#endif
