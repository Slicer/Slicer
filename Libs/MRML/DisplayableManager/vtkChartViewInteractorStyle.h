

///  vtkChartViewInteractorStyle - interactive manipulation of the camera
/// 
/// This class is based on vtkInteractorStyleTrackballCamera, but includes
/// extra features and event invocations to support extra features of slicer.
//
/// vtkInteractorStyleTrackballCamera allows the user to interactively
/// manipulate (rotate, pan, etc.) the camera, the viewpoint of the scene.  In
/// trackball interaction, the magnitude of the mouse motion is proportional
/// to the camera motion associated with a particular mouse binding. For
/// example, small left-button motions cause small changes in the rotation of
/// the camera around its focal point. For a 3-button mouse, the left button
/// is for rotation, the right button for zooming, the middle button for
/// panning, and ctrl + left button for spinning.  (With fewer mouse buttons,
/// ctrl + shift + left button is for zooming, and shift + left button is for
/// panning.)

/// .SECTION See Also
/// vtkInteractorStyleTrackballActor vtkInteractorStyleJoystickCamera
/// vtkInteractorStyleJoystickActor

#ifndef __vtkChartViewInteractorStyle_h
#define __vtkChartViewInteractorStyle_h

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLCameraNode.h"

// VTK includes
#include "vtkObject.h"
#include "vtkInteractorStyle.h"

#include "vtkMRMLDisplayableManagerWin32Header.h"

class vtkMRMLModelDisplayableManager;
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkChartViewInteractorStyle :
    public vtkInteractorStyle
{
public:
  static vtkChartViewInteractorStyle *New();
  vtkTypeRevisionMacro(vtkChartViewInteractorStyle,vtkInteractorStyle);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// 
  /// Get/Set the CameraNode
  vtkGetObjectMacro ( CameraNode, vtkMRMLCameraNode );
  vtkSetObjectMacro ( CameraNode, vtkMRMLCameraNode );

  /// 
  /// Get/Set the ModelDisplayableManager, for picking
  vtkGetObjectMacro(ModelDisplayableManager, vtkMRMLModelDisplayableManager);
  virtual void SetModelDisplayableManager(vtkMRMLModelDisplayableManager *modelDisplayableManager);

protected:
  vtkChartViewInteractorStyle();
  ~vtkChartViewInteractorStyle();

  vtkMRMLCameraNode *CameraNode;

  /// 
  /// A pointer back to the ModelDisplayableManager, useful for picking
  vtkMRMLModelDisplayableManager * ModelDisplayableManager;
  
private:
  vtkChartViewInteractorStyle(const vtkChartViewInteractorStyle&);  /// Not implemented.
  void operator=(const vtkChartViewInteractorStyle&);  /// Not implemented.
};

#endif
