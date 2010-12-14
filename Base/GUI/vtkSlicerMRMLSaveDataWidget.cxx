#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

#include "vtkSlicerMRMLSaveDataWidget.h"
#include "vtkSlicerSaveDataWidgetIcons.h"
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

#include "vtkMRMLLogic.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"
#include "vtkMRMLSceneSnapshotNode.h"

#include <vtksys/stl/string>
#include <vtksys/SystemTools.hxx>

#define PICK_FORMAT_MESSAGE "Pick format for saving"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerMRMLSaveDataWidget );
vtkCxxRevisionMacro ( vtkSlicerMRMLSaveDataWidget, "$Revision$");

//---------------------------------------------------------------------------
vtkSlicerMRMLSaveDataWidget::vtkSlicerMRMLSaveDataWidget ( )
{
  this->SaveDialog = NULL;
  this->ChangeAllSelectedDirButton = NULL;
  this->Version = NULL;

  this->SelectAllButton = NULL;
  this->SelectNoneButton = NULL;
  this->SelectModifiedDataButton = NULL;
  this->SelectSceneAndModifiedDataButton = NULL;

  this->DataDirectoryName = NULL;
  this->OkButton = NULL;
  this->CancelButton = NULL;
  this->MultiColumnList = NULL;

  this->IsProcessing = false;
  this->GUIIcons = NULL;
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
  if (this->SelectAllButton)
    {
    this->SelectAllButton->SetParent(NULL);
    this->SelectAllButton->Delete();
    }
  if (this->SelectNoneButton)
    {
    this->SelectNoneButton->SetParent(NULL);
    this->SelectNoneButton->Delete();
    }
  if (this->SelectModifiedDataButton)
    {
    this->SelectModifiedDataButton->SetParent(NULL);
    this->SelectModifiedDataButton->Delete();
    }
  if (this->SelectSceneAndModifiedDataButton)
    {
    this->SelectSceneAndModifiedDataButton->SetParent(NULL);
    this->SelectSceneAndModifiedDataButton->Delete();
    }
  if (this->ChangeAllSelectedDirButton)
    {
    this->ChangeAllSelectedDirButton->SetParent(NULL);
    this->ChangeAllSelectedDirButton->Delete();
    }
  if(this->GUIIcons)
    {
    this->GUIIcons->Delete();
    this->GUIIcons = NULL;
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
  if (this->ChangeAllSelectedDirButton->GetWidget()->GetLoadSaveDialog() == 
    vtkKWLoadSaveDialog::SafeDownCast(caller) && 
    event == vtkKWTopLevel::WithdrawEvent )
    {
    const char *fileName = this->ChangeAllSelectedDirButton->GetWidget()->GetFileName();
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
  else if (this->SelectAllButton ==  vtkKWPushButton::SafeDownCast(caller) && 
    event ==  vtkKWPushButton::InvokedEvent)
    {
    this->SetAllRowsSelected(1);
    }
  else if (this->SelectModifiedDataButton ==  vtkKWPushButton::SafeDownCast(caller) && 
    event ==  vtkKWPushButton::InvokedEvent)
    {
    this->SetAllRowsSelected(0);
    int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
    for (int row=0; row<nrows; row++)
      {
      if(this->IsRowFileFormatSet(row) 
        && this->IsRowModified(row) && !this->IsSceneRow(row))
        {
        this->SetRowMarkedForSave(row, 1, 0);
        }
      }
    this->UpdateEnableState();
    }
  else if (this->SelectSceneAndModifiedDataButton ==  vtkKWPushButton::SafeDownCast(caller) && 
    event ==  vtkKWPushButton::InvokedEvent)
    {
    this->SetAllRowsSelected(0);
    int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
    for (int row=0; row<nrows; row++)
      {
      if(this->IsRowFileFormatSet(row) && this->IsRowModified(row))
        {
        this->SetRowMarkedForSave(row, 1, 0);
        }
      }
    this->UpdateEnableState();
    }
  else if (this->SelectNoneButton ==  vtkKWPushButton::SafeDownCast(caller) && 
    event ==  vtkKWPushButton::InvokedEvent)
    {
    this->SetAllRowsSelected(0);
    }
  else if (this->OkButton ==  vtkKWPushButton::SafeDownCast(caller) && 
    event ==  vtkKWPushButton::InvokedEvent)
    {
    this->MultiColumnList->GetWidget()->FinishEditing();
    // If scene row is selected for save, then try to save
    // the scene related data first.
    int sceneRow = this->GetSceneRowIndex();
    int saveScene = 0;
    if(sceneRow >=0)
      {
      saveScene = this->MultiColumnList->GetWidget()->GetCellTextAsInt(
        sceneRow, Save_Column);
      if(saveScene)
        {
        this->SaveSceneWithData(sceneRow);
        }
      else
        {
        this->SaveMarkedData();
        }
      }
    }
  else if (this->CancelButton ==  vtkKWPushButton::SafeDownCast(caller) && 
           event ==  vtkKWPushButton::InvokedEvent)
    {
    if (this->SaveDialog)
      {
      this->SaveDialog->Cancel();
      }
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

      this->UpdateDataTableCell(*row, *col);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SaveSceneWithData(int sceneRow)
{
  std::string sceneFileName = this->GetRowFullFileName(sceneRow);
  if (sceneFileName.empty())
    {
    vtkKWMessageDialog *message = vtkKWMessageDialog::New();
    message->SetParent ( this->GetParent() );
    if (this->SaveDialog)
      {
      message->SetMasterWindow ( this->SaveDialog );
      }
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
    if (this->SaveDialog)
      {
      message->SetMasterWindow ( this->SaveDialog );
      }
    message->SetStyleToYesNo();
    std::string msg = "File " + sceneFileName + " exists. Do you want to replace it?";
    message->SetText(msg.c_str());
    message->Create();
    writeScene = message->Invoke();
    message->Delete();
    }          
  fin.close();

  // set the scene's root directory so that storage nodes can use it
  if (this->GetMRMLScene())
    {
      vtksys_stl::string directory = 
        vtksys::SystemTools::GetParentDirectory(sceneFileName.c_str());
      this->MRMLScene->SetRootDirectory(directory.c_str());
      this->SetSnapshotsRootDirectory();
    }
  
  if(writeScene && this->SaveModifiedData())
    {
    if(this->SaveScene(sceneRow))
      {
      if (this->SaveDialog)
        {
        this->SaveDialog->OK();
        }
      this->InvokeEvent(vtkSlicerMRMLSaveDataWidget::DataSavedEvent);
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
    if(this->IsSceneRow(row))
      {
      continue;
      }
    if (this->IsRowModified(row) 
      && !mcList->GetCellTextAsInt(row, Save_Column))
      {
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->GetParent() );
      message->SetMasterWindow ( this->SaveDialog );
      //message->SetStyleToMessage();
      message->SetStyleToYesNo();
      std::string msg("The node: ");
      msg.append(mcList->GetCellText(row, NodeName_Column)).append( 
        ", is modified, but not marked for Save.\n ").append(
        "If a scene is saved without saving all modified data, \nthe saved mrml file may not describe this dataset properly!\n").append(
        "Do you still want to continue?");
      message->SetText(msg.c_str());
      message->Create();
      int isOK = message->Invoke();
      message->Delete();
      if(!isOK)
        {
        return 0;
        }
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
  for (int row=0; row<nrows; row++)
    {
    if (!this->IsSceneRow(row) &&
      this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, Save_Column))
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
  if (!this->CheckUniqueFilenames(arrayRows))
    {
    return 0;
    }

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
    
    std::string filePath = this->GetRowFullFileName(row);
    if(filePath.empty())
      {
      vtkKWMessageDialog *message = vtkKWMessageDialog::New();
      message->SetParent ( this->GetParent() );
      message->SetMasterWindow ( this->SaveDialog );
      message->SetStyleToMessage ();
      std::string msg(this->GetClassName());
      msg.append(" did not have a file name for: ").append(node->GetName()).append(
        ",\n so this node is not saved!");
      message->SetText(msg.c_str());
      message->Create();
      message->Invoke();
      message->Delete();
      numNotWrite++;
      continue;
      }

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
      std::string msg = "File " + filePath + ", \n already exists. Do you want to replace it?";
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
        std::string msg = "Cannot write data file, " + filePath + ". \nDo you want to continue saving?";
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
int vtkSlicerMRMLSaveDataWidget::CheckUniqueFilenames(vtkIntArray* arrayRows)
{
  std::map<std::string, std::string> fileNames;
  std::map<std::string, std::string>::iterator iter;

  int nrows = arrayRows->GetNumberOfTuples();
  int row = 0;
  for (int i=0; i<nrows; i++)
    {
    row = arrayRows->GetValue(i);

    std::string filePath = this->GetRowFullFileName(row);

    if(filePath.empty())
      {
      continue;
      }

    std::string fileFormat (this->MultiColumnList->GetWidget()->
                            GetCellText(row, Format_Column));

    iter = fileNames.find(filePath);

    if (iter != fileNames.end())
      {
      std::string msg = "WARNING! Two or more files marked for have the same name: " + filePath + " One will overwrite the other. \nDo you want to continue saving?";
      if (! (vtkKWMessageDialog::PopupOkCancel( 
                                             this->GetApplication(), this->GetParent(), 
                                             "Duplicate file names WARNING", msg.c_str(),
            vtkKWMessageDialog::WarningIcon | 
                                             vtkKWMessageDialog::InvokeAtPointer)) )
        {
        return 0;
        }
      }
    else
      {
      fileNames[filePath] = filePath;
      }
    }
  return 1;
}

//---------------------------------------------------------------------------
int vtkSlicerMRMLSaveDataWidget::SaveScene(int sceneRow)
{
  std::string fileName = this->GetRowFullFileName(sceneRow);
  //const char *fileName = this->SaveSceneButton->GetWidget()->GetFileName();
  if (fileName.empty() && this->MRMLScene)
    {
    fileName = this->MRMLScene->GetURL();
    }
  if (fileName.empty())
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
      vtksys::SystemTools::GetParentDirectory(fileName.c_str());
    this->MRMLScene->SetRootDirectory(directory.c_str());
    this->SetSnapshotsRootDirectory();
    
    // remove unreferenced nodes
    vtkMRMLLogic *mrmlLogic = vtkMRMLLogic::New();
    mrmlLogic->SetScene(this->GetMRMLScene());
    mrmlLogic->RemoveUnreferencedDisplayNodes();
    mrmlLogic->RemoveUnreferencedStorageNodes();
    mrmlLogic->Delete();

    this->GetMRMLScene()->SetURL(fileName.c_str());
    this->GetMRMLScene()->SetVersion(this->GetVersion());
    this->GetMRMLScene()->Commit();  
    return 1;
    }
  
  return 0;
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SetSnapshotsRootDirectory()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;

  int nnodes = scene->GetNumberOfNodesByClass("vtkMRMLSceneSnapshotNode");
  for (int n=0; n<nnodes; n++)
    {
    node = scene->GetNthNodeByClass(n, "vtkMRMLSceneSnapshotNode");
    vtkMRMLSceneViewNode *snode = vtkMRMLSceneViewNode::SafeDownCast(node);
    snode->GetNodes()->SetRootDirectory(scene->GetRootDirectory());
    }
}

//---------------------------------------------------------------------------
std::string vtkSlicerMRMLSaveDataWidget::GetRowFullFileName(int row)
{
  std::string empStr("");
  std::string fileName (this->MultiColumnList->GetWidget()->GetCellText(row, FileName_Column));
  if(fileName.empty())
    {
    //vtkErrorMacro(<< this->GetClassName() << " did not have a file name for: " << node->GetName());
    return empStr;
    }

  std::string filePath (this->MultiColumnList->GetWidget()->GetCellText(row, FileDirectory_Column));
  if(filePath.empty() || !vtksys::SystemTools::FileIsDirectory(filePath.c_str()))
    {
    //vtkErrorMacro(<< this->GetClassName() << " did not have a valid path for: " << node->GetName());
    return empStr;
    }

  std::string fileExt = vtksys::SystemTools::GetFilenameLastExtension(fileName);
  std::string currExt = this->GetRowCurrentFileExtension(row); 
  if(strcmp(currExt.c_str(), ".*") &&
    (fileExt.empty() || strcmp(currExt.c_str(), fileExt.c_str())))
    {
    fileName.append(currExt);
    }
  if (filePath[filePath.size()-1] != '/')
    {
    filePath.append("/");
    }
  filePath.append(fileName.c_str());

  return filePath;
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

  int nModified = 0;
  if (this->MultiColumnList->GetWidget()->GetNumberOfRows())
    {
    this->MultiColumnList->GetWidget()->DeleteAllRows ();
    }

  this->AddMRMLSceneRow();
  // get length of MRMLScene Directory...
  int directoryColumnWidth;
  std::string dirtxt = this->MultiColumnList->GetWidget()->GetCellText(0, FileDirectory_Column );
  // Note: Since std::string::length() always return a 'unsigned int' (size_t)
  // there is not need to check for length < 0.
  directoryColumnWidth = dirtxt.length();


  vtkKWMultiColumnList* dataTable = this->MultiColumnList->GetWidget();
  
  // Process all Storable nodes
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLStorableNode");
  int n;
  int row = dataTable->GetNumberOfRows();

  vtkMRMLStorableNode *node = NULL;

  for (n=0; n<nnodes; n++)
    {
    node = vtkMRMLStorableNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLStorableNode"));
    if (node->GetHideFromEditors()) 
      {
      continue;
      }
    vtkMRMLStorageNode* storageNode = node->CreateDefaultStorageNode();
    if (storageNode == NULL)
      {
      continue;
      }
    
    vtkMRMLStorageNode* snode = node->GetStorageNode();
    if (snode == NULL) 
      {
      storageNode->SetScene(this->GetMRMLScene());
      this->SetMRMLScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode);  
      this->SetAndObserveMRMLScene(this->GetMRMLScene());
      node->SetAndObserveStorageNodeID(storageNode->GetID());
      storageNode->Delete();
      snode = storageNode;
      }
    else
      {
      // we don't need a new default storage node
      storageNode->Delete();
      }
    
    if (snode->GetFileName() == NULL && this->DataDirectoryName != NULL) 
      {
      std::string name (this->DataDirectoryName);
      name += std::string(node->GetName());
      const char* ext = snode->GetDefaultWriteFileExtension();
      if (ext) 
        {
        name += std::string(".");
        name += std::string(ext);
        }
      
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

    // Need to set the width of the directory column so entire path is visible.
    // So keep a running measure of max filename lengths, and set
    // column width once all storables have been added.

    std::string tmpstr = name;
    std::string tmpstr2 = vtksys::SystemTools::CollapseFullPath ( tmpstr.c_str());
    std::string tmpstr3 = vtksys::SystemTools::GetFilenamePath ( tmpstr2.c_str());
    int tmpmax = tmpstr3.length();
    if ( tmpmax > directoryColumnWidth )
      {
      directoryColumnWidth = tmpmax;
      }

    this->MultiColumnList->GetWidget()->AddRow();
    this->MultiColumnList->GetWidget()->SetCellText(
      row,NodeName_Column,node->GetName());

    // As a safety measure:
    // If the data is sitting in cache, it's vulnerable to overwriting or deleting.
    // Mark the node as modified since read so that a user will be more likely
    // to save it to a reliable location on local (or remote) disk.
    if ( this->MRMLScene->GetCacheManager() )
      {
      if ( this->MRMLScene->GetCacheManager()->GetRemoteCacheDirectory() )
        {
        std::string cacheDir = this->MRMLScene->GetCacheManager()->GetRemoteCacheDirectory();
        size_t pos = name.find ( cacheDir.c_str() );
        if ( pos != std::string::npos)
          {
          node->ModifiedSinceReadOn();
          }
        }
      else
        {
        vtkWarningMacro ( "Warning saving data: cannot get a default cache directory, so not able to check whether any datafiles are residing in cache and should be marked for save by default. Please take care when saving data." );
        }
      }
    else
      {
      vtkWarningMacro ( "Warning saving data: cannot get a default cache directory, so not able to check whether any datafiles are residing in cache and should be marked for save by default. Please take care when saving data." );
      }

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
    this->MultiColumnList->GetWidget()->SetCellText(row,Type_Column,node->GetNodeTagName());
    this->SetFileNameRelatedCells(row, name.c_str(), snode->GetSupportedWriteFileTypes());
    this->AddNodeId(node->GetID(), row);
    this->AddStorageNodeId(snode->GetID(), row);
    row++;
    }

  //--- adjust width of directory column to fit the longest directory name plus a little buffer
  //--- to accommodate a larger font size, which the kww doesn't seem to account for... (?).
  directoryColumnWidth += 10;
  this->MultiColumnList->GetWidget()->SetColumnWidth (FileDirectory_Column, directoryColumnWidth);

  this->IsProcessing = false;

  return nModified;
  
}

//---------------------------------------------------------------------------
int vtkSlicerMRMLSaveDataWidget::IsSceneRow(int row)
{
  std::string strType(
    this->MultiColumnList->GetWidget()->GetCellText(row,Type_Column));
  return (strType.find("SCENE")!=std::string::npos);
}

//---------------------------------------------------------------------------
int vtkSlicerMRMLSaveDataWidget::GetSceneRowIndex()
{
  int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {    
    if (this->IsSceneRow(row))
      {
      return row;
      }
    }

  return -1;
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::AddMRMLSceneRow()
{
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
      this->ChangeAllSelectedDirButton->GetWidget()->SetInitialFileName(
        this->DataDirectoryName);
      this->ChangeAllSelectedDirButton->GetWidget()->GetLoadSaveDialog()->
        SetFileName(this->DataDirectoryName);
      }
    std::string uriName;
    const char *url = this->MRMLScene->GetURL();
    if (!url || !(*url))
      {
      uriName = dir.append("SlicerScene1");
      }
    else
      {
      uriName = url;
      }

    if(!uriName.empty())
      {
      vtkKWMultiColumnList* dataTable = this->MultiColumnList->GetWidget();
      int row = dataTable->GetNumberOfRows();
      dataTable->AddRow();

      // Initialize the mrml scene row
      dataTable->SetCellText(row,NodeName_Column,"(Scene Description)");
      this->SetRowModified(row, 1);
      this->SetRowMarkedForSave(row, 1);

      dataTable->SetCellText(row,Type_Column,"(SCENE)");
      const char* mrmlFileFormats[] = {"MRML (.mrml)"};
      dataTable->SetCellText(row,Format_Column, "MRML (.mrml)");
      dataTable->SetCellWindowCommandToComboBoxWithValues(
        row, Format_Column, 1, mrmlFileFormats);

      this->SetFileNameAndDirectoryCells(row, uriName.c_str());
      dataTable->SetCellEditable(row,Format_Column, 0);
      dataTable->SetCellEnabledAttribute(row, Format_Column, 0);

      //this->SaveSceneButton->GetWidget()->SetInitialFileName(uriName.c_str());
      //this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->SetFileName(uriName.c_str());
      }
    }
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
  const char* tmpExt = this->GetRowCurrentFileExtension(row);
  if(!tmpExt || !(*tmpExt))
    {
    vtkWarningMacro(<< this->GetClassName() << 
      ": Invalid file format for saving!");
    this->SetRowMarkedForSave(row, 0);
    }

  std::string currExt = tmpExt; 
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
void vtkSlicerMRMLSaveDataWidget::UpdateRowFileFormatWithName(int row)
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
  std::string fileExt = vtksys::SystemTools::GetFilenameLastExtension(fileName);

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
    else
      {
      vtkWarningMacro(<< "The new extension: " << fileExt.c_str() << 
        ", is not supported! The current extension will be used.");
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
    for(int i=0; i<cb->GetNumberOfValues(); i++)
      {
      strExt = cb->GetValueFromIndex(i);
      const char* tmpExt = vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
        strExt.c_str());
      if(!tmpExt || !(*tmpExt))
        {
        continue;
        }
      value = tmpExt;
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
  const char* fileformat = this->MultiColumnList->GetWidget()->
    GetCellText(row,Format_Column);
  if(fileformat && *fileformat)
    {
    std::string tmpFormat(fileformat);
    return vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
      tmpFormat.c_str());
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
  if(!this->MultiColumnList->IsCreated() || !filename || !(*filename) )
    {
    return;
    }

  if(!supportedFileFormats || 
    supportedFileFormats->GetNumberOfTuples()==0)
    {
    this->DisableRowForSaving(row, filename);
    return;
    }

  std::string name = vtksys::SystemTools::CollapseFullPath(filename);
  std::string strExt = vtksys::SystemTools::GetFilenameLastExtension(name);
  const char* pFileFormat = this->GetFileFormatWithExtension(strExt.c_str(),
    supportedFileFormats);
  if(!pFileFormat || !(*pFileFormat))
    {
    this->InitRowForNonSupportedFormat(row, name.c_str(), supportedFileFormats);
    return;
    }
  this->SetFileNameAndDirectoryCells(row, name.c_str());
  std::string fileFormat = pFileFormat;
  this->MultiColumnList->GetWidget()->SetCellText(
    row,Format_Column, fileFormat.c_str());
  this->MultiColumnList->GetWidget()->
    SetCellWindowCommandToComboBoxWithValuesAsArray(
    row, Format_Column, supportedFileFormats);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SetFileNameAndDirectoryCells(
  int row, const char* filename)
{
  if(!filename || !(*filename))
    {
    return;
    }
  std::string name = vtksys::SystemTools::CollapseFullPath(filename);
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
void vtkSlicerMRMLSaveDataWidget::DisableRowForSaving(
  int row, const char* filename)
{
  vtkKWMultiColumnList* dataTable = this->MultiColumnList->GetWidget();
  dataTable->SetCellTextAsInt(row,Save_Column,0);
  dataTable->SetCellWindowCommandToCheckButton(row, Save_Column);

  const char* noFileFormats[] = {"Not supported"};
  dataTable->SetCellText(row,Format_Column, "Not supported");
  dataTable->SetCellWindowCommandToComboBoxWithValues(
    row, Format_Column, 1, noFileFormats);

  this->SetFileNameAndDirectoryCells(row, filename);
  this->SetFileNameRelatedCellsEnabled(row, 0);

  // Disable the "Save" checkbox
  //vtkKWCheckButton* cb = dataTable->
  //  GetCellWindowAsComboBox(row, Save_Column);
  //if(cb)
  //  {
  //  cb->SetEnabled(0);
  //  }
  dataTable->CellEditableOff(row,Save_Column);
  dataTable->SetCellEnabledAttribute(row, Save_Column, 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::InitRowForNonSupportedFormat(
  int row, const char* filename, vtkStringArray* supportedFileFormats)
{
  vtkKWMultiColumnList* dataTable = this->MultiColumnList->GetWidget();
  dataTable->SetCellTextAsInt(row,Save_Column,0);
  dataTable->SetCellWindowCommandToCheckButton(row, Save_Column);

  supportedFileFormats->InsertNextValue(PICK_FORMAT_MESSAGE);
  dataTable->SetCellText(row,Format_Column, PICK_FORMAT_MESSAGE);
  dataTable->SetCellWindowCommandToComboBoxWithValuesAsArray(
    row, Format_Column, supportedFileFormats);

  this->SetFileNameAndDirectoryCells(row, filename);
  //this->SetFileNameRelatedCellsEnabled(row, 0);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SetFileNameRelatedCellsEnabled(
  int row, int enable)
{
  vtkKWMultiColumnList* dataTable = this->MultiColumnList->GetWidget();
  dataTable->SetCellEditable(row,Format_Column, enable);
  dataTable->SetCellEditable(row,FileName_Column, enable);
  dataTable->SetCellEditable(row,FileDirectory_Column, enable);
  dataTable->SetCellEnabledAttribute(row, Format_Column, enable);
  dataTable->SetCellEnabledAttribute(row, FileName_Column, enable);
  dataTable->SetCellEnabledAttribute(row, FileDirectory_Column, enable);
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
  
  this->ChangeAllSelectedDirButton->GetWidget()->SetInitialFileName(
    this->DataDirectoryName);

  int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {    
    if (this->MultiColumnList->GetWidget()->GetCellTextAsInt(row, Save_Column))
      {
      // Do we really need this?
      if(!this->IsSceneRow(row))
        {
        this->UpdateNodeDataDirectory(row);
        }
      // Update the data table
      this->SetDataDirectoryRelatedCells(row, this->DataDirectoryName);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::ProcessMRMLEvents(vtkObject *vtkNotUsed(caller),
                                                    unsigned long event,
                                                    void *vtkNotUsed(callData))
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
  //this->SaveDataOnlyButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->CancelButton->AddObserver ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  //this->SaveSceneButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ChangeAllSelectedDirButton->GetWidget()->GetLoadSaveDialog()->AddObserver ( vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectAllButton->AddObserver(  vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SelectNoneButton->AddObserver(  vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SelectModifiedDataButton->AddObserver(  vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  this->SelectSceneAndModifiedDataButton->AddObserver(  vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  
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
  if (this->CancelButton)
    {
    this->CancelButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->ChangeAllSelectedDirButton)
    {
    this->ChangeAllSelectedDirButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent,
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SelectAllButton)
    {
    this->SelectAllButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SelectNoneButton)
    {
    this->SelectNoneButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SelectSceneAndModifiedDataButton)
    {
    this->SelectSceneAndModifiedDataButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
        (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->SelectModifiedDataButton)
    {
    this->SelectModifiedDataButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  
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

  if(!this->GUIIcons)
    {
    this->GUIIcons = vtkSlicerSaveDataWidgetIcons::New();
    }

  // Call the superclass to create the whole widget

  this->Superclass::CreateWidget();

  this->SaveDialog = vtkKWDialog::New();
  this->SaveDialog->SetMasterWindow ( this->GetParent());
  this->SaveDialog->SetParent ( this->GetParent());
  this->SaveDialog->SetTitle("Save Scene and Unsaved Data");
  this->SaveDialog->SetSize(400, 200);
  this->SaveDialog->Create ( );
  this->SaveDialog->ModalOn();

  // Data Frame
  vtkKWFrameWithLabel *dataFrame = vtkKWFrameWithLabel::New ( );
  dataFrame->SetParent ( this->SaveDialog );
  dataFrame->Create ( );
  dataFrame->SetLabelText ("Save Scene & Data Options");
  this->Script ( "pack %s -side top -anchor nw -fill both -expand true -padx 2 -pady 2",
                 dataFrame->GetWidgetName() );

  // add a button frame
  vtkKWFrame *buttonFrame = vtkKWFrame::New();
  buttonFrame->SetParent(dataFrame->GetFrame());
  buttonFrame->Create();
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                 buttonFrame->GetWidgetName() );

  // add a button to change the save flags of all modified data and scene to ON
  this->SelectSceneAndModifiedDataButton = vtkKWPushButton::New();
  this->SelectSceneAndModifiedDataButton->SetParent ( buttonFrame );
  this->SelectSceneAndModifiedDataButton->Create ( );
  //this->SelectSceneAndModifiedDataButton->SetText ("Select Scene & Modified Data Only");
  this->SelectSceneAndModifiedDataButton->SetImageToIcon(this->GUIIcons->GetCheckModified());
  this->SelectSceneAndModifiedDataButton->SetBalloonHelpString("Select scene & modified data only");
  this->Script("pack %s -side left -anchor w -padx 2 -pady 2", 
              this->SelectSceneAndModifiedDataButton->GetWidgetName());

  // add a button to change the save flags of all modified data to ON
  this->SelectModifiedDataButton = vtkKWPushButton::New();
  this->SelectModifiedDataButton->SetParent ( buttonFrame );
  this->SelectModifiedDataButton->Create ( );
  //this->SelectModifiedDataButton->SetText ("Select Modified Data Only");
  this->SelectModifiedDataButton->SetImageToIcon(this->GUIIcons->GetCheckModifiedData());
  this->SelectModifiedDataButton->SetBalloonHelpString("Select modified data only");
  this->Script("pack %s -side left -anchor w -padx 2 -pady 2", 
              this->SelectModifiedDataButton->GetWidgetName());
  
  // add a button to change all the save flags to on
  this->SelectAllButton = vtkKWPushButton::New();
  this->SelectAllButton->SetParent ( buttonFrame );
  this->SelectAllButton->Create ( );
  //this->SelectAllButton->SetText ("Select All");
  this->SelectAllButton->SetImageToIcon(this->GUIIcons->GetCheckAll());
  this->SelectAllButton->SetBalloonHelpString("Select all");
  this->Script("pack %s -side left -anchor w -padx 2 -pady 2", 
              this->SelectAllButton->GetWidgetName());

  // add a button to change all the save flags to off
  this->SelectNoneButton = vtkKWPushButton::New();
  this->SelectNoneButton->SetParent ( buttonFrame );
  this->SelectNoneButton->Create ( );
  //this->SelectNoneButton->SetText ("Select None");
  this->SelectNoneButton->SetImageToIcon(this->GUIIcons->GetUncheckAll());
  this->SelectNoneButton->SetBalloonHelpString("Select none");
  this->Script("pack %s -side left -anchor w -padx 2 -pady 2", 
              this->SelectNoneButton->GetWidgetName());

  // add a data file browser 
  this->ChangeAllSelectedDirButton = vtkKWLoadSaveButtonWithLabel::New ( );
  this->ChangeAllSelectedDirButton->SetParent ( buttonFrame );
  this->ChangeAllSelectedDirButton->Create ( );
  this->ChangeAllSelectedDirButton->SetLabelPositionToLeft();
  this->ChangeAllSelectedDirButton->SetLabelText ("Change Destination for All Selected:");
  this->ChangeAllSelectedDirButton->GetWidget()->TrimPathFromFileNameOff();
  this->ChangeAllSelectedDirButton->GetWidget()->SetMaximumFileNameLength(64);
  this->ChangeAllSelectedDirButton->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
  this->ChangeAllSelectedDirButton->GetWidget()->GetLoadSaveDialog()->RetrieveLastPathFromRegistry(
    "OpenPath");
  this->Script("pack %s -side right -anchor nw -expand n -padx 2 -pady 2", 
                 this->ChangeAllSelectedDirButton->GetWidgetName());
 
  // add the multicolumn list to show the points
  this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->MultiColumnList->SetParent ( dataFrame->GetFrame() );
  this->MultiColumnList->Create ( );
  //this->MultiColumnList->SetHeight(4);
  this->MultiColumnList->GetWidget()->SetSelectionTypeToCell();
  
  this->SetupSaveDataListWidget();
  this->Script ( "pack %s -side top -fill both -expand y",
                     this->MultiColumnList->GetWidgetName());

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
  this->OkButton->SetText ("Save Selected");
  //this->Script("pack %s -side right -anchor center -padx 16 -pady 4", 
  //            this->OkButton->GetWidgetName());
  this->Script ( "grid %s -row 0 -column 0 -sticky news -padx 8 -pady 4 ", 
    this->OkButton->GetWidgetName());

  // add Cancel button
  this->CancelButton = vtkKWPushButton::New ( );
  this->CancelButton->SetParent ( saveFrame );
  this->CancelButton->Create ( );
  this->CancelButton->SetWidth (this->OkButton->GetWidth());
  this->CancelButton->SetText ("Cancel");

  //this->Script("pack %s -side right -anchor center -padx 16 -pady 4", 
  //            this->CancelButton->GetWidgetName());
  this->Script ( "grid %s -row 0 -column 1 -sticky news -padx 8 -pady 4 ", 
    this->CancelButton->GetWidgetName());

  // add observers
  this->AddWidgetObservers();
  
  this->MultiColumnList->SetEnabled(1);
  this->OkButton->SetEnabled(1);
  
  if (this->MRMLScene != NULL)
    {
    this->SetAndObserveMRMLScene(this->MRMLScene);
    }

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
  saveDataMCList->ColumnEditableOn(col_index);
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

  this->UpdateFromMRML();

  // don't allow node name edits to scene description
  int sceneRow = this->GetSceneRowIndex();
  this->MultiColumnList->GetWidget()->CellEditableOff ( sceneRow, NodeName_Column);

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
  std::string oldFileName(this->MultiColumnList->GetWidget()->GetCellText(
    row, Hidden_FileName_Column));
  std::string currText = this->MultiColumnList->GetWidget()->GetCellText(row, col);
  const char* pText = currText.c_str();
  int sceneRow = this->GetSceneRowIndex();
  vtkMRMLNode *node = this->MRMLScene->GetNodeByID(
     this->MultiColumnList->GetWidget()->GetCellText(row, Hidden_NodeID_Column));

  switch(col)
    {
    case Save_Column:
      this->IsRowFileFormatSet(row);
      this->UpdateEnableState();
      break;
    case NodeName_Column:
      // dump out with error if the node is invalid.
      if ( node )
        {
        // reset GUI to old node name if input text is empty
        const char *oldNodeName = node->GetName ();
        if(!pText || !(*pText) ||!strlen(pText))
          {      
          if ( !oldNodeName || !(*oldNodeName) || !strlen (oldNodeName))
            {
            vtkErrorMacro(<< this->GetClassName() << ": Both new and old node names invalid. Using ID as name.");
            node->SetName(node->GetID());
            this->SetRowMarkedForSave(sceneRow, 1);
            if ( strcmp (pText, node->GetID() ) )
              {
              this->MultiColumnList->GetWidget()->SetCellText( row, col, node->GetID() );
              }
            }
          else
            {
            vtkErrorMacro(<< this->GetClassName() << ": New node name invalid. Using old node name.");
            this->MultiColumnList->GetWidget()->SetCellText( row, col, oldNodeName);
            }
          }
        else
          {
          // update the node name if it differs from previous and mark scene for saving.
          if ( strcmp (pText, oldNodeName ))
            {
            node->SetName(pText);
            this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::SceneEditedEvent, node);
            this->SetRowMarkedForSave(sceneRow, 1);
            }
          }
        }
      else
        {
        vtkErrorMacro(<< this->GetClassName() << ": Invalid node. Cannot set new name.");
        }
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

      if(strcmp(pText, oldFileName.c_str())
        && !this->IsSceneRow(row)
        && this->IsRowFileFormatSet(row))
        {
        this->UpdateRowFileFormatWithName(row);
        this->MultiColumnList->GetWidget()->SetCellText(
          row, Hidden_FileName_Column, pText);
        this->SetRowMarkedForSave(row, 1);
        }
      break;
    case Format_Column:
      if(!pText || !(*pText) ||!strlen(pText))
        {
        vtkWarningMacro(<< this->GetClassName() << 
          ": Invalid file format for saving!");
        this->SetRowMarkedForSave(row, 0);
        return;
        }

      if(this->IsRowFileFormatSet(row) && !fileName.empty())
        {
        std::string currExt = this->GetRowCurrentFileExtension(row);
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
int vtkSlicerMRMLSaveDataWidget::IsRowFileFormatSet(int row)
{
  const char* tmpExt = this->GetRowCurrentFileExtension(row);
  if(!tmpExt || !(*tmpExt))
    {
    this->SetFirstAvailableFormat(row);
    //check again of the file format
    tmpExt = this->GetRowCurrentFileExtension(row);
    if(!tmpExt || !(*tmpExt))
      {
      vtkWarningMacro(<< this->GetClassName() << 
        ": The file format is not set for saving!");
      this->SetRowMarkedForSave(row, 0);
      return 0;
      }
    }

  return 1;
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SetFirstAvailableFormat(int row)
{
  vtkKWComboBox* cb = this->MultiColumnList->GetWidget()->
    GetCellWindowAsComboBox(row, Format_Column);
  if(cb)
    {
    std::string value, strExt;
    for(int i=0; i<cb->GetNumberOfValues(); i++)
      {
      strExt = cb->GetValueFromIndex(i);
      const char* tmpExt = vtkDataFileFormatHelper::GetFileExtensionFromFormatString(
        strExt.c_str());
      if(!tmpExt || !(*tmpExt))
        {
        continue;
        }
      cb->SetValue(strExt.c_str());
      this->MultiColumnList->GetWidget()->SetCellText(
        row,Format_Column,strExt.c_str());
      this->UpdateRowFileNameWithExtension(row);
      break;
      }
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

//---------------------------------------------------------------------------
int vtkSlicerMRMLSaveDataWidget::IsRowModified( int row)
{
  return (strcmp(this->MultiColumnList->GetWidget()
    ->GetCellText(row,Status_Column), "Modified")==0);
}

//---------------------------------------------------------------------------
void vtkSlicerMRMLSaveDataWidget::SetAllRowsSelected(int selected)
{
  int nrows = this->MultiColumnList->GetWidget()->GetNumberOfRows();
  for (int row=0; row<nrows; row++)
    {
    if(selected && this->IsRowFileFormatSet(row))
      {
      this->SetRowMarkedForSave(row, 1, 0);
      }
    else
      {
      this->SetRowMarkedForSave(row, 0, 0);
      }
    }
  this->UpdateEnableState();
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
  this->OkButton->SetEnabled(numSelected>0 ? 1 : 0);
  this->ChangeAllSelectedDirButton->SetEnabled(numSelected>0 ? 1 : 0);

  this->SelectAllButton->SetEnabled(numSelected < nrows ? 1 : 0);
  this->SelectNoneButton->SetEnabled(numSelected > 0 ? 1 : 0);
  this->SelectSceneAndModifiedDataButton->SetEnabled(nrows>0 ? 1 : 0);
  this->SelectModifiedDataButton->SetEnabled(nrows>0 ? 1 : 0);
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
