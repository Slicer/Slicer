
#include "vtkObjectFactory.h"
#include "vtkSlicerTheme.h"
#include "vtkKWApplication.h"
#include "vtkKWMimxApplication.h"
#include "vtkKWOptionDataBase.h"
#include "vtkKWIcon.h"
#include "vtkSlicerCheckRadioButtonIcons.h"
#include "vtkKWTkUtilities.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerTheme );
vtkCxxRevisionMacro ( vtkSlicerTheme, "$Revision: 1.2 $");

//---------------------------------------------------------------------------
vtkSlicerTheme::vtkSlicerTheme ( )
{
    this->SlicerColors = vtkSlicerColor::New ( );
    this->SlicerFonts = vtkSlicerFont::New ( );
    this->CheckRadioIcons = vtkSlicerCheckRadioButtonIcons::New();

    this->FontSize0 = this->SlicerFonts->GetFontSizeSmall0();
    this->FontSize1 = this->SlicerFonts->GetFontSizeSmall1();
    this->FontSize2 = this->SlicerFonts->GetFontSizeSmall2();
    this->FontFamily = this->SlicerFonts->GetFontFamily(0);

    // ---
    // Create a named font that can be reconfigured live
    // ---
    this->SetApplicationFont2 ( "ApplicationFont2" );
    this->SetApplicationFont1 ( "ApplicationFont1" );
    this->SetApplicationFont0 ( "ApplicationFont0" );
}



//---------------------------------------------------------------------------
vtkSlicerTheme::~vtkSlicerTheme ( )
{
    if ( this->SlicerColors != NULL ) {
        this->SlicerColors->Delete ( );
        this->SlicerColors = NULL;
    }
    if ( this->SlicerFonts )
      {
      this->SlicerFonts->Delete();
      this->SlicerFonts = NULL;
      }
    if ( this->CheckRadioIcons )
      {
      this->CheckRadioIcons->Delete();
      this->CheckRadioIcons = NULL;
      }
}


//---------------------------------------------------------------------------
void vtkSlicerTheme::InstallFonts ( )
{
  vtkKWMimxApplication *app = vtkKWMimxApplication::SafeDownCast(this->GetApplication() );
  if ( app )
    {

    //--- get font stored in app,
    //--- set thru application settings interface
    this->FontFamily = app->GetApplicationFontFamily();
    if ( !(strcmp (app->GetApplicationFontSize(), this->SlicerFonts->GetFontSize(3) )))
      {
      this->FontSize0 = this->SlicerFonts->GetFontSizeLargest0();
      this->FontSize1 = this->SlicerFonts->GetFontSizeLargest1();
      this->FontSize2 = this->SlicerFonts->GetFontSizeLargest2();
      }
    else if (!(strcmp (app->GetApplicationFontSize(), this->SlicerFonts->GetFontSize(2) )))
      {
      this->FontSize0 = this->SlicerFonts->GetFontSizeLarge0();
      this->FontSize1 = this->SlicerFonts->GetFontSizeLarge1();
      this->FontSize2 = this->SlicerFonts->GetFontSizeLarge2();
      }
    else if ( !(strcmp(app->GetApplicationFontSize(), this->SlicerFonts->GetFontSize(1) )))
      {
      this->FontSize0 = this->SlicerFonts->GetFontSizeMedium0();
      this->FontSize1 = this->SlicerFonts->GetFontSizeMedium1();
      this->FontSize2 = this->SlicerFonts->GetFontSizeMedium2();
      }
    else if ( !(strcmp(app->GetApplicationFontSize(), this->SlicerFonts->GetFontSize(0) )))
      {
      this->FontSize0 = this->SlicerFonts->GetFontSizeSmall0();
      this->FontSize1 = this->SlicerFonts->GetFontSizeSmall1();
      this->FontSize2 = this->SlicerFonts->GetFontSizeSmall2();
      }
    else 
      {
      this->FontSize0 = this->SlicerFonts->GetFontSizeSmall0();
      this->FontSize1 = this->SlicerFonts->GetFontSizeSmall1();
      this->FontSize2 = this->SlicerFonts->GetFontSizeSmall2();
      }
    app->Script ( "font create %s -family %s -size %d", this->GetApplicationFont2(), this->FontFamily, this->FontSize2 );
    app->Script ( "font create %s -family %s -size %d", this->GetApplicationFont1(), this->FontFamily, this->FontSize1 );
    app->Script ( "font create %s -family %s -size %d", this->GetApplicationFont0(), this->FontFamily, this->FontSize0 );      
    }
}


//---------------------------------------------------------------------------
void vtkSlicerTheme::Install ( )
{
    vtkKWMimxApplication *app = vtkKWMimxApplication::SafeDownCast(this->GetApplication() );
    
    if ( app == NULL )
        {
            return;
        }

    this->Superclass::Install ( );

    vtkKWOptionDataBase *odb = app->GetOptionDataBase ( );
    
    this->InstallFonts();
    
    //--- Set Font2 to be the default widget font.
    //--- use Font0 and Font1 for specific widgets (slice controller menu buttons
    //--- and inside viewcontrolGUI and toolbarGUI, which can't be captured by theme)...
    odb->AddFontOptions ( this->GetApplicationFont2() );
    odb->AddEntry ( "vtkSlicerSliceControllerWidget*vtkKWMenuButton", "SetFont", this->GetApplicationFont1());

    // ---
    // Background and foreground for all widgets in general (some will be overridden):
    // ---
    odb->AddBackgroundColorOptions( this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetDisabledForegroundColor",
                    this->SlicerColors->DisabledTextColor );
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetActiveForegroundColor",
                             this->SlicerColors->FocusTextColor );
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetActiveBackgroundColor",
                             this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetBackgroundColor",
                    this->SlicerColors->GUIBgColor );
    odb->AddEntryAsInt ("vtkKWWidget", "SetHighlightThickness", 0 );
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetHighlightBackground",
                             this->SlicerColors->HighlightBackground);
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetHighlightColor",
                             this->SlicerColors->HighlightColor);
    odb->AddEntryAsInt ("vtkKWWidget", "SetBorderWidth", 2 );
    odb->AddEntryAsInt ("vtkKWWidget", "SetActiveBorderWidth", 2 );
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetTroughColor", this->SlicerColors->LightGrey );

    // ---
    // Individual widgets:
    // ---
    
    // Slicer Scales
    odb->AddEntryAsDouble3 ( "vtkKWScale", "SetTroughColor", this->SlicerColors->LightGrey );

    // Slicer Scrollbars 
    odb->AddEntryAsDouble3 ( "vtkKWScrollbar", "SetBackgroundColor",
                    this->SlicerColors->LightGrey );
    odb->AddEntryAsDouble3 ( "vtkKWScrollbar", "SetActiveBackgroundColor",
                    this->SlicerColors->LightGrey );
    odb->AddEntryAsDouble3 ( "vtkKWScrollbar", "SetTroughColor", this->SlicerColors->LightGrey );

    // Slicer Frames
    odb->AddEntryAsDouble3 ( "vtkKWFrame", "SetBackgroundColor",
                    this->SlicerColors->GUIBgColor );
    odb->AddEntryAsInt ( "vtkKWFrame", "SetBorderWidth", 0 );
    odb->AddEntryAsDouble3 ( "vtkKWFrame", "SetHighlightColor",
                             this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3 ( "vtkKWFrame", "SetHighlightColor",
                             this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3 ( "vtkKWFrame", "SetHighlightBackground",
                             this->SlicerColors->GUIBgColor );    
    odb->AddEntryAsInt ( "vtkKWFrame", "SetHighlightThickness", 0 );
    odb->AddEntry ( "vtkKWFrame", "SetRelief", "flat" );
    
    // Slicer Frames inside Scrolled windows
    odb->AddEntryAsDouble3 ( "vtkKWFrameWithScrollbar:Frame", "SetBackgroundColor",
                    this->SlicerColors->GUIBgColor );
    odb->AddEntryAsInt ( "vtkKWFrameWithScrollbar:Frame", "SetBorderWidth", 0 );
    odb->AddEntryAsInt ( "vtkKWFrameWithScrollbar:Frame", "SetHighlightThickness", 0 );
    odb->AddEntry ( "vtkKWFrameWithScrollbar:Frame", "SetRelief", "sunken" );
    
    // Slicer subtle decoration Frames inside ApplicationGUI
    /*
    odb->AddEntryAsDouble3 ( "vtkSlicerApplicationGUI:DropShadowFrame", "SetBackgroundColor",
                    this->SlicerColors->LightestGreyBlue );
    */
    // Module stacked, collapsing frames 
    /*
    odb->AddEntryAsDouble3 ( "vtkSlicerModuleCollapsibleFrame:LabelFrame", "SetBackgroundColor",
                             this->SlicerColors->MediumCoolStone );    
    odb->AddEntryAsInt ( "vtkSlicerModuleCollapsibleFrame:LabelFrame", "SetHighlightThickness", 1 );
    odb->AddEntry ( "vtkSlicerModuleCollapsibleFrame:LabelFrame", "SetReliefToGroove", NULL );
    odb->AddEntryAsDouble3 ( "vtkSlicerModuleCollapsibleFrame:Label", "SetBackgroundColor",
                             this->SlicerColors->MediumCoolStone );
    odb->AddEntryAsDouble3 ( "vtkSlicerModuleCollapsibleFrame:Icon", "SetBackgroundColor",
                             this->SlicerColors->MediumCoolStone );
    */
    // Slicer vtkKWSeparators
    odb->AddEntryAsDouble3 ( "vtkKWSeparator", "SetBackgroundColor",
                             this->SlicerColors->GUIBgColor );
    odb->AddEntryAsInt ( "vtkKWSeparator", "SetBorderWidth", 2 );

    // Slicer Notebooks
    odb->AddEntryAsDouble3 ( "vtkKWNotebook", "SetBackgroundColor", this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3 ( "vtkKWNotebook", "SetPageTabColor", this->SlicerColors->MediumCoolStone );
    odb->AddEntryAsDouble3 ( "vtkKWNotebook", "SetSelectedPageTabColor", this->SlicerColors->LightestStone );
    odb->AddEntryAsInt ( "vtkKWNotebook", "SetSelectedPageTabPadding", 0 );


    
    // Slicer Pushbuttons
    odb->AddEntry ( "vtkKWPushButton", "SetReliefToGroove", NULL );
    odb->AddEntry ( "vtkKWPushButton", "SetDefault", "active" );
    odb->AddEntryAsInt ( "vtkKWPushButton", "SetHighlightThickness", 0 );
    odb->AddEntryAsDouble3 ( "vtkKWPushButton", "SetActiveForeground",
                             this->SlicerColors->FocusTextColor );

    
    // Slicer KW Checkbuttons
    const char *checkOnImage = "checkOn";
    const char *offImage = "checkRadioOff";
    odb->AddEntryAsDouble3 ("vtkKWCheckButton", "SetActiveBackgroundColor",
                            this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3( "vtkKWCheckButton", "SetSelectColor", this->SlicerColors->GUIBgColor);
    vtkKWTkUtilities::UpdatePhotoFromIcon ( app, checkOnImage, this->CheckRadioIcons->GetSelectCheckIcon(), 0 );
    vtkKWTkUtilities::UpdatePhotoFromIcon ( app, offImage, this->CheckRadioIcons->GetDeselectIcon(), 0 );
    odb->AddEntry( "vtkKWCheckButton", "IndicatorVisibilityOff", NULL );
    odb->AddEntry ( "vtkKWCheckButton", "SetReliefToFlat", NULL );
    odb->AddEntry ( "vtkKWCheckButton", "SetOffReliefToFlat", NULL );
    odb->AddEntry ( "vtkKWCheckButton", "SetOverReliefToFlat", NULL );
    odb->AddEntryAsInt ("vtkKWCheckButton", "SetBorderWidth", 0 );
    odb->AddEntryAsInt ("vtkKWCheckButton", "SetHighlightThickness", 0 );
    odb->AddEntryAsInt ( "vtkKWCheckButton", "SetPadX", 4 );
    odb->AddEntryAsInt ( "vtkKWCheckButton", "SetPadY", 4 );
    odb->AddEntry ( "vtkKWCheckButton", "SetConfigurationOption -image", offImage);
    odb->AddEntry ( "vtkKWCheckButton", "SetConfigurationOption -selectimage", checkOnImage);    
    odb->AddEntry ( "vtkKWCheckButton", "SetCompoundModeToLeft", NULL );


    // Slicer KW Radiobuttons
    const char *radioOnImage = "radioOn";
    vtkKWTkUtilities::UpdatePhotoFromIcon ( app, radioOnImage, this->CheckRadioIcons->GetSelectRadioIcon(), 0 );
    odb->AddEntryAsDouble3 ("vtkKWRadioButton", "SetSelectColor", this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3 ("vtkKWRadioButton", "SetActiveBackgroundColor",
                            this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3 ("vtkKWRadioButton", "SetActiveBackgroundColor",
                            this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3( "vtkKWRadioButton", "SetSelectColor", this->SlicerColors->GUIBgColor);
    odb->AddEntry( "vtkKWRadioButton", "IndicatorVisibilityOff", NULL );
    odb->AddEntry ( "vtkKWRadioButton", "SetReliefToFlat", NULL );
    odb->AddEntry ( "vtkKWRadioButton", "SetOffReliefToFlat", NULL );
    odb->AddEntry ( "vtkKWRadioButton", "SetOverReliefToFlat", NULL );
    odb->AddEntryAsInt ("vtkKWRadioButton", "SetBorderWidth", 0 );
    odb->AddEntryAsInt ("vtkKWRadioButton", "SetHighlightThickness", 0 );
    odb->AddEntryAsInt ( "vtkKWRadioButton", "SetPadX", 4 );
    odb->AddEntryAsInt ( "vtkKWRadioButton", "SetPadY", 4 );
    odb->AddEntry ( "vtkKWRadioButton", "SetConfigurationOption -image", offImage);
    odb->AddEntry ( "vtkKWRadioButton", "SetConfigurationOption -selectimage", radioOnImage);    
    odb->AddEntry ( "vtkKWRadioButton", "SetCompoundModeToLeft", NULL );

    // Slicer WidgetWithLabel (scrap the groove around all)
    odb->AddEntry ( "vtkKWWidgetWithLabel", "SetRelief", "flat" );    


    


    // Slicer MultiColumnLists
    // font
//    odb->AddEntry ( "vtkKWMultiColumnList", "SetFont", "{Helvetica 8 normal}" );
    // column header 
    odb->AddEntryAsDouble3 ( "vtkKWMultiColumnList", "SetColumnLabelBackgroundColor",this->SlicerColors->DarkRed);
                             //this->SlicerColors->MediumBlue );
    odb->AddEntryAsDouble3 ( "vtkKWMultiColumnList", "SetColumnLabelForegroundColor",
                             this->SlicerColors->White );
    // selection
    odb->AddEntryAsDouble3 ( "vtkKWMultiColumnList", "SetSelectionBackgroundColor",
         this->SlicerColors->ActiveMenuBackgroundColor);
    odb->AddEntryAsDouble3 ( "vtkKWMultiColumnList", "SetSelectionForegroundColor",
         this->SlicerColors->ActiveTextColor);
    // MulticolumnList
    odb->AddEntryAsDouble3 ("vtkKWMultiColumnList", "SetStripeBackgroundColor",
                   this->SlicerColors->LightestGreyBlue );

    // Window's status fame
    odb->AddEntryAsDouble3 ( "vtkKWWindow:TrayFrame", "SetBackgroundColor",
                    this->SlicerColors->MediumCoolStone );    


    
    // Slicer MultiColumnListsWithScrollbars
    // scroll bars
    odb->AddEntryAsInt("vtkKWScrollbar", "SetWidth", 10);
    odb->AddEntryAsDouble3("vtkKWScrollbar", "SetTroughColor", this->SlicerColors->RecessedColor);
    

    
    // Slicer Menu and MenuButtons
    odb->AddEntryAsDouble3 ("vtkKWMenu", "SetActiveBackgroundColor",
                            this->SlicerColors->ActiveMenuBackgroundColor );
    odb->AddEntryAsDouble3 ("vtkKWMenu", "SetActiveForegroundColor",
                            this->SlicerColors->ActiveMenuForegroundColor );
    odb->AddEntry ( "vtkKWMenuButton", "SetReliefToGroove", NULL );

    // Wizard
    odb->AddEntryAsDouble3 ( "vtkKWWizardWidget", "SetTitleAreaBackgroundColor",
                    this->SlicerColors->LightestGreyBlue);    

    // anything special here?
    // Slicer Menubar
    // Slicer Entry
    // Slicer Listbox
    // Slicer Messages
    // ....?

}



//---------------------------------------------------------------------------
void vtkSlicerTheme::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->Superclass::PrintSelf ( os, indent );
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerTheme: " << this->GetClassName ( ) << "\n";
}
