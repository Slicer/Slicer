
#include "vtkObjectFactory.h"
#include "vtkSlicerTheme.h"
#include "vtkKWApplication.h"
#include "vtkSlicerApplication.h"
#include "vtkKWOptionDataBase.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerTheme );
vtkCxxRevisionMacro ( vtkSlicerTheme, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerTheme::vtkSlicerTheme ( )
{
    this->SlicerColors = vtkSlicerColor::New ( );
}



//---------------------------------------------------------------------------
vtkSlicerTheme::~vtkSlicerTheme ( )
{
    if ( this->SlicerColors != NULL ) {
        this->SlicerColors->Delete ( );
        this->SlicerColors = NULL;
    }
}



//---------------------------------------------------------------------------
void vtkSlicerTheme::Install ( )
{
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication() );
    
    if ( app == NULL )
        {
            return;
        }
    this->Superclass::Install ( );

    vtkKWOptionDataBase *odb = app->GetOptionDataBase ( );
    
    // ---
    // Set default font:
    // ---
    odb->AddFontOptions ( "{Helvetica 8 normal}" );

    // ---
    // Background and foreground for all widgets:
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
    
    odb->AddEntryAsInt ( "vtkKWSeparator", "SetBorderWidth", 2 );

    
    // Slicer Frames inside Scrolled windows
    odb->AddEntryAsDouble3 ( "vtkKWFrameWithScrollbar:Frame", "SetBackgroundColor",
                    this->SlicerColors->GUIBgColor );
    odb->AddEntryAsInt ( "vtkKWFrameWithScrollbar:Frame", "SetBorderWidth", 0 );
    odb->AddEntryAsInt ( "vtkKWFrameWithScrollbar:Frame", "SetHighlightThickness", 0 );
    odb->AddEntry ( "vtkKWFrameWithScrollbar:Frame", "SetRelief", "sunken" );

    // Slicer Frames inside notebooks
    // wjptest
    odb->AddEntry ( "vtkKWNotebook:Body", "SetRelief", "flat" );
    odb->AddEntryAsDouble3 ( "vtkKWNotebook", "SetBackgroundColor", this->SlicerColors->GUIBgColor );


    // Slicer Frames inside ApplicationGUI
    odb->AddEntryAsDouble3 ( "vtkSlicerApplicationGUI:DropShadowFrame", "SetBackgroundColor",
                    this->SlicerColors->LightestGreyBlue );
    
    // Module collapsing frames 
    odb->AddEntryAsDouble3 ( "vtkSlicerModuleCollapsibleFrame:LabelFrame", "SetBackgroundColor",
                             this->SlicerColors->MediumCoolStone );    
    odb->AddEntryAsInt ( "vtkSlicerModuleCollapsibleFrame:LabelFrame", "SetHighlightThickness", 1 );
    odb->AddEntry ( "vtkSlicerModuleCollapsibleFrame:LabelFrame", "SetReliefToGroove", NULL );
    odb->AddEntryAsDouble3 ( "vtkSlicerModuleCollapsibleFrame:Label", "SetBackgroundColor",
                             this->SlicerColors->MediumCoolStone );
    odb->AddEntryAsDouble3 ( "vtkSlicerModuleCollapsibleFrame:Icon", "SetBackgroundColor",
                             this->SlicerColors->MediumCoolStone );
    // Main Module Tabs

    // Slicer Pushbuttons
    odb->AddEntry ( "vtkKWPushButton", "SetReliefToGroove", NULL );
    odb->AddEntry ( "vtkKWPushButton", "SetDefault", "active" );
    odb->AddEntryAsInt ( "vtkKWPushButton", "SetHighlightThickness", 0 );
    odb->AddEntryAsDouble3 ( "vtkKWPushButton", "SetActiveForeground",
                             this->SlicerColors->FocusTextColor );    
    // Slicer Checkbuttons
    odb->AddEntryAsDouble3 ("vtkKWCheckButton", "SetSelectColor",
                   this->SlicerColors->LightGrey );
    odb->AddEntryAsDouble3 ("vtkKWCheckButton", "SetActiveBackgroundColor",
                            this->SlicerColors->ActiveMenuBackgroundColor );


    // Slicer Radiobuttons
    odb->AddEntryAsDouble3 ("vtkKWRadioButton", "SetSelectColor",
                   this->SlicerColors->LightGrey );
    odb->AddEntryAsDouble3 ("vtkKWRadioButton", "SetActiveBackgroundColor",
                            this->SlicerColors->ActiveMenuBackgroundColor );

    // Slicer Menu
    odb->AddEntryAsDouble3 ("vtkKWMenu", "SetActiveBackgroundColor",
                            this->SlicerColors->ActiveMenuBackgroundColor );

    // Slicer WidgetWithLabel (scrap the groove around all)
    odb->AddEntry ( "vtkKWWidgetWithLabel", "SetRelief", "flat" );    

    // Slicer Menubuttons (keeps the groove around the menu button)
    odb->AddEntry ( "vtkKWMenuButton", "SetReliefToGroove", NULL );
    
    // Slicer vtkKWSeparators
    odb->AddEntryAsDouble3 ( "vtkKWSeparator", "SetBackgroundColor",
                             this->SlicerColors->GUIBgColor );

    // Slicer MultiColumnLists
    // font
    odb->AddEntry ( "vtkKWMultiColumnList", "SetFont", "{Helvetica 8 normal}" );
    // column header 
    odb->AddEntryAsDouble3 ( "vtkKWMultiColumnList", "SetColumnLabelBackgroundColor",
                             this->SlicerColors->MediumBlue );
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
    
    // Slicer Checkbutton
    odb->AddEntry( "vtkKWCheckButton", "IndicatorVisibilityOn", NULL);
    odb->AddEntryAsDouble3( "vtkKWCheckButton", "SetSelectColor", this->SlicerColors->HighlightColor);
    
    // Wizard

    odb->AddEntryAsDouble3 ( "vtkKWWizardWidget", "SetTitleAreaBackgroundColor",
                    this->SlicerColors->LightestGreyBlue);    

    // Slicer Menubar
    // Slicer Labels
    // Slicer Menus
    // Slicer Entry
    // Slicer Radiobuttons
    // Slicer Checkbox
    // Slicer Listbox
    // Slicer Messages
    // Slicer Texts
    // ....?

}


//---------------------------------------------------------------------------
void vtkSlicerTheme::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->Superclass::PrintSelf ( os, indent );
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerTheme: " << this->GetClassName ( ) << "\n";
}
