
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
    odb->AddEntryAsDouble3 ( "vtkSlicerApplicationGUI:DropShadowFrame", "SetBackgroundColor",
                    this->SlicerColors->LightestGreyBlue );

    // Module stacked, collapsing frames 
    odb->AddEntryAsDouble3 ( "vtkSlicerModuleCollapsibleFrame:LabelFrame", "SetBackgroundColor",
                             this->SlicerColors->MediumCoolStone );    
    odb->AddEntryAsInt ( "vtkSlicerModuleCollapsibleFrame:LabelFrame", "SetHighlightThickness", 1 );
    odb->AddEntry ( "vtkSlicerModuleCollapsibleFrame:LabelFrame", "SetReliefToGroove", NULL );
    odb->AddEntryAsDouble3 ( "vtkSlicerModuleCollapsibleFrame:Label", "SetBackgroundColor",
                             this->SlicerColors->MediumCoolStone );
    odb->AddEntryAsDouble3 ( "vtkSlicerModuleCollapsibleFrame:Icon", "SetBackgroundColor",
                             this->SlicerColors->MediumCoolStone );

    // Slicer vtkKWSeparators
    odb->AddEntryAsDouble3 ( "vtkKWSeparator", "SetBackgroundColor",
                             this->SlicerColors->GUIBgColor );
    odb->AddEntryAsInt ( "vtkKWSeparator", "SetBorderWidth", 2 );

    // Slicer Notebooks
    odb->AddEntryAsDouble3 ( "vtkKWNotebook", "SetBackgroundColor", this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3 ( "vtkKWNotebook", "SetPageTabColor", this->SlicerColors->MediumCoolStone );
    odb->AddEntryAsDouble3 ( "vtkKWNotebook", "SetSelectedPageTabColor", this->SlicerColors->LightCoolStone );
    odb->AddEntryAsInt ( "vtkKWNotebook", "SetSelectedPageTabPadding", 0 );


    
    // Slicer Pushbuttons
    odb->AddEntry ( "vtkKWPushButton", "SetReliefToGroove", NULL );
    odb->AddEntry ( "vtkKWPushButton", "SetDefault", "active" );
    odb->AddEntryAsInt ( "vtkKWPushButton", "SetHighlightThickness", 0 );
    odb->AddEntryAsDouble3 ( "vtkKWPushButton", "SetActiveForeground",
                             this->SlicerColors->FocusTextColor );

    
    // Slicer Checkbuttons
    odb->AddEntryAsDouble3 ("vtkKWCheckButton", "SetSelectColor",
                   this->SlicerColors->LightGrey );
//    odb->AddEntryAsDouble3 ("vtkKWCheckButton", "SetActiveBackgroundColor",
//                            this->SlicerColors->ActiveMenuBackgroundColor );
    odb->AddEntryAsDouble3 ("vtkKWCheckButton", "SetActiveBackgroundColor",
                            this->SlicerColors->GUIBgColor );
    odb->AddEntry( "vtkKWCheckButton", "IndicatorVisibilityOn", NULL);
    odb->AddEntryAsDouble3( "vtkKWCheckButton", "SetSelectColor", this->SlicerColors->HighlightColor);

    

    // Slicer Radiobuttons
    odb->AddEntryAsDouble3 ("vtkKWRadioButton", "SetSelectColor",
                   this->SlicerColors->LightGrey );
//    odb->AddEntryAsDouble3 ("vtkKWRadioButton", "SetActiveBackgroundColor",
//                            this->SlicerColors->ActiveMenuBackgroundColor );
    odb->AddEntryAsDouble3 ("vtkKWRadioButton", "SetActiveBackgroundColor",
                            this->SlicerColors->GUIBgColor );

    
    // Slicer WidgetWithLabel (scrap the groove around all)
    odb->AddEntry ( "vtkKWWidgetWithLabel", "SetRelief", "flat" );    


    


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
