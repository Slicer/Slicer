// .NAME vtkSlicerModuleCollapsibleFrame
// .SECTION Description
// Collapsible frame for module GUI logical sections.

#ifndef __vtkSlicerModuleCollapsibleFrame_h
#define __vtkSlicerModuleCollapsibleFrame_h

// Description:
// This class is a reimplementation of vtkKWFrameWithLabel, with a few
// ajustments, to give the collapsing frames in Slicer's Module GUI panel
// a distinct themed look, feel and behavior.

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWCompositeWidget.h"

class vtkKWFrame;
class vtkKWIcon;
class vtkKWLabel;
class vtkKWLabelWithLabel;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModuleCollapsibleFrame : public vtkKWCompositeWidget
{
 public:
  static vtkSlicerModuleCollapsibleFrame* New();
  vtkTypeRevisionMacro(vtkSlicerModuleCollapsibleFrame, vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the label for the frame.
  void SetLabelText(const char *);
  
  // Description:
  // Get the internal frame.
  vtkGetObjectMacro(Frame, vtkKWFrame);

  // Description:
  // Get the internal frame containing the label.
  vtkGetObjectMacro(LabelFrame, vtkKWFrame);

  // Description:
  // Get the label (frame title).
  virtual vtkKWLabel *GetLabel();

  // Description:
  // Collapse/expand the frame.
  virtual void CollapseFrame();
  virtual void ExpandFrame();
  virtual int IsFrameCollapsed();

  // Description:
  // Set/Get if the frame can be collapsed, i.e. display a button that will
  // let the user collapse the frame. On by default.
  vtkSetMacro(AllowFrameToCollapse, int);
  vtkBooleanMacro(AllowFrameToCollapse, int);
  vtkGetMacro(AllowFrameToCollapse, int);

  // Description:
  // Globally Set/Get if the frame can be collapsed.
  // NOTE: this is a *static* method that defines the behaviour for *all*
  // instances of this class to be created afterwards. This is typically
  // called in your application class or your main executable, before 
  // creating any UI; this will ensure all instances have a consistent look.
  // By default it is globally enabled.
  static int GetDefaultAllowFrameToCollapse();
  static void SetDefaultAllowFrameToCollapse(int);
  static void DefaultAllowFrameToCollapseOn()
    { vtkSlicerModuleCollapsibleFrame::SetDefaultAllowFrameToCollapse(1); };
  static void DefaultAllowFrameToCollapseOff()
    { vtkSlicerModuleCollapsibleFrame::SetDefaultAllowFrameToCollapse(0); };

  // Description:
  // Globally override the case of the label to ensure GUI consistency.
  // This will change the label when SetLabelText() is called.
  // NOTE: this is a *static* method that defines the behaviour for *all*
  // instances of this class to be created afterwards. This is typically
  // called in your application class or your main executable, before 
  // creating any UI; this will ensure all instances have a consistent look.
  // Defaults to LabelCaseUppercaseFirst.
  //BTX
  enum
  {
    LabelCaseUserSpecified = 0,
    LabelCaseUppercaseFirst,
    LabelCaseLowercaseFirst
  };
  //ETX
  static void SetDefaultLabelCase(int v);
  static int GetDefaultLabelCase();
  static void SetDefaultLabelCaseToUserSpecified() 
    { vtkSlicerModuleCollapsibleFrame::SetDefaultLabelCase(
      vtkSlicerModuleCollapsibleFrame::LabelCaseUserSpecified);};
  static void SetDefaultLabelCaseToUppercaseFirst() 
    {vtkSlicerModuleCollapsibleFrame::SetDefaultLabelCase(
      vtkSlicerModuleCollapsibleFrame::LabelCaseUppercaseFirst);};
  static void SetDefaultLabelCaseToLowercaseFirst() 
    {vtkSlicerModuleCollapsibleFrame::SetDefaultLabelCase(
      vtkSlicerModuleCollapsibleFrame::LabelCaseLowercaseFirst);};

  // Description:
  // Globally enable or disable the font weight of the label.
  // NOTE: this is a *static* method that defines the behaviour for *all*
  // instances of this class to be created afterwards. This is typically
  // called in your application class or your main executable, before 
  // creating any UI; this will ensure all instances have a consistent look.
  // By default it is set to bold.
  //BTX
  enum
  {
    LabelFontWeightNormal = 0,
    LabelFontWeightBold
  };
  //ETX
  static void SetDefaultLabelFontWeight(int v);
  static int GetDefaultLabelFontWeight();
  static void SetDefaultLabelFontWeightToNormal() 
    { vtkSlicerModuleCollapsibleFrame::SetDefaultLabelFontWeight(
      vtkSlicerModuleCollapsibleFrame::LabelFontWeightNormal);};
  static void SetDefaultLabelFontWeightToBold() 
    {vtkSlicerModuleCollapsibleFrame::SetDefaultLabelFontWeight(
      vtkSlicerModuleCollapsibleFrame::LabelFontWeightBold);};

  // Description:
  // Set/Get the visibility of a special icon (lock) when the application
  // is in Limited Edition Mode and the label frame is disabled.
  virtual void SetLimitedEditionModeIconVisibility(int);
  vtkBooleanMacro(LimitedEditionModeIconVisibility, int);
  vtkGetMacro(LimitedEditionModeIconVisibility, int);

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Get the drag and drop framework.
  // Override the superclass to allow the frame to be dragged using
  // the label.
  virtual vtkKWDragAndDropTargetSet* GetDragAndDropTargetSet();

  // Description:
  // Override parent's SetWidth/SetHeight to propagate to internal frames.
  // A frame's size can be set explicitly, until something else is packed
  // inside.
  virtual void SetWidth(int);
  virtual int GetWidth();
  virtual void SetHeight(int);
  virtual int GetHeight();

  // Description:
  // Get the internal frame responsible for drawing the collapsible border.
  vtkGetObjectMacro(CollapsibleFrame, vtkKWFrame);
  vtkGetObjectMacro (Icon, vtkKWLabel);
  
  // Description:
  // Callbacks. Internal, do not use.
  virtual void AdjustMarginCallback();
  virtual void CollapseButtonCallback();
  virtual void LabelSingleClickCallback();
  

 protected:
  vtkSlicerModuleCollapsibleFrame();
  ~vtkSlicerModuleCollapsibleFrame();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  vtkKWFrame          *Frame;
  vtkKWFrame          *LabelFrame;
  vtkKWLabelWithLabel *Label;
  vtkKWFrame          *ExternalMarginFrame;
  vtkKWFrame          *InternalMarginFrame;
  vtkKWFrame          *CollapsibleFrame;
  vtkKWLabel          *Icon;
  vtkKWIcon           *IconData;

  int AllowFrameToCollapse;
  int LimitedEditionModeIconVisibility;

  static int DefaultAllowFrameToCollapse;
  static int DefaultLabelFontWeight;
  static int DefaultLabelCase;

  virtual vtkKWLabel *GetLabelIcon();

 private:
  vtkSlicerModuleCollapsibleFrame(const vtkSlicerModuleCollapsibleFrame&); // Not implemented
  void operator=(const vtkSlicerModuleCollapsibleFrame&); // Not implemented
};

#endif
