#ifndef __vtkMRMLApplicationLogic_h
#define __vtkMRMLApplicationLogic_h

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

#include "vtkMRMLLogicWin32Header.h"

class vtkMRMLSelectionNode;
class vtkMRMLInteractionNode;

class VTK_MRML_LOGIC_EXPORT vtkMRMLApplicationLogic : public vtkMRMLAbstractLogic 
{
public:
  
  static vtkMRMLApplicationLogic *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkMRMLApplicationLogic, vtkMRMLAbstractLogic);

  /// Get current Selection node
  vtkMRMLSelectionNode * GetSelectionNode()const;

  /// Get current Interaction node
  vtkMRMLInteractionNode * GetInteractionNode()const;

  /// All the slice logics in the application
  void SetSliceLogics(vtkCollection* sliceLogics);
  vtkCollection* GetSliceLogics()const;

  /// Apply the active volumes in the SelectionNode to the slice composite nodes
  void PropagateVolumeSelection();

  /// Fit all the volumes into their views
  void FitSliceToAll();

protected:

  vtkMRMLApplicationLogic();
  virtual ~vtkMRMLApplicationLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene *newScene);
  
private:
  
  vtkMRMLApplicationLogic(const vtkMRMLApplicationLogic&);
  void operator=(const vtkMRMLApplicationLogic&);
  
  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX

};


#endif

