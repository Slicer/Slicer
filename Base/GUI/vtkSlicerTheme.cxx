
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
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetForegroundColor",
                    this->SlicerColors->ActiveTextColor);
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetDisabledForegroundColor",
                    this->SlicerColors->DisabledTextColor );
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetActiveForegroundColor",
                             this->SlicerColors->FocusTextColor );
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetActiveBackgroundColor",
                             this->SlicerColors->GUIBgColor );
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetBackgroundColor",
                    this->SlicerColors->White );

    odb->AddEntryAsInt ("vtkKWWidget", "SetHighlightThickness", 0 );
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetHighlightBackground",
                             this->SlicerColors->HighlightBackground);
    odb->AddEntryAsDouble3 ( "vtkKWWidget", "SetHighlightColor",
                             this->SlicerColors->HighlightColor);
    
    odb->AddEntryAsInt ("vtkKWWidget", "SetBorderWidth", 2 );
    odb->AddEntryAsInt ("vtkKWWidget", "SetActiveBorderWidth", 2 );

    
    // ---
    // Individual widgets:
    // ---
    
    
    // Slicer Toplevels
    odb->AddEntryAsDouble3 ( "vtkKWScale", "SetTroughColor", this->SlicerColors->LightGrey );
    
    // Slicer Frames
    odb->AddEntryAsInt ( "vtkKWFrame", "SetBorderWidth", 0 );
    odb->AddEntryAsDouble3 ( "vtkKWFrame", "SetHighlightColor",
                             this->SlicerColors->White );
    odb->AddEntryAsDouble3 ( "vtkKWFrame", "SetHighlightColor",
                             this->SlicerColors->White );
    odb->AddEntryAsDouble3 ( "vtkKWFrame", "SetHighlightBackground",
                             this->SlicerColors->White );    
    odb->AddEntryAsInt ( "vtkKWFrame", "SetHighlightThickness", 0 );
    odb->AddEntry ( "vtkKWFrame", "SetRelief", "flat" );


    // Slicer Menubar
    
    // Slicer Labels

    
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

    // Slicer MultiColumnLists
    
    
    // Slicer Menubuttons
    odb->AddEntry ( "vtkKWMenuButton", "SetReliefToGroove", NULL );

    // Slicer Menus
    // Slicer Entry
    // Slicer Scales
    // Slicer Scrollbars
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
