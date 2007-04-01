#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkSlicerFileBrowserIcons.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerFileBrowserIcons );
vtkCxxRevisionMacro ( vtkSlicerFileBrowserIcons, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
vtkSlicerFileBrowserIcons::vtkSlicerFileBrowserIcons ( )
{
    this->CreateFolderIcon = vtkKWIcon::New();
    this->GoBackIcon = vtkKWIcon::New();
    this->GoForwardIcon = vtkKWIcon::New();
    this->GoUpIcon = vtkKWIcon::New();
    this->FavoritesIcon = vtkKWIcon::New();
    this->HistoryIcon = vtkKWIcon::New();
    
    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerFileBrowserIcons::~vtkSlicerFileBrowserIcons ( )
{
  if ( this->CreateFolderIcon )
    {
    this->CreateFolderIcon->Delete();
    this->CreateFolderIcon = NULL;
    }
  if ( this->GoBackIcon )
    {
    this->GoBackIcon->Delete();
    this->GoBackIcon = NULL;
    }
  if ( this->GoForwardIcon )
    {
    this->GoForwardIcon->Delete();
    this->GoForwardIcon = NULL;
    }
  if ( this->GoUpIcon )
    {
    this->GoUpIcon->Delete();
    this->GoUpIcon = NULL;
    }
  if ( this->FavoritesIcon )
    {
    this->FavoritesIcon->Delete();
    this->FavoritesIcon = NULL;
    }
  if (this->HistoryIcon )
    {
    this->HistoryIcon->Delete();
    this->HistoryIcon = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerFileBrowserIcons::AssignImageDataToIcons ( )
{
 
  this->CreateFolderIcon->SetImage ( image_FileBrowserCreateNew,
                                     image_FileBrowserCreateNew_width,
                                     image_FileBrowserCreateNew_height,
                                     image_FileBrowserCreateNew_pixel_size,
                                     image_FileBrowserCreateNew_length, 0);                                     
  this->GoBackIcon->SetImage ( image_FileBrowserGoBack,
                               image_FileBrowserGoBack_width,
                               image_FileBrowserGoBack_height,
                               image_FileBrowserGoBack_pixel_size,
                               image_FileBrowserGoBack_length, 0);                               
  this->GoForwardIcon->SetImage(image_FileBrowserGoForward,
                                image_FileBrowserGoForward_width,
                                image_FileBrowserGoForward_height,
                                image_FileBrowserGoForward_pixel_size,
                                image_FileBrowserGoForward_length, 0);                                
  this->GoUpIcon->SetImage (image_FileBrowserGoUp,
                            image_FileBrowserGoUp_width,
                            image_FileBrowserGoUp_height,
                            image_FileBrowserGoUp_pixel_size,
                            image_FileBrowserGoUp_length, 0);                            
  this->FavoritesIcon->SetImage (image_FileBrowserFavorites,
                                 image_FileBrowserFavorites_width,
                                 image_FileBrowserFavorites_height,
                                 image_FileBrowserFavorites_pixel_size,
                                 image_FileBrowserFavorites_length, 0);                                 
  this->HistoryIcon->SetImage (image_FileBrowserHistory,
                               image_FileBrowserHistory_width,
                               image_FileBrowserHistory_height,
                               image_FileBrowserHistory_pixel_size,
                               image_FileBrowserHistory_length, 0);                               
}



//---------------------------------------------------------------------------
void vtkSlicerFileBrowserIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerFileBrowserIcons: " << this->GetClassName ( ) << "\n";
    os << indent << "CreateFolderIcon: " << this->GetCreateFolderIcon ( ) << "\n";
    os << indent << "GoBackIcon: " << this->GetGoBackIcon () << "\n";
    os << indent << "GoForwardIcon: " << this->GetGoForwardIcon() << "\n";
    os << indent << "GoUpIcon: " << this->GetGoUpIcon() << "\n";
    os << indent << "FavoritesIcon: " << this->GetFavoritesIcon() << "\n";
    os << indent << "HistoryIcon: " << this->GetHistoryIcon() << "\n";
}
