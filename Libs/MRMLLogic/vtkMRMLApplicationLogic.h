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

  /// Set / Get current Selection node
  vtkMRMLSelectionNode * GetSelectionNode();
  void SetSelectionNode(vtkMRMLSelectionNode * newSelectionNode);

  /// Set / Get current Interaction node
  vtkMRMLInteractionNode * GetInteractionNode();
  void SetInteractionNode(vtkMRMLInteractionNode * newInteractionNode);

  void ProcessMRMLEvents(vtkObject * caller, unsigned long event, void * callData);

protected:

  vtkMRMLApplicationLogic();
  virtual ~vtkMRMLApplicationLogic();
  
private:
  
  vtkMRMLApplicationLogic(const vtkMRMLApplicationLogic&);
  void operator=(const vtkMRMLApplicationLogic&);
  
  //BTX
  class vtkInternal;
  vtkInternal* Internal;
  //ETX

};


#endif

