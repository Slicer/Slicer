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
    vtkGetObjectMacro (ApplyTagsIcon, vtkKWIcon );
    vtkGetObjectMacro (RemoveTagsIcon, vtkKWIcon );
    vtkGetObjectMacro (ShowDataTagsIcon, vtkKWIcon );
    vtkGetObjectMacro (RefreshServerIcon, vtkKWIcon );
    vtkGetObjectMacro (HelpIcon, vtkKWIcon );
    
    void AssignImageDataToIcons ( );
    
 protected:
    vtkFetchMIIcons ( );
    ~vtkFetchMIIcons ( );
    vtkKWIcon *HelpIcon;
    vtkKWIcon *AddNewIcon;
    vtkKWIcon *DeleteAllIcon;
    vtkKWIcon *DeleteSelectedIcon;
    vtkKWIcon *SelectAllIcon;
    vtkKWIcon *DeselectAllIcon;
    vtkKWIcon *SearchIcon;
    vtkKWIcon *DownloadIcon;
    vtkKWIcon *UploadIcon;
    vtkKWIcon *ApplyTagsIcon;
    vtkKWIcon *RemoveTagsIcon;
    vtkKWIcon *ShowDataTagsIcon;
    vtkKWIcon *RefreshServerIcon;
    
 private:
    vtkFetchMIIcons (const vtkFetchMIIcons&); // Not implemented
    void operator = ( const vtkFetchMIIcons& ); // Not implemented
    
};

#endif
