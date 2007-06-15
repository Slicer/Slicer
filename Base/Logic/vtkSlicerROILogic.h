// .NAME vtkSlicerROILogic - slicer logic class for ROI manipulation
// .SECTION Description
// This class manages the logic associated ROI


#ifndef __vtkSlicerROILogic_h
#define __vtkSlicerROILogic_h

#include <stdlib.h>

#include "vtkSlicerBaseLogic.h"
#include "vtkSlicerLogic.h"

#include "vtkMRML.h"
#include "vtkMRMLROINode.h" 
#include "vtkMRMLROIListNode.h"

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerROILogic : public vtkSlicerLogic 
{
public:
  // The Usual vtk class functions
  static vtkSlicerROILogic *New();
  vtkTypeRevisionMacro(vtkSlicerROILogic,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Update logic state when MRML scene changes
  void ProcessMRMLEvents(); 

  vtkMRMLROIListNode * AddROIList();

protected:
  vtkSlicerROILogic();
  ~vtkSlicerROILogic();
  vtkSlicerROILogic(const vtkSlicerROILogic&);
  void operator=(const vtkSlicerROILogic&);
};

#endif
