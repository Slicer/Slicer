
#include "vtkObjectFactory.h"
#include "vtkQueryAtlasIcons.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkQueryAtlasIcons );
vtkCxxRevisionMacro ( vtkQueryAtlasIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkQueryAtlasIcons::vtkQueryAtlasIcons ( )
{
    this->OntologyBrowserIcon = vtkKWIcon::New ( );
    this->OntologyBrowserDisabledIcon = vtkKWIcon::New ( );
    this->AddIcon = vtkKWIcon::New();
    this->DeleteIcon = vtkKWIcon::New ( );
    this->UseAllIcon = vtkKWIcon::New ( );
    this->UseNoneIcon = vtkKWIcon::New ( );
    this->DeselectAllIcon = vtkKWIcon::New ( );
    this->SelectAllIcon = vtkKWIcon::New();
    this->ClearAllIcon = vtkKWIcon::New ( );
    this->ClearSelectedIcon = vtkKWIcon::New ( );
    this->WithAnyIcon = vtkKWIcon::New();
    this->WithAllIcon = vtkKWIcon::New();
    this->WithExactIcon = vtkKWIcon::New();
    this->WithAnySelectedIcon = vtkKWIcon::New();
    this->WithAllSelectedIcon = vtkKWIcon::New();
    this->WithExactSelectedIcon = vtkKWIcon::New();
    this->WithAnyDisabledIcon = vtkKWIcon::New();
    this->WithAllDisabledIcon = vtkKWIcon::New();
    this->WithExactDisabledIcon = vtkKWIcon::New();
    this->SearchIcon = vtkKWIcon::New();
    this->ReserveURIsIcon = vtkKWIcon::New();
    this->ReserveSelectedURIsIcon = vtkKWIcon::New();
    this->SetUpIcon = vtkKWIcon::New();
    this->SelectOverlayIcon = vtkKWIcon::New();
    this->ToggleQuotesIcon = vtkKWIcon::New();
    this->WebIcon = vtkKWIcon::New();
    this->SPLlogo = vtkKWIcon::New();
    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkQueryAtlasIcons::~vtkQueryAtlasIcons ( )
{

  if ( this->SPLlogo )
    {
    this->SPLlogo->Delete();
    this->SPLlogo = NULL;
    }
  if ( this->WebIcon)
    {
    this->WebIcon->Delete();
    this->WebIcon = NULL;
    }
  if ( this->ToggleQuotesIcon )
    {
    this->ToggleQuotesIcon->Delete();
    this->ToggleQuotesIcon = NULL;
    }
  if ( this->SelectOverlayIcon )
    {
    this->SelectOverlayIcon->Delete();
    this->SelectOverlayIcon = NULL;
    }
  if ( this->SetUpIcon )
    {
    this->SetUpIcon->Delete();
    this->SetUpIcon = NULL;
    } 
  if ( this->ReserveSelectedURIsIcon )
    {
    this->ReserveSelectedURIsIcon->Delete();
    this->ReserveSelectedURIsIcon = NULL;
    }
  if ( this->ReserveURIsIcon )
    {
    this->ReserveURIsIcon->Delete();
    this->ReserveURIsIcon = NULL;
    }
  if ( this->SearchIcon )
    {
    this->SearchIcon->Delete();
    this->SearchIcon = NULL;
    }
  if ( this->WithAnyIcon )
    {
    this->WithAnyIcon->Delete();
    this->WithAnyIcon = NULL;
    }
  if ( this->WithAnySelectedIcon )
    {
    this->WithAnySelectedIcon->Delete();
    this->WithAnySelectedIcon = NULL;
    }
  if ( this->WithAnyDisabledIcon )
    {
    this->WithAnyDisabledIcon->Delete();
    this->WithAnyDisabledIcon = NULL;
    }
  if ( this->WithAllIcon )
    {
    this->WithAllIcon->Delete();
    this->WithAllIcon = NULL;
    }
  if ( this->WithAllSelectedIcon )
    {
    this->WithAllSelectedIcon->Delete();
    this->WithAllSelectedIcon = NULL;
    }
  if ( this->WithAllDisabledIcon )
    {
    this->WithAllDisabledIcon->Delete();
    this->WithAllDisabledIcon = NULL;
    }
  if ( this->WithExactIcon )
    {
    this->WithExactIcon->Delete();
    this->WithExactIcon = NULL;
    }
  if ( this->WithExactSelectedIcon )
    {
    this->WithExactSelectedIcon->Delete();
    this->WithExactSelectedIcon = NULL;
    }
  if ( this->WithExactDisabledIcon )
    {
    this->WithExactDisabledIcon->Delete();
    this->WithExactDisabledIcon = NULL;
    }
  if ( this->OntologyBrowserIcon )
    {
    this->OntologyBrowserIcon->Delete();
    this->OntologyBrowserIcon = NULL;
    }
  if ( this->OntologyBrowserDisabledIcon )
    {
    this->OntologyBrowserDisabledIcon->Delete();
    this->OntologyBrowserDisabledIcon = NULL;
    }
  if ( this->AddIcon )
    {
    this->AddIcon->Delete();
    this->AddIcon = NULL;
    }
  if ( this->DeleteIcon )
    {
    this->DeleteIcon->Delete();
    this->DeleteIcon = NULL;
    }
  if ( this->UseAllIcon )
    {
    this->UseAllIcon->Delete();
    this->UseAllIcon = NULL;
    }
  if ( this->UseNoneIcon )
    {
    this->UseNoneIcon->Delete();
    this->UseNoneIcon = NULL;
    }
  if (  this->SelectAllIcon )
    {
    this->SelectAllIcon->Delete();
    this->SelectAllIcon = NULL;
    }
  if ( this->DeselectAllIcon )
    {
    this->DeselectAllIcon->Delete();
    this->DeselectAllIcon = NULL;
    }
  if ( this->ClearAllIcon )
    {
    this->ClearAllIcon->Delete();
    this->ClearAllIcon = NULL;
    }
  if ( this->ClearSelectedIcon )
    {
    this->ClearSelectedIcon->Delete();
    this->ClearSelectedIcon = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkQueryAtlasIcons::AssignImageDataToIcons ( )
{

  this->OntologyBrowserIcon->SetImage( image_OntologyBrowserIcon,
                                image_OntologyBrowserIcon_width,
                                image_OntologyBrowserIcon_height,
                                image_OntologyBrowserIcon_pixel_size,
                                image_OntologyBrowserIcon_length, 0);
  this->OntologyBrowserDisabledIcon->SetImage( image_OntologyBrowserIconDisabled,
                                image_OntologyBrowserIconDisabled_width,
                                image_OntologyBrowserIconDisabled_height,
                                image_OntologyBrowserIconDisabled_pixel_size,
                                image_OntologyBrowserIconDisabled_length, 0);
  this->AddIcon->SetImage( image_AddIcon,
                                image_AddIcon_width,
                                image_AddIcon_height,
                                image_AddIcon_pixel_size,
                                image_AddIcon_length, 0);
  this->DeleteIcon->SetImage( image_DeleteIcon,
                                image_DeleteIcon_width,
                                image_DeleteIcon_height,
                                image_DeleteIcon_pixel_size,
                                image_DeleteIcon_length, 0);
  this->UseAllIcon->SetImage( image_UseAll,
                                image_UseAll_width,
                                image_UseAll_height,
                                image_UseAll_pixel_size,
                                image_UseAll_length, 0);
  this->UseNoneIcon->SetImage( image_UseNone,
                                image_UseNone_width,
                                image_UseNone_height,
                                image_UseNone_pixel_size,
                                image_UseNone_length, 0);
  this->ClearAllIcon->SetImage( image_ClearAll,
                                image_ClearAll_width,
                                image_ClearAll_height,
                                image_ClearAll_pixel_size,
                                image_ClearAll_length, 0);
  this->DeselectAllIcon->SetImage( image_DeselectAll,
                                image_DeselectAll_width,
                                image_DeselectAll_height,
                                image_DeselectAll_pixel_size,
                                image_DeselectAll_length, 0);
  this->SelectAllIcon->SetImage( image_SelectAll,
                                image_SelectAll_width,
                                image_SelectAll_height,
                                image_SelectAll_pixel_size,
                                image_SelectAll_length, 0);
  this->ClearSelectedIcon->SetImage( image_ClearSelected,
                                image_ClearSelected_width,
                                image_ClearSelected_height,
                                image_ClearSelected_pixel_size,
                                image_ClearSelected_length, 0);
  this->WithAnyIcon->SetImage( image_WithAny,
                                image_WithAny_width,
                                image_WithAny_height,
                                image_WithAny_pixel_size,
                                image_WithAny_length, 0);
  this->WithAnySelectedIcon->SetImage( image_WithAnySelected,
                                image_WithAnySelected_width,
                                image_WithAnySelected_height,
                                image_WithAnySelected_pixel_size,
                                image_WithAnySelected_length, 0);
  this->WithAnyDisabledIcon->SetImage( image_WithAnyDisabled,
                                image_WithAnyDisabled_width,
                                image_WithAnyDisabled_height,
                                image_WithAnyDisabled_pixel_size,
                                image_WithAnyDisabled_length, 0);
  this->WithAllIcon->SetImage( image_WithAll,
                                image_WithAll_width,
                                image_WithAll_height,
                                image_WithAll_pixel_size,
                                image_WithAll_length, 0);
  this->WithAllSelectedIcon->SetImage( image_WithAllSelected,
                                image_WithAllSelected_width,
                                image_WithAllSelected_height,
                                image_WithAllSelected_pixel_size,
                                image_WithAllSelected_length, 0);
  this->WithAllDisabledIcon->SetImage( image_WithAllDisabled,
                                image_WithAllDisabled_width,
                                image_WithAllDisabled_height,
                                image_WithAllDisabled_pixel_size,
                                image_WithAllDisabled_length, 0);
  this->WithExactIcon->SetImage( image_WithExact,
                                image_WithExact_width,
                                image_WithExact_height,
                                image_WithExact_pixel_size,
                                image_WithExact_length, 0);
  this->WithExactSelectedIcon->SetImage( image_WithExactSelected,
                                image_WithExactSelected_width,
                                image_WithExactSelected_height,
                                image_WithExactSelected_pixel_size,
                                image_WithExactSelected_length, 0);
  this->WithExactDisabledIcon->SetImage( image_WithExactDisabled,
                                image_WithExactDisabled_width,
                                image_WithExactDisabled_height,
                                image_WithExactDisabled_pixel_size,
                                image_WithExactDisabled_length, 0);
  this->SearchIcon->SetImage( image_Search,
                                image_Search_width,
                                image_Search_height,
                                image_Search_pixel_size,
                                image_Search_length, 0);
  this->ReserveURIsIcon->SetImage( image_ReserveURIs,
                                image_ReserveURIs_width,
                                image_ReserveURIs_height,
                                image_ReserveURIs_pixel_size,
                                image_ReserveURIs_length, 0);
  this->ReserveSelectedURIsIcon->SetImage( image_ReserveSelectedURIs,
                                image_ReserveSelectedURIs_width,
                                image_ReserveSelectedURIs_height,
                                image_ReserveSelectedURIs_pixel_size,
                                image_ReserveSelectedURIs_length, 0);
  this->SetUpIcon->SetImage( image_SetUp,
                                image_SetUp_width,
                                image_SetUp_height,
                                image_SetUp_pixel_size,
                                image_SetUp_length, 0);
  this->SelectOverlayIcon->SetImage( image_SelectOverlay,
                                image_SelectOverlay_width,
                                image_SelectOverlay_height,
                                image_SelectOverlay_pixel_size,
                                image_SelectOverlay_length, 0);
  this->ToggleQuotesIcon->SetImage( image_ToggleQuotes,
                                image_ToggleQuotes_width,
                                image_ToggleQuotes_height,
                                image_ToggleQuotes_pixel_size,
                                image_ToggleQuotes_length, 0);
  this->WebIcon->SetImage( image_www,
                                image_www_width,
                                image_www_height,
                                image_www_pixel_size,
                                image_www_length, 0);
  this->SPLlogo->SetImage( image_SPLlogo,
                                image_SPLlogo_width,
                                image_SPLlogo_height,
                                image_SPLlogo_pixel_size,
                                image_SPLlogo_length, 0);

}



//---------------------------------------------------------------------------
void vtkQueryAtlasIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "QueryAtlasIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "OntologyBrowserIcon: " << this->GetOntologyBrowserIcon ( ) << "\n";
    os << indent << "AddIcon: " << this->GetAddIcon ( ) << "\n";
    os << indent << "DeleteIcon: " << this->GetDeleteIcon ( ) << "\n";
    os << indent << "UseAllIcon: " << this->GetUseAllIcon ( ) << "\n";
    os << indent << "UseNoneIcon: " << this->GetUseNoneIcon ( ) << "\n";
    os << indent << "ClearAllIcon: " << this->GetClearAllIcon ( ) << "\n";
    os << indent << "DeselectAllIcon: " << this->GetDeselectAllIcon ( ) << "\n";
    os << indent << "SelectAllIcon: " << this->GetSelectAllIcon ( ) << "\n";
    os << indent << "ClearSelectedIcon: " << this->GetClearSelectedIcon ( ) << "\n";
    os << indent << "WithAnyIcon: " << this->GetWithAnyIcon ( ) << "\n";
    os << indent << "WithAnySelectedIcon: " << this->GetWithAnySelectedIcon ( ) << "\n";
    os << indent << "WithAnyDisabledIcon: " << this->GetWithAnyDisabledIcon ( ) << "\n";
    os << indent << "WithAllIcon: " << this->GetWithAllIcon ( ) << "\n";
    os << indent << "WithAllSelectedIcon: " << this->GetWithAllSelectedIcon ( ) << "\n";
    os << indent << "WithAllDisabledIcon: " << this->GetWithAllDisabledIcon ( ) << "\n";
    os << indent << "WithExactIcon: " << this->GetWithExactIcon ( ) << "\n";
    os << indent << "WithExactSelectedIcon: " << this->GetWithExactSelectedIcon ( ) << "\n";
    os << indent << "WithExactDisabledIcon: " << this->GetWithExactDisabledIcon ( ) << "\n";
    os << indent << "SearchIcon: " << this->GetSearchIcon ( ) << "\n";
    os << indent << "ReserveURIsIcon: " << this->GetReserveURIsIcon ( ) << "\n";
    os << indent << "ReserveSelectedURIsIcon: " << this->GetReserveSelectedURIsIcon ( ) << "\n";
    os << indent << "SetUpIcon: " << this->GetSetUpIcon ( ) << "\n";
    os << indent << "SelectOverlayIcon: " << this->GetSelectOverlayIcon ( ) << "\n";
    os << indent << "ToggleQuotesIcon: " << this->GetToggleQuotesIcon ( ) << "\n";
    os << indent << "WebIcon: " << this->GetWebIcon ( ) << "\n";
    os << indent << "SPLlogo: " << this->GetSPLlogo ( ) << "\n";
}
