

#include "vtkObjectFactory.h"
#include "vtkSlicerSaveDataWidgetIcons.h"
 
//---------------------------------------------------------------------------
vtkStandardNewMacro ( vtkSlicerSaveDataWidgetIcons );
vtkCxxRevisionMacro ( vtkSlicerSaveDataWidgetIcons, "$Revision$");

//---------------------------------------------------------------------------
vtkSlicerSaveDataWidgetIcons::vtkSlicerSaveDataWidgetIcons ( )
{
    this->CheckModifiedData = vtkKWIcon::New ( );
    this->CheckModified = vtkKWIcon::New ( );
    this->CheckAll = vtkKWIcon::New ( );
    this->UncheckAll = vtkKWIcon::New ( );    
    this->AssignImageDataToIcons ( );
}


//---------------------------------------------------------------------------
vtkSlicerSaveDataWidgetIcons::~vtkSlicerSaveDataWidgetIcons ( )
{

    if ( this->CheckAll ) {
            this->CheckAll->Delete ( );
            this->CheckAll = NULL;
        }
    if ( this->CheckModifiedData ) {
            this->CheckModifiedData->Delete ( );
            this->CheckModifiedData = NULL;
        }
    if ( this->UncheckAll ) {
            this->UncheckAll->Delete ( );
            this->UncheckAll = NULL;
        }
    if ( this->CheckModified ) {
            this->CheckModified->Delete ( );
            this->CheckModified = NULL;
        }
}

//---------------------------------------------------------------------------
void vtkSlicerSaveDataWidgetIcons::AssignImageDataToIcons ( )
{
  this->CheckAll->SetImage( image_CheckAll,
                                image_CheckAll_width,
                                image_CheckAll_height,
                                image_CheckAll_pixel_size,
                                image_CheckAll_length, 0);
  this->UncheckAll->SetImage( image_UncheckAll,
                                image_UncheckAll_width,
                                image_UncheckAll_height,
                                image_UncheckAll_pixel_size,
                                image_UncheckAll_length, 0);
  this->CheckModifiedData->SetImage( image_CheckModifiedData,
                                image_CheckModifiedData_width,
                                image_CheckModifiedData_height,
                                image_CheckModifiedData_pixel_size,
                                image_CheckModifiedData_length, 0);
  this->CheckModified->SetImage( image_CheckModified,
                                image_CheckModified_width,
                                image_CheckModified_height,
                                image_CheckModified_pixel_size,
                                image_CheckModified_length, 0);

}



//---------------------------------------------------------------------------
void vtkSlicerSaveDataWidgetIcons::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "SlicerBaseAcknowledgementLogoIcons: " << this->GetClassName( ) << "\n";
    os << indent << "CheckAll: " << this->GetCheckAll( ) << "\n";
    os << indent << "CheckModifiedData: " << this->GetCheckModifiedData( ) << "\n";
    os << indent << "UncheckAll: " << this->GetUncheckAll( ) << "\n";
    os << indent << "CheckModified: " << this->GetCheckModified( ) << "\n";

}
