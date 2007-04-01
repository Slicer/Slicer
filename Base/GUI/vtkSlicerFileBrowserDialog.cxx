#include "vtkSlicerFileBrowserDialog.h"
#include "vtkKWFileBrowserWidget.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWFavoriteDirectoriesFrame.h"
#include "vtkKWDirectoryExplorer.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonWithMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkSlicerFileBrowserIcons.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerFileBrowserDialog );
vtkCxxRevisionMacro(vtkSlicerFileBrowserDialog, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
vtkSlicerFileBrowserDialog::vtkSlicerFileBrowserDialog()
{
  this->Icons = vtkSlicerFileBrowserIcons::New();
}

//----------------------------------------------------------------------------
vtkSlicerFileBrowserDialog::~vtkSlicerFileBrowserDialog()
{

  if ( this->Icons )
    {
    this->Icons->Delete();
    this->Icons = NULL;
    }

}

//----------------------------------------------------------------------------
void vtkSlicerFileBrowserDialog::CreateWidget()
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // fix background color...
  // to do, use color as specified in vtkSlicerColors.cxx...GUIBgColor();
  this->GetFileBrowserWidget()->GetFavoriteDirectoriesFrame()->SetContainerFrameBackgroundColor (1.0, 1.0, 1.0);


  // set icons to our own...
  this->GetFileBrowserWidget()->GetDirectoryExplorer()->GetCreateFolderButton()->
    SetImageToIcon (this->Icons->GetCreateFolderIcon () );
  this->GetFileBrowserWidget()->GetDirectoryExplorer()->GetBackButton()->GetPushButton()->
    SetImageToIcon (this->Icons->GetGoBackIcon());
  this->GetFileBrowserWidget()->GetDirectoryExplorer()->GetBackButton()->GetMenuButton()->
    SetImageToIcon ( this->Icons->GetHistoryIcon());
  this->GetFileBrowserWidget()->GetDirectoryExplorer()->GetForwardButton()->GetPushButton()->
    SetImageToIcon (this->Icons->GetGoForwardIcon());
  this->GetFileBrowserWidget()->GetDirectoryExplorer()->GetForwardButton()->GetMenuButton()->
    SetImageToIcon ( this->Icons->GetHistoryIcon());
  this->GetFileBrowserWidget()->GetDirectoryExplorer()->GetUpButton()->
    SetImageToIcon (this->Icons->GetGoUpIcon());
  this->GetFileBrowserWidget()->GetFavoriteDirectoriesFrame()->GetAddFavoriteDirectoryButton()->
    SetImageToIcon(this->Icons->GetFavoritesIcon());


  this->Update();
}

//----------------------------------------------------------------------------
void vtkSlicerFileBrowserDialog::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
