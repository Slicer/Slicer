#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include <itksys/SystemTools.hxx> 

#include "vtkSlicerMRMLSaveDataWidget.h"

#include "vtkKWDialog.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWCheckButton.h"

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
  this->SaveDialog = NULL;
  this->SaveSceneButton = NULL;
  this->SceneName = NULL;
  this->SaveSceneCheckBox = NULL;
  this->SaveDataButton = NULL;

  this->DataDirectoryName = NULL;
  this->OkButton = NULL;
  this->MultiColumnList = NULL;

  this->DataDirectoryExists = false;
  this->IsProcessing = false;
}


//---------------------------------------------------------------------------
vtkSlicerMRMLSaveDataWidget::~vtkSlicerMRMLSaveDataWidget ( )
{
  if (this->MultiColumnList)
    {
    this->MultiColumnList->SetParent(NULL);
    this->MultiColumnList->Delete();
    }
  if (this->OkButton)
    {
    this->OkButton->SetParent(NULL);
    this->OkButton->Delete();
    }
  if (this->SaveDialog)
    {
    this->SaveDialog->SetParent(NULL);
    this->SaveDialog->Delete();
    }
  if (this->SaveSceneButton)
    {
    this->SaveSceneButton->SetParent(NULL);
    this->SaveSceneButton->Delete();
    }
  if (this->SceneName)
    {
    this->SceneName->SetParent(NULL);
    this->SceneName->Delete();
    }
  if (this->SaveSceneCheckBox)
    {
    this->SaveSceneCheckBox->SetParent(NULL);
    this->SaveSceneCheckBox->Delete();
    }
  if (this->SaveDataButton)
    {
    this->SaveDataButton->SetParent(NULL);
    this->SaveDataButton->Delete();
    }

  
  this->SetDataDirectoryName( NULL);
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

  if (this->SaveSceneButton->GetWidget() == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent) 
    {
    char *fileName = this->SaveSceneButton->GetWidget()->GetFileName();
    if (fileName && !this->DataDirectoryExists) 
      {
      vtksys_stl::string dir =  vtksys::SystemTools::GetParentDirectory(fileName);   
      dir = dir + vtksys_stl::string("/");
      this->SetDataDirectoryName(dir.c_str());
      if (this->UpdateFromMRML() == 0)
        {
        /**
        if (this->SaveSceneCheckBox->GetSelectedState())
          {
          this->SaveScene();
          }
        this->SaveDialog->OK();
        this->InvokeEvent(vtkSlicerMRMLSaveDataWidget::DataSavedEvent);
        **/
        }
      }
    }

  else if (this->SaveDataButton->GetWidget() == vtkKWPushButton::SafeDownCast(caller) && event == vtkKWPushButton::InvokedEvent) 
    {
    char *fileName = this->SaveDataButton->GetWidget()->GetFileName();
    if (fileName) 
      {
      this->DataDirectoryExists = true;
      std::string name(fileName);
      name += "/";
      this->SetDataDirectoryName(name.c_str());
      this->UpdateFromMRML();
      }
    }
  else if (this->OkButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    {
    int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
    for (int row=0; row<nrows; row++)
      {
      if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, 1))
        {
        vtkMRMLNode *node = this->MRMLScene->GetNodeByID(this->Nodes[row].c_str());
        vtkMRMLStorageNode* snode = vtkMRMLStorageNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->StorageNodes[row].c_str()));
        std::string fileName (this->MultiColumnList->GetWidget()->GetCellText(row, 3));
        const char *filePath = fileName.c_str();
        snode->SetFileName(filePath);
        int res = snode->WriteData(node);
        if (res)
          {
          node->SetModifiedSinceRead(0);
          }
        }
      }
    char *fileName = this->SaveSceneButton->GetWidget()->GetFileName();
    if (fileName == NULL)
      {
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->GetParent() );
      
      message->SetStyleToMessage ();
      message->SetText("No scene file selected. Scene is not saved");
      message->Create();
      message->Invoke();
      message->Delete();
      }
    if (this->SaveSceneCheckBox->GetSelectedState())
      {
      this->SaveScene();
      }
    this->SaveDialog->OK();
    this->InvokeEvent(vtkSlicerMRMLSaveDataWidget::DataSavedEvent);
    }
  
} 

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SaveScene()
{
  char *fileName = this->SaveSceneButton->GetWidget()->GetFileName();

  if (fileName && this->GetMRMLScene()) 
    {
    // convert absolute paths to relative
    this->MRMLScene->InitTraversal();
    
    vtkMRMLNode *node;
    while ( (node = this->MRMLScene->GetNextNodeByClass("vtkMRMLStorageNode") ) != NULL)
      {
      vtkMRMLStorageNode *snode = vtkMRMLStorageNode::SafeDownCast(node);
      if (!this->MRMLScene->IsFilePathRelative(snode->GetFileName()))
        {
        vtksys_stl::string directory = vtksys::SystemTools::GetParentDirectory(fileName);   
        directory = directory + vtksys_stl::string("/");
        
        itksys_stl::string relPath = itksys::SystemTools::RelativePath((const char*)directory.c_str(), snode->GetFileName());
        snode->SetFileName(relPath.c_str());
        }
      }

    this->GetMRMLScene()->SetURL(fileName);
    this->GetMRMLScene()->Commit();  
    }
}

//---------------------------------------------------------------------------
int vtkSlicerMRMLSaveDataWidget::UpdateFromMRML()
  
{
  if (this->IsProcessing) 
  {
    return 0;
  }

  this->IsProcessing = true;

  this->Nodes.clear();
  this->StorageNodes.clear();
  
  if (this->DataDirectoryName == NULL)
  {
    this->MultiColumnList->SetEnabled(0);
    this->IsProcessing = false;
    return 0;
  }
  if (this->SaveSceneButton->GetWidget()->GetFileName())
    {
    this->SaveSceneCheckBox->SetEnabled(1);
    }
  this->MultiColumnList->SetEnabled(1);
  this->OkButton->SetEnabled(1);

  this->MRMLScene->InitTraversal();
  vtkMRMLNode *node;
  int row = 0;

  std::string filename (this->DataDirectoryName);
  this->MultiColumnList->GetWidget()->DeleteAllRows ();

  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLVolumeNode");
  int n;
  for (n=0; n<nnodes; n++)
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLVolumeNode");
    if (node->GetModifiedSinceRead()) 
      {
      vtkMRMLVolumeNode *vnode = vtkMRMLVolumeNode::SafeDownCast(node);
      vtkMRMLStorageNode* snode = vnode->GetStorageNode();
      std::string name (filename);
      if (snode == NULL) \
        {
        vtkMRMLVolumeArchetypeStorageNode *storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
        storageNode->SetScene(this->GetMRMLScene());
        this->SetMRMLScene(this->GetMRMLScene());
        this->GetMRMLScene()->AddNode(storageNode);  
        this->SetAndObserveMRMLScene(this->GetMRMLScene());
        vnode->SetStorageNodeID(storageNode->GetID());
        storageNode->Delete();
        snode = storageNode;
        }
      name += std::string(node->GetName());
      name += std::string(".nrrd");
      snode->SetFileName(name.c_str());

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

  nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLModelNode");
  for (n=0; n<nnodes; n++)
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLModelNode");
    if (node->GetModifiedSinceRead() && !node->GetHideFromEditors()) 
      {
      vtkMRMLModelNode *vnode = vtkMRMLModelNode::SafeDownCast(node);
      vtkMRMLStorageNode* snode = vnode->GetStorageNode();
      std::string name (filename);
      if (snode == NULL) \
        {
        vtkMRMLModelStorageNode *storageNode = vtkMRMLModelStorageNode::New();
        storageNode->SetScene(this->GetMRMLScene());
        this->SetMRMLScene(this->GetMRMLScene());
        this->GetMRMLScene()->AddNode(storageNode);  
        this->SetAndObserveMRMLScene(this->GetMRMLScene());
        vnode->SetStorageNodeID(storageNode->GetID());
        storageNode->Delete();
        snode = storageNode;
        }
      name += std::string(node->GetName());
      name += std::string(".vtk");
      snode->SetFileName(name.c_str());

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
  
  this->IsProcessing = false;

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
  this->SaveSceneButton->GetWidget()->RemoveObservers( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveDataButton->GetWidget()->RemoveObservers( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

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

  this->SaveDialog = vtkKWDialog::New();
  this->SaveDialog->SetParent ( this->GetParent());
  this->SaveDialog->SetTitle("Save Scene and Unsaved Data");
  this->SaveDialog->SetSize(400, 200);
  this->SaveDialog->Create ( );

  // Scene Frame
  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New ( );
  frame->SetParent ( this->SaveDialog );
  frame->Create ( );
  frame->SetLabelText ("Save Scene");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 frame->GetWidgetName() );
  

  // add a scene file browser 
  this->SaveSceneButton = vtkKWLoadSaveButtonWithLabel::New ( );
  this->SaveSceneButton->SetParent ( frame->GetFrame() );
  this->SaveSceneButton->Create ( );
  this->SaveSceneButton->SetWidth(20);
  this->SaveSceneButton->GetWidget()->SetText ("Select Scene File");
  this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
                                                            "{ {scene} {*.mrml} }");
  this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->SaveDialogOn();
  this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
    "OpenPath");
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
              this->SaveSceneButton->GetWidgetName());
  // scene name
  this->SceneName = vtkKWEntryWithLabel::New();
  /***
  this->SceneName->SetParent(frame->GetFrame());
  this->SceneName->Create();
  this->SceneName->SetWidth(20);
  this->SceneName->SetLabelWidth(12);
  this->SceneName->SetLabelText("Scene Name:");
  this->SceneName->GetWidget()->SetValue ( "" );
  this->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->SceneName->GetWidgetName());
  ***/

  // save the scene
  this->SaveSceneCheckBox = vtkKWCheckButton::New();
  this->SaveSceneCheckBox->SetParent(frame->GetFrame());
  this->SaveSceneCheckBox->Create();
  this->SaveSceneCheckBox->SelectedStateOn();
  this->SaveSceneCheckBox->SetText("Save Scene");
  this->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
    this->SaveSceneCheckBox->GetWidgetName());

  // Data Frame
  vtkKWFrameWithLabel *dataFrame = vtkKWFrameWithLabel::New ( );
  dataFrame->SetParent ( this->SaveDialog );
  dataFrame->Create ( );
  dataFrame->SetLabelText ("Save Data");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 dataFrame->GetWidgetName() );


  // add a data file browser 
  this->SaveDataButton = vtkKWLoadSaveButtonWithLabel::New ( );
  this->SaveDataButton->SetParent ( dataFrame->GetFrame() );
  this->SaveDataButton->Create ( );
  this->SaveDataButton->SetWidth(20);
  this->SaveDataButton->GetWidget()->SetText ("Select Data Directory");
  this->SaveDataButton->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
  this->SaveDataButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
    "OpenPath");
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
                 this->SaveDataButton->GetWidgetName());

  // add the multicolumn list to show the points
  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( dataFrame->GetFrame() );
  this->MultiColumnList->Create ( );
  this->MultiColumnList->SetHeight(4);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
    
  // set up the columns of data for each point
  // name, x, y, z, orientation w, x, y, z, selected
  this->MultiColumnList->GetWidget()->AddColumn("Node Name");
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
    
  // save Frame
  vtkKWFrame *saveFrame = vtkKWFrame::New ( );
  saveFrame->SetParent ( this->SaveDialog );
  saveFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 saveFrame->GetWidgetName() );

  // add OK button
  this->OkButton = vtkKWPushButton::New ( );
  this->OkButton->SetParent ( saveFrame );
  this->OkButton->Create ( );
  this->OkButton->SetText ("Save");
  this->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
              this->OkButton->GetWidgetName());
  

  // add observers
  this->OkButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SaveSceneButton->GetWidget()->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveDataButton->GetWidget()->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  
  
  if (this->MRMLScene != NULL)
    {
    this->SetAndObserveMRMLScene(this->MRMLScene);
    }
  
   
  this->MultiColumnList->SetEnabled(0);
  this->OkButton->SetEnabled(0);
  this->SaveSceneCheckBox->SetEnabled(0);
    
  frame->Delete();
  dataFrame->Delete();
  saveFrame->Delete();
  
  this->SaveDialog->Invoke ( );
  
}

