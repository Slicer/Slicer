#ifndef __vtkSlicerFileBrowserIcons_h
#define __vtkSlicerFileBrowserIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerFileBrowser_ImageData.h"

class vtkSlicerFileBrowserIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerFileBrowserIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerFileBrowserIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro ( CreateFolderIcon, vtkKWIcon );
    vtkGetObjectMacro ( GoBackIcon, vtkKWIcon );
    vtkGetObjectMacro ( GoForwardIcon, vtkKWIcon );
    vtkGetObjectMacro ( GoUpIcon, vtkKWIcon );
    vtkGetObjectMacro ( FavoritesIcon, vtkKWIcon );
    vtkGetObjectMacro ( HistoryIcon, vtkKWIcon );
    
    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerFileBrowserIcons ( );
    ~vtkSlicerFileBrowserIcons ( );

  vtkKWIcon *CreateFolderIcon;
  vtkKWIcon *GoBackIcon;
  vtkKWIcon *GoForwardIcon;
  vtkKWIcon *GoUpIcon;
  vtkKWIcon *FavoritesIcon;
  vtkKWIcon *HistoryIcon;

 private:
    vtkSlicerFileBrowserIcons (const vtkSlicerFileBrowserIcons&); // Not implemented
    void operator = ( const vtkSlicerFileBrowserIcons& ); // Not implemented
    
};

#endif
