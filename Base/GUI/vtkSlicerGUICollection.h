#ifndef __vtkSlicerGUICollection_h
#define __vtkSlicerGUICollection_h

#include "vtkCollection.h"
#include "vtkSlicerComponentGUI.h"  

#include "vtkSlicerBaseGUIWin32Header.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerGUICollection : public vtkCollection
{
 public:
    static vtkSlicerGUICollection *New ( );
    vtkTypeRevisionMacro (vtkSlicerGUICollection, vtkCollection );

  // Description:
  // Add a GUI to the list.
  void AddItem(vtkSlicerComponentGUI *a) {
    this->vtkCollection::AddItem((vtkObject *)a);};
    
  // Description:
  // Get the next GUI in the list. Return NULL
  // when at the end of the list.
  vtkSlicerComponentGUI *GetNextGUI() {
      return static_cast<vtkSlicerComponentGUI *>(this->GetNextItemAsObject());};


  // Description:
  // Access routine provided for compatibility with previous
  // versions of VTK.  Please use the GetNextGUI() variant
  // where possible.
  vtkSlicerComponentGUI *GetNextItem() { return this->GetNextGUI(); };

  //BTX
  // Description: 
  // Reentrant safe way to get an object in a collection. 
  // Just pass the same cookie back and forth. 
  vtkSlicerComponentGUI *GetNextGUI(vtkCollectionSimpleIterator &cookie) {
    return static_cast<vtkSlicerComponentGUI *>(this->GetNextItemAsObject(cookie));};
  //ETX

protected:
  vtkSlicerGUICollection();
  virtual ~vtkSlicerGUICollection() ;

private:
  // hide the standard AddItem from the user and the compiler.
  void AddItem(vtkObject *o) { this->vtkCollection::AddItem(o); };

private:
  vtkSlicerGUICollection(const vtkSlicerGUICollection&);  // Not implemented.
  void operator=(const vtkSlicerGUICollection&);  // Not implemented.
};


#endif





