#ifndef __vtkSlicerRadioButtonWidgetSet_h
#define __vtkSlicerRadioButtonWidgetSet_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWWidgetSet.h"

class vtkSlicerRadioButtonWidget;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerRadioButtonWidgetSet : public vtkKWWidgetSet
{
public:
  static vtkSlicerRadioButtonWidgetSet* New();
  vtkTypeRevisionMacro(vtkSlicerRadioButtonWidgetSet,vtkKWWidgetSet);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add a vtkSlicerRadioButtonWidget to the set.
  // The id has to be unique among the set.
  // Return a pointer to the vtkSlicerRadioButtonWidget, or NULL on error.
  virtual vtkSlicerRadioButtonWidget* AddWidget(int id);

  // Description:
  // Insert a vtkSlicerRadioButtonWidget at a specific position in the set.
  // The id has to be unique among the set.
  // Return a pointer to the vtkSlicerRadioButtonWidget, or NULL on error.
  virtual vtkSlicerRadioButtonWidget* InsertWidget(int id, int pos);

  // Description:
  // Get a vtkSlicerRadioButtonWidget from the set, given its unique id.
  // Return a pointer to the vtkSlicerRadioButtonWidget, or NULL on error.
  virtual vtkSlicerRadioButtonWidget* GetWidget(int id);

protected:
  vtkSlicerRadioButtonWidgetSet() {};
  ~vtkSlicerRadioButtonWidgetSet() {};

  // Helper methods

  virtual vtkKWWidget* AllocateAndCreateWidget();
  virtual vtkKWWidget* InsertWidgetInternal(int id, int pos);

private:
  vtkSlicerRadioButtonWidgetSet(const vtkSlicerRadioButtonWidgetSet&); // Not implemented
  void operator=(const vtkSlicerRadioButtonWidgetSet&); // Not implemented
};

#endif
