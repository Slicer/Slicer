
#include "vtkObjectFactory.h"
#include "vtkFetchMIIcons.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkFetchMIIcons );
vtkCxxRevisionMacro ( vtkFetchMIIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkFetchMIIcons::vtkFetchMIIcons ( )
{
  this->AddNewIcon = vtkKWIcon::New ();
  this->SelectAllIcon = vtkKWIcon::New();
  this->DeselectAllIcon = vtkKWIcon::New();
  this->DeleteSelectedIcon = vtkKWIcon::New();
  this->DeleteAllIcon = vtkKWIcon::New();
  this->SearchIcon = vtkKWIcon::New();
  this->DownloadIcon = vtkKWIcon::New();
  this->UploadIcon = vtkKWIcon::New();
  this->ApplyTagsIcon = vtkKWIcon::New();
  this->RemoveTagsIcon = vtkKWIcon::New();
  this->ShowDataTagsIcon = vtkKWIcon::New();
  this->HelpIcon = vtkKWIcon::New();
  this->RefreshServerIcon = vtkKWIcon::New();
  
  this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkFetchMIIcons::~vtkFetchMIIcons ( )
{

  if ( this->AddNewIcon )
    {
    this->AddNewIcon->Delete();
    this->AddNewIcon = NULL;
    }
  if ( this->RefreshServerIcon )
    {
    this->RefreshServerIcon->Delete();
    this->RefreshServerIcon = NULL;
    }
  if ( this->HelpIcon)
    {
    this->HelpIcon->Delete();
    this->HelpIcon = NULL;
    }
  if ( this->SelectAllIcon )
    {
    this->SelectAllIcon->Delete();
    this->SelectAllIcon = NULL;
    }
  if ( this->DeselectAllIcon )
    {
    this->DeselectAllIcon->Delete();
    this->DeselectAllIcon = NULL;
    }
  if ( this->DeleteSelectedIcon )
    {
    this->DeleteSelectedIcon->Delete();
    this->DeleteSelectedIcon = NULL;
    }
  if ( this->DeleteAllIcon )
    {
    this->DeleteAllIcon->Delete();
    this->DeleteAllIcon = NULL;
    }
  if ( this->SearchIcon )
    {
    this->SearchIcon->Delete();
    this->SearchIcon = NULL;
    }
  if ( this->DownloadIcon )
    {
    this->DownloadIcon->Delete();
    this->DownloadIcon = NULL;    
    }
  if ( this->UploadIcon )
    {
    this->UploadIcon->Delete();
    this->UploadIcon = NULL;    
    }
  if ( this->ApplyTagsIcon )
    {
    this->ApplyTagsIcon->Delete();
    this->ApplyTagsIcon = NULL;    
    }
  if ( this->RemoveTagsIcon )
    {
    this->RemoveTagsIcon->Delete();
    this->RemoveTagsIcon = NULL;    
    }
  if ( this->ShowDataTagsIcon )
    {
    this->ShowDataTagsIcon->Delete();
    this->ShowDataTagsIcon = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkFetchMIIcons::AssignImageDataToIcons ( )
{

  this->AddNewIcon->SetImage (image_AddIcon,
                              image_AddIcon_width,
                              image_AddIcon_height,
                              image_AddIcon_pixel_size,
                              image_AddIcon_length, 0 );

  this->RefreshServerIcon->SetImage (image_ServerRefresh,
                                     image_ServerRefresh_width,
                                     image_ServerRefresh_height,
                                     image_ServerRefresh_pixel_size,
                                     image_ServerRefresh_length, 0);
  this->HelpIcon->SetImage(image_TagInformation,
                           image_TagInformation_width,
                           image_TagInformation_height,
                           image_TagInformation_pixel_size,
                           image_TagInformation_length, 0);
  this->DeleteAllIcon->SetImage (image_CleanUpAll,
                                 image_CleanUpAll_width,
                                 image_CleanUpAll_height,
                                 image_CleanUpAll_pixel_size,
                                 image_CleanUpAll_length, 0 );
  this->DeleteSelectedIcon->SetImage ( image_CleanUpSelected,
                                       image_CleanUpSelected_width,
                                       image_CleanUpSelected_height,
                                       image_CleanUpSelected_pixel_size,
                                       image_CleanUpSelected_length, 0 );
  this->SelectAllIcon->SetImage ( image_SelectAll,
                                  image_SelectAll_width,
                                  image_SelectAll_height,
                                  image_SelectAll_pixel_size,
                                  image_SelectAll_length, 0 );
  this->DeselectAllIcon->SetImage ( image_DeselectAll,
                                    image_DeselectAll_width,
                                    image_DeselectAll_height,
                                    image_DeselectAll_pixel_size,
                                    image_DeselectAll_length, 0 );
  this->SearchIcon->SetImage ( image_Search,
                               image_Search_width,
                               image_Search_height,
                               image_Search_pixel_size,
                               image_Search_length, 0 );
  this->DownloadIcon->SetImage ( image_Download,
                               image_Download_width,
                               image_Download_height,
                               image_Download_pixel_size,
                               image_Download_length, 0 );
  this->UploadIcon->SetImage ( image_Upload,
                               image_Upload_width,
                               image_Upload_height,
                               image_Upload_pixel_size,
                               image_Upload_length, 0 );
  this->ApplyTagsIcon->SetImage ( image_TagSelected,
                               image_TagSelected_width,
                               image_TagSelected_height,
                               image_TagSelected_pixel_size,
                               image_TagSelected_length, 0 );
  this->RemoveTagsIcon->SetImage ( image_RemoveTagSelected,
                               image_RemoveTagSelected_width,
                               image_RemoveTagSelected_height,
                               image_RemoveTagSelected_pixel_size,
                               image_RemoveTagSelected_length, 0 );
  this->ShowDataTagsIcon->SetImage ( image_ShowDataTags,
                               image_ShowDataTags_width,
                               image_ShowDataTags_height,
                               image_ShowDataTags_pixel_size,
                               image_ShowDataTags_length, 0 );

}



//---------------------------------------------------------------------------
void vtkFetchMIIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "FetchMIIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "AddNewIcon: " << this->GetAddNewIcon ( ) << "\n";
    os << indent << "RefreshServerIcon: " << this->GetRefreshServerIcon ( ) << "\n";
    os << indent << "HelpIcon: " << this->GetHelpIcon ( ) << "\n";
    os << indent << "DeleteAllIcon: " << this->GetDeleteAllIcon ( ) << "\n";
    os << indent << "DeleteSelectedIcon: " << this->GetDeleteSelectedIcon ( ) << "\n";
    os << indent << "SelectAllIcon: " << this->GetSelectAllIcon() << "\n";
    os << indent << "DeselectAllIcon: " << this->GetDeselectAllIcon() << "\n";
    os << indent << "SearchIcon: " << this->GetSearchIcon() << "\n";
    os << indent << "DownloadIcon: " << this->GetDownloadIcon() << "\n";
    os << indent << "UploadIcon: " << this->GetUploadIcon() << "\n";
    os << indent << "ApplyTagsIcon: " << this->GetApplyTagsIcon() << "\n";
    os << indent << "RemoveTagsIcon: " << this->GetRemoveTagsIcon() << "\n";
    os << indent << "ShowDataTagsIcon: " << this->GetShowDataTagsIcon() << "\n";
}
