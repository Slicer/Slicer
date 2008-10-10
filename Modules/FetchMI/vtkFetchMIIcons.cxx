
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
  this->CopyTagsIcon = vtkKWIcon::New();
  this->ApplyTagsIcon = vtkKWIcon::New();
  this->TagTableIcon = vtkKWIcon::New();
  this->QueryTagsIcon = vtkKWIcon::New();
  this->ShowDataTagsIcon = vtkKWIcon::New();
  
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
  if ( this->CopyTagsIcon )
    {
    this->CopyTagsIcon->Delete();
    this->CopyTagsIcon =NULL;    
    }
  if ( this->ApplyTagsIcon )
    {
    this->ApplyTagsIcon->Delete();
    this->ApplyTagsIcon = NULL;    
    }
  if ( this->TagTableIcon )
    {
    this->TagTableIcon->Delete();
    this->TagTableIcon = NULL;
    }
  if ( this->QueryTagsIcon )
    {
    this->QueryTagsIcon->Delete();
    this->QueryTagsIcon = NULL;
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

  this->DeleteAllIcon->SetImage (image_DeleteAll,
                                 image_DeleteAll_width,
                                 image_DeleteAll_height,
                                 image_DeleteAll_pixel_size,
                                 image_DeleteAll_length, 0 );
  this->DeleteSelectedIcon->SetImage ( image_DeleteSelected,
                                       image_DeleteSelected_width,
                                       image_DeleteSelected_height,
                                       image_DeleteSelected_pixel_size,
                                       image_DeleteSelected_length, 0 );
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
  this->CopyTagsIcon->SetImage ( image_CopyTags,
                               image_CopyTags_width,
                               image_CopyTags_height,
                               image_CopyTags_pixel_size,
                               image_CopyTags_length, 0 );
  this->ApplyTagsIcon->SetImage ( image_ApplyTags,
                               image_ApplyTags_width,
                               image_ApplyTags_height,
                               image_ApplyTags_pixel_size,
                               image_ApplyTags_length, 0 );
  this->TagTableIcon->SetImage ( image_TagTable,
                               image_TagTable_width,
                               image_TagTable_height,
                               image_TagTable_pixel_size,
                               image_TagTable_length, 0 );
  this->QueryTagsIcon->SetImage ( image_QueryTags,
                               image_QueryTags_width,
                               image_QueryTags_height,
                               image_QueryTags_pixel_size,
                               image_QueryTags_length, 0 );
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
    os << indent << "DeleteAllIcon: " << this->GetDeleteAllIcon ( ) << "\n";
    os << indent << "DeleteSelectedIcon: " << this->GetDeleteSelectedIcon ( ) << "\n";
    os << indent << "SelectAllIcon: " << this->GetSelectAllIcon() << "\n";
    os << indent << "DeselectAllIcon: " << this->GetDeselectAllIcon() << "\n";
    os << indent << "SearchIcon: " << this->GetSearchIcon() << "\n";
    os << indent << "DownloadIcon: " << this->GetDownloadIcon() << "\n";
    os << indent << "UploadIcon: " << this->GetUploadIcon() << "\n";
    os << indent << "CopyTagsIcon: " << this->GetCopyTagsIcon() << "\n";
    os << indent << "ApplyTagsIcon: " << this->GetApplyTagsIcon() << "\n";
    os << indent << "TagTableIcon: " << this->GetTagTableIcon() << "\n";
    os << indent << "QueryTagsIcon: " << this->GetQueryTagsIcon() << "\n";
    os << indent << "ShowDataTagsIcon: " << this->GetShowDataTagsIcon() << "\n";
}
