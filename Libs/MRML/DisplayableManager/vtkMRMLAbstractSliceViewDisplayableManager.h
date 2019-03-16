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

#ifndef __vtkMRMLAbstractSliceViewDisplayableManager_h
#define __vtkMRMLAbstractSliceViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"

#include "vtkMRMLDisplayableManagerExport.h"

class vtkMRMLSliceNode;

/// \brief Superclass for displayable manager classes.
///
/// A displayable manager class is responsible to represent a
/// MRMLDisplayable node in a renderer.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAbstractSliceViewDisplayableManager :
    public vtkMRMLAbstractDisplayableManager
{
public:

  typedef vtkMRMLAbstractSliceViewDisplayableManager Self;

  static vtkMRMLAbstractSliceViewDisplayableManager *New();
  void PrintSelf(ostream& os, vtkIndent indent) override;
  vtkTypeMacro(vtkMRMLAbstractSliceViewDisplayableManager,
                       vtkMRMLAbstractDisplayableManager);

  /// Get MRML SliceNode
  vtkMRMLSliceNode * GetMRMLSliceNode();

  /// Convert device coordinates (display) to XYZ coordinates (viewport).
  /// Parameter \a xyz is double[3]
  /// \sa ConvertDeviceToXYZ(vtkRenderWindowInteractor *, vtkMRMLSliceNode *, double x, double y, double xyz[3])
  void ConvertDeviceToXYZ(double x, double y, double xyz[3]);

  /// Convenience function allowing to convert device coordinates (display) to XYZ coordinates (viewport).
  /// Parameter \a xyz is double[3]
  static void ConvertDeviceToXYZ(vtkRenderWindowInteractor * interactor,
                                 vtkMRMLSliceNode * sliceNode, double x, double y, double xyz[3]);

  /// Convenience function allowing to convert device coordinates (display) to XYZ coordinates (viewport).
  /// Parameter \a xyz is double[3]
  static void ConvertDeviceToXYZ(vtkRenderer * renderer,
    vtkMRMLSliceNode * sliceNode, double x, double y, double xyz[3]);


  /// Convert RAS to XYZ coordinates (viewport).
  /// Parameters \a ras and \a xyz are double[3]. \a xyz[2] is the lightbox id.
  /// \sa ConvertRASToXYZ(vtkMRMLSliceNode * sliceNode, double ras[3], double xyz[3])
  void ConvertRASToXYZ(double ras[3], double xyz[3]);

  /// Convenience function allowing to convert RAS to XYZ coordinates (viewport).
  /// Parameters \a ras and \a xyz are double[3]. \a xyz[2] is the lightbox id.
  static void ConvertRASToXYZ(vtkMRMLSliceNode * sliceNode, double ras[3], double xyz[3]);

  /// Convert XYZ (viewport) to RAS coordinates.
  /// Parameters \a ras and \a xyz are double[3]. \a xyz[2] is the lightbox id.
  /// \sa ConvertXYZToRAS(vtkMRMLSliceNode * sliceNode, double xyz[3], double ras[3])
  void ConvertXYZToRAS(double xyz[3], double ras[3]);

  /// Convenience function allowing to Convert XYZ (viewport) to RAS coordinates.
  /// Parameters \a ras and \a xyz are double[3]. \a xyz[2] is the lightbox id.
  static void ConvertXYZToRAS(vtkMRMLSliceNode * sliceNode, double xyz[3], double ras[3]);

protected:

  vtkMRMLAbstractSliceViewDisplayableManager();
  ~vtkMRMLAbstractSliceViewDisplayableManager() override;

  void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Could be overloaded if DisplayableManager subclass
  virtual void OnMRMLSliceNodeModifiedEvent(){}

private:

  vtkMRMLAbstractSliceViewDisplayableManager(const vtkMRMLAbstractSliceViewDisplayableManager&) = delete;
  void operator=(const vtkMRMLAbstractSliceViewDisplayableManager&) = delete;
};

#endif
