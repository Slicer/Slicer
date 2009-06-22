#ifndef __vtkMRMLCustomLayoutNodeCollection_h
#define __vtkMRMLCustomLayoutNodeCollection_h

#include "vtkStringArray.h"
#include "vtkCollection.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkCustomLayoutWin32Header.h"
#include "vtkMRMLCustomLayoutNode.h"

#include <string>
#include <vector>


class VTK_CUSTOMLAYOUT_EXPORT vtkMRMLCustomLayoutNodeCollection : public vtkCollection
{
  public:
  static vtkMRMLCustomLayoutNodeCollection *New();
  vtkTypeRevisionMacro(vtkMRMLCustomLayoutNodeCollection, vtkCollection);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void AddCustomLayout ( vtkMRMLCustomLayoutNode *node );
  virtual void DeleteCustomLayoutByName ( const char *name );
  vtkMRMLCustomLayoutNode *GetLayoutByName (const char *name);

  //BTX
  enum
    {
      LayoutAddedEvent  = 11000,
      LayoutRemovedEvent,
    };
  //ETX
  
 protected:
  vtkMRMLCustomLayoutNodeCollection();
  ~vtkMRMLCustomLayoutNodeCollection();
  vtkMRMLCustomLayoutNodeCollection(const vtkMRMLCustomLayoutNodeCollection&);
  void operator=(const vtkMRMLCustomLayoutNodeCollection&);

};

#endif

