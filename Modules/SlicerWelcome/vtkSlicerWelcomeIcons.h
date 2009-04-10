#ifndef __vtkSlicerWelcomeIcons_h
#define __vtkSlicerWelcomeIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerWelcome_ImageData.h"

class vtkSlicerWelcomeIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerWelcomeIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerWelcomeIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );

    vtkGetObjectMacro (LogoIcon, vtkKWIcon );
    vtkGetObjectMacro (CommunityIcon, vtkKWIcon );
    vtkGetObjectMacro (MapIcon, vtkKWIcon );
    vtkGetObjectMacro (EditorIcon, vtkKWIcon );
    vtkGetObjectMacro (HelpIcon, vtkKWIcon );
    vtkGetObjectMacro (LoadIcon, vtkKWIcon );
    vtkGetObjectMacro (SaveIcon, vtkKWIcon );
    vtkGetObjectMacro (ModelsIcon, vtkKWIcon );
    vtkGetObjectMacro (FiducialsIcon, vtkKWIcon );
    vtkGetObjectMacro (SceneIcon, vtkKWIcon );
    vtkGetObjectMacro (SceneSnapshotsIcon, vtkKWIcon );
    vtkGetObjectMacro (SceneTreeIcon, vtkKWIcon );
    vtkGetObjectMacro (VolumesIcon, vtkKWIcon );
    vtkGetObjectMacro (MouseModeIcon, vtkKWIcon );
    vtkGetObjectMacro (ModuleNavIcon, vtkKWIcon );
    vtkGetObjectMacro (ModuleIcon, vtkKWIcon );    
    vtkGetObjectMacro (SlicesControlIcon, vtkKWIcon );
    vtkGetObjectMacro (ViewControlIcon, vtkKWIcon );
    vtkGetObjectMacro ( LayoutIcon, vtkKWIcon );
    vtkGetObjectMacro ( ModuleConfigureIcon, vtkKWIcon );

    void AssignImageDataToIcons ( );
    
 protected:
    vtkSlicerWelcomeIcons ( );
    ~vtkSlicerWelcomeIcons ( );
    vtkKWIcon *LogoIcon;
    vtkKWIcon *CommunityIcon;
    vtkKWIcon *MapIcon;
    vtkKWIcon *EditorIcon;
    vtkKWIcon *HelpIcon;
    vtkKWIcon *SaveIcon;
    vtkKWIcon *LoadIcon;
    vtkKWIcon *ModelsIcon;
    vtkKWIcon *FiducialsIcon;
    vtkKWIcon *SceneIcon;
    vtkKWIcon *SceneSnapshotsIcon;
    vtkKWIcon *SceneTreeIcon;
    vtkKWIcon *VolumesIcon;
    vtkKWIcon *MouseModeIcon;
    vtkKWIcon *ModuleNavIcon;
    vtkKWIcon *ModuleIcon;
    vtkKWIcon *SlicesControlIcon;
    vtkKWIcon *ViewControlIcon;
    vtkKWIcon *LayoutIcon;
    vtkKWIcon *ModuleConfigureIcon;
    
 private:
    vtkSlicerWelcomeIcons (const vtkSlicerWelcomeIcons&); // Not implemented
    void operator = ( const vtkSlicerWelcomeIcons& ); // Not implemented
    
};

#endif
