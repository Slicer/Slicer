#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

#include "vtkSlicerMRMLSaveDataWidget.h"
#include "vtkDataFileFormatHelper.h"

#include "vtkKWTopLevel.h"
#include "vtkKWComboBox.h"
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
#include <vtksys/SystemTools.hxx>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerMRMLSaveDataWidget );
vtkCxxRevisionMacro ( vtkSlicerMRMLSaveDataWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerMRMLSaveDataWidget::vtkSlicerMRMLSaveDataWidget ( )
{
  this->SaveDialog = NULL;
  this->SaveSceneButton = NULL;
  //this->SceneName = NULL;
  this->SaveDataButton = NULL;

  this->SaveAllDataButton = NULL;
  this->SaveNoDataButton = NULL;
  
  this->DataDirectoryName = NULL;
  this->OkButton = NULL;
  this->SaveDataOnlyButton = NULL;
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
  if (this->SaveDataOnlyButton)
    {
    this->SaveDataOnlyButton->SetParent(NULL);
    this->SaveDataOnlyButton->Delete();
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
  if (this->SaveAllDataButton)
    {
    this->SaveAllDataButton->SetParent(NULL);
    this->SaveAllDataButton->Delete();
    }
  if (this->SaveNoDataButton)
    {
    this->SaveNoDataButton->SetParent(NULL);
    this->SaveNoDataButton->Delete();
    }
  if (this->SaveSceneButton)
    {
    this->SaveSceneButton->SetParent(NULL);
    this->SaveSceneButton->Delete();
    }
  //if (this->SceneName)
  //  {
  //  this->SceneName->SetParent(NULL);
  //  this->SceneName->Delete();
  //  }
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
  else if (this->SaveAllDataButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    {
    int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
    for (int row=0; row<nrows; row++)
      {
      this->SetRowMarkedForSave(row, 1, 0);
      }
    this->UpdateEnableState();
    }
  else if (this->SaveNoDataButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    {
    int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
    for (int row=0; row<nrows; row++)
      {
      this->SetRowMarkedForSave(row, 0, 0);
      }
    this->UpdateEnableState();
    }
  else if (this->SaveDataOnlyButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    {
    this->MultiColumnList->GetWidget()->FinishEditing();
    if(this->SaveMarkedData())
      {
      this->SaveDialog->OK();
      this->InvokeEvent(vtkSlicerMRMLSaveDataWidget::DataSavedEvent);
      }
    }
  else if (this->OkButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    {
    this->MultiColumnList->GetWidget()->FinishEditing();
    std::string sceneFileName = this->SaveSceneButton->GetWidget()->GetFileName();
    if (sceneFileName.empty())
      {
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->GetParent() );
      message->SetMasterWindow ( this->SaveDialog );
      message->SetStyleToMessage ();
      message->SetText("No scene file selected. Scene is not saved");
      message->Create();
      message->Invoke();
      message->Delete();
      return;
      }
    int writeScene = 1;
    fstream fin;
    fin.open(sceneFileName.c_str(), ios::in);
    if( fin.is_open() )
      {
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->GetParent() );
      message->SetMasterWindow ( this->SaveDialog );
      message->SetStyleToYesNo();
      std::string msg = "File " + sceneFileName + " exists. Do you want to replace it?";
      message->SetText(msg.c_str());
      message->Create();
      writeScene = message->Invoke();
      message->Delete();
      }          
    fin.close();

    if(writeScene && this->SaveModifiedData())
      {
      if(this->SaveScene())
        {
        this->SaveDialog->OK();
        this->InvokeEvent(vtkSlicerMRMLSaveDataWidget::DataSavedEvent);
        }
      }
    }
  else if (this->CancelButton ==  vtkKWPushButton::SafeDownCast(caller) && event ==  vtkKWPushButton::InvokedEvent)
    { 
    this->SaveDialog->Cancel();
    }
  else if(event == vtkKWMultiColumnList::CellUpdatedEvent && 
    this->MultiColumnList->GetWidget() == vtkKWMultiColumnList::SafeDownCast(caller))
    {
    this->MultiColumnList->GetWidget()->FinishEditing();
    if(callData)
      {
      void **data = static_cast< void ** >(callData);
      int* row = static_cast< int* >(data[0]);
      int* col = static_cast< int* >(data[1]);
      const char* text = static_cast< const char* >(data[2]);

      this->UpdateDataTableCell(*row, *col);
      }
    }
}

//---------------------------------------------------------------------------
int vtkSlicerMRMLSaveDataWidget::SaveModifiedData()
{
  vtkKWMultiColumnList* mcList = this->MultiColumnList->GetWidget();
  int nrows = mcList->GetNumberOfRows();
  int numMarked = 0;
  for (int row=0; row<nrows; row++)
    {
    if (strcmp(mcList->GetCellText(row,Status_Column), "Modified")==0 && 
      !mcList->GetCellTextAsInt(row, Save_Column))
      {
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->GetParent() );
      message->SetMasterWindow ( this->SaveDialog );
      message->SetStyleToMessage();
      std::string msg("Node: ");
      msg.append(mcList->GetCellText(row, NodeName_Column)).append( 
        " is modified, but not marked for Save.\n You must mark all modified nodes for Save in order to save a scene!");
      message->SetText(msg.c_str());
      message->Create();
      message->Invoke();
      message->Delete();
      return 0;
      }
    numMarked = mcList->GetCellTextAsInt(row, Save_Column) ? numMarked+1 : numMarked;
    }

  int res = 1;//If no modified data, return success
  if(numMarked>0)
    {
    res = this->SaveMarkedData(); 
    }
  return res;
}

//---------------------------------------------------------------------------
int vtkSlicerMRMLSaveDataWidget::SaveMarkedData()
{
  vtkIntArray* arrayRows = vtkIntArray::New();
  arrayRows->SetNumberOfComponents(1);
  int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  int numNotWrite = 0;
  for (int row=0; row<nrows; row++)
    {
    if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, Save_Column))
      {
      arrayRows->InsertNextValue(row);
      }
    }
  int res = 0;//If no marked data, return failure
  if(arrayRows->GetNumberOfTuples()>0)
    {
    res = this->SaveData(arrayRows); 
    }
  arrayRows->Delete();
  return res;
}

//---------------------------------------------------------------------------
int vtkSlicerMRMLSaveDataWidget::SaveData(vtkIntArray* arrayRows)
{
  int nrows = arrayRows->GetNumberOfTuples();
  int row, numNotWrite = 0;
  for (int i=0; i<nrows; i++)
    {
    row = arrayRows->GetValue(i);
    vtkMRMLNode *node = this->MRMLScene->GetNodeByID(
      this->MultiColumnList->GetWidget()->GetCellText(row, Hidden_NodeID_Column));
    vtkMRMLStorageNode* snode = vtkMRMLStorageNode::SafeDownCast(
      this->MRMLScene->GetNodeByID(
      this->MultiColumnList->GetWidget()->GetCellText(row, Hidden_StorageNodeID_Column)));
    std::string fileName (this->MultiColumnList->GetWidget()->GetCellText(row, FileName_Column));
    if(fileName.empty())
      {
      vtkErrorMacro(<< this->GetClassName() << " did not have a file name for: " << node->GetName());
      return 0;
      }
    //vtkKWLoadSaveButton* lsButton = this->MultiColumnList->GetWidget()->
    //  GetCellWindowAsLoadSaveButton(row, FileDirectory_Column);
    //if(!lsButton || lsButton->GetFileName()==NULL )
    //  {
    //  vtkErrorMacro(<< this->GetClassName() << " did not have a directory for: " << fileName);
    //  return;
    //  }
    //std::string filePath(lsButton->GetFileName());
    std::string filePath (this->MultiColumnList->GetWidget()->GetCellText(row, FileDirectory_Column));
    if(filePath.empty() || !vtksys::SystemTools::FileIsDirectory(filePath.c_str()))
      {
      vtkErrorMacro(<< this->GetClassName() << " did not have a valid path for: " << node->GetName());
      return 0;
      }
    if (filePath[filePath.size()-1] != '/')
      {
      filePath.append("/");
      }
    filePath.append(fileName.c_str());
    std::string oldFileName = snode->GetFileName();
    std::string oldURI = snode->GetURI() ? snode->GetURI() : "";
    snode->SetFileName(filePath.c_str());
    // set the write file format. The actual node will decide what to do with the format
    std::string fileFormat (this->MultiColumnList->GetWidget()->
      GetCellText(row, Format_Column));
    snode->SetWriteFileFormat(fileFormat.c_str());
    // undo any URI
    snode->SetURI(NULL);
    
    //: ask override
    int  writeFile = 1;
    fstream fin;
    fin.open(filePath.c_str(),ios::in);
    if( fin.is_open() )
      {
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->GetParent() );
      message->SetMasterWindow ( this->SaveDialog );
      message->SetStyleToYesNo();
      std::string msg = "File " + fileName + " exists. Do you want to replace it?";
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
        this->SetRowMarkedForSave(row, 0);
        this->SetRowModified(row, 0);
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
          return 0;
          }
        else
          {
          numNotWrite++;
          }
        }
      }
    else
      {
      snode->SetFileName(oldFileName.c_str()); 
      // undo any URI
      snode->SetURI(oldURI.empty() ? NULL: oldURI.c_str());
      numNotWrite++;
      }
    }
  if(numNotWrite)
    {
    return 0;
    }
  return 1;
}

//---------------------------------------------------------------------------
int vtkSlicerMRMLSaveDataWidget::SaveScene()
{
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
  else if(this->GetMRMLScene())
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
      if (snode->GetFileName())
        {
        itksys_stl::string absPath = snode->GetFullNameFromFileName();
        itksys_stl::string relPath = itksys::SystemTools::RelativePath(
          directory.c_str(), absPath.c_str());
        snode->SetFileName(relPath.c_str());
        snode->SetSceneRootDir(directory.c_str());
        }
      }

    this->GetMRMLScene()->SetURL(fileName);
    this->GetMRMLScene()->Commit();  
    return 1;
    }

  return 0;
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

    this->MultiColumnList->GetWidget()->AddRow();
    this->MultiColumnList->GetWidget()->SetCellText(
      row,NodeName_Column,node->GetName());
    if (node->GetModifiedSinceRead()) 
      {
      this->SetRowModified(row, 1);
      this->SetRowMarkedForSave(row, 1);
      nModified++;
      }
    else
      {
      this->SetRowModified(row, 0);
      this->SetRowMarkedForSave(row, 0);
      }
    this->MultiColumnList->GetWidget()->SetCellText(row,Type_Column,"Volume");
    this->SetFileNameRelatedCells(row, name.c_str(), snode->GetSupportedWriteFileTypes());
    this->AddNodeId(node->GetID(), row);
    this->AddStorageNodeId(snode->GetID(), row);
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
    
    this->MultiColumnList->GetWidget()->AddRow();
    this->MultiColumnList->GetWidget()->SetCellText(
      row,NodeName_Column,node->GetName());
    if (node->GetModifiedSinceRead()) 
      {
      this->SetRowModified(row, 1);
      this->SetRowMarkedForSave(row, 1);
      nModified++;
      }
    else
      {
      this->SetRowModified(row, 0);
      this->SetRowMarkedForSave(row, 0);
      }
    this->MultiColumnList->GetWidget()->SetCellText(row,Type_Column,"Model");
    this->SetFileNameRelatedCells(row, name.c_str(), snode->GetSupportedWriteFileTypes());
    this->AddNodeId(node->GetID(), row);
    this->AddStorageNodeId(snode->GetID(), row);

    row++;
    }
  
#if !defined(MESHING_DEBUG) && defined(Slicer3_BUILD_MODULES)  
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

    this->MultiColumnList->GetWidget()->AddRow();
    this->MultiColumnList->GetWidget()->SetCellText(row,NodeName_Column,node->GetName());
    if (node->GetModifiedSinceRead()) 
      {
      this->SetRowModified(row, 1);
      this->SetRowMarkedForSave(row, 1);
      nModified++;
      }
    else
      {
      this->SetRowModified(row, 0);
      this->SetRowMarkedForSave(row, 0);
      }
    this->MultiColumnList->GetWidget()->SetCellText(row,Type_Column,"UnstructuredGrid");
    this->SetFileNameRelatedCells(row, name.c_str(), snode->GetSupportedWriteFileTypes());
    this->AddNodeId(node->GetID(), row);
    this->AddStorageNodeId(snode->GetID(), row);
    row++;
    }
    // end of UGrid MRML node processing
#endif  
  
  this->IsProcessing = false;

  return nModified;
  
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::AddNodeId(const char* strID, int row)
{
  this->Nodes.push_back(strID);
  this->MultiColumnList->GetWidget()->SetCellText(row,
    Hidden_NodeID_Column,strID);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::AddStorageNodeId(const char* strID, int row)
{
  this->StorageNodes.push_back(strID);    
  this->MultiColumnList->GetWidget()->SetCellText(row,
    Hidden_StorageNodeID_Column,strID);
}

//---------------------------------------------------------------------------
const char* vtkSlicerMRMLSaveDataWidget::GetFileFormatWithExtension(
 const char* fileext, vtkStringArray* supportedFileFormats)
{
  if(fileext && *fileext && supportedFileFormats &&
    supportedFileFormats->GetNumberOfTuples()>0)
    {
    std::string::size_type ext_pos;
    std::string fileformat;
    for(int i=0; i<supportedFileFormats->GetNumberOfTuples(); i++)
      {
      fileformat=supportedFileFormats->GetValue(i);
      ext_pos = fileformat.find(fileext);
      if(ext_pos != std::string::npos)
        {
        return supportedFileFormats->GetValue(i).c_str();
        }
      }
    }

  return NULL;
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::UpdateRowFileNameWithExtension(
  int row)
{
  if(row>=this->MultiColumnList->GetWidget()->GetNumberOfRows())
    {
    return;
    }
  std::string currExt = this->GetRowCurrentFileExtension(row);
  if(strcmp(currExt.c_str(), ".*")==0)
    {
    return;
    }
  std::string fileName = this->MultiColumnList->GetWidget()->GetCellText(
    row,FileName_Column);
  std::string fileExt = vtksys::SystemTools::GetFilenameExtension(fileName);

  if(!currExt.empty() && !fileName.empty() &&
    strcmp(fileExt.c_str(), currExt.c_str()))
    {
    fileName = vtksys::SystemTools::GetFilenameWithoutExtension(fileName);
    fileName.append(currExt);
    this->MultiColumnList->GetWidget()->SetCellText(
      row,FileName_Column,fileName.c_str());
    this->MultiColumnList->GetWidget()->SetCellText(
      row,Hidden_FileName_Column,fileName.c_str());
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::UpdateRowFileFormatWithName(
  int row)
{
  if(row>=this->MultiColumnList->GetWidget()->GetNumberOfRows())
    {
    return;
    }
  std::string currExt = this->GetRowCurrentFileExtension(row);
  if(strcmp(currExt.c_str(), ".*")==0)
    {
    return;
    }

  std::string fileName = this->MultiColumnList->GetWidget()->GetCellText(
    row,FileName_Column);
  std::string fileExt = vtksys::SystemTools::GetFilenameExtension(fileName);
  if(!fileExt.empty() && strcmp(fileExt.c_str(), currExt.c_str()))
    {
    const char* fileFormat =
      this->GetRowFileFormatWithExtension(row,fileExt.c_str());
    if(fileFormat)
      {
      std::string strFormat(fileFormat);
      vtkKWComboBox* cb = this->MultiColumnList->GetWidget()->
        GetCellWindowAsComboBox(row, Format_Column);
      if(cb)
        {
        cb->SetValue(strFormat.c_str());
        }
      this->MultiColumnList->GetWidget()->SetCellText(
        row,Format_Column,fileName.c_str());
      }
    }
}

//---------------------------------------------------------------------------
const char* vtkSlicerMRMLSaveDataWidget::GetRowFileFormatWithExtension(
  int row, const char* extension)
{
  if(!extension || !(*extension) || strcmp(extension,".")==0)
    {
    return NULL;
    }

  vtkKWComboBox* cb = this->MultiColumnList->GetWidget()->
    GetCellWindowAsComboBox(row, Format_Column);
  if(cb)
    {
    std::string value, strExt;
    std::string::size_type ext_pos;
    for(int i=0; cb->GetNumberOfValues(); i++)
      {
      strExt = cb->GetValueFromIndex(i);
      value = vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
        strExt.c_str());
      ext_pos = value.find(extension);
      if(ext_pos != std::string::npos)
        {
        return cb->GetValueFromIndex(i);
        }
      }
    }
  return NULL;
}

//---------------------------------------------------------------------------
const char* vtkSlicerMRMLSaveDataWidget::GetRowCurrentFileExtension(int row)
{
  if(row>=this->MultiColumnList->GetWidget()->GetNumberOfRows())
    {
    return NULL;
    }
  const char* fileformat = this->MultiColumnList->GetWidget()->GetCellText(row,Format_Column);
  if(fileformat)
    {
    return vtkDataFileFormatHelper::GetFileExtensionFromFormatString(fileformat);
    }
  else
    {
    return NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SetFileNameRelatedCells(
  int row, const char* filename, vtkStringArray* supportedFileFormats)
{
  if(!this->MultiColumnList->IsCreated() || !filename || !(*filename) ||
    !supportedFileFormats || supportedFileFormats->GetNumberOfTuples()==0)
   {
   return;
   }
  std::string name = vtksys::SystemTools::CollapseFullPath(filename);
  std::string strExt = vtksys::SystemTools::GetFilenameLastExtension(name);
  const char* pFileFormat = this->GetFileFormatWithExtension(strExt.c_str(),
    supportedFileFormats);
  if(!pFileFormat || !(*pFileFormat))
    {
    return;
    }
  std::string fileFormat = pFileFormat;
  this->MultiColumnList->GetWidget()->SetCellText(
    row,Format_Column, fileFormat.c_str());
  this->MultiColumnList->GetWidget()->
    SetCellWindowCommandToComboBoxWithValuesAsArray(
    row, Format_Column, supportedFileFormats);
  std::string text = vtksys::SystemTools::GetFilenameName(name);
  this->MultiColumnList->GetWidget()->SetCellText(row,FileName_Column,
    text.c_str());
  this->MultiColumnList->GetWidget()->SetCellText(row,Hidden_FileName_Column,
    text.c_str());

  text = vtksys::SystemTools::GetFilenamePath(name);
  this->MultiColumnList->GetWidget()->SetCellText(row,FileDirectory_Column,
    text.c_str());
  this->MultiColumnList->GetWidget()->SetCellWindowCommandToPickDirectoryButton(
    row,FileDirectory_Column);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SetDataDirectoryRelatedCells(
  int row, const char* dirname)
{
  if(!this->MultiColumnList->IsCreated() || !dirname || 
    !(*dirname) || !vtksys::SystemTools::FileIsDirectory(dirname))
   {
   return;
   }
  std::string name = dirname;
  this->MultiColumnList->GetWidget()->SetCellText(
    row,FileDirectory_Column, dirname);

  this->MultiColumnList->GetWidget()->SetCellWindowCommandToPickDirectoryButton(
    row,FileDirectory_Column);
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
    if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, Save_Column))
      {
      // Do we really need this?
      this->UpdateNodeDataDirectory(row);

      // Update the data table
      this->SetDataDirectoryRelatedCells(row, this->DataDirectoryName);
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
void vtkSlicerMRMLSaveDataWidget::AddWidgetObservers ( ) 
{
  this->OkButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SaveDataOnlyButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->CancelButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveDataButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveAllDataButton->AddObserver(  vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SaveNoDataButton->AddObserver(  vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->MultiColumnList->GetWidget()->AddObserver(  vtkKWMultiColumnList::CellUpdatedEvent,  
    (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::RemoveWidgetObservers ( ) 
{
  if (this->OkButton)
    {
    this->OkButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SaveDataOnlyButton)
    {
    this->SaveDataOnlyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
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
  if (this->SaveAllDataButton)
    {
    this->SaveAllDataButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SaveNoDataButton)
    {
    this->SaveNoDataButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->MultiColumnList)
    {
    this->MultiColumnList->GetWidget()->RemoveObservers(  vtkKWMultiColumnList::CellUpdatedEvent,  
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

  // Data Frame
  vtkKWFrameWithLabel *dataFrame = vtkKWFrameWithLabel::New ( );
  dataFrame->SetParent ( this->SaveDialog );
  dataFrame->Create ( );
  dataFrame->SetLabelText ("Save Data Options");
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 dataFrame->GetWidgetName() );
 
  // add the multicolumn list to show the points
  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( dataFrame->GetFrame() );
  this->MultiColumnList->Create ( );
  this->MultiColumnList->SetHeight(4);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
  
  this->SetupSaveDataListWidget();
  this->Script ( "pack %s -fill both -expand true",
                     this->MultiColumnList->GetWidgetName());
 
  // saveDataMCList->SetCellUpdatedCommand(this, "UpdateElement");

  // add a button frame
  vtkKWFrame *buttonFrame = vtkKWFrame::New();
  buttonFrame->SetParent(dataFrame->GetFrame());
  buttonFrame->Create();
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 buttonFrame->GetWidgetName() );
  
  // add a button to change all the save flags to on
  this->SaveAllDataButton = vtkKWPushButton::New();
  this->SaveAllDataButton->SetParent ( buttonFrame );
  this->SaveAllDataButton->Create ( );
  this->SaveAllDataButton->SetText ("Select All");
  this->SaveAllDataButton->SetBalloonHelpString("Check all save boxes");
  this->Script("pack %s -side left -anchor w -padx 2 -pady 4", 
              this->SaveAllDataButton->GetWidgetName());

  // add a button to change all the save flags to off
  this->SaveNoDataButton = vtkKWPushButton::New();
  this->SaveNoDataButton->SetParent ( buttonFrame );
  this->SaveNoDataButton->Create ( );
  this->SaveNoDataButton->SetText ("Select None");
  this->SaveNoDataButton->SetBalloonHelpString("Uncheck all save boxes");
  this->Script("pack %s -side left -anchor w -padx 2 -pady 4", 
              this->SaveNoDataButton->GetWidgetName());

  // add a data file browser 
  this->SaveDataButton = vtkKWLoadSaveButtonWithLabel::New ( );
  this->SaveDataButton->SetParent ( dataFrame->GetFrame() );
  this->SaveDataButton->Create ( );
  this->SaveDataButton->SetLabelPositionToLeft();
  this->SaveDataButton->SetLabelText ("Set Data Directory for All Selected:");
  this->SaveDataButton->GetWidget()->TrimPathFromFileNameOff();
  this->SaveDataButton->GetWidget()->SetMaximumFileNameLength(64);
  this->SaveDataButton->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
  this->SaveDataButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
    "OpenPath");
  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2", 
                 this->SaveDataButton->GetWidgetName());

  // Scene Frame
  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New ( );
  frame->SetParent ( this->SaveDialog );
  frame->Create ( );
  frame->SetLabelText ("Save Scene Options");
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
  //this->SceneName = vtkKWEntryWithLabel::New();
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

  // save Frame
  vtkKWFrame *saveFrame = vtkKWFrame::New ( );
  saveFrame->SetParent ( this->SaveDialog );
  saveFrame->Create ( );
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 saveFrame->GetWidgetName() );

  // add Cancel button
  this->CancelButton = vtkKWPushButton::New ( );
  this->CancelButton->SetParent ( saveFrame );
  this->CancelButton->Create ( );
  this->CancelButton->SetText ("Cancel");
  this->Script("pack %s -side right -anchor w -padx 16 -pady 4", 
              this->CancelButton->GetWidgetName());

  // add SaveDataOnlyButton button
  this->SaveDataOnlyButton = vtkKWPushButton::New ( );
  this->SaveDataOnlyButton->SetParent ( saveFrame );
  this->SaveDataOnlyButton->Create ( );
  this->SaveDataOnlyButton->SetText ("Save Selected Data Only");
  this->Script("pack %s -side right -anchor w -padx 4 -pady 4", 
              this->SaveDataOnlyButton->GetWidgetName());

  // add OK button
  this->OkButton = vtkKWPushButton::New ( );
  this->OkButton->SetParent ( saveFrame );
  this->OkButton->Create ( );
  this->OkButton->SetText ("Save Scene & Selected Data");
  this->Script("pack %s -side right -anchor w -padx 4 -pady 4", 
              this->OkButton->GetWidgetName());

  // add observers
  this->AddWidgetObservers();
  
  this->MultiColumnList->SetEnabled(1);
  this->OkButton->SetEnabled(1);
  this->SaveDataOnlyButton->SetEnabled(1);
  
  if (this->MRMLScene != NULL)
    {
    this->SetAndObserveMRMLScene(this->MRMLScene);
    }

  frame->Delete();
  dataFrame->Delete();
  saveFrame->Delete();
  buttonFrame->Delete();
  
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SetupSaveDataListWidget()
{
  vtkKWMultiColumnList* saveDataMCList = this->MultiColumnList->GetWidget();
  // set up the columns of data for each point
  // name, x, y, z, orientation w, x, y, z, selected
  int col_index;
  col_index = saveDataMCList->AddColumn("Select");
  saveDataMCList->SetColumnWidth(col_index, 6);
  saveDataMCList->SetColumnFormatCommandToEmptyOutput(col_index);
  saveDataMCList->SetColumnEditWindowToCheckButton(col_index);
  saveDataMCList->ColumnEditableOn(col_index);

  col_index = saveDataMCList->AddColumn("Node Name");
  saveDataMCList->ColumnEditableOff(col_index);
  saveDataMCList->SetColumnWidth(col_index, 12);

  col_index = saveDataMCList->AddColumn("Node Type");
  saveDataMCList->ColumnEditableOff(col_index);
  saveDataMCList->SetColumnWidth(col_index, 12);

  col_index = saveDataMCList->AddColumn("Node Status");
  saveDataMCList->ColumnEditableOff(col_index);
  saveDataMCList->SetColumnWidth(col_index, 12);

  col_index = saveDataMCList->AddColumn("File Format");
  saveDataMCList->SetColumnFormatCommandToEmptyOutput(col_index);
  saveDataMCList->ColumnEditableOn(col_index);
  //saveDataMCList->SetColumnWidth(col_index, 12);

  col_index = saveDataMCList->AddColumn("File Name");
  //saveDataMCList->SetColumnFormatCommandToEmptyOutput(col_index);
  saveDataMCList->ColumnEditableOn(col_index);
  saveDataMCList->SetColumnWidth(col_index, 20);

  col_index = saveDataMCList->AddColumn("Data Directory");
  saveDataMCList->SetColumnFormatCommandToEmptyOutput(col_index);
  saveDataMCList->ColumnEditableOn(col_index);
  saveDataMCList->SetColumnWidth(col_index, 40);
  // make the save column editable by checkbox
  // now set the attributes that are equal across the columns
  for (col_index = 0; col_index <= 6; col_index++)
    {        
    saveDataMCList->SetColumnAlignmentToLeft(col_index);
    }

  // Keep track of Node Id and storage node Id, 
  //so that we can still find out the node by row after sorting.
  col_index = saveDataMCList->AddColumn("Node ID");
  saveDataMCList->ColumnVisibilityOff(col_index);
  col_index = saveDataMCList->AddColumn("Storage Node ID");
  saveDataMCList->ColumnVisibilityOff(col_index);

  // Last edited file name, so that we know whether the 
  // file name is actually changed or not.
  col_index = saveDataMCList->AddColumn("Last Edited File Name");
  saveDataMCList->ColumnVisibilityOff(col_index);
}

//---------------------------------------------------------------------------
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
      this->SaveDataButton->GetWidget()->GetLoadSaveDialog()->
        SetFileName(this->DataDirectoryName);
      }
    const char *url = this->MRMLScene->GetURL();
    if (url && *url)
      {
      this->SaveSceneButton->GetWidget()->SetInitialFileName(url);
      this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->SetFileName(url);
      }
    }
    
  this->UpdateFromMRML();

  if (this->SaveDialog)
    {
    this->SaveDialog->Invoke ( );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::UpdateDataTableCell(
  int row, int col)
{
  std::string fileName =this->MultiColumnList->GetWidget()->GetCellText(
    row,FileName_Column);
  std::string currExt = this->GetRowCurrentFileExtension(row);
  std::string oldFileName(this->MultiColumnList->GetWidget()->GetCellText(
    row, Hidden_FileName_Column));
  std::string currText = this->MultiColumnList->GetWidget()->GetCellText(row, col);
  const char* pText = currText.c_str();

  switch(col)
    {
    case Save_Column:
      this->UpdateEnableState();
      break;
    case FileName_Column:
      if(!pText || !(*pText) ||!strlen(pText))
        {
        vtkErrorMacro(<< this->GetClassName() << 
          ": Invalid file name for saving");
        this->MultiColumnList->GetWidget()->SetCellText(
          row, col, oldFileName.c_str());
        break;
        }

      if(strcmp(pText, oldFileName.c_str()))
        {
        this->SetRowMarkedForSave(row, 1);
        this->UpdateRowFileFormatWithName(row);
        this->MultiColumnList->GetWidget()->SetCellText(
          row, Hidden_FileName_Column, pText);
        }
      break;
    case Format_Column:
      if(!pText || !(*pText) ||!strlen(pText))
        {
        vtkErrorMacro(<< this->GetClassName() << 
          ": Invalid file format for saving!");
        return;
        }

      if(!currExt.empty() && !fileName.empty())
        {
        std::string fileExt = vtksys::SystemTools::GetFilenameExtension(fileName);
        if(fileExt.empty() || strcmp(fileExt.c_str(), currExt.c_str()))
          {
          this->SetRowMarkedForSave(row, 1);
          this->UpdateRowFileNameWithExtension(row);
          }
        }
      break;
    case FileDirectory_Column:
      this->SetRowMarkedForSave(row, 1);
      break;
    default:
      break;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SetRowMarkedForSave(
  int row, int marked, int doUpdate)
{
  this->MultiColumnList->GetWidget()->SetCellTextAsInt(row,Save_Column,marked);
  this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(row, Save_Column);

  if(doUpdate)
    {
    this->UpdateEnableState();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SetRowModified(
  int row, int modified)
{
  std::string strText = modified ? "Modified" : "Not Modified";
  this->MultiColumnList->GetWidget()->SetCellText(
    row,Status_Column,strText.c_str());
}

//----------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();

  int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  int numSelected = 0;
  for (int row=0; row<nrows; row++)
    {
    if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, Save_Column))
      {
      numSelected++;
      }
    }
  this->OkButton->SetEnabled(nrows>0 ? 1 : 0);
  this->SaveDataOnlyButton->SetEnabled(numSelected>0 ? 1 : 0);
  this->SaveDataButton->SetEnabled(numSelected>0 ? 1 : 0);
  this->SaveSceneButton->SetEnabled(nrows>0 ? 1 : 0);

  this->SaveAllDataButton->SetEnabled(numSelected < nrows ? 1 : 0);
  this->SaveNoDataButton->SetEnabled(numSelected > 0 ? 1 : 0);
}

//----------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::UpdateNodeDataDirectory(int row)
{
  vtkMRMLNode *node = this->MRMLScene->GetNodeByID(
    this->MultiColumnList->GetWidget()->GetCellText(row, Hidden_NodeID_Column));
  vtkMRMLStorageNode* snode = vtkMRMLStorageNode::SafeDownCast(
    this->MRMLScene->GetNodeByID(
    this->MultiColumnList->GetWidget()->GetCellText(row, Hidden_StorageNodeID_Column)));
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
    // undo the URI if it's set
    snode->SetURI(NULL);
    //name = itksys::SystemTools::GetFilenameName(snode->GetFileName());
    }
  //itksys_stl::string sname (this->DataDirectoryName);
  //sname += name;
}
