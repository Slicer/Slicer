#ifndef __vtkSlicerSliceLogicCollection_h
#define __vtkSlicerSliceLogicCollection_h

#include "vtkCollection.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerBaseGUIWin32Header.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceLogicCollection : public vtkCollection
{
 public:
    static vtkSlicerSliceLogicCollection *New ( );
    vtkTypeRevisionMacro (vtkSlicerSliceLogicCollection, vtkCollection );

    void AddItem ( vtkSlicerSliceLogic *w ) {
        this->vtkCollection::AddItem ( (vtkObject *)w );
    };

    vtkSlicerSliceLogic *GetNextSlicerSliceLogic ( ) {
        return static_cast<vtkSlicerSliceLogic *>(this->GetNextItemAsObject( ) );
    };

    //BTX
    vtkSlicerSliceLogic *GetNextSlicerSliceLogic ( vtkCollectionSimpleIterator &cookie) {
        return static_cast<vtkSlicerSliceLogic*>(this->GetNextItemAsObject(cookie));
    };
    //ETX

    vtkSlicerSliceLogic *GetNextItem ( ) { return this->GetNextSlicerSliceLogic(); };

 protected:
    vtkSlicerSliceLogicCollection ( );
    ~vtkSlicerSliceLogicCollection ( );

 private:
    vtkSlicerSliceLogicCollection (const vtkSlicerSliceLogicCollection & ); // Not implemented
    void operator=(const vtkSlicerSliceLogicCollection &); // Not implemented
};

#endif
