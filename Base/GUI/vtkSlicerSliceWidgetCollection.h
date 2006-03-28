#ifndef __vtkSlicerSliceWidgetCollection_h
#define __vtkSlicerSliceWidgetCollection_h

#include "vtkCollection.h"
#include "vtkSlicerSliceWidget.h"
#include "vtkSlicerBaseGUIWin32Header.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceWidgetCollection : public vtkCollection
{
 public:
    static vtkSlicerSliceWidgetCollection *New ( );
    vtkTypeRevisionMacro (vtkSlicerSliceWidgetCollection, vtkCollection );

    void AddItem ( vtkSlicerSliceWidget *w ) {
        this->vtkCollection::AddItem ( (vtkObject *)w );
    };

    vtkSlicerSliceWidget *GetNextSlicerSliceWidget ( ) {
        return static_cast<vtkSlicerSliceWidget *>(this->GetNextItemAsObject( ) );
    };

    //BTX
    vtkSlicerSliceWidget *GetNextSlicerSliceWidget ( vtkCollectionSimpleIterator &cookie) {
        return static_cast<vtkSlicerSliceWidget*>(this->GetNextItemAsObject(cookie));
    };
    //ETX

    vtkSlicerSliceWidget *GetNextItem ( ) { return this->GetNextSlicerSliceWidget(); };

 protected:
    vtkSlicerSliceWidgetCollection ( );
    ~vtkSlicerSliceWidgetCollection ( );

 private:
    vtkSlicerSliceWidgetCollection (const vtkSlicerSliceWidgetCollection & ); // Not implemented
    void operator=(const vtkSlicerSliceWidgetCollection &); // Not implemented
};

#endif
