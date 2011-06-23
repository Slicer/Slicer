/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

/// Superclass for displayable manager classes.
/// 
/// A displayable manager class is responsible to represent a
/// MRMLDisplayable node in a renderer.
/// 


#ifndef __vtkMRMLAbstractThreeDViewDisplayableManager_h
#define __vtkMRMLAbstractThreeDViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLViewNode;

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractThreeDViewDisplayableManager :
    public vtkMRMLAbstractDisplayableManager
{
public:
  
  static vtkMRMLAbstractThreeDViewDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLAbstractThreeDViewDisplayableManager,
                       vtkMRMLAbstractDisplayableManager);

  ///
  /// Get MRML ViewNode
  vtkMRMLViewNode * GetMRMLViewNode();

protected:

  vtkMRMLAbstractThreeDViewDisplayableManager();
  virtual ~vtkMRMLAbstractThreeDViewDisplayableManager();

  virtual void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller);

  /// Could be overloaded in DisplayableManager subclass
  virtual void OnMRMLViewNodeModifiedEvent(){}
  
private:

  vtkMRMLAbstractThreeDViewDisplayableManager(const vtkMRMLAbstractThreeDViewDisplayableManager&); // Not implemented
  void operator=(const vtkMRMLAbstractThreeDViewDisplayableManager&);                    // Not implemented
};

#endif

