#ifndef __vtkSlicerSaveDataWidgetIcons_h
#define __vtkSlicerSaveDataWidgetIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerSaveDataWidget_ImageData.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSaveDataWidgetIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerSaveDataWidgetIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerSaveDataWidgetIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( CheckAll, vtkKWIcon);
    vtkGetObjectMacro ( UncheckAll, vtkKWIcon);
    vtkGetObjectMacro ( CheckModifiedData, vtkKWIcon);
    vtkGetObjectMacro ( CheckModified, vtkKWIcon);

    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerSaveDataWidgetIcons ( );
    ~vtkSlicerSaveDataWidgetIcons ( );
    vtkKWIcon *CheckAll;
    vtkKWIcon *UncheckAll;
    vtkKWIcon *CheckModifiedData;
    vtkKWIcon *CheckModified;    
    
 private:
    vtkSlicerSaveDataWidgetIcons (const vtkSlicerSaveDataWidgetIcons&); /// Not implemented
    void operator = ( const vtkSlicerSaveDataWidgetIcons& ); /// Not implemented
    
};

#endif
