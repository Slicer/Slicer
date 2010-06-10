#ifndef __vtkSlicerColorLUTIcons_h
#define __vtkSlicerColorLUTIcons_h

#include "vtkKWObject.h"
#include "vtkKWResourceUtilities.h"
#include "vtkKWIcon.h"
#include "vtkSlicerIcons.h"
#include "./Resources/vtkSlicerColorLUT_ImageData.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerColorLUTIcons : public vtkSlicerIcons
{
 public:
    static vtkSlicerColorLUTIcons* New ( );
    vtkTypeRevisionMacro ( vtkSlicerColorLUTIcons, vtkSlicerIcons);
    void PrintSelf ( ostream& os, vtkIndent indent );
    vtkGetObjectMacro ( BlankLUTIcon, vtkKWIcon );
    vtkGetObjectMacro (CartilegeMIdGEMIC3TIcon, vtkKWIcon );
    vtkGetObjectMacro (CartilegeMIdGEMIC15TIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteBlueIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteCool1Icon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteCool2Icon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteCool3Icon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteCyanIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteDesertIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscretefMRIIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscretefMRIPAIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteFullRainbowIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteGreenIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteGreyIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteInvertedGreyIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteIronIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteLabelsIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteMagentaIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteOceanIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteRainbowIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteRandomIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteRandomIntegersIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteRedIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteReverseRainbowIcon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteWarm1Icon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteWarm2Icon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteWarm3Icon, vtkKWIcon );
    vtkGetObjectMacro (DiscreteYellowIcon, vtkKWIcon );
    vtkGetObjectMacro (FreeSurferBlueRedIcon, vtkKWIcon );
    vtkGetObjectMacro (FreeSurferGreenRedIcon, vtkKWIcon );
    vtkGetObjectMacro (FreeSurferHeatIcon, vtkKWIcon );
    vtkGetObjectMacro (FreeSurferRedBlueIcon, vtkKWIcon );
    vtkGetObjectMacro (FreeSurferRedGreenIcon, vtkKWIcon );
    vtkGetObjectMacro (LabelsCustomIcon, vtkKWIcon );
    vtkGetObjectMacro (LabelsNonSemanticIcon, vtkKWIcon );
    vtkGetObjectMacro (LabelsPelvisIcon, vtkKWIcon );
    vtkGetObjectMacro (PETHeatIcon, vtkKWIcon );
    vtkGetObjectMacro (PETMIPIcon, vtkKWIcon );
    vtkGetObjectMacro (PETRainbowIcon, vtkKWIcon );
    vtkGetObjectMacro (ShadeCoolShade1Icon, vtkKWIcon );
    vtkGetObjectMacro (ShadeCoolShade2Icon, vtkKWIcon );
    vtkGetObjectMacro (ShadeCoolShade3Icon, vtkKWIcon );
    vtkGetObjectMacro (ShadeWarmShade1Icon, vtkKWIcon );
    vtkGetObjectMacro (ShadeWarmShade2Icon, vtkKWIcon );
    vtkGetObjectMacro (ShadeWarmShade3Icon, vtkKWIcon );
    vtkGetObjectMacro (SlicerLabels2010Icon, vtkKWIcon );
    vtkGetObjectMacro (SlicerDefaultBrainLUTIcon, vtkKWIcon);
    vtkGetObjectMacro (SlicerBrainLUT2010Icon, vtkKWIcon );
    vtkGetObjectMacro (SlicerShortLUTIcon, vtkKWIcon );
    vtkGetObjectMacro (TintCoolTint1Icon, vtkKWIcon );
    vtkGetObjectMacro (TintCoolTint2Icon, vtkKWIcon );
    vtkGetObjectMacro (TintCoolTint3Icon, vtkKWIcon );
    vtkGetObjectMacro (TintWarmTint1Icon, vtkKWIcon );
    vtkGetObjectMacro (TintWarmTint2Icon, vtkKWIcon );
    vtkGetObjectMacro (TintWarmTint3Icon, vtkKWIcon );    
    vtkGetObjectMacro (AbdomenIcon, vtkKWIcon );
    vtkGetObjectMacro (SPLBrainAtlasIcon, vtkKWIcon );
    vtkGetObjectMacro (GenericColorsIcon, vtkKWIcon );
    vtkGetObjectMacro (GenericAnatomyColorsIcon, vtkKWIcon);

    virtual void SetIconName ( vtkKWIcon *icon, const char *name );
    virtual vtkKWIcon *GetIconByName ( const char *name );
    void AssignImageDataToIcons ( );
    void AssignNamesToIcons ( );
    
 protected:
    vtkSlicerColorLUTIcons ( );
    ~vtkSlicerColorLUTIcons ( );
    vtkKWIcon *GenericColorsIcon;
    vtkKWIcon *GenericAnatomyColorsIcon;
    vtkKWIcon *CartilegeMIdGEMIC3TIcon;
    vtkKWIcon *CartilegeMIdGEMIC15TIcon;
    vtkKWIcon *DiscreteBlueIcon;
    vtkKWIcon *DiscreteCool1Icon;
    vtkKWIcon *DiscreteCool2Icon;
    vtkKWIcon *DiscreteCool3Icon;
    vtkKWIcon *DiscreteCyanIcon;
    vtkKWIcon *DiscreteDesertIcon;
    vtkKWIcon *DiscretefMRIIcon;
    vtkKWIcon *DiscretefMRIPAIcon;
    vtkKWIcon *DiscreteFullRainbowIcon;
    vtkKWIcon *DiscreteGreenIcon;
    vtkKWIcon *DiscreteGreyIcon;
    vtkKWIcon *DiscreteInvertedGreyIcon;
    vtkKWIcon *DiscreteIronIcon;
    vtkKWIcon *DiscreteLabelsIcon;
    vtkKWIcon *DiscreteMagentaIcon;
    vtkKWIcon *DiscreteOceanIcon;
    vtkKWIcon *DiscreteRainbowIcon;
    vtkKWIcon *DiscreteRandomIcon;
    vtkKWIcon *DiscreteRandomIntegersIcon;
    vtkKWIcon *DiscreteRedIcon;
    vtkKWIcon *DiscreteReverseRainbowIcon;
    vtkKWIcon *DiscreteWarm1Icon;
    vtkKWIcon *DiscreteWarm2Icon;
    vtkKWIcon *DiscreteWarm3Icon;
    vtkKWIcon *DiscreteYellowIcon;
    vtkKWIcon *FreeSurferBlueRedIcon;
    vtkKWIcon *FreeSurferGreenRedIcon;
    vtkKWIcon *FreeSurferHeatIcon;
    vtkKWIcon *FreeSurferRedBlueIcon;
    vtkKWIcon *FreeSurferRedGreenIcon;
    vtkKWIcon *LabelsCustomIcon;
    vtkKWIcon *LabelsNonSemanticIcon;
    vtkKWIcon *LabelsPelvisIcon;
    vtkKWIcon *PETHeatIcon;
    vtkKWIcon *PETMIPIcon;
    vtkKWIcon *PETRainbowIcon;
    vtkKWIcon *ShadeCoolShade1Icon;
    vtkKWIcon *ShadeCoolShade2Icon;
    vtkKWIcon *ShadeCoolShade3Icon;
    vtkKWIcon *ShadeWarmShade1Icon;
    vtkKWIcon *ShadeWarmShade2Icon;
    vtkKWIcon *ShadeWarmShade3Icon;
    vtkKWIcon *SlicerBrainLUT2010Icon;
    vtkKWIcon *SlicerDefaultBrainLUTIcon;
    vtkKWIcon *SlicerLabels2010Icon;
    vtkKWIcon *SlicerShortLUTIcon;
    vtkKWIcon *TintCoolTint1Icon;
    vtkKWIcon *TintCoolTint2Icon;
    vtkKWIcon *TintCoolTint3Icon;
    vtkKWIcon *TintWarmTint1Icon;
    vtkKWIcon *TintWarmTint2Icon;
    vtkKWIcon *TintWarmTint3Icon;    
    vtkKWIcon *BlankLUTIcon;
    vtkKWIcon *AbdomenIcon;
    vtkKWIcon *SPLBrainAtlasIcon;
    
 private:
    vtkSlicerColorLUTIcons (const vtkSlicerColorLUTIcons&); /// Not implemented
    void operator = ( const vtkSlicerColorLUTIcons& ); /// Not implemented
    
};

#endif
