

#ifndef __vtkMRMLChartViewDisplayableManager_h
#define __vtkMRMLChartViewDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"
#include <vtkMRMLChartViewNode.h>

#include "vtkMRMLDisplayableManagerWin32Header.h"

class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLChartViewDisplayableManager : 
  public vtkMRMLAbstractDisplayableManager
{

public:
  static vtkMRMLChartViewDisplayableManager* New();
  vtkTypeRevisionMacro(vtkMRMLChartViewDisplayableManager,vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  ///
  /// Get MRML ChartViewNode
  vtkMRMLChartViewNode* GetMRMLChartViewNode();

protected:

  vtkMRMLChartViewDisplayableManager();
  virtual ~vtkMRMLChartViewDisplayableManager();

  virtual void AdditionnalInitializeStep();

  virtual void Create();
  
private:

  vtkMRMLChartViewDisplayableManager(const vtkMRMLChartViewDisplayableManager&);// Not implemented
  void operator=(const vtkMRMLChartViewDisplayableManager&);                     // Not Implemented
  
  //BTX
  class vtkInternal;
  vtkInternal * Internal;
  //ETX

};

#endif
