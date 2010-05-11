///  vtkSlicerROIDisplayWidget 
/// 
/// Main ROI GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerROIDisplayWidget_h
#define __vtkSlicerROIDisplayWidget_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerROILogic.h"
#include "vtkMRMLROINode.h"


#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkKWChangeColorButton.h"

#include "vtkKWLabel.h"
#include "vtkKWRange.h"
#include "vtkKWExtent.h"

#include "vtkSlicerBoxWidget2.h"
#include "vtkSlicerBoxRepresentation.h"

class vtkRenderWindowInteractor;

class vtkKWMessage;
class vtkSlicerVisibilityIcons;
class vtkKWChangeColorButton;
class vtkKWMenuButtonWithLabel;
class vtkKWCheckButtonWithLabel;


class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerROIDisplayWidget : public vtkSlicerWidget
{
public:
  ///  
  /// Usual vtk class functions 
  static vtkSlicerROIDisplayWidget* New (  );
  vtkTypeRevisionMacro ( vtkSlicerROIDisplayWidget, vtkSlicerWidget );
  void PrintSelf ( ostream& os, vtkIndent indent );

  
   /// 
  /// Set  MRML ModelNode for dscalar colors
  void SetROINode ( vtkMRMLROINode *node );
  
    /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// add observers on widgets in the class
  virtual void AddWidgetObservers();
  
  /// 
  /// removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  /// 
  /// add observers on display node
  virtual void AddMRMLObservers ( );

  /// 
  /// remove observers on display node
  virtual void RemoveMRMLObservers ( );
  
  /// 
  /// Get/Set interactive mode (update while changing ROI range)
  void SetInteractiveMode(int val);
  int GetInteractiveMode();
      
  /// 
  /// Get methods on class members ( no Set methods required. )
  vtkGetObjectMacro ( VisibilityToggle, vtkKWPushButton);
  vtkGetObjectMacro ( VisibilityIcons, vtkSlicerVisibilityIcons);

  /// 
  /// Get/Set individual range extents for the ROI position
  void SetXRangeExtent(double r0, double r1);
  void SetYRangeExtent(double r0, double r1);
  void SetZRangeExtent(double r0, double r1);

  /// 
  /// Set slider resolution for each extent
  void SetXResolution(double r);
  void SetYResolution(double r);
  void SetZResolution(double r);

  ///
  /// Update Widget from the ROI MRML node
  void UpdateWidget();
  
protected:
  vtkSlicerROIDisplayWidget ( );
  virtual ~vtkSlicerROIDisplayWidget ( );

   /// Create the widget.
  virtual void CreateWidget();

  //void UpdateMRML();
  
  vtkMRMLROINode *ROINode;
    
  /// ROI position and Raidus scale
  vtkKWRange *XRange;
  vtkKWRange *YRange;
  vtkKWRange *ZRange;
  
  vtkKWLabel *XLabel;
  vtkKWLabel *YLabel;
  vtkKWLabel *ZLabel;

  /// ROI visibility
  vtkKWPushButton *VisibilityToggle;
  vtkSlicerVisibilityIcons *VisibilityIcons;
  vtkKWLabel *VisibilityLabel;

  /// interactive mode
  vtkKWCheckButtonWithLabel *InteractiveButton;
  
  int ProcessingMRMLEvent;
  int ProcessingWidgetEvent;

   /// 
   /// Colors of the six clipping handles
   double ColorsClippingHandles[6][3];

   /// 
   /// Values for the extent of each of the scales
   double XRangeExtent[2];
   double YRangeExtent[2];
   double ZRangeExtent[2];

   /// 
   /// Values for resolution in each extent
   double XResolution;
   double YResolution;
   double ZResolution;

private:
  vtkSlicerROIDisplayWidget ( const vtkSlicerROIDisplayWidget& ); /// Not implemented.
  void operator = ( const vtkSlicerROIDisplayWidget& ); //Not implemented.
};
#endif
