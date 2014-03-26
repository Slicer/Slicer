#ifndef __vtkModelTransformLogic_h
#define __vtkModelTransformLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// VTK includes
#include "vtkObject.h"
#include "vtkMatrix4x4.h"
#include "vtkPolyData.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelTransformNode.h"

// STL includes
#include <string>
#include <vector>
#include <map>
#include <iterator>

#include "vtkSlicerModelTransformModuleLogicExport.h"

// TODO Node registration needs to be done in the Logic. See RegisterNodes

class VTK_SLICER_MODELTRANSFORM_MODULE_LOGIC_EXPORT vtkModelTransformLogic : public vtkSlicerModuleLogic
{
public:
  static vtkModelTransformLogic *New();
  vtkTypeMacro(vtkModelTransformLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent){ this->Superclass::PrintSelf(os, indent); }

protected:
  vtkModelTransformLogic();
  ~vtkModelTransformLogic();
  vtkModelTransformLogic(const vtkModelTransformLogic&);
  void operator=(const vtkModelTransformLogic&);
};


#endif

