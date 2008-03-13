#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include <itksys/SystemTools.hxx> 

#include "vtkSlicerMRMLSaveDataWidget.h"

#include "vtkKWTopLevel.h"
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
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"

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
  this->CancelButton = NULL;
  this->MultiColumnList = NULL;

  this->IsProcessing = false;
}


//---------------------------------------------------------------------------
vtkSlicerMRMLSaveDataWidget::~vtkSlicerMRMLSaveDataWidget ( )
{
  this->RemoveWidgetObservers();

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
  if (this->CancelButton)
    {
    this->CancelButton->SetParent(NULL);
    this->CancelButton->Delete();
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

  if (this->SaveSceneButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent )
    {
    const char *fileName = this->SaveSceneButton->GetWidget()->GetFileName();
    if (fileName)
      {
      this->SaveSceneCheckBox->SetEnabled(1);
      this->SaveSceneCheckBox->Select();
      if (this->DataDirectoryName == NULL) 
        {
        vtksys_stl::string dir =  
          vtksys::SystemTools::GetParentDirectory(fileName);   
        if (dir[dir.size()-1] != '/')
          {
          dir = dir + vtksys_stl::string("/");
          }
        this->SetDataDirectoryName(dir.c_str());
        this->UpdateDataDirectory();
        }
      }
    }

  else if (this->SaveDataButton->GetWidget()->GetLoadSaveDialog() == vtkKWLoadSaveDialog::SafeDownCast(caller) && event == vtkKWTopLevel::WithdrawEvent )
    {
    const char *fileName = this->SaveDataButton->GetWidget()->GetFileName();
    if (fileName) 
      {
      std::string name(fileName);
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      this->SetDataDirectoryName(name.c_str());
      this->UpdateDataDirectory();
      }
    }
  else if (this->OkButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    {
    this->MultiColumnList->GetWidget()->FinishEditing();
    int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
    for (int row=0; row<nrows; row++)
      {
      if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, 2))
        {
        vtkMRMLNode *node = this->MRMLScene->GetNodeByID(this->Nodes[row].c_str());
        vtkMRMLStorageNode* snode = vtkMRMLStorageNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->StorageNodes[row].c_str()));
        std::string fileName (this->MultiColumnList->GetWidget()->GetCellText(row, 4));
        const char *filePath = fileName.c_str();
        snode->SetFileName(filePath); 

        //: ask override
        int  writeFile = 1;
        fstream fin;
        fin.open(filePath,ios::in);
        if( fin.is_open() )
          {
          vtkKWMessageDialog *message = vtkKWMessageDialog::New();
          message->SetParent ( this->GetParent() );
          message->SetMasterWindow ( this->SaveDialog );
          message->SetStyleToYesNo();
          std::string msg = "File " + fileName + " exists. Do you want to override it?";
          message->SetText(msg.c_str());
          message->Create();
          writeFile = message->Invoke();
          message->Delete();
          }          
        fin.close();
        if (writeFile)
          {
          int res = snode->WriteData(node);
          if (res)
            {
            node->SetModifiedSinceRead(0);
            } 
          else
            {
            vtkKWMessageDialog *message = vtkKWMessageDialog::New();
            message->SetParent ( this->GetParent() );      
            message->SetMasterWindow ( this->SaveDialog );
            message->SetStyleToOkCancel();
            std::string msg = "Cannot write data file " + fileName + ". Do you want to continue saving?";
            message->SetText(msg.c_str());
            message->Create();
            int ok = message->Invoke();
            message->Delete(); 
            if (!ok)
              {
              return;
              }
            }
          }
        }
      }
    const char *fileName = this->SaveSceneButton->GetWidget()->GetFileName();
    if (!fileName && this->MRMLScene)
      {
      fileName = this->MRMLScene->GetURL();
      }
    if (fileName == NULL)
      {
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->GetParent() );
      message->SetMasterWindow ( this->SaveDialog );
      message->SetStyleToMessage ();
      message->SetText("No scene file selected. Scene is not saved");
      message->Create();
      message->Invoke();
      message->Delete();
      }
    else
      {
      if (this->SaveSceneCheckBox->GetSelectedState())
        {
        this->SaveScene();
        }
      this->SaveDialog->OK();
      this->InvokeEvent(vtkSlicerMRMLSaveDataWidget::DataSavedEvent);
      }
    }
   else if (this->CancelButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    { 
    this->SaveDialog->Cancel();
    }
} 


//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SaveScene()
{
  // Pick selected filename. If none, use the old one
  const char *fileName = this->SaveSceneButton->GetWidget()->GetFileName();
  if (!fileName && this->MRMLScene)
    {
    fileName = this->MRMLScene->GetURL();
    }

  if (fileName && this->GetMRMLScene()) 
    {
    vtksys_stl::string directory = 
      vtksys::SystemTools::GetParentDirectory(fileName);
    this->MRMLScene->SetRootDirectory(directory.c_str());
    directory = directory + vtksys_stl::string("/");

    // convert absolute paths to relative
    vtkMRMLScene *scene = this->GetMRMLScene();
    vtkMRMLNode *node;
    int nnodes = scene->GetNumberOfNodesByClass("vtkMRMLStorageNode");
    for (int n=0; n<nnodes; n++)
      {
      node = scene->GetNthNodeByClass(n, "vtkMRMLStorageNode");
      vtkMRMLStorageNode *snode = vtkMRMLStorageNode::SafeDownCast(node);
      if (snode->GetFileName() && 
          !this->MRMLScene->IsFilePathRelative(snode->GetFileName()))
        {        
        itksys_stl::string relPath = itksys::SystemTools::RelativePath(
          (const char*)directory.c_str(), snode->GetFileName());
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
  if (!this->IsCreated())
    {
    return 0;
    }
  if (this->IsProcessing) 
    {
    return 0;
    }
  
  this->IsProcessing = true;

  this->Nodes.clear();
  this->StorageNodes.clear();

  vtkMRMLNode *node;
  int nModified = 0;
  if (this->MultiColumnList->GetWidget()->GetNumberOfRows())
    {
    this->MultiColumnList->GetWidget()->DeleteAllRows ();
    }

  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLVolumeNode");
  int n;
  int row = 0;
  for (n=0; n<nnodes; n++)
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLVolumeNode");
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    vtkMRMLVolumeNode *vnode = vtkMRMLVolumeNode::SafeDownCast(node);
    vtkMRMLStorageNode* snode = vnode->GetStorageNode();
    if (snode == NULL) 
      {
      vtkMRMLStorageNode *storageNode;
      if ( vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(node) || 
            vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(node) )
        {
        storageNode = vtkMRMLNRRDStorageNode::New();
        }
      else
        {
        storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
        }
      storageNode->SetScene(this->GetMRMLScene());
      this->SetMRMLScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode);  
      this->SetAndObserveMRMLScene(this->GetMRMLScene());
      vnode->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = storageNode;
      }
    if (snode->GetFileName() == NULL && this->DataDirectoryName != NULL) 
      {
      std::string name (this->DataDirectoryName);
      name += std::string(node->GetName());
      name += std::string(".nrrd");
      snode->SetFileName(name.c_str());
      }

    // get absolute filename
    std::string name;
    if (this->MRMLScene->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->MRMLScene->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();

    this->Nodes.push_back(node->GetID());
    this->StorageNodes.push_back(snode->GetID());
    
    this->MultiColumnList->GetWidget()->AddRow();
    this->MultiColumnList->GetWidget()->SetCellText(row,0,node->GetName());
    if (node->GetModifiedSinceRead()) 
      {
      this->MultiColumnList->GetWidget()->SetCellText(row,1,"Modified");
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,2,1);
      nModified++;
      }
    else
      {
      this->MultiColumnList->GetWidget()->SetCellText(row,1,"NotModified");
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,2,0);
      }
    this->MultiColumnList->GetWidget()->SetCellText(row,3,"Volume");
    this->MultiColumnList->GetWidget()->SetCellText(row,4,name.c_str());
    this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row, 2);

    row++;
    }

  nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLModelNode");
  for (n=0; n<nnodes; n++)
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLModelNode");
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    vtkMRMLModelNode *vnode = vtkMRMLModelNode::SafeDownCast(node);
    vtkMRMLStorageNode* snode = vnode->GetStorageNode();
    if (snode == NULL && !node->GetModifiedSinceRead())
      {
      continue;
      }
    if (snode == NULL && node->GetModifiedSinceRead()) 
      {
      vtkMRMLModelStorageNode *storageNode = vtkMRMLModelStorageNode::New();
      storageNode->SetScene(this->GetMRMLScene());
      this->SetMRMLScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode);  
      this->SetAndObserveMRMLScene(this->GetMRMLScene());
      vnode->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = storageNode;
      }
    if (snode->GetFileName() == NULL && this->DataDirectoryName != NULL) {
      std::string name (this->DataDirectoryName);
      name += std::string(node->GetName());
      name += std::string(".vtk");
      snode->SetFileName(name.c_str());
    }

    // get absolute filename
    std::string name;
    if (this->MRMLScene->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->MRMLScene->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();

    this->Nodes.push_back(node->GetID());
    this->StorageNodes.push_back(snode->GetID());
    
    this->MultiColumnList->GetWidget()->AddRow();
    this->MultiColumnList->GetWidget()->SetCellText(row,0,node->GetName());
    if (node->GetModifiedSinceRead()) 
      {
      this->MultiColumnList->GetWidget()->SetCellText(row,1,"Modified");
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,2,1);
      nModified++;
      }
    else
      {
      this->MultiColumnList->GetWidget()->SetCellText(row,1,"NotModified");
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,2,0);
      }
    this->MultiColumnList->GetWidget()->SetCellText(row,3,"Model");
    this->MultiColumnList->GetWidget()->SetCellText(row,4,name.c_str());
    this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row, 2);
    row++;
    }
  
#if !defined(MESHING_DEBUG) && defined(BUILD_MODULES)  
  // *** add UnstructuredGrid types 
  // An additional datatype, MRMLUnstructuredGrid and its subclasses are 
  // also searched in the MRML tree.  This is done so instances of FiniteElement
  // meshes and other vtkUnstructuredGrid datatypes can be stored persistently.
  // this code is gated by MESHING_DEBUG since the MEshing MRML modules 
  
  nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLUnstructuredGridNode");
  for (n=0; n<nnodes; n++)
    {
    node = this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLUnstructuredGridNode");
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    vtkMRMLUnstructuredGridNode *vnode = vtkMRMLUnstructuredGridNode::SafeDownCast(node);
    vtkMRMLStorageNode* snode = vnode->GetStorageNode();
    if (snode == NULL && !node->GetModifiedSinceRead())
      {
      continue;
      }
    if (snode == NULL && node->GetModifiedSinceRead()) 
      {
        vtkMRMLUnstructuredGridStorageNode *storageNode = vtkMRMLUnstructuredGridStorageNode::New();
      storageNode->SetScene(this->GetMRMLScene());
      this->SetMRMLScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode);  
      this->SetAndObserveMRMLScene(this->GetMRMLScene());
      vnode->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = storageNode;
      }
    if (snode->GetFileName() == NULL && this->DataDirectoryName != NULL) {
      std::string name (this->DataDirectoryName);
      name += std::string(node->GetName());
      name += std::string(".vtk");
      snode->SetFileName(name.c_str());
    }

    // get absolute filename
    std::string name;
    if (this->MRMLScene->IsFilePathRelative(snode->GetFileName()))
      {
      name = this->MRMLScene->GetRootDirectory();
      if (name[name.size()-1] != '/')
        {
        name = name + std::string("/");
        }
      }
    name += snode->GetFileName();

    this->Nodes.push_back(node->GetID());
    this->StorageNodes.push_back(snode->GetID());
    
    this->MultiColumnList->GetWidget()->AddRow();
    this->MultiColumnList->GetWidget()->SetCellText(row,0,node->GetName());
    if (node->GetModifiedSinceRead()) 
      {
      this->MultiColumnList->GetWidget()->SetCellText(row,1,"Modified");
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,2,1);
      nModified++;
      }
    else
      {
      this->MultiColumnList->GetWidget()->SetCellText(row,1,"NotModified");
      this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,2,0);
      }
    this->MultiColumnList->GetWidget()->SetCellText(row,3,"UnstructuredGrid");
    this->MultiColumnList->GetWidget()->SetCellText(row,4,name.c_str());
    this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row, 2);
    row++;
    }
    // end of UGrid MRML node processing
#endif  
  
  this->IsProcessing = false;

  return nModified;
  
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::UpdateDataDirectory()
{
  if (this->DataDirectoryName == NULL)
    {
    return;
    }
  
  this->SaveDataButton->GetWidget()->SetInitialFileName(
    this->DataDirectoryName);

  int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, 2))
      {
      vtkMRMLNode *node = this->MRMLScene->GetNodeByID(this->Nodes[row].c_str());
      vtkMRMLStorageNode* snode = vtkMRMLStorageNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->StorageNodes[row].c_str()));
      itksys_stl::string name;
      if (snode->GetFileName() != NULL)
        {
        name = itksys::SystemTools::GetFilenameName(snode->GetFileName());
        }
      if (name.empty()) 
        {
        std::string sname (node->GetName());
        if (node->IsA("vtkMRMLVolumeNode")) 
          {
          sname += std::string(".nrrd");
          }
        else 
          {
          sname += std::string(".vtk");
          }
        snode->SetFileName(sname.c_str());
        name = itksys::SystemTools::GetFilenameName(snode->GetFileName());
        }
      itksys_stl::string sname (this->DataDirectoryName);
      sname += name;
      this->MultiColumnList->GetWidget()->SetCellText(row,4,sname.c_str());
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *callData )
{
  if (event == vtkCommand::ModifiedEvent)
    {
    //this->UpdateFromMRML();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::RemoveWidgetObservers ( ) 
{
  if (this->OkButton)
    {
    this->OkButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->CancelButton)
    {
    this->CancelButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SaveSceneButton)
    {
        this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers( vtkKWTopLevel::WithdrawEvent,
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SaveDataButton)
    {
    this->SaveDataButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent,
        (vtkCommand *)this->GUICallbackCommand );
    }

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
  this->SaveDialog->SetMasterWindow ( this->GetParent());
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
  this->SaveSceneButton->SetLabelPositionToLeft();
  this->SaveSceneButton->SetLabelText ("Scene File:");
  this->SaveSceneButton->GetWidget()->TrimPathFromFileNameOff();
  this->SaveSceneButton->GetWidget()->SetMaximumFileNameLength(100);
  this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
                                                            "{ {scene} {*.mrml} }");
  this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->SetDefaultExtension(".mrml");
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
  this->SaveDataButton->SetLabelPositionToLeft();
  this->SaveDataButton->SetLabelText ("Data Directory:");
  this->SaveDataButton->GetWidget()->TrimPathFromFileNameOff();
  this->SaveDataButton->GetWidget()->SetMaximumFileNameLength(100);
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

  this->MultiColumnList->GetWidget()->AddColumn("Status");
  this->MultiColumnList->GetWidget()->ColumnEditableOff(1);
  this->MultiColumnList->GetWidget()->SetColumnWidth(1, 12);

  this->MultiColumnList->GetWidget()->AddColumn("Save");
  this->MultiColumnList->GetWidget()->SetColumnWidth(2, 6);
  this->MultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(2);
  this->MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput(2);
  this->MultiColumnList->GetWidget()->ColumnEditableOn(2);


  this->MultiColumnList->GetWidget()->AddColumn("Type");
  this->MultiColumnList->GetWidget()->ColumnEditableOff(3);
  this->MultiColumnList->GetWidget()->SetColumnWidth(3, 10);

  this->MultiColumnList->GetWidget()->AddColumn("File");
  this->MultiColumnList->GetWidget()->ColumnEditableOn(4);
  this->MultiColumnList->GetWidget()->SetColumnWidth(4, 64);
  // make the save column editable by checkbox
  // now set the attributes that are equal across the columns
  int col;
  for (col = 0; col <= 4; col++)
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
  this->Script("pack %s -side left -anchor w -padx 2 -pady 4", 
              this->OkButton->GetWidgetName());

  // add Cancel button
  this->CancelButton = vtkKWPushButton::New ( );
  this->CancelButton->SetParent ( saveFrame );
  this->CancelButton->Create ( );
  this->CancelButton->SetText ("Cancel");
  this->Script("pack %s -side left -anchor w -padx 36 -pady 4", 
              this->CancelButton->GetWidgetName());

  // add observers
  this->OkButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->CancelButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveDataButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );

  
  this->MultiColumnList->SetEnabled(1);
  this->OkButton->SetEnabled(1);
  this->SaveSceneCheckBox->SetEnabled(0);
  this->SaveSceneCheckBox->SetSelectedState(0);
  
  if (this->MRMLScene != NULL)
    {
    this->SetAndObserveMRMLScene(this->MRMLScene);
    }

  frame->Delete();
  dataFrame->Delete();
  saveFrame->Delete();
  
}

void vtkSlicerMRMLSaveDataWidget::Invoke ( )
{
  this->Create();

  if (this->MRMLScene != NULL)
    {
    std::string dir = this->MRMLScene->GetRootDirectory();
    if (dir[dir.size()-1] != '/')
      {
      dir += std::string("/");
      }
    this->SetDataDirectoryName(dir.c_str());
    if (this->DataDirectoryName && *this->DataDirectoryName)
      {
      this->SaveDataButton->GetWidget()->SetInitialFileName(
        this->DataDirectoryName);
      }
    const char *url = this->MRMLScene->GetURL();
    if (url && *url)
      {
      this->SaveSceneButton->GetWidget()->SetInitialFileName(url);
      this->SaveSceneCheckBox->SetEnabled(1);
      this->SaveSceneCheckBox->SelectedStateOn();
      }
    }
    
  this->UpdateFromMRML();

  if (this->SaveDialog)
    {
    this->SaveDialog->Invoke ( );
    }
}
