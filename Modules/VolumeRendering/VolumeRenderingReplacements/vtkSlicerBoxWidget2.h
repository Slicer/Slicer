/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkSlicerBoxWidget2.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSlicerBoxWidget2 - 3D widget for manipulating a box
// .SECTION Description
// This 3D widget interacts with a vtkSlicerBoxRepresentation class (i.e., it
// handles the events that drive its corresponding representation). The
// representation is assumed to represent a region of interest that is
// represented by an arbitrarily oriented hexahedron (or box) with interior
// face angles of 90 degrees (i.e., orthogonal faces). The representation
// manifests seven handles that can be moused on and manipulated, plus the
// six faces can also be interacted with. The first six handles are placed on
// the six faces, the seventh is in the center of the box. In addition, a
// bounding box outline is shown, the "faces" of which can be selected for
// object rotation or scaling. A nice feature of vtkSlicerBoxWidget2, like any 3D
// widget, will work with the current interactor style. That is, if
// vtkSlicerBoxWidget2 does not handle an event, then all other registered
// observers (including the interactor style) have an opportunity to process
// the event. Otherwise, the vtkSlicerBoxWidget will terminate the processing of
// the event that it handles.
//
// To use this widget, you generally pair it with a vtkSlicerBoxRepresentation
// (or a subclass). Variuos options are available in the representation for 
// controlling how the widget appears, and how the widget functions.
//
// .SECTION Event Bindings
// By default, the widget responds to the following VTK events (i.e., it
// watches the vtkRenderWindowInteractor for these events):
// <pre>
// If one of the seven handles are selected:
//   LeftButtonPressEvent - select the appropriate handle 
//   LeftButtonReleaseEvent - release the currently selected handle 
//   MouseMoveEvent - move the handle
// If one of the faces is selected:
//   LeftButtonPressEvent - select a box face
//   LeftButtonReleaseEvent - release the box face
//   MouseMoveEvent - rotate the box
// In all the cases, independent of what is picked, the widget responds to the 
// following VTK events:
//   MiddleButtonPressEvent - translate the widget
//   MiddleButtonReleaseEvent - release the widget
//   RightButtonPressEvent - scale the widget's representation
//   RightButtonReleaseEvent - stop scaling the widget
//   MouseMoveEvent - scale (if right button) or move (if middle button) the widget
// </pre>
//
// Note that the event bindings described above can be changed using this
// class's vtkWidgetEventTranslator. This class translates VTK events 
// into the vtkSlicerBoxWidget2's widget events:
// <pre>
//   vtkWidgetEvent::Select -- some part of the widget has been selected
//   vtkWidgetEvent::EndSelect -- the selection process has completed
//   vtkWidgetEvent::Scale -- some part of the widget has been selected
//   vtkWidgetEvent::EndScale -- the selection process has completed
//   vtkWidgetEvent::Translate -- some part of the widget has been selected
//   vtkWidgetEvent::EndTranslate -- the selection process has completed
//   vtkWidgetEvent::Move -- a request for motion has been invoked
// </pre>
//
// In turn, when these widget events are processed, the vtkSlicerBoxWidget2
// invokes the following VTK events on itself (which observers can listen for):
// <pre>
//   vtkCommand::StartInteractionEvent (on vtkWidgetEvent::Select)
//   vtkCommand::EndInteractionEvent (on vtkWidgetEvent::EndSelect)
//   vtkCommand::InteractionEvent (on vtkWidgetEvent::Move)
// </pre>

// .SECTION Caveats
// Note that in some cases the widget can be picked even when it is "behind"
// other actors.  This is an intended feature and not a bug.
// 
// This class, and the affiliated vtkSlicerBoxRepresentation, are second generation
// VTK widgets. An earlier version of this functionality was defined in the
// class vtkSlicerBoxWidget.

// .SECTION See Also
// vtkSlicerBoxRepresentation vtkSlicerBoxWidget

#ifndef __vtkSlicerBoxWidget2_h
#define __vtkSlicerBoxWidget2_h

#include "vtkAbstractWidget.h"
#include "vtkVolumeRenderingReplacements.h" 

class vtkSlicerBoxRepresentation;
class vtkHandleWidget;


class VTK_VOLUMERENDERINGREPLACEMENTS_EXPORT vtkSlicerBoxWidget2 : public vtkAbstractWidget
{
public:
  // Description:
  // Instantiate the object.
  static vtkSlicerBoxWidget2 *New();

  // Description:
  // Standard class methods for type information and printing.
  vtkTypeRevisionMacro(vtkSlicerBoxWidget2,vtkAbstractWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify an instance of vtkWidgetRepresentation used to represent this
  // widget in the scene. Note that the representation is a subclass of vtkProp
  // so it can be added to the renderer independent of the widget.
  void SetRepresentation(vtkSlicerBoxRepresentation *r)
    {this->Superclass::SetWidgetRepresentation(reinterpret_cast<vtkWidgetRepresentation*>(r));}
  
  // Description:
  // Control the behavior of the widget (i.e., how it processes
  // events). Translation, rotation, and scaling can all be enabled and
  // disabled.
  vtkSetMacro(TranslationEnabled,int);
  vtkGetMacro(TranslationEnabled,int);
  vtkBooleanMacro(TranslationEnabled,int);
  vtkSetMacro(ScalingEnabled,int);
  vtkGetMacro(ScalingEnabled,int);
  vtkBooleanMacro(ScalingEnabled,int);
  vtkSetMacro(RotationEnabled,int);
  vtkGetMacro(RotationEnabled,int);
  vtkBooleanMacro(RotationEnabled,int);

  // Description:
  // Create the default widget representation if one is not set. By default,
  // this is an instance of the vtkSlicerBoxRepresentation class.
  void CreateDefaultRepresentation();

protected:
  vtkSlicerBoxWidget2();
  ~vtkSlicerBoxWidget2();

//BTX - manage the state of the widget
  int WidgetState;
  enum _WidgetState {Start=0,Active};
//ETX
  
  // These methods handle events
  static void SelectAction(vtkAbstractWidget*);
  static void EndSelectAction(vtkAbstractWidget*);
  static void TranslateAction(vtkAbstractWidget*);
  static void ScaleAction(vtkAbstractWidget*);
  static void MoveAction(vtkAbstractWidget*);

  // Control whether scaling, rotation, and translation are supported
  int TranslationEnabled;
  int ScalingEnabled;
  int RotationEnabled;

private:
  vtkSlicerBoxWidget2(const vtkSlicerBoxWidget2&);  //Not implemented
  void operator=(const vtkSlicerBoxWidget2&);  //Not implemented
};

#endif
