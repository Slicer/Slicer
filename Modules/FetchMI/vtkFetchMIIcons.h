#ifndef __vtkFetchMIIcons_h
#define __vtkFetchMIIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkFetchMI_ImageData.h"

class vtkFetchMIIcons : public vtkSlicerIcons
{
 public:
    static vtkFetchMIIcons* New ( );
    vtkTypeRevisionMacro ( vtkFetchMIIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( DeleteAllIcon, vtkKWIcon );
    vtkGetObjectMacro ( DeleteSelectedIcon, vtkKWIcon );
    vtkGetObjectMacro ( SelectAllIcon, vtkKWIcon );
    vtkGetObjectMacro ( DeselectAllIcon, vtkKWIcon );    
    vtkGetObjectMacro ( AddNewIcon, vtkKWIcon );
    vtkGetObjectMacro ( SearchIcon, vtkKWIcon );
    vtkGetObjectMacro ( DownloadIcon, vtkKWIcon );
    vtkGetObjectMacro (UploadIcon, vtkKWIcon );
    vtkGetObjectMacro (CopyTagsIcon, vtkKWIcon );
    vtkGetObjectMacro (ApplyTagsIcon, vtkKWIcon );
    vtkGetObjectMacro (TagTableIcon, vtkKWIcon );
    vtkGetObjectMacro (QueryTagsIcon, vtkKWIcon );
    vtkGetObjectMacro (ShowDataTagsIcon, vtkKWIcon );
    
    void AssignImageDataToIcons ( );
    
 protected:
    vtkFetchMIIcons ( );
    ~vtkFetchMIIcons ( );
    vtkKWIcon *AddNewIcon;
    vtkKWIcon *DeleteAllIcon;
    vtkKWIcon *DeleteSelectedIcon;
    vtkKWIcon *SelectAllIcon;
    vtkKWIcon *DeselectAllIcon;
    vtkKWIcon *SearchIcon;
    vtkKWIcon *DownloadIcon;
    vtkKWIcon *UploadIcon;
    vtkKWIcon *CopyTagsIcon;
    vtkKWIcon *ApplyTagsIcon;
    vtkKWIcon *TagTableIcon;
    vtkKWIcon *QueryTagsIcon;
    vtkKWIcon *ShowDataTagsIcon;
    
 private:
    vtkFetchMIIcons (const vtkFetchMIIcons&); // Not implemented
    void operator = ( const vtkFetchMIIcons& ); // Not implemented
    
};

#endif
