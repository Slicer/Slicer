#ifndef __vtkHIDTagTable_h
#define __vtkHIDTagTable_h

#include <vtkRemoteIOConfigure.h>
#include "vtkRemoteIO.h"
#include "vtkObject.h"
#include "vtkTagTable.h"
#include "vtkMRML.h"

class VTK_RemoteIO_EXPORT vtkHIDTagTable : public vtkTagTable
{
  public:
  /// The Usual vtk class functions
  static vtkHIDTagTable *New();
  vtkTypeRevisionMacro(vtkHIDTagTable, vtkTagTable);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  /// 
  /// Adds all default and required tags for the service.
  void Initialize();
  
 private:

 protected:
  vtkHIDTagTable();
  virtual ~vtkHIDTagTable();
  vtkHIDTagTable(const vtkHIDTagTable&);
  void operator=(const vtkHIDTagTable&);

};

#endif






