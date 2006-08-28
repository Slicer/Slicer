#ifndef __vtkSlicerSliceGUICollection_h
#define __vtkSlicerSliceGUICollection_h

#include "vtkCollection.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerBaseGUIWin32Header.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceGUICollection : public vtkCollection
{
 public:
    static vtkSlicerSliceGUICollection *New ( );
    vtkTypeRevisionMacro (vtkSlicerSliceGUICollection, vtkCollection );

    void AddItem ( vtkSlicerSliceGUI *w ) {
        this->vtkCollection::AddItem ( (vtkObject *)w );
    };

    vtkSlicerSliceGUI *GetNextSlicerSliceGUI ( ) {
        return static_cast<vtkSlicerSliceGUI *>(this->GetNextItemAsObject( ) );
    };

    //BTX
    vtkSlicerSliceGUI *GetNextSlicerSliceGUI ( vtkCollectionSimpleIterator &cookie) {
        return static_cast<vtkSlicerSliceGUI*>(this->GetNextItemAsObject(cookie));
    };
    //ETX

    vtkSlicerSliceGUI *GetNextItem ( ) { return this->GetNextSlicerSliceGUI(); };

 protected:
    vtkSlicerSliceGUICollection ( );
    virtual ~vtkSlicerSliceGUICollection ( );

 private:
    vtkSlicerSliceGUICollection (const vtkSlicerSliceGUICollection & ); // Not implemented
    void operator=(const vtkSlicerSliceGUICollection &); // Not implemented
};

#endif
