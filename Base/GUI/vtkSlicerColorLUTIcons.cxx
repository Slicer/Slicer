
#include "vtkObjectFactory.h"
#include "vtkSlicerColorLUTIcons.h"
#include <map>
#include <string>

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerColorLUTIcons );
vtkCxxRevisionMacro ( vtkSlicerColorLUTIcons, "$Revision: 12141 $");

//---------------------------------------------------------------------------
vtkSlicerColorLUTIcons::vtkSlicerColorLUTIcons ( )
{
  this->GenericColorsIcon = vtkKWIcon::New();
  this->GenericAnatomyColorsIcon = vtkKWIcon::New();
  this->AbdomenIcon = vtkKWIcon::New();
  this->SPLBrainAtlasIcon = vtkKWIcon::New();
  this->BlankLUTIcon = vtkKWIcon::New();
  this->CartilegeMIdGEMIC3TIcon = vtkKWIcon::New();
  this->CartilegeMIdGEMIC15TIcon = vtkKWIcon::New();
  this->DiscreteBlueIcon = vtkKWIcon::New();
  this->DiscreteCool1Icon = vtkKWIcon::New();
  this->DiscreteCool2Icon = vtkKWIcon::New();
  this->DiscreteCool3Icon = vtkKWIcon::New();
  this->DiscreteCyanIcon = vtkKWIcon::New();
  this->DiscreteDesertIcon = vtkKWIcon::New();
  this->DiscretefMRIIcon = vtkKWIcon::New();
  this->DiscretefMRIPAIcon = vtkKWIcon::New();
  this->DiscreteFullRainbowIcon = vtkKWIcon::New();
  this->DiscreteGreenIcon = vtkKWIcon::New();
  this->DiscreteGreyIcon = vtkKWIcon::New();
  this->DiscreteInvertedGreyIcon = vtkKWIcon::New();
  this->DiscreteIronIcon = vtkKWIcon::New();
  this->DiscreteLabelsIcon = vtkKWIcon::New();
  this->DiscreteMagentaIcon = vtkKWIcon::New();
  this->DiscreteOceanIcon = vtkKWIcon::New();
  this->DiscreteRainbowIcon = vtkKWIcon::New();
  this->DiscreteRandomIcon = vtkKWIcon::New();
  this->DiscreteRandomIntegersIcon = vtkKWIcon::New();
  this->DiscreteRedIcon = vtkKWIcon::New();
  this->DiscreteReverseRainbowIcon = vtkKWIcon::New();
  this->DiscreteWarm1Icon = vtkKWIcon::New();
  this->DiscreteWarm2Icon = vtkKWIcon::New();
  this->DiscreteWarm3Icon = vtkKWIcon::New();
  this->DiscreteYellowIcon = vtkKWIcon::New();
  this->FreeSurferBlueRedIcon = vtkKWIcon::New();
  this->FreeSurferGreenRedIcon = vtkKWIcon::New();
  this->FreeSurferHeatIcon = vtkKWIcon::New();
  this->FreeSurferRedBlueIcon = vtkKWIcon::New();
  this->FreeSurferRedGreenIcon = vtkKWIcon::New();
  this->LabelsCustomIcon = vtkKWIcon::New();
  this->LabelsNonSemanticIcon = vtkKWIcon::New();
  this->LabelsPelvisIcon = vtkKWIcon::New();
  this->PETHeatIcon = vtkKWIcon::New();
  this->PETMIPIcon = vtkKWIcon::New();
  this->PETRainbowIcon = vtkKWIcon::New();
  this->ShadeCoolShade1Icon = vtkKWIcon::New();
  this->ShadeCoolShade2Icon = vtkKWIcon::New();
  this->ShadeCoolShade3Icon = vtkKWIcon::New();
  this->ShadeWarmShade1Icon = vtkKWIcon::New();
  this->ShadeWarmShade2Icon = vtkKWIcon::New();
  this->ShadeWarmShade3Icon = vtkKWIcon::New();
  this->SlicerLabels2010Icon = vtkKWIcon::New();
  this->SlicerBrainLUT2010Icon = vtkKWIcon::New();
  this->SlicerDefaultBrainLUTIcon = vtkKWIcon::New();
  this->SlicerShortLUTIcon = vtkKWIcon::New();
  this->TintCoolTint1Icon = vtkKWIcon::New();
  this->TintCoolTint2Icon = vtkKWIcon::New();
  this->TintCoolTint3Icon = vtkKWIcon::New();
  this->TintWarmTint1Icon = vtkKWIcon::New();
  this->TintWarmTint2Icon = vtkKWIcon::New();
  this->TintWarmTint3Icon = vtkKWIcon::New();
  this->AssignImageDataToIcons ( );
  this->NamedIcons.clear();
  this->AssignNamesToIcons ();
}


//---------------------------------------------------------------------------
vtkSlicerColorLUTIcons::~vtkSlicerColorLUTIcons ( )
{

  this->NamedIcons.clear();
  
  if ( this->GenericColorsIcon )
    {
    this->GenericColorsIcon->Delete();
    this->GenericColorsIcon = NULL;
    }
  if ( this->GenericAnatomyColorsIcon)
    {
    this->GenericAnatomyColorsIcon->Delete();
    this->GenericAnatomyColorsIcon = NULL;    
    }
  if ( this->AbdomenIcon )
    {
    this->AbdomenIcon->Delete();
    this->AbdomenIcon = NULL;
    }
  if ( this->SPLBrainAtlasIcon )
    {
    this->SPLBrainAtlasIcon->Delete();
    this->SPLBrainAtlasIcon = NULL;
    }
  if ( this->BlankLUTIcon)
    {
    this->BlankLUTIcon->Delete();
    this->BlankLUTIcon = NULL;
    }
  if ( this->CartilegeMIdGEMIC3TIcon)
    {
    this->CartilegeMIdGEMIC3TIcon->Delete();
    this->CartilegeMIdGEMIC3TIcon = NULL;      
    }
  if ( this->CartilegeMIdGEMIC15TIcon)
    {
    this->CartilegeMIdGEMIC15TIcon->Delete();
    this->CartilegeMIdGEMIC15TIcon = NULL;      
    }
  if ( this->DiscreteBlueIcon)
    {
    this->DiscreteBlueIcon->Delete();
    this->DiscreteBlueIcon= NULL;      
    }
  if ( this->DiscreteCool1Icon)
    {
    this->DiscreteCool1Icon->Delete();
    this->DiscreteCool1Icon = NULL;        
    }
  if ( this->DiscreteCool2Icon)
    {
    this->DiscreteCool2Icon->Delete();
    this->DiscreteCool2Icon = NULL;      
    }
  if ( this->DiscreteCool3Icon)
    {
    this->DiscreteCool3Icon->Delete();
    this->DiscreteCool3Icon = NULL;      
    }
  if ( this->DiscreteCyanIcon)
    {
    this->DiscreteCyanIcon->Delete();
    this->DiscreteCyanIcon=NULL;      
    }
  if ( this->DiscreteDesertIcon)
    {
    this->DiscreteDesertIcon->Delete();
    this->DiscreteDesertIcon=NULL;      
    }
  if ( this->DiscretefMRIIcon)
    {
    this->DiscretefMRIIcon->Delete();
    this->DiscretefMRIIcon = NULL;      
    }
  if ( this->DiscretefMRIPAIcon)
    {
    this->DiscretefMRIPAIcon->Delete();
    this->DiscretefMRIPAIcon = NULL;      
    }
  if ( this->DiscreteFullRainbowIcon)
    {
    this->DiscreteFullRainbowIcon->Delete();
    this->DiscreteFullRainbowIcon = NULL;      
    }
  if ( this->DiscreteGreenIcon)
    {
    this->DiscreteGreenIcon->Delete();
    this->DiscreteGreenIcon = NULL;      
    }
  if ( this->DiscreteGreyIcon)
    {
    this->DiscreteGreyIcon->Delete();
    this->DiscreteGreyIcon = NULL;      
    }
  if ( this->DiscreteInvertedGreyIcon)
    {
    this->DiscreteInvertedGreyIcon->Delete();
    this->DiscreteInvertedGreyIcon = NULL;      
    }
  if ( this->DiscreteIronIcon)
    {
    this->DiscreteIronIcon->Delete();
    this->DiscreteIronIcon = NULL;      
    }
  if ( this->DiscreteLabelsIcon)
    {
    this->DiscreteLabelsIcon->Delete();
    this->DiscreteLabelsIcon = NULL;      
    }
  if ( this->DiscreteMagentaIcon)
    {
    this->DiscreteMagentaIcon->Delete();
    this->DiscreteMagentaIcon = NULL;
    }
  if ( this->DiscreteOceanIcon)
    {
    this->DiscreteOceanIcon->Delete();
    this->DiscreteOceanIcon = NULL;      
    }
  if ( this->DiscreteRainbowIcon)
    {
    this->DiscreteRainbowIcon->Delete();
    this->DiscreteRainbowIcon = NULL;      
    }
  if ( this->DiscreteRandomIcon)
    {
    this->DiscreteRandomIcon->Delete();
    this->DiscreteRandomIcon = NULL;      
    }
  if ( this->DiscreteRandomIntegersIcon)
    {
    this->DiscreteRandomIntegersIcon->Delete();
    this->DiscreteRandomIntegersIcon = NULL;      
    }
  if ( this->DiscreteRedIcon)
    {
    this->DiscreteRedIcon->Delete();
    this->DiscreteRedIcon = NULL;      
    }
  if ( this->DiscreteReverseRainbowIcon)
    {
    this->DiscreteReverseRainbowIcon->Delete();
    this->DiscreteReverseRainbowIcon = NULL;      
    }
  if ( this->DiscreteWarm1Icon)
    {
    this->DiscreteWarm1Icon->Delete();
    this->DiscreteWarm1Icon = NULL;
    }
  if ( this->DiscreteWarm2Icon)
    {
    this->DiscreteWarm2Icon->Delete();
    this->DiscreteWarm2Icon = NULL;      
    }
  if ( this->DiscreteWarm3Icon)
    {
    this->DiscreteWarm3Icon->Delete();
    this->DiscreteWarm3Icon = NULL;      
    }
  if ( this->DiscreteYellowIcon)
    {
    this->DiscreteYellowIcon->Delete();
    this->DiscreteYellowIcon = NULL;      
    }
  if ( this->FreeSurferBlueRedIcon)
    {
    this->FreeSurferBlueRedIcon->Delete();
    this->FreeSurferBlueRedIcon = NULL;      
    }
  if ( this->FreeSurferGreenRedIcon)
    {
    this->FreeSurferGreenRedIcon->Delete();
    this->FreeSurferGreenRedIcon = NULL;      
    }
  if ( this->FreeSurferHeatIcon)
    {
    this->FreeSurferHeatIcon->Delete();
    this->FreeSurferHeatIcon = NULL;      
    }
  if ( this->FreeSurferRedBlueIcon)
    {
    this->FreeSurferRedBlueIcon->Delete();
    this->FreeSurferRedBlueIcon = NULL;      
    }
  if ( this->FreeSurferRedGreenIcon)
    {
    this->FreeSurferRedGreenIcon->Delete();
    this->FreeSurferRedGreenIcon = NULL;      
    }
  if ( this->LabelsCustomIcon)
    {
    this->LabelsCustomIcon->Delete();
    this->LabelsCustomIcon = NULL;      
    }
  if ( this->LabelsNonSemanticIcon)
    {
    this->LabelsNonSemanticIcon->Delete();
    this->LabelsNonSemanticIcon = NULL;      
    }
  if ( this->LabelsPelvisIcon)
    {
    this->LabelsPelvisIcon->Delete();
    this->LabelsPelvisIcon = NULL;      
    }
  if ( this->PETHeatIcon)
    {
    this->PETHeatIcon->Delete();
    this->PETHeatIcon = NULL;      
    }
  if ( this->PETMIPIcon)
    {
    this->PETMIPIcon->Delete();
    this->PETMIPIcon= NULL;      
    }
  if ( this->PETRainbowIcon)
    {
    this->PETRainbowIcon->Delete();
    this->PETRainbowIcon = NULL;      
    }
  if ( this->ShadeCoolShade1Icon)
    {
    this->ShadeCoolShade1Icon->Delete();
    this->ShadeCoolShade1Icon = NULL;      
    }
  if ( this->ShadeCoolShade2Icon)
    {
    this->ShadeCoolShade2Icon->Delete();
    this->ShadeCoolShade2Icon = NULL;      
    }
  if ( this->ShadeCoolShade3Icon)
    {
    this->ShadeCoolShade3Icon->Delete();
    this->ShadeCoolShade3Icon = NULL;      
    }
  if ( this->ShadeWarmShade1Icon)
    {
    this->ShadeWarmShade1Icon->Delete();
    this->ShadeWarmShade1Icon = NULL;      
    }
  if ( this->ShadeWarmShade2Icon)
    {
    this->ShadeWarmShade2Icon->Delete();
    this->ShadeWarmShade2Icon = NULL;      
    }
  if ( this->ShadeWarmShade3Icon)
    {
    this->ShadeWarmShade3Icon->Delete();
    this->ShadeWarmShade3Icon = NULL;      
    }
  if ( this->SlicerLabels2010Icon )
    {
    this->SlicerLabels2010Icon->Delete();
    this->SlicerLabels2010Icon = NULL;
    }
  if ( this->SlicerBrainLUT2010Icon)
    {
    this->SlicerBrainLUT2010Icon->Delete();
    this->SlicerBrainLUT2010Icon = NULL;      
    }
  if ( this->SlicerDefaultBrainLUTIcon )
    {
    this->SlicerDefaultBrainLUTIcon->Delete();
    this->SlicerDefaultBrainLUTIcon = NULL;
    }
  if ( this->SlicerShortLUTIcon)
    {
    this->SlicerShortLUTIcon->Delete();
    this->SlicerShortLUTIcon=NULL;      
    }
  if ( this->TintCoolTint1Icon)
    {
    this->TintCoolTint1Icon->Delete();
    this->TintCoolTint1Icon = NULL;      
    }
  if ( this->TintCoolTint2Icon)
    {
    this->TintCoolTint2Icon->Delete();
    this->TintCoolTint2Icon = NULL;      
    }
  if ( this->TintCoolTint3Icon)
    {
    this->TintCoolTint3Icon->Delete();
    this->TintCoolTint3Icon = NULL;      
    }
  if ( this->TintWarmTint1Icon)
    {
    this->TintWarmTint1Icon->Delete();
    this->TintWarmTint1Icon = NULL;      
    }
  if ( this->TintWarmTint2Icon)
    {
    this->TintWarmTint2Icon->Delete();
    this->TintWarmTint2Icon = NULL;      
    }
  if ( this->TintWarmTint3Icon)    
    {
    this->TintWarmTint3Icon->Delete();
    this->TintWarmTint3Icon = NULL;      
    }
}

//---------------------------------------------------------------------------
void vtkSlicerColorLUTIcons::SetIconName ( vtkKWIcon *icon, const char *name )
{

  if ( icon == NULL )
    {
    vtkErrorMacro ( "Got NULL Icon." );
    return;
    }
  if ( name == NULL )
    {
    vtkErrorMacro ( "Got NULL Icon name." );
    return;
    }

  std::map < std::string, vtkKWIcon *>::iterator iter;

  // Is icon already in the map?
  for ( iter = this->NamedIcons.begin(); iter != this->NamedIcons.end(); iter++ )
    {
    if ( iter->second == icon )
      {
      //--- already here; erase it temporarily
      this->NamedIcons.erase(iter);
      break;
      }
    }
  //--- and add new name/icon pair.
  std::string namestr = name;
  this->NamedIcons.insert ( std::make_pair(namestr, icon) );

}



//--------------------------------------------------------------------------
vtkKWIcon* vtkSlicerColorLUTIcons::GetIconByName ( const char *name )
{

  if (name == NULL )
    {
    vtkErrorMacro ( "Got NULL Icon name." );
    if ( this->BlankLUTIcon )
      {
      return ( this->BlankLUTIcon );
      }
    else
      {
      return NULL;
      }
    }

  std::map <std::string, vtkKWIcon *>::iterator iter;
  std::string namestring;
  // Find name in map and pop out
  for ( iter = this->NamedIcons.begin(); iter != this->NamedIcons.end(); iter++ )
    {
    namestring.clear();
    namestring = iter->first;
    if ( !strcmp(namestring.c_str(), name ))
      {
      return ( iter->second );
      }
    }
  if ( this->BlankLUTIcon )
    {
    return (this->BlankLUTIcon);
    }
  else
    {
    vtkErrorMacro ("Got NULL Blank Icon." );
    return ( NULL );
    }
}


//---------------------------------------------------------------------------
void vtkSlicerColorLUTIcons::AssignImageDataToIcons ( )
{

  this->GenericColorsIcon->SetImage ( image_genericColors,
                                      image_genericColors_width,
                                      image_genericColors_height,
                                      image_genericColors_pixel_size,
                                      image_genericColors_length, 0);
  this->GenericAnatomyColorsIcon->SetImage (image_genericAnatomyColors,
                                            image_genericAnatomyColors_width,
                                            image_genericAnatomyColors_height,
                                            image_genericAnatomyColors_pixel_size,
                                            image_genericAnatomyColors_length, 0);
  this->AbdomenIcon->SetImage ( image_abdomenColors,
                                image_abdomenColors_width,
                                image_abdomenColors_height,
                                image_abdomenColors_pixel_size,
                                image_abdomenColors_length, 0);
  this->SPLBrainAtlasIcon->SetImage ( image_SPL_BrainAtlas_ColorFile,
                                      image_SPL_BrainAtlas_ColorFile_width,
                                      image_SPL_BrainAtlas_ColorFile_height,
                                      image_SPL_BrainAtlas_ColorFile_pixel_size,
                                      image_SPL_BrainAtlas_ColorFile_length, 0);
  this->BlankLUTIcon->SetImage ( image_blankLUT,
                                 image_blankLUT_width,
                                 image_blankLUT_height,
                                 image_blankLUT_pixel_size,
                                 image_blankLUT_length, 0);
  this->CartilegeMIdGEMIC3TIcon->SetImage ( image_cartilegeMIdGEMRIC3T,
                                            image_cartilegeMIdGEMRIC3T_width,
                                            image_cartilegeMIdGEMRIC3T_height,
                                            image_cartilegeMIdGEMRIC3T_pixel_size,
                                            image_cartilegeMIdGEMRIC3T_length, 0 );
  this->CartilegeMIdGEMIC15TIcon->SetImage ( image_cartilegeMRIdGEMRIC15T,
                                             image_cartilegeMRIdGEMRIC15T_width,
                                             image_cartilegeMRIdGEMRIC15T_height,
                                             image_cartilegeMRIdGEMRIC15T_pixel_size,
                                             image_cartilegeMRIdGEMRIC15T_length, 0);
  this->DiscreteBlueIcon->SetImage ( image_discreteBlue,
                                     image_discreteBlue_width,
                                     image_discreteBlue_height,
                                     image_discreteBlue_pixel_size,
                                     image_discreteBlue_length, 0);
  this->DiscreteCool1Icon->SetImage ( image_discreteCool1,
                                      image_discreteCool1_width,
                                      image_discreteCool1_height,
                                      image_discreteCool1_pixel_size,
                                      image_discreteCool1_length, 0);
  this->DiscreteCool2Icon->SetImage ( image_discreteCool2,
                                      image_discreteCool2_width,
                                      image_discreteCool2_height,
                                      image_discreteCool2_pixel_size,
                                      image_discreteCool2_length, 0);
  this->DiscreteCool3Icon->SetImage ( image_discreteCool3,
                                      image_discreteCool3_width,
                                      image_discreteCool3_height,
                                      image_discreteCool3_pixel_size,
                                      image_discreteCool3_length, 0);
  this->DiscreteCyanIcon->SetImage ( image_discreteCyan,
                                     image_discreteCyan_width,
                                     image_discreteCyan_height,
                                     image_discreteCyan_pixel_size,
                                     image_discreteCyan_length, 0);
  this->DiscreteDesertIcon->SetImage ( image_discreteDesert,
                                       image_discreteDesert_width,
                                       image_discreteDesert_height,
                                       image_discreteDesert_pixel_size,
                                       image_discreteDesert_length, 0);
  this->DiscretefMRIIcon->SetImage ( image_discretefMRI,
                                     image_discretefMRI_width,
                                     image_discretefMRI_height,
                                     image_discretefMRI_pixel_size,
                                     image_discretefMRI_length, 0);
  this->DiscretefMRIPAIcon->SetImage ( image_discretefMRIPA,
                                       image_discretefMRIPA_width,
                                       image_discretefMRIPA_height,
                                       image_discretefMRIPA_pixel_size,
                                       image_discretefMRIPA_length, 0);
  this->DiscreteFullRainbowIcon->SetImage ( image_discreteFullRainbow,
                                            image_discreteFullRainbow_width,
                                            image_discreteFullRainbow_height,
                                            image_discreteFullRainbow_pixel_size,
                                            image_discreteFullRainbow_length, 0);
  this->DiscreteGreenIcon->SetImage ( image_discreteGreen,
                                      image_discreteGreen_width,
                                      image_discreteGreen_height,
                                      image_discreteGreen_pixel_size,
                                      image_discreteGreen_length, 0);
  this->DiscreteGreyIcon->SetImage ( image_discreteGrey,
                                     image_discreteGrey_width,
                                     image_discreteGrey_height,
                                     image_discreteGrey_pixel_size,
                                     image_discreteGrey_length, 0);
    this->DiscreteInvertedGreyIcon->SetImage ( image_discreteInvertedGrey,
                                             image_discreteInvertedGrey_width,
                                             image_discreteInvertedGrey_height,
                                             image_discreteInvertedGrey_pixel_size,
                                             image_discreteInvertedGrey_length, 0);
    this->DiscreteIronIcon->SetImage ( image_discreteIron,
                                       image_discreteIron_width,
                                       image_discreteIron_height,
                                       image_discreteIron_pixel_size,
                                       image_discreteIron_length, 0);
    this->DiscreteLabelsIcon->SetImage ( image_discretelabels,
                                         image_discretelabels_width,
                                         image_discretelabels_height,
                                         image_discretelabels_pixel_size,
                                         image_discretelabels_length, 0);
    this->DiscreteMagentaIcon->SetImage ( image_discreteMagenta,
                                          image_discreteMagenta_width,
                                          image_discreteMagenta_height,
                                          image_discreteMagenta_pixel_size,
                                          image_discreteMagenta_length, 0);
    this->DiscreteOceanIcon->SetImage ( image_discreteOcean,
                                        image_discreteOcean_width,
                                        image_discreteOcean_height,
                                        image_discreteOcean_pixel_size,
                                        image_discreteOcean_length, 0);
    this->DiscreteRainbowIcon->SetImage ( image_discreteRainbow,
                                          image_discreteRainbow_width,
                                          image_discreteRainbow_height,
                                          image_discreteRainbow_pixel_size,
                                          image_discreteRainbow_length, 0);
    this->DiscreteRandomIcon->SetImage ( image_discreteRandom,
                                         image_discreteRandom_width,
                                         image_discreteRandom_height,
                                         image_discreteRandom_pixel_size,
                                         image_discreteRandom_length, 0);
    this->DiscreteRandomIntegersIcon->SetImage ( image_discreteRandomIntegers,
                                                 image_discreteRandomIntegers_width,
                                                 image_discreteRandomIntegers_height,
                                                 image_discreteRandomIntegers_pixel_size,
                                                 image_discreteRandomIntegers_length, 0);
    this->DiscreteRedIcon->SetImage ( image_discreteRed,
                                      image_discreteRed_width,
                                      image_discreteRed_height,
                                      image_discreteRed_pixel_size,
                                      image_discreteRed_length, 0);
    this->DiscreteReverseRainbowIcon->SetImage ( image_discreteReverseRainbow,
                                                 image_discreteReverseRainbow_width,
                                                 image_discreteReverseRainbow_height,
                                                 image_discreteReverseRainbow_pixel_size,
                                                 image_discreteReverseRainbow_length, 0);
    this->DiscreteWarm1Icon->SetImage ( image_discreteWarm1,
                                        image_discreteWarm1_width,
                                        image_discreteWarm1_height,
                                        image_discreteWarm1_pixel_size,
                                        image_discreteWarm1_length, 0);
    this->DiscreteWarm2Icon->SetImage ( image_discreteWarm2,
                                        image_discreteWarm2_width,
                                        image_discreteWarm2_height,
                                        image_discreteWarm2_pixel_size,
                                        image_discreteWarm2_length, 0);
    this->DiscreteWarm3Icon->SetImage ( image_discreteWarm3,
                                        image_discreteWarm3_width,
                                        image_discreteWarm3_height,
                                        image_discreteWarm3_pixel_size,
                                        image_discreteWarm3_length, 0);
    this->DiscreteYellowIcon->SetImage ( image_discreteYellow,
                                         image_discreteYellow_width,
                                         image_discreteYellow_height,
                                         image_discreteYellow_pixel_size,
                                         image_discreteYellow_length, 0);
    this->FreeSurferBlueRedIcon->SetImage ( image_freeSurferBlueRed,
                                            image_freeSurferBlueRed_width,
                                            image_freeSurferBlueRed_height,
                                            image_freeSurferBlueRed_pixel_size,
                                            image_freeSurferBlueRed_length, 0);
    this->FreeSurferGreenRedIcon->SetImage ( image_freeSurferGreenRed,
                                             image_freeSurferGreenRed_width,
                                             image_freeSurferGreenRed_height,
                                             image_freeSurferGreenRed_pixel_size,
                                             image_freeSurferGreenRed_length, 0);
    this->FreeSurferHeatIcon->SetImage ( image_freeSurferHeat,
                                         image_freeSurferHeat_width,
                                         image_freeSurferHeat_height,
                                         image_freeSurferHeat_pixel_size,
                                         image_freeSurferHeat_length, 0);
    this->FreeSurferRedBlueIcon->SetImage ( image_freeSurferRedBlue,
                                            image_freeSurferRedBlue_width,
                                            image_freeSurferRedBlue_height,
                                            image_freeSurferRedBlue_pixel_size,
                                            image_freeSurferRedBlue_length, 0);
    this->FreeSurferRedGreenIcon->SetImage ( image_freeSurferRedGreen,
                                             image_freeSurferRedGreen_width,
                                             image_freeSurferRedGreen_height,
                                             image_freeSurferRedGreen_pixel_size,
                                             image_freeSurferRedGreen_length, 0);
    this->LabelsCustomIcon->SetImage ( image_labelsCustom,
                                       image_labelsCustom_width,
                                       image_labelsCustom_height,
                                       image_labelsCustom_pixel_size,
                                       image_labelsCustom_length, 0);
    this->LabelsNonSemanticIcon->SetImage ( image_labelsNonSemantic,
                                            image_labelsNonSemantic_width,
                                            image_labelsNonSemantic_height,
                                            image_labelsNonSemantic_pixel_size,
                                            image_labelsNonSemantic_length, 0);
    this->LabelsPelvisIcon->SetImage ( image_labelsPelvis,
                                       image_labelsPelvis_width,
                                       image_labelsPelvis_height,
                                       image_labelsPelvis_pixel_size,
                                       image_labelsPelvis_length, 0);
    this->PETHeatIcon->SetImage ( image_petPETHeat,
                                  image_petPETHeat_width,
                                  image_petPETHeat_height,
                                  image_petPETHeat_pixel_size,
                                  image_petPETHeat_length, 0);
    this->PETMIPIcon->SetImage ( image_petPETMIP,
                                 image_petPETMIP_width,
                                 image_petPETMIP_height,
                                 image_petPETMIP_pixel_size,
                                 image_petPETMIP_length, 0);
    this->PETRainbowIcon->SetImage ( image_petPETRainbow,
                                     image_petPETRainbow_width,
                                     image_petPETRainbow_height,
                                     image_petPETRainbow_pixel_size,
                                     image_petPETRainbow_length, 0);
    this->ShadeCoolShade1Icon->SetImage ( image_shadeCoolShade1,
                                          image_shadeCoolShade1_width,
                                          image_shadeCoolShade1_height,
                                          image_shadeCoolShade1_pixel_size,
                                          image_shadeCoolShade1_length, 0);
    this->ShadeCoolShade2Icon->SetImage ( image_shadeCoolShade2,
                                          image_shadeCoolShade2_width,
                                          image_shadeCoolShade2_height,
                                          image_shadeCoolShade2_pixel_size,
                                          image_shadeCoolShade2_length, 0);
    this->ShadeCoolShade3Icon->SetImage ( image_shadeCoolShade3,
                                          image_shadeCoolShade3_width,
                                          image_shadeCoolShade3_height,
                                          image_shadeCoolShade3_pixel_size,
                                          image_shadeCoolShade3_length, 0);
    this->ShadeWarmShade1Icon->SetImage ( image_shadeWarmShade1,
                                          image_shadeWarmShade1_width,
                                          image_shadeWarmShade1_height,
                                          image_shadeWarmShade1_pixel_size,
                                          image_shadeWarmShade1_length, 0);
    this->ShadeWarmShade2Icon->SetImage ( image_shadeWarmShade2,
                                          image_shadeWarmShade2_width,
                                          image_shadeWarmShade2_height,
                                          image_shadeWarmShade2_pixel_size,
                                          image_shadeWarmShade2_length, 0);
    this->ShadeWarmShade3Icon->SetImage ( image_shadeWarmShade3,
                                          image_shadeWarmShade3_width,
                                          image_shadeWarmShade3_height,
                                          image_shadeWarmShade3_pixel_size,
                                          image_shadeWarmShade3_length, 0);
    this->SlicerBrainLUT2010Icon->SetImage ( image_slicerBrainLUT2010,
                                             image_slicerBrainLUT2010_width,
                                             image_slicerBrainLUT2010_height,
                                             image_slicerBrainLUT2010_pixel_size,
                                             image_slicerBrainLUT2010_length, 0);
    this->SlicerDefaultBrainLUTIcon->SetImage ( image_slicerBrainLUT2010,
                                             image_slicerBrainLUT2010_width,
                                             image_slicerBrainLUT2010_height,
                                             image_slicerBrainLUT2010_pixel_size,
                                             image_slicerBrainLUT2010_length, 0);
    this->SlicerLabels2010Icon->SetImage ( image_slicerShortLUT,
                                         image_slicerShortLUT_width,
                                         image_slicerShortLUT_height,
                                         image_slicerShortLUT_pixel_size,
                                         image_slicerShortLUT_length, 0);
    this->SlicerShortLUTIcon->SetImage ( image_slicerShortLUT,
                                         image_slicerShortLUT_width,
                                         image_slicerShortLUT_height,
                                         image_slicerShortLUT_pixel_size,
                                         image_slicerShortLUT_length, 0);
    this->TintCoolTint1Icon->SetImage ( image_tintCoolTint1,
                                        image_tintCoolTint1_width,
                                        image_tintCoolTint1_height,
                                        image_tintCoolTint1_pixel_size,
                                        image_tintCoolTint1_length, 0);
    this->TintCoolTint2Icon->SetImage ( image_tintCoolTint2,
                                        image_tintCoolTint2_width,
                                        image_tintCoolTint2_height,
                                        image_tintCoolTint2_pixel_size,
                                        image_tintCoolTint2_length, 0);
    this->TintCoolTint3Icon->SetImage ( image_tintCoolTint3,
                                        image_tintCoolTint3_width,
                                        image_tintCoolTint3_height,
                                        image_tintCoolTint3_pixel_size,
                                        image_tintCoolTint3_length, 0);
    this->TintWarmTint1Icon->SetImage ( image_tintWarmTint1,
                                        image_tintWarmTint1_width,
                                        image_tintWarmTint1_height,
                                        image_tintWarmTint1_pixel_size,
                                        image_tintWarmTint1_length, 0);
    this->TintWarmTint2Icon->SetImage ( image_tintWarmTint2,
                                        image_tintWarmTint2_width,
                                        image_tintWarmTint2_height,
                                        image_tintWarmTint2_pixel_size,
                                        image_tintWarmTint2_length, 0);
    this->TintWarmTint3Icon->SetImage ( image_tintWarmTint3,
                                        image_tintWarmTint3_width,
                                        image_tintWarmTint3_height,
                                        image_tintWarmTint3_pixel_size,
                                        image_tintWarmTint3_length, 0);
}


//---------------------------------------------------------------------------
void vtkSlicerColorLUTIcons::AssignNamesToIcons ()
{
  
  //---
  // In this icon set, icons are given a name
  // that corresponds to what a vtkSlicerNodeSelectorWidget
  // for color nodes displays as menuitems.
  // these menuitems are generated in the
  // widget's MakeEntryName method, and
  // stored in its NodeID_to_EntryName map.
  //---
  this->SetIconName ( this->AbdomenIcon, "AbdomenColors");
  this->SetIconName ( this->SPLBrainAtlasIcon, "SPL-BrainAtlas-ColorFile");
  this->SetIconName ( this->BlankLUTIcon, "Blank" );
  this->SetIconName ( this->CartilegeMIdGEMIC3TIcon, "dGEMRIC-3T" );
  this->SetIconName ( this->CartilegeMIdGEMIC15TIcon, "dGEMRIC-1.5T");
  this->SetIconName ( this->DiscreteBlueIcon, "Blue");
  this->SetIconName ( this->DiscreteCool1Icon, "Cool1");
  this->SetIconName ( this->DiscreteCool2Icon, "Cool2");
  this->SetIconName ( this->DiscreteCool3Icon, "Cool3");
  this->SetIconName ( this->DiscreteCyanIcon, "Cyan");
  this->SetIconName ( this->DiscreteDesertIcon, "Desert");
  this->SetIconName ( this->DiscretefMRIIcon, "fMRI");
  this->SetIconName ( this->DiscretefMRIPAIcon, "fMRIPA");
  this->SetIconName ( this->DiscreteFullRainbowIcon, "FullRainbow");
  this->SetIconName ( this->DiscreteGreenIcon, "Green");
  this->SetIconName ( this->DiscreteGreyIcon, "Grey");
  this->SetIconName ( this->DiscreteInvertedGreyIcon, "InvertedGrey");
  this->SetIconName ( this->DiscreteIronIcon, "Iron");
  this->SetIconName ( this->DiscreteLabelsIcon, "Labels");
  this->SetIconName ( this->DiscreteMagentaIcon, "Magenta");
  this->SetIconName ( this->DiscreteOceanIcon, "Ocean");
  this->SetIconName ( this->DiscreteRainbowIcon, "Rainbow");
  this->SetIconName ( this->DiscreteRandomIcon, "Random");
  this->SetIconName ( this->DiscreteRandomIntegersIcon, "RandomIntegers");
  this->SetIconName ( this->DiscreteRedIcon, "Red");
  this->SetIconName ( this->DiscreteReverseRainbowIcon, "ReverseRainbow");
  this->SetIconName ( this->DiscreteWarm1Icon, "Warm1");
  this->SetIconName ( this->DiscreteWarm2Icon, "Warm2");
  this->SetIconName ( this->DiscreteWarm3Icon, "Warm3");
  this->SetIconName ( this->DiscreteYellowIcon, "Yellow");
  this->SetIconName ( this->FreeSurferBlueRedIcon, "BlueRed");
  this->SetIconName ( this->FreeSurferGreenRedIcon, "GreenRed");
  this->SetIconName ( this->FreeSurferHeatIcon, "Heat");
  this->SetIconName ( this->FreeSurferRedBlueIcon, "RedBlue");
  this->SetIconName ( this->FreeSurferRedGreenIcon, "RedGreen");
  this->SetIconName ( this->LabelsCustomIcon, "Custom");
  this->SetIconName ( this->LabelsNonSemanticIcon, "64Color-Nonsemantic");
  this->SetIconName ( this->LabelsPelvisIcon, "PelvisColor");
  this->SetIconName ( this->PETHeatIcon, "PET-Heat");
  this->SetIconName ( this->PETMIPIcon, "PET-MaximumIntensityProjection");
  this->SetIconName ( this->PETRainbowIcon, "PET-Rainbow");
  this->SetIconName ( this->ShadeCoolShade1Icon, "CoolShade1");
  this->SetIconName ( this->ShadeCoolShade2Icon, "CoolShade2");
  this->SetIconName ( this->ShadeCoolShade3Icon, "CoolShade3");
  this->SetIconName ( this->ShadeWarmShade1Icon, "WarmShade1");
  this->SetIconName ( this->ShadeWarmShade2Icon, "WarmShade2");
  this->SetIconName ( this->ShadeWarmShade3Icon, "WarmShade3");
  this->SetIconName ( this->SlicerLabels2010Icon, "Slicer3_2010_Label_Colors");
  this->SetIconName ( this->SlicerBrainLUT2010Icon, "Slicer3_2010_Brain_Labels");
  this->SetIconName ( this->SlicerDefaultBrainLUTIcon, "Slicer3_2010_Default_Brain_Lut");
  this->SetIconName ( this->SlicerShortLUTIcon, "Slicer3_2010_Short_LUT");
  this->SetIconName ( this->TintCoolTint1Icon, "CoolTint1");
  this->SetIconName ( this->TintCoolTint2Icon, "CoolTint2");
  this->SetIconName ( this->TintCoolTint3Icon, "CoolTint3");
  this->SetIconName ( this->TintWarmTint1Icon, "WarmTint1");
  this->SetIconName ( this->TintWarmTint2Icon, "WarmTint2");
  this->SetIconName ( this->TintWarmTint3Icon, "WarmTint3");
  this->SetIconName ( this->GenericColorsIcon, "GenericColors");
  this->SetIconName ( this->GenericAnatomyColorsIcon, "GenericAnatomyColors" );
}


//---------------------------------------------------------------------------
void vtkSlicerColorLUTIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerColorLUTIcons: " << this->GetClassName ( ) << "\n";

    os << indent << "GenericColorsIcon: " << this->GetGenericColorsIcon() << "\n";
    os << indent << "GenericAnatomyColorsIcon: " << this->GetGenericAnatomyColorsIcon() << "\n";
    os << indent << "AbdomenIcon: " << this->GetAbdomenIcon() << "\n";
    os << indent << "SPLBrainAtlasIcon: " << this->GetSPLBrainAtlasIcon() << "\n";
    os << indent << "BlankLUTIcon: " << this->GetBlankLUTIcon() << "\n";
    os << indent << "CartilegeMIdGEMIC3TIcon: " << this->GetCartilegeMIdGEMIC3TIcon() << "\n";
    os << indent << "CartilegeMIdGEMIC15TIcon: " << this->GetCartilegeMIdGEMIC15TIcon() << "\n";
    os << indent << "DiscreteBlueIcon: " << this->GetDiscreteBlueIcon() << "\n";
    os << indent << "DiscreteCool1Icon: " << this->GetDiscreteCool1Icon() << "\n";
    os << indent << "DiscreteCool2Icon: " << this->GetDiscreteCool2Icon() << "\n";
    os << indent << "DiscreteCool3Icon: " << this->GetDiscreteCool3Icon() << "\n";
    os << indent << "DiscreteCyanIcon: " << this->GetDiscreteCyanIcon() << "\n";
    os << indent << "DiscreteDesertIcon: " << this->GetDiscreteDesertIcon() << "\n";
    os << indent << "DiscretefMRIIcon: " << this->GetDiscretefMRIIcon() << "\n";
    os << indent << "DiscretefMRIPAIcon: " << this->GetDiscretefMRIPAIcon() << "\n";
    os << indent << "DiscreteFullRainbowIcon: " << this->GetDiscreteFullRainbowIcon() << "\n";
    os << indent << "DiscreteGreenIcon: " << this->GetDiscreteGreenIcon() << "\n";
    os << indent << "DiscreteGreyIcon: " << this->GetDiscreteGreyIcon() << "\n";
    os << indent << "DiscreteInvertedGreyIcon: " << this->GetDiscreteInvertedGreyIcon() << "\n";
    os << indent << "DiscreteIronIcon: " << this->GetDiscreteIronIcon() << "\n";
    os << indent << "DiscreteLabelsIcon: " << this->GetDiscreteLabelsIcon() << "\n";
    os << indent << "DiscreteMagentaIcon: " << this->GetDiscreteMagentaIcon() << "\n";
    os << indent << "DiscreteOceanIcon: " << this->GetDiscreteOceanIcon() << "\n";
    os << indent << "DiscreteRainbowIcon: " << this->GetDiscreteRainbowIcon() << "\n";
    os << indent << "DiscreteRandomIcon: " << this->GetDiscreteRandomIcon() << "\n";
    os << indent << "DiscreteRandomIntegersIcon: " << this->GetDiscreteRandomIntegersIcon() << "\n";
    os << indent << "DiscreteRedIcon: " << this->GetDiscreteRedIcon() << "\n";
    os << indent << "DiscreteReverseRainbowIcon: " << this->GetDiscreteReverseRainbowIcon() << "\n";
    os << indent << "DiscreteWarm1Icon: " << this->GetDiscreteWarm1Icon() << "\n";
    os << indent << "DiscreteWarm2Icon: " << this->GetDiscreteWarm2Icon() << "\n";
    os << indent << "DiscreteWarm3Icon: " << this->GetDiscreteWarm3Icon() << "\n";
    os << indent << "DiscreteYellowIcon: " << this->GetDiscreteYellowIcon() << "\n";
    os << indent << "FreeSurferBlueRedIcon: " << this->GetFreeSurferBlueRedIcon() << "\n";
    os << indent << "FreeSurferGreenRedIcon: " << this->GetFreeSurferGreenRedIcon() << "\n";
    os << indent << "FreeSurferHeatIcon: " << this->GetFreeSurferHeatIcon() << "\n";
    os << indent << "FreeSurferRedBlueIcon: " << this->GetFreeSurferRedBlueIcon() << "\n";
    os << indent << "FreeSurferRedGreenIcon: " << this->GetFreeSurferRedGreenIcon() << "\n";
    os << indent << "LabelsCustomIcon: " << this->GetLabelsCustomIcon() << "\n";
    os << indent << "LabelsNonSemanticIcon: " << this->GetLabelsNonSemanticIcon() << "\n";
    os << indent << "LabelsPelvisIcon: " << this->GetLabelsPelvisIcon() << "\n";
    os << indent << "PETHeatIcon: " << this->GetPETHeatIcon() << "\n";
    os << indent << "PETMIPIcon: " << this->GetPETMIPIcon() << "\n";
    os << indent << "PETRainbowIcon: " << this->GetPETRainbowIcon() << "\n";
    os << indent << "ShadeCoolShade1Icon: " << this->GetShadeCoolShade1Icon() << "\n";
    os << indent << "ShadeCoolShade2Icon: " << this->GetShadeCoolShade2Icon() << "\n";
    os << indent << "ShadeCoolShade3Icon: " << this->GetShadeCoolShade3Icon() << "\n";
    os << indent << "ShadeWarmShade1Icon: " << this->GetShadeWarmShade1Icon() << "\n";
    os << indent << "ShadeWarmShade2Icon: " << this->GetShadeWarmShade2Icon() << "\n";
    os << indent << "ShadeWarmShade3Icon: " << this->GetShadeWarmShade3Icon() << "\n";
    os << indent << "SlicerLabels2010Icon: " << this->GetSlicerBrainLUT2010Icon() << "\n";
    os << indent << "SlicerBrainLUT2010Icon: " << this->GetSlicerBrainLUT2010Icon() << "\n";
    os << indent << "SlicerDefaultBrainLUTIcon: " << this->GetSlicerDefaultBrainLUTIcon() << "\n";
    os << indent << "SlicerShortLUTIcon: " << this->GetSlicerShortLUTIcon() << "\n";
    os << indent << "TintCoolTint1Icon: " << this->GetTintCoolTint1Icon() << "\n";
    os << indent << "TintCoolTint2Icon: " << this->GetTintCoolTint2Icon() << "\n";
    os << indent << "TintCoolTint3Icon: " << this->GetTintCoolTint3Icon() << "\n";
    os << indent << "TintWarmTint1Icon: " << this->GetTintWarmTint1Icon() << "\n";
    os << indent << "TintWarmTint2Icon: " << this->GetTintWarmTint2Icon() << "\n";
    os << indent << "TintWarmTint3Icon: " << this->GetTintWarmTint3Icon() << "\n";    
}
