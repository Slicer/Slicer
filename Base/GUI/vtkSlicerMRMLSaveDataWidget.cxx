#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerMRMLSaveDataWidget.h"

#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLModelStorageNode.h"

#include <vtksys/stl/string>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerMRMLSaveDataWidget );
vtkCxxRevisionMacro ( vtkSlicerMRMLSaveDataWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerMRMLSaveDataWidget::vtkSlicerMRMLSaveDataWidget ( )
{
  this->FileDirectoryName = NULL;
  this->OkButton = NULL;
  this->MultiColumnList = NULL;
  
}


//---------------------------------------------------------------------------
vtkSlicerMRMLSaveDataWidget::~vtkSlicerMRMLSaveDataWidget ( )
{
  if (this->MultiColumnList)
    {
    this->MultiColumnList->Delete();
    }
  if (this->OkButton)
    {
    this->OkButton->Delete();
    }
  
  this->SetFileDirectoryName( NULL);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerMRMLSaveDataWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                    unsigned long event, 
                                                    void *callData )
{
  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);
  if (button == this->OkButton && event ==  vtkKWPushButton::InvokedEvent)
    {
    int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
    for (int row=0; row<nrows; row++)
      {
      if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, 1))
        {
        vtkMRMLNode *node = this->MRMLScene->GetNodeByID(this->Nodes[row].c_str());
        vtkMRMLStorageNode* snode = vtkMRMLStorageNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->StorageNodes[row].c_str()));
        std::string fileName (this->MultiColumnList->GetWidget()->GetCellText(row, 3));
        snode->SetFileName(fileName.c_str());
        int res = snode->WriteData(node);
        if (res)
          {
          node->SetModifiedSinceRead(0);
          }
        }
      }
    }
  this->InvokeEvent(vtkSlicerMRMLSaveDataWidget::DataSavedEvent);
  
} 

//---------------------------------------------------------------------------
int vtkSlicerMRMLSaveDataWidget::UpdateFromMRML()
  
{
  this->Nodes.clear();
  this->StorageNodes.clear();
  
 if (this->FileDirectoryName == NULL)
  {
    return 0;
  }

  this->MRMLScene->InitTraversal();
  vtkMRMLNode *node;
  int row = 0;

  std::string filename (this->FileDirectoryName);

  while ( (node = this->MRMLScene->GetNextNodeByClass("vtkMRMLVolumeNode") ) != NULL)
    {
    if (node->GetModifiedSinceRead()) 
      {
      vtkMRMLVolumeNode *vnode = vtkMRMLVolumeNode::SafeDownCast(node);
      vtkMRMLStorageNode* snode = vnode->GetStorageNode();
      if (snode == NULL) \
        {
        vtkMRMLVolumeArchetypeStorageNode *storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
        storageNode->SetScene(this->GetMRMLScene());
        this->SetMRMLScene(this->GetMRMLScene());
        this->GetMRMLScene()->AddNode(storageNode);  
        this->SetAndObserveMRMLScene(this->GetMRMLScene());
        vnode->SetStorageNodeID(storageNode->GetID());
        storageNode->Delete();

        filename += std::string(node->GetName());
        filename += std::string(".nrrd");
        storageNode->SetFileName(filename.c_str());
        storageNode->SetAbsoluteFileName(1);
        snode = storageNode;
        }
      this->Nodes.push_back(node->GetID());
      this->StorageNodes.push_back(snode->GetID());
      
      this->MultiColumnList->GetWidget()->AddRow();
      this->MultiColumnList->GetWidget()->SetCellText(row,0,node->GetName());
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,1,1);
      this->MultiColumnList->GetWidget()->SetCellText(row,2,"Volume");
      this->MultiColumnList->GetWidget()->SetCellText(row,3,snode->GetFileName());
      
      row++;
      }
    }
  while ( (node = this->MRMLScene->GetNextNodeByClass("vtkMRMLModelNode") ) != NULL)
    {
    if (node->GetModifiedSinceRead()) 
      {
      vtkMRMLModelNode *vnode = vtkMRMLModelNode::SafeDownCast(node);
      vtkMRMLStorageNode* snode = vnode->GetStorageNode();
      if (snode == NULL) \
        {
        vtkMRMLModelStorageNode *storageNode = vtkMRMLModelStorageNode::New();
        storageNode->SetScene(this->GetMRMLScene());
        this->SetMRMLScene(this->GetMRMLScene());
        this->GetMRMLScene()->AddNode(storageNode);  
        this->SetAndObserveMRMLScene(this->GetMRMLScene());
        vnode->SetStorageNodeID(storageNode->GetID());
        storageNode->Delete();

        filename += std::string(node->GetName());
        filename += std::string(".vtk");
        storageNode->SetFileName(filename.c_str());
        snode = storageNode;
        }
      this->Nodes.push_back(node->GetID());
      this->StorageNodes.push_back(snode->GetID());

      this->MultiColumnList->GetWidget()->AddRow();
      this->MultiColumnList->GetWidget()->SetCellText(row,0,node->GetName());
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,1,1);
      this->MultiColumnList->GetWidget()->SetCellText(row,2,"Model");
      this->MultiColumnList->GetWidget()->SetCellText(row,3,snode->GetFileName());
      
      row++;
      }
    }
  return row;
  
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *callData )
{
  if (event == vtkCommand::ModifiedEvent)
    {
    this->UpdateFromMRML();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::RemoveWidgetObservers ( ) 
{
  this->OkButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::CreateWidget ( )
{
  // Check if already created

  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  // Frame
  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New ( );
  frame->SetParent ( this->GetParent() );
  frame->Create ( );
  frame->SetLabelText ("Save Data");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );
  
  
  // add the multicolumn list to show the points
  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( frame->GetFrame() );
  this->MultiColumnList->Create ( );
  this->MultiColumnList->SetHeight(4);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
    
  // set up the columns of data for each point
  // name, x, y, z, orientation w, x, y, z, selected
  this->MultiColumnList->GetWidget()->AddColumn("Name");
  this->MultiColumnList->GetWidget()->ColumnEditableOff(0);
  this->MultiColumnList->GetWidget()->SetColumnWidth(0, 12);

  this->MultiColumnList->GetWidget()->AddColumn("Save");
  this->MultiColumnList->GetWidget()->SetColumnWidth(1, 8);
  this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(1);
  this->MultiColumnList->GetWidget()->ColumnEditableOn(1);

  this->MultiColumnList->GetWidget()->AddColumn("Type");
  this->MultiColumnList->GetWidget()->ColumnEditableOff(2);
  this->MultiColumnList->GetWidget()->SetColumnWidth(2, 10);

  this->MultiColumnList->GetWidget()->AddColumn("File");
  this->MultiColumnList->GetWidget()->ColumnEditableOn(3);
  this->MultiColumnList->GetWidget()->SetColumnWidth(3, 64);
  
 
  // make the save column editable by checkbox
  
  
  // now set the attributes that are equal across the columns
  int col;
  for (col = 0; col < 4; col++)
    {        
    this->MultiColumnList->GetWidget()->SetColumnAlignmentToLeft(col);
    }
  this->Script ( "pack %s -fill both -expand true",
                this->MultiColumnList->GetWidgetName());

  // this->MultiColumnList->GetWidget()->SetCellUpdatedCommand(this, "UpdateElement");
    
  // add OK button
  this->OkButton = vtkKWPushButton::New ( );
  this->OkButton->SetParent ( frame->GetFrame() );
  this->OkButton->Create ( );
  this->OkButton->SetText ("Save");
  
  this->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
              this->OkButton->GetWidgetName());

  // add observers
  this->OkButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  
  
  if (this->MRMLScene != NULL)
    {
    this->MRMLScene->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
    }
  
    
  frame->Delete();
  
  this->UpdateFromMRML();
  
}

