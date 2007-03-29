/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the entries of the pop up menu to correspond
// to the currently available volumes.  This widget also has a notion of the current selection
// that can be observed or set externally
//


#ifndef __vtkSlicerVolumeFileHeaderWidget_h
#define __vtkSlicerVolumeFileHeaderWidget_h

#include "vtkSlicerWidget.h"

#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"

class vtkMRMLVolumeHeaderlessStorageNode;
class vtkKWMenuButtonWithLabel;
class vtkKWCheckButton;
class vtkKWPushButton;
class vtkKWDialog;
class vtkKWLabel;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerVolumeFileHeaderWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerVolumeFileHeaderWidget* New();
  vtkTypeRevisionMacro(vtkSlicerVolumeFileHeaderWidget,vtkSlicerWidget);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );

  // Description:
  // get headerless storage node
  vtkMRMLVolumeHeaderlessStorageNode* GetVolumeHeaderlessStorageNode ();

  // Description
  // set info string
  void SetInfo(char * info);

  // Description
  // Invoke the widget dialog
  void Invoke();

//BTX
  enum
    {
      FileHeaderOKEvent = 67310,
    };
//ETX
protected:
  vtkSlicerVolumeFileHeaderWidget();
  virtual ~vtkSlicerVolumeFileHeaderWidget();

  vtkMRMLVolumeHeaderlessStorageNode* VolumeHeaderlessStorageNode;
  
  // Description:
  // Create the widget.
  virtual void CreateWidget();
  vtkKWDialog  *HeaderDialog;

  vtkKWLabel          *InfoLabel;
  vtkKWEntryWithLabel *DimensionEntry0;
  vtkKWEntry          *DimensionEntry1;

  vtkKWEntryWithLabel *SpacingEntry0;
  vtkKWEntry          *SpacingEntry1;

  vtkKWMenuButtonWithLabel *ScanOrderMenu;
  vtkKWEntryWithLabel *NumScalarsEntry;
  vtkKWMenuButtonWithLabel *ScalarTypeMenu;
  
  vtkKWEntryWithLabel *SliceThicknessEntry;
  vtkKWEntryWithLabel *SliceSpacingEntry;

  vtkKWCheckButton *LittleEndianCheckButton;

  vtkKWPushButton *OkButton;
  vtkKWPushButton *CancelButton;

private:


  vtkSlicerVolumeFileHeaderWidget(const vtkSlicerVolumeFileHeaderWidget&); // Not implemented
  void operator=(const vtkSlicerVolumeFileHeaderWidget&); // Not Implemented
};

#endif

