/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/OpenIGTLinkIF/vtkOpenIGTLinkIFGUI.h $
  Date:      $Date: 2010-04-01 11:42:15 -0400 (Thu, 01 Apr 2010) $
  Version:   $Revision: 12582 $

==========================================================================*/


#include "vtkObjectFactory.h"

#include "vtkIGTLRemoteDataListWindow.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerTheme.h"

#include "vtkRenderer.h"

#include "vtkCornerAnnotation.h"

#include "vtkMRMLIGTLQueryNode.h"
#include "vtkMRMLImageMetaListNode.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkIGTLRemoteDataListWindow);
vtkCxxRevisionMacro(vtkIGTLRemoteDataListWindow, "$Revision: 1.0 $");
//----------------------------------------------------------------------------
vtkIGTLRemoteDataListWindow::vtkIGTLRemoteDataListWindow()
{
  this->MainFrame = vtkKWFrame::New();
  this->MultipleMonitorsAvailable = false; 
  this->WindowPosition[0]=0;
  this->WindowPosition[1]=0;
  this->WindowSize[0]=0;
  this->WindowSize[1]=0;

  this->RemoteDataList = NULL;
  this->GetListButton  = NULL;
  this->GetImageButton = NULL;
  this->CloseButton    = NULL;

  //this->CurrentTarget=NULL;

  // GUI callback command
  this->InGUICallbackFlag = 0;
  this->InMRMLCallbackFlag = 0;
  this->IsObserverAddedFlag = 0;

  this->GUICallbackCommand = vtkCallbackCommand::New();
  this->GUICallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->GUICallbackCommand->SetCallback(&vtkIGTLRemoteDataListWindow::GUICallback);

  this->MRMLObserverManager = vtkObserverManager::New();
  this->MRMLObserverManager->GetCallbackCommand()->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLObserverManager->GetCallbackCommand()->SetCallback(vtkIGTLRemoteDataListWindow::MRMLCallback);
  this->MRMLCallbackCommand = this->MRMLObserverManager->GetCallbackCommand();

  this->ModuleGUI = NULL; 
  this->MRMLScene = NULL;

  this->Connector = NULL;
  this->ImageMetaListQueryNode = NULL;

}


//----------------------------------------------------------------------------
vtkIGTLRemoteDataListWindow::~vtkIGTLRemoteDataListWindow()
{
  //if (this->CurrentTarget)
  //{
  //  this->CurrentTarget->UnRegister(this);
  //  this->CurrentTarget=NULL;
  //}

  if ( this->GUICallbackCommand != NULL )
    {
    this->GUICallbackCommand->Delete ( );
    this->GUICallbackCommand = NULL;
    }

  if (this->RemoteDataList)
    {
    this->RemoteDataList->SetParent(NULL);
    this->RemoteDataList->Delete();
    }

  if (this->GetListButton)
    {
    this->GetListButton->SetParent(NULL);
    this->GetListButton->Delete();
    }
  if (this->GetImageButton)
    {
    this->GetImageButton->SetParent(NULL);
    this->GetImageButton->Delete();
    }
  if (this->CloseButton)
    {
    this->CloseButton->SetParent(NULL);
    this->CloseButton->Delete();
    }

  this->MRMLObserverManager->Delete();
  this->MainFrame->Delete();
  this->SetApplication(NULL);

  this->ModuleGUI = NULL; 
}


//----------------------------------------------------------------------------
void vtkIGTLRemoteDataListWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkIGTLRemoteDataListWindow::GUICallback( vtkObject *caller,
                           unsigned long eid, void *clientData, void *callData )
{
  
  vtkIGTLRemoteDataListWindow *self = reinterpret_cast<vtkIGTLRemoteDataListWindow *>(clientData);
  
  if (self->GetInGUICallbackFlag())
    {
    }

  self->SetInGUICallbackFlag(1);
  self->ProcessGUIEvents(caller, eid, callData);
  self->SetInGUICallbackFlag(0);
  
}

//----------------------------------------------------------------------------
void vtkIGTLRemoteDataListWindow::MRMLCallback(vtkObject *caller, 
                                    unsigned long eid, void *clientData, void *callData)
{
  
  vtkIGTLRemoteDataListWindow *self = reinterpret_cast<vtkIGTLRemoteDataListWindow *>(clientData);
  
  if (self->GetInMRMLCallbackFlag())
    {
    return;
    }

  self->SetInMRMLCallbackFlag(1);
  self->ProcessMRMLEvents(caller, eid, callData);
  self->SetInMRMLCallbackFlag(0);
}


void vtkIGTLRemoteDataListWindow::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData)
{

  if (this->GetListButton == vtkKWPushButton::SafeDownCast(caller) 
      && event == vtkKWPushButton::InvokedEvent )
    {
    std::cerr << "GetListButton is pressed. " << std::endl;
    if (this->MRMLScene && this->Connector)
      {
      if (this->ImageMetaListQueryNode == NULL)
        {
        this->ImageMetaListQueryNode = vtkMRMLIGTLQueryNode::New();
        this->ImageMetaListQueryNode->SetIGTLName("IMGMETA");
        //this->ImageMetaListQueryNode->SetIGTLName(igtl::ImageMetaMessage::GetDeviceType());
        this->MRMLScene->AddNode(this->ImageMetaListQueryNode);
        this->ImageMetaListQueryNode->AddObserver(vtkMRMLIGTLQueryNode::ResponseEvent,this->MRMLCallbackCommand);
        }
      this->ImageMetaListQueryNode->SetQueryStatus(vtkMRMLIGTLQueryNode::STATUS_PREPARED);
      this->ImageMetaListQueryNode->SetQueryType(vtkMRMLIGTLQueryNode::TYPE_GET);
      this->Connector->PushQuery((vtkMRMLIGTLQueryNode*)this->ImageMetaListQueryNode);
      }
    }
  else if (this->GetImageButton == vtkKWPushButton::SafeDownCast(caller) 
      && event == vtkKWPushButton::InvokedEvent )
    {
    std::cerr << "GetImageButton is pressed. " << std::endl;
    if (this->RemoteDataList == NULL || this->ImageMetaListQueryNode ==NULL || this->MRMLScene ==NULL)
      {
      return;
      }
    // Obtain ImageMetaList node
    vtkMRMLImageMetaListNode* node 
      = vtkMRMLImageMetaListNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ImageMetaListQueryNode->GetResponseDataNodeID()));

    if (node && node->GetNumberOfImageMetaElement() > 0)
      {
      if (this->MRMLScene && this->Connector)
        {
        int selected = this->RemoteDataList->GetWidget()->GetIndexOfFirstSelectedRow();
        vtkMRMLImageMetaListNode::ImageMetaElement element;
        node->GetImageMetaElement(selected, &element);
        
        vtkMRMLIGTLQueryNode* node = vtkMRMLIGTLQueryNode::New();
        node->SetIGTLName("IMAGE");
        node->SetQueryStatus(vtkMRMLIGTLQueryNode::STATUS_PREPARED);
        node->SetQueryType(vtkMRMLIGTLQueryNode::TYPE_GET);
        node->SetName(element.DeviceName.c_str());
        this->ImageQueryNodeList.push_back(node);
        this->MRMLScene->AddNode(node);
        node->AddObserver(vtkMRMLIGTLQueryNode::ResponseEvent, this->MRMLCallbackCommand);
        this->Connector->PushQuery(node);
        }
      }
    }
  else if (this->CloseButton == vtkKWPushButton::SafeDownCast(caller) 
      && event == vtkKWPushButton::InvokedEvent )
    {
    std::cerr << "CloseButton is pressed. " << std::endl;
    }

}


void vtkIGTLRemoteDataListWindow::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  vtkMRMLIGTLQueryNode* qnode = vtkMRMLIGTLQueryNode::SafeDownCast(caller);
  if (qnode)
    {
    if (qnode == this->ImageMetaListQueryNode)
      {
      switch (event)
        {
        case vtkMRMLIGTLQueryNode::ResponseEvent:
          this->UpdateRemoteDataList();
          break;
        default:
          break;
        }
      }
    else if (event == vtkMRMLIGTLQueryNode::ResponseEvent &&
             qnode->GetQueryStatus() == vtkMRMLIGTLQueryNode::STATUS_SUCCESS)
      {
      std::list<vtkMRMLIGTLQueryNode*>::iterator iter;
      for (iter = this->ImageQueryNodeList.begin();
           iter != this->ImageQueryNodeList.end();
           iter ++)
        {
        if (qnode == *iter)
          {
          (*iter)->RemoveObserver(this->MRMLCallbackCommand);
          if (this->MRMLScene)
            {
            this->MRMLScene->RemoveNode(*iter);
            }
          this->ImageQueryNodeList.remove(*iter);
          break;
          }
        }
      }
    }
}


void vtkIGTLRemoteDataListWindow::AddGUIObservers()
{
  this->RemoveGUIObservers();

  if (this->GetListButton)
    {
    this->GetListButton->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
    }

  if (this->GetImageButton)
    {
    this->GetImageButton->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
    }

  if (this->CloseButton)
    {
    this->CloseButton->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
    }

}


void vtkIGTLRemoteDataListWindow::RemoveGUIObservers()
{
  if (this->GetListButton)
    {
    this->GetListButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->GetImageButton)
    {
    this->GetImageButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->CloseButton)
    {
    this->CloseButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
}


#ifdef _WIN32
//----------------------------------------------------------------------------
BOOL CALLBACK StoreNonPrimaryMonitorRectEnumProc(HMONITOR hMonitor, HDC hdc, LPRECT prc, LPARAM dwData) 
{
  MONITORINFO mi;
  mi.cbSize = sizeof(MONITORINFO);
  if (GetMonitorInfo(hMonitor, &mi))
  {
    if (!(mi.dwFlags & MONITORINFOF_PRIMARY))
    {
      RECT *rect = (RECT*)dwData;
      if (rect==NULL)
      {
        return false;
      }
      // store the rect of the non-primary monitor only
      rect->left=mi.rcMonitor.left;
      rect->right=mi.rcMonitor.right;
      rect->top=mi.rcMonitor.top;
      rect->bottom=mi.rcMonitor.bottom;     
    }
  }
  return true;
} 


//----------------------------------------------------------------------------
void vtkIGTLRemoteDataListWindow::UpdateWindowPoisition()
{
  // Window rect
  RECT rect;    

  // Fill rect by default with primary monitor info
  rect.left=0;
  rect.right=GetSystemMetrics(SM_CXFULLSCREEN);
  rect.top=0;
  rect.bottom=GetSystemMetrics(SM_CYFULLSCREEN);    

  // if a secondary monitor is available then get its virtual screen coordinates
  int nMonitors = GetSystemMetrics(SM_CMONITORS);
  if (nMonitors>1)
  {
    this->MultipleMonitorsAvailable = true; 
    if (!EnumDisplayMonitors(NULL, NULL, StoreNonPrimaryMonitorRectEnumProc, (LPARAM) &rect))
    {
      vtkErrorMacro("DetectMonitors: EnumDisplayMonitors failed");
    }
  }
  else
  {
    this->MultipleMonitorsAvailable = false; 
  }

  this->WindowPosition[0]=rect.left;
  this->WindowPosition[1]=rect.top;
  this->WindowSize[0]=rect.right-rect.left;
  this->WindowSize[1]=rect.bottom-rect.top;
}

#else // _WIN32

void vtkIGTLRemoteDataListWindow::UpdateWindowPoisition()
{
  // TODO: implement monitor detection for linux
  this->MultipleMonitorsAvailable = false; 
  this->WindowPosition[0]=0;
  this->WindowPosition[1]=0;
  this->WindowSize[0]=1024;
  this->WindowSize[1]=768;
}

#endif // _WIN32

//----------------------------------------------------------------------------
void vtkIGTLRemoteDataListWindow::CreateWidget()
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  if (app==NULL)
  {
  vtkErrorMacro("CreateWindow: application is invalid");
  return;
  }

  if (this->IsCreated())
  {
    return;
  }

  vtkKWTopLevel::CreateWidget();

  //this->SetMasterWindow (this->GetServerMenuButton() );
  this->SetApplication ( app );
  //this->Create();
  this->SetBorderWidth ( 1 );
  this->SetReliefToFlat();

  //this->SetParent (this->GetApplicationGUI()->GetMainSlicerWindow());

  this->SetTitle ("OpenIGTLink Remote Data List");
  //this->SetSize (400, 100);
  this->Withdraw();

  this->MainFrame->SetParent ( this );
  this->MainFrame->Create();
  this->MainFrame->SetBorderWidth ( 1 );
  this->Script ( "pack %s -side top -anchor nw -fill both -expand 1 -padx 0 -pady 1", this->MainFrame->GetWidgetName() ); 

  this->RemoteDataList = vtkKWMultiColumnListWithScrollbars::New();
  this->RemoteDataList->SetParent(this->MainFrame);
  this->RemoteDataList->Create();
  this->RemoteDataList->SetHeight(1);
  this->RemoteDataList->GetWidget()->SetSelectionTypeToRow();
  this->RemoteDataList->GetWidget()->SetSelectionModeToSingle();
  this->RemoteDataList->GetWidget()->MovableRowsOff();
  this->RemoteDataList->GetWidget()->MovableColumnsOff();

  const char* labels[] =
    { "IGTL NAME", "Patient ID", "Patient Name", "Modality", "Date", "Status", "Description"};
  const int widths[] = 
    { 12, 20, 20, 20, 10, 10, 30};

  for (int col = 0; col < 7; col ++)
    {
    this->RemoteDataList->GetWidget()->AddColumn(labels[col]);
    this->RemoteDataList->GetWidget()->SetColumnWidth(col, widths[col]);
    this->RemoteDataList->GetWidget()->SetColumnAlignmentToLeft(col);
    this->RemoteDataList->GetWidget()->ColumnEditableOff(col);
    //this->RemoteDataList->GetWidget()->ColumnEditableOn(col);
    this->RemoteDataList->GetWidget()->SetColumnEditWindowToSpinBox(col);
    }
  this->RemoteDataList->GetWidget()->SetColumnEditWindowToCheckButton(0);

  this->Script ("pack %s -side top -fill both -expand true",  this->RemoteDataList->GetWidgetName());
  
  // Connector Property -- Connector type (server or client)
  vtkKWFrame *buttonFrame = vtkKWFrame::New();
  buttonFrame->SetParent(this->MainFrame);
  buttonFrame->Create();
  app->Script ( "pack %s -side top -fill both -expand true",  
                buttonFrame->GetWidgetName());

  this->GetListButton = vtkKWPushButton::New();
  this->GetListButton->SetParent(buttonFrame);
  this->GetListButton->Create();
  this->GetListButton->SetText( "Get List" );
  this->GetListButton->SetWidth (10);

  this->GetImageButton = vtkKWPushButton::New();
  this->GetImageButton->SetParent(buttonFrame);
  this->GetImageButton->Create();
  this->GetImageButton->SetText( "Get Image" );
  this->GetImageButton->SetWidth (10);

  this->CloseButton = vtkKWPushButton::New();
  this->CloseButton->SetParent(buttonFrame);
  this->CloseButton->Create();
  this->CloseButton->SetText( "Close" );
  this->CloseButton->SetWidth (6);

  app->Script ( "pack %s %s %s -side left -anchor nw -expand n -padx 2 -pady 2",
                this->GetListButton->GetWidgetName(),
                this->GetImageButton->GetWidgetName(),
                this->CloseButton->GetWidgetName());

  buttonFrame->Delete();
}


//----------------------------------------------------------------------------
void vtkIGTLRemoteDataListWindow::DisplayOnWindow()
{
  //-- display
  this->DeIconify();
  this->Raise();

  this->UpdateWindowPoisition();

  this->SetPosition(this->WindowPosition[0], this->WindowPosition[1]);
  //this->SetSize(this->WindowSize[0], this->WindowSize[1]);

  /*
  if (this->MultipleMonitorsAvailable)
  {
    this->HideDecorationOn();
  }
  */

  if (!this->IsObserverAddedFlag)
    {
    this->IsObserverAddedFlag = 1;
    this->AddGUIObservers();
    }

}


//----------------------------------------------------------------------------
void vtkIGTLRemoteDataListWindow::UpdateRemoteDataList()
{

  if (this->RemoteDataList == NULL || this->ImageMetaListQueryNode ==NULL || this->MRMLScene ==NULL)
    {
    return;
    }

  // Obtain ImageMetaList node
  vtkMRMLImageMetaListNode* node 
    = vtkMRMLImageMetaListNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ImageMetaListQueryNode->GetResponseDataNodeID()));
  if (node)
    {
    int numRows = this->RemoteDataList->GetWidget()->GetNumberOfRows();
    int numImages = node->GetNumberOfImageMetaElement();
    if (numRows < numImages)
      {
      this->RemoteDataList->GetWidget()->AddRows(numImages-numRows);
      }
    else if (numRows > numImages)
      {
      int ndel = numRows-numImages;
      for (int i = 0; i < ndel; i ++)
        {
        this->RemoteDataList->GetWidget()->DeleteRow(numImages);
        }
      }

    for (int i = 0; i < numImages; i ++)
      {
      vtkMRMLImageMetaListNode::ImageMetaElement element;
      node->GetImageMetaElement(i, &element);
      this->RemoteDataList->GetWidget()->SetCellText(i, 0, element.DeviceName.c_str());
      this->RemoteDataList->GetWidget()->SetCellText(i, 1, element.PatientID.c_str());
      this->RemoteDataList->GetWidget()->SetCellText(i, 2, element.Modality.c_str());
      this->RemoteDataList->GetWidget()->SetCellText(i, 3, "--"); // Date
      this->RemoteDataList->GetWidget()->SetCellText(i, 4, "--"); // Status
      this->RemoteDataList->GetWidget()->SetCellText(i, 5, "--"); // Description
      }
    }
  
}
