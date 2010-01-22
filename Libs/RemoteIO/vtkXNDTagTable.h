#ifndef __vtkXNDTagTable_h
#define __vtkXNDTagTable_h

#include <vtkRemoteIOConfigure.h>
#include "vtkRemoteIO.h"
#include "vtkObject.h"
#include "vtkTagTable.h"
#include "vtkMRML.h"

class VTK_RemoteIO_EXPORT vtkXNDTagTable : public vtkTagTable
{
  public:
  /// The Usual vtk class functions
  static vtkXNDTagTable *New();
  vtkTypeRevisionMacro(vtkXNDTagTable, vtkTagTable);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  /// 
  /// Adds all default and required tags for the service.
  void Initialize();
  
 private:

 protected:
  vtkXNDTagTable();
  virtual ~vtkXNDTagTable();
  vtkXNDTagTable(const vtkXNDTagTable&);
  void operator=(const vtkXNDTagTable&);

};

#endif






