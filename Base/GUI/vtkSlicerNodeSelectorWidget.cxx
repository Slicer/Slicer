/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkSlicerNodeSelectorWidget.cxx,v $
Date:      $Date$
Version:   $Revision$

=========================================================================auto=*/

#include "vtkSlicerNodeSelectorWidget.h"


#include "vtkKWMenu.h"
#include "vtkKWMenuButtonWithSpinButtons.h"
#include "vtkSlicerColorLUTIcons.h"

#include <sstream>

//----------------------------------------------------------------------------
vtkStandardNewMacro( vtkSlicerNodeSelectorWidget );
vtkCxxRevisionMacro(vtkSlicerNodeSelectorWidget, "$Revision$");


//----------------------------------------------------------------------------
// Description:
// the MRMLCallback is a static function to relay modified events from the
// observed mrml scene back into the logic layer for further processing
// - this can also end up calling observers of the logic (i.e. in the GUI)
//
static void MRMLCallback(vtkObject *vtkNotUsed(caller), unsigned long eid,
                         void *__clientData, void *callData)
{
  vtkSlicerNodeSelectorWidget *self = reinterpret_cast<vtkSlicerNodeSelectorWidget *>(__clientData);
  
  if (eid == vtkMRMLScene::SceneAboutToBeImportedEvent)
    {
    self->SetIgnoreNodeAddedEvents(1);
    return;
    }
  else if (eid == vtkMRMLScene::SceneImportedEvent)
    {
    self->SetIgnoreNodeAddedEvents(0);
    }
  else if (self->GetIgnoreNodeAddedEvents())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "In vtkSlicerNodeSelectorWidget"
                            " *********Scene is loading, don't refresh menu");
#endif
    return;
    }
  
  if (self->GetInMRMLCallbackFlag())
    {
#ifdef _DEBUG
    vtkDebugWithObjectMacro(self, "In vtkSlicerNodeSelectorWidget *********MRMLCallback called recursively?");
#endif
    return;
    }

  vtkDebugWithObjectMacro(self, "In vtkSlicerWidget MRMLCallback");

  vtkMRMLNode *node = reinterpret_cast<vtkMRMLNode *>(callData);

  self->SetInMRMLCallbackFlag(1);

  if (node == NULL || self->CheckNodeClass(node))
    {
    self->UpdateMenu();
    }

  self->SetInMRMLCallbackFlag(0);
}


//----------------------------------------------------------------------------
vtkSlicerNodeSelectorWidget::vtkSlicerNodeSelectorWidget()
{
  this->NewNodeCount = 0;
  this->NewNodeEnabled = 0;
  this->DefaultEnabled = 1;
  this->NoneEnabled = 0;
  this->ShowHidden = 0;
  this->ChildClassesEnabled = 1;
  this->MRMLScene      = NULL;
  this->ContextMenuHelper      = NULL;
  this->MRMLCallbackCommand = vtkCallbackCommand::New();
  this->MRMLCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->MRMLCallbackCommand->SetCallback(MRMLCallback);
  this->InMRMLCallbackFlag = 0;
  this->IgnoreNodeAddedEvents = 0;
  this->SetBalloonHelpString("Select a node");

  this->ColorIcons = NULL;
  this->ColorIcons = vtkSlicerColorLUTIcons::New();
}

//----------------------------------------------------------------------------
vtkSlicerNodeSelectorWidget::~vtkSlicerNodeSelectorWidget()
{
  this->SetMRMLScene ( NULL );
  if (this->MRMLCallbackCommand)
    {
    this->MRMLCallbackCommand->Delete();
    this->MRMLCallbackCommand = NULL;
    }
  if (this->ContextMenuHelper)
    {
    this->ContextMenuHelper->Delete();
    }

  if ( this->ColorIcons)
    {
    this->ColorIcons->Delete();
    this->ColorIcons = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::CreateWidget()
{
  // Call the superclass
  this->Superclass::CreateWidget();
  // This has to be after CreateWidget();
  this->GetWidget()->GetWidget()->SetAdjustLabelWidthToWidgetSize(1);
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::SetMRMLScene( vtkMRMLScene *aMRMLScene)
{
  if ( this->MRMLScene )
    {
    this->MRMLScene->RemoveObserver( this->MRMLCallbackCommand );
    this->MRMLScene->Delete ( );
    this->MRMLScene = NULL;
    }

  this->MRMLScene = aMRMLScene;

  if ( this->MRMLScene )
    {
    this->MRMLScene->Register(this);
    this->MRMLScene->AddObserver( vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::NewSceneEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::SceneClosedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::SceneEditedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::SceneAboutToBeImportedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver( vtkMRMLScene::SceneImportedEvent, this->MRMLCallbackCommand );
    this->SetBinding ( "<Map>", this, "UpdateMenu");
    }

  this->UpdateMenu();
}

//----------------------------------------------------------------------------
bool vtkSlicerNodeSelectorWidget::CheckNodeClass(vtkMRMLNode *node)
{
  for (int c=0; c < this->GetNumberOfNodeClasses(); c++)
    {
    const char *className = this->GetNodeClass(c);
    if (this->GetChildClassesEnabled() && node->IsA(className))
      {
      for (int exc=0; exc < this->GetNumberOfExcludedChildClasses(); exc++)
        {
        if (!strcmp(this->GetExcludedChildClass(exc), node->GetClassName()))
          {
          return false;
          }
        }
      return true;
      }

    if (!this->GetChildClassesEnabled() && !strcmp(node->GetClassName(), className))
      {
      return true;
      }
    }
  return false;
}


//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::SetNodeClass(const char *className,
                                               const char *attName,
                                               const char *attValue,
                                               const char *nodeName)
{
  NodeClasses.clear();
  NodeNames.clear();
  AttributeNames.clear();
  AttributeValues.clear();

  const char *str = NULL;

  if (className)
    {
    NodeClasses.push_back(std::string(className));

    str = nodeName == NULL ? "" : nodeName;
    NodeNames.push_back(std::string(str));

    str = attName == NULL ? "" : attName;
    AttributeNames.push_back(std::string(str));

    str = attValue == NULL ? "" : attValue;
    AttributeValues.push_back(std::string(str));
    }

}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::AddNodeClass(const char *className,
                                               const char *attName,
                                               const char *attValue,
                                               const char *nodeName)
{
  const char *str = NULL;

  if (className)
    {
    NodeClasses.push_back(std::string(className));

    str = nodeName == NULL ? "" : nodeName;
    NodeNames.push_back(std::string(str));

    str = attName == NULL ? "" : attName;
    AttributeNames.push_back(std::string(str));

    str = attValue == NULL ? "" : attValue;
    AttributeValues.push_back(std::string(str));
    }

}
//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::ClearMenu()
{
    vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
    vtkKWMenu *m = mb->GetMenu();
    m->DeleteAllItems();
}

//----------------------------------------------------------------------------
std::string vtkSlicerNodeSelectorWidget::MakeEntryName(vtkMRMLNode *node)
{
  std::string entryName("");
  std::string nodeName("");
  if (node == NULL || node->GetName() == NULL)
    {
    return entryName;
    }
  else
    {
    nodeName = node->GetName();
    }
  entryName = nodeName;
  std::map<std::string, std::string>::iterator iter;
  int count = 1;
  for (iter = this->NodeID_to_EntryName.begin();
       iter != this->NodeID_to_EntryName.end();
       iter++)
    {
    if (iter->second == entryName)
      {
      std::stringstream ss;
      ss << nodeName << "_" << count++;
      entryName = ss.str();
      }
    }
  return entryName;
}

//----------------------------------------------------------------------------
std::string vtkSlicerNodeSelectorWidget::FindEntryName(vtkMRMLNode *node)
{
  std::string entryName("");
  if (node == NULL || node->GetName() == NULL)
    {
    return entryName;
    }

  std::map<std::string, std::string>::iterator iter = this->NodeID_to_EntryName.find(node->GetID());

  if (iter != this->NodeID_to_EntryName.end() )
    {
    entryName = iter->second;
    }
  return entryName;
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::UpdateMenu()
{

  if (this->IsCreated() == 0 || this->IsMapped() == 0)
    {
    return;
    }

  this->UnconditionalUpdateMenu();
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::UnconditionalUpdateMenu()
{
  if ( !this || !this->MRMLScene )
    {
    return;
    }

  if (this->NodeClasses.size() == 0)
    {
    return;
    }

  NodeID_to_EntryName.clear();

  vtkMRMLNode *oldSelectedNode = this->GetSelected();
  std::string oldSelectedName = this->FindEntryName(oldSelectedNode);

  this->ClearMenu();

  int c=0;

  if (this->NewNodeEnabled)
    {
    for (c=0; c < this->GetNumberOfNodeClasses(); c++)
      {
      const char *name = this->GetNodeName(c);
      if (name == NULL || !strcmp(name, "") )
        {
        name = this->MRMLScene->GetTagByClassName(this->GetNodeClass(c));
        }

      std::stringstream ss;
      const char *node_class = this->GetNodeClass(c);
      const char *tag = this->MRMLScene->GetTagByClassName(node_class);
      if (!tag)
        {
        vtkWarningMacro("Call to this->MRMLScene->GetTagByClassName(\"" <<
                        node_class << "\") failed (returned NULL)...");
        }
      else
        {
        ss << "Create New " << tag;

        // Build the command.  Since node name can contain spaces, we
        // need to quote the node name in the constructed Tcl command
        std::stringstream sc;
        sc << "ProcessNewNodeCommand " << this->GetNodeClass(c) << " \"" << name << "\"";

        this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(ss.str().c_str(), this, sc.str().c_str());
        // default action should never be to create a new node(?) - jvm
        // this->GetWidget()->GetWidget()->SetValue(ss.str().c_str());
        }
      }
    }

  if (this->NoneEnabled)
    {
    this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton("None", this, "ProcessCommand None");
    }

  vtkMRMLNode *node = NULL;
  vtkMRMLNode *selectedNode = NULL;
  std::string selectedName;
  bool selected = false;
  int resultAddAdditionalNodes=this->AddAditionalNodes();
  int columnBreakEvery = 30;
  for (c=0; c < this->GetNumberOfNodeClasses(); c++)
  {
    const char *className = this->GetNodeClass(c);
    this->MRMLScene->InitTraversal();
    while ( (node = this->MRMLScene->GetNextNodeByClass(className) ) != NULL)
      {
      if (!node->GetSelectable())
        {
        continue;
        }
      if (!this->ShowHidden && node->GetHideFromEditors())
        {
        continue;
        }

      if (!this->GetChildClassesEnabled() && strcmp(node->GetClassName(), className) != 0)
        {
        continue;
        }
      
      bool found = true;
      if (this->GetChildClassesEnabled())
        {
        for (int exc=0; exc < this->GetNumberOfExcludedChildClasses(); exc++)
          {
          if (!strcmp(this->GetExcludedChildClass(exc), node->GetClassName()))
            {
            found = false;
            break;
            }
          }
        }
      if (!found)
        {
        continue;
        }

      // If there is a Attribute Name-Value  specified, then only include nodes that
      // match both the NodeClass and Attribute
      if ((this->GetNodeAttributeName(c)== NULL  ||
          this->GetNodeAttributeValue(c)== NULL ||
          (node->GetAttribute( this->GetNodeAttributeName(c)) != NULL &&
          strcmp( node->GetAttribute( this->GetNodeAttributeName(c) ), this->GetNodeAttributeValue(c) ) == 0) )&&this->CheckAdditionalConditions(node))
        {
          std::stringstream sc;
          sc << "ProcessCommand " << node->GetID();

          std::string entryName = this->MakeEntryName(node);
          this->NodeID_to_EntryName[node->GetID()] = entryName.c_str();

          // does this node have a category?
          if (node->GetAttribute("Category") != NULL)
            {
            // is there a cascade for this category already?
            int categoryIndex = this->GetWidget()->GetWidget()->GetMenu()->GetIndexOfItem(node->GetAttribute("Category"));
            if (categoryIndex == -1)
              {
              vtkKWMenu *categoryMenu = vtkKWMenu::New();
              categoryMenu->SetParent(this->GetWidget()->GetWidget()->GetMenu());
              categoryMenu->Create();
              categoryIndex = this->GetWidget()->GetWidget()->GetMenu()->AddCascade(node->GetAttribute("Category"), categoryMenu);
              categoryMenu->AddRadioButton(entryName.c_str(), this, sc.str().c_str());
              // do we need a column break? number of items needs to be
              // decremented to match with the nodeIndex returned which is 0 based
              int count = categoryMenu->GetNumberOfItems() - 1;
              if (count != 0 && count % columnBreakEvery == 0)
                {
                categoryMenu->SetItemColumnBreak(count, 1);
                }
              categoryMenu->Delete();
              }
            else
              {
              // Adding this node to cascade with categoryIndex
              vtkKWMenu *categoryMenu = this->GetWidget()->GetWidget()->GetMenu()->GetItemCascade(categoryIndex);
              if (categoryMenu)
                {
                categoryMenu->AddRadioButton(entryName.c_str(), this, sc.str().c_str());
                // do we need a column break?
                int count = categoryMenu->GetNumberOfItems() - 1;
                if (count != 0 && count % columnBreakEvery == 0)
                  {
                  categoryMenu->SetItemColumnBreak(count, 1);
                  }
                }
              }
            }
          else
            {
            // uncategorised nodes will end up scattered amid the category
            // cascades, but that saves having to do two passes through the
            // list of nodes to get out all the nodes by category
            this->GetWidget()->GetWidget()->GetMenu()->AddRadioButton(entryName.c_str(), this, sc.str().c_str());
            // do we need a column break?
            int count = this->GetWidget()->GetWidget()->GetMenu()->GetNumberOfItems() - 1;
            if (count  != 0 && count % columnBreakEvery == 0)
              {
              this->GetWidget()->GetWidget()->GetMenu()->SetItemColumnBreak(count, 1);
              }
            }
          if (oldSelectedNode == node)
          {
            selectedNode = node;
            selected = true;
            selectedName = entryName;
          }
          //Only choose first one wenn the Additional Nodes didn't take care about this
          else if (!selected && this->DefaultEnabled
                   && !this->NoneEnabled && (resultAddAdditionalNodes==0))
          {
            selectedNode = node;
            selected = true;
            selectedName = entryName;
          }
        }
     }
  }

  //Node already selected in additonal Nodes
  if(resultAddAdditionalNodes!=0&&selectedNode==NULL)
    {
    selectedNode=this->GetSelected();
    selectedName = this->FindEntryName(selectedNode);
    }
  if (selectedNode != NULL)
    {
    this->GetWidget()->GetWidget()->SetValue(selectedName.c_str());
    this->SelectedID = std::string(selectedNode->GetID());
    }
  else
    {
    const char *name = "";
    if (this->NoneEnabled)
      {
      name = "None";
      }
    this->GetWidget()->GetWidget()->SetValue(name);
    this->SelectedID = std::string(name);
    }

  // Add Context menu for operations on selected node
  // - first create it if needed
  // - then populate it based on the currently selected node
  if ( !this->ContextMenuHelper )
    {
    this->ContextMenuHelper = vtkSlicerContextMenuHelper::New();
    this->ContextMenuHelper->SetMRMLScene( this->GetMRMLScene() );
    }

  vtkKWMenu *menu = this->GetWidget()->GetWidget()->GetMenu();
  menu->AddSeparator();
  this->ContextMenuHelper->SetMRMLNode(selectedNode);
  this->ContextMenuHelper->SetContextMenu(menu);
  this->ContextMenuHelper->PopulateMenu();
  this->ContextMenuHelper->UpdateMenuState();

  // add icons if these are color nodes
  this->AddColorIcons();
  
  if (oldSelectedNode != selectedNode)
    {
    this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
    }
  if ( oldSelectedName != selectedName )
    {
    this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeRenameEvent, NULL);
    }

}

//----------------------------------------------------------------------------
vtkMRMLNode *vtkSlicerNodeSelectorWidget::GetSelected()
{
  vtkMRMLNode *node = this->MRMLScene->GetNodeByID (this->SelectedID.c_str());
  if(node==NULL)
    {
    node=this->GetSelectedInAdditional();
    }
  return node;
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::ProcessNewNodeCommand(const char *className, const char *nodeName)
{
  vtkMRMLNode *node = NULL;
  vtkMRMLNode *retNode = NULL;

  if (className)
    {
    node = this->MRMLScene->CreateNodeByClass( className );
    if (node == NULL)
      {
      return;
      }

    // Invoke a new node event giving an observer an opportunity to
    // configure the node.
    this->InvokeEvent(vtkSlicerNodeSelectorWidget::NewNodeEvent, node);

    node->SetScene(this->MRMLScene);

    std::stringstream ss;
    const char *name;
    if (nodeName == NULL || !strcmp(nodeName,"") )
      {
      name = this->MRMLScene->GetTagByClassName(className);
      }
    else
      {
      name = nodeName;
      }
    NewNodeCount++;
    vtkDebugMacro("Node selector process new node, new node count is " << NewNodeCount);
    ss << this->MRMLScene->GetUniqueNameByString(name);
    node->SetName(ss.str().c_str());
    vtkDebugMacro("\tset the name to " << node->GetName() << endl);

    // If there is a Attribute Name-Value specified, then set that
    // attribute on the node
    for (int c=0; c < this->GetNumberOfNodeClasses(); c++)
      {
      if (!strcmp(this->GetNodeClass(c), className))
        {
        if (this->GetNodeAttributeName(c) != NULL)
          {
          node->SetAttribute(this->GetNodeAttributeName(c),
                             this->GetNodeAttributeValue(c));
          }
        break;
        }
      }

    // the ID is set in the call to AddNode
    //node->SetID(this->MRMLScene->GetUniqueIDByClass(className));
    retNode = this->MRMLScene->AddNode(node);

    // Invoke a node added event giving an observer an opportunity to
    // do something; this is *different* than just listening to the
    // scene for added nodes. For example, nodes can be added when
    // loading a scene. This event provides additional flexibility when
    // something needs to be done when a node has been added *interactively*.
    this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeAddedEvent, node);

    node->Delete();
    }

  this->SetSelected(retNode);
}


//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::ProcessCommand(char *selectedID)
{
  this->SelectedID = std::string(selectedID);

  if (this->ContextMenuHelper && selectedID)
    {
    vtkMRMLNode *node = this->MRMLScene->GetNodeByID(this->SelectedID);
    this->ContextMenuHelper->SetMRMLNode(node);
    this->ContextMenuHelper->UpdateMenuState();
    }

  this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);
}


//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::SetSelected(vtkMRMLNode *node)
{

  vtkKWMenuButton *m = this->GetWidget()->GetWidget();

  if ( node != NULL)
    {
    std::string name = this->FindEntryName(node);
    this->SelectedID = std::string(node->GetID());
    if ( !strcmp ( m->GetValue(), name.c_str() ) )
      {
      return; // no change, don't propogate events
      }
    m->SetValue(name.c_str());

    // new value, set it and notify observers
    std::string oldName = std::string(node->GetName());
    this->InvokeEvent(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL);

    // in case modules rename the node need to refresh all selectors
    // by sending NodeAddedEvent
    if (oldName != std::string(node->GetName()))
      {
      this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent, node);
      }
    }
  else
    {
    this->SelectedID = std::string("");
    m->SetValue("");
    }

  if (this->ContextMenuHelper)
    {
    this->ContextMenuHelper->SetMRMLNode(node);
    this->ContextMenuHelper->UpdateMenuState();
    }
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::SetSelectedNew(const char *className)
{
  if (this->NewNodeEnabled)
    {
    const char *name = this->MRMLScene->GetTagByClassName(className);
    std::stringstream ss;
    ss << "Create New " << name;
    this->GetWidget()->GetWidget()->SetValue(ss.str().c_str());
    }
}
//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkSlicerNodeSelectorWidget::HasNodeClass(const char *className)
{
  for (int c=0; c < this->GetNumberOfNodeClasses(); c++)
    {
    if (!strcmp(this->GetNodeClass(c), className))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
void vtkSlicerNodeSelectorWidget::AddColorIcons()
{
  if (!this->HasNodeClass("vtkMRMLColorNode") &&
      !this->HasNodeClass("vtkMRMLColorTableNode"))
    {
    return;
    }
  //---
  //--- This processes only one level of menu cascade.
  //--- assume nodes or node categories are in the main
  //--- menu and nodes are in any cascade. 
  //---
  vtkKWMenuButton *mb = this->GetWidget()->GetWidget();
  vtkKWMenu *m = mb->GetMenu();
  vtkKWMenu *c = NULL;

  std::string s;
  std::string subs;

  int num = m->GetNumberOfItems();
  //don't process commands?
  for ( int i = 0; i < num; i++)
    {
    // is item a cascade?
    c = m->GetItemCascade ( i );
    if ( c != NULL )
      {
      // if cascade, process its cascade menu items.
      int numm = c->GetNumberOfItems();
      for ( int j=0; j < numm; j++ )
        {
        subs.clear();
        if ( c->GetItemLabel(j) != NULL )
          {
          subs = c->GetItemLabel ( j );

          // insert icon in cascade menu
          // TODO: get nodeid for this menu item and pass that as well.
          c->SetItemImageToIcon ( j, this->ColorIcons->GetIconByName ( subs.c_str() ));
          c->SetItemCompoundModeToLeft ( j );
          }
        }
      continue;
      }
    // otherwise... insert icon in main menu
    // TODO: get nodeid for this menu item and pass that as well.
    s.clear();
    if ( m->GetItemLabel ( i) != NULL )
      {
      s = m->GetItemLabel( i );
      m->SetItemImageToIcon ( i, this->ColorIcons->GetIconByName ( s.c_str() ));
      m->SetItemCompoundModeToLeft ( i );
      }
    }  
}
