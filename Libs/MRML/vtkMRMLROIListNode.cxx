#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLROIListNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLROIListNode* vtkMRMLROIListNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLROIListNode");
  if(ret)
    {
    return (vtkMRMLROIListNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLROIListNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLROIListNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLROIListNode");
  if(ret)
    {
    return (vtkMRMLROIListNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLROIListNode;
}


//----------------------------------------------------------------------------
vtkMRMLROIListNode::vtkMRMLROIListNode()
{
  this->ROIList = vtkCollection::New();
  this->Indent = 1;
  this->TextScale = 4.5;
  this->Visibility = 1;
  this->Color[0]=0.4; this->Color[1]=1.0; this->Color[2]=1.0;
  // from slicer 2: 1.0 0.5 0.5"
  this->SelectedColor[0]=1.0; this->SelectedColor[1]=0.5; this->SelectedColor[2]=0.5;
  this->Name = NULL;
  this->SetName("");

  this->Opacity = 1.0;
  this->Ambient = 0;
  this->Diffuse = 1.0;
  this->Specular = 0;
  this->Power = 1;
  this->HideFromEditors = 0;
  this->VolumeNodeID = NULL;
  return;
}

//----------------------------------------------------------------------------
vtkMRMLROIListNode::~vtkMRMLROIListNode()
{
  if (this->ROIList)
    {    
    this->ROIList->RemoveAllItems();        
    this->ROIList->Delete();
    this->ROIList = NULL;
    }
  if (this->Name)
    {
    delete [] this->Name;
    this->Name = NULL;
    }
  if (this->VolumeNodeID)
    {
    delete [] this->VolumeNodeID;
    this->VolumeNodeID = NULL;
    }
  return;
}

//----------------------------------------------------------------------------
void vtkMRMLROIListNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  of << indent <<" TextScale=\"" << this->TextScale << "\"";
  of << indent <<" Visibility=\"" << this->Visibility << "\"";

  of << " color=\"" << this->Color[0] << " " << 
    this->Color[1] << " " <<
    this->Color[2] << "\"";

  of << " selectedcolor=\"" << this->SelectedColor[0] << " " << 
    this->SelectedColor[1] << " " <<
    this->SelectedColor[2] << "\"";

  of << " ambient=\"" << this->Ambient << "\"";
  of << " diffuse=\"" << this->Diffuse << "\"";
  of << " specular=\"" << this->Specular << "\"";
  of << " power=\"" << this->Power << "\"";
  of << " opacity=\"" << this->Opacity << "\"";
  
  if (this->VolumeNodeID != NULL)
    {
    of << " VolumeNodeID=\"" << this->VolumeNodeID << "\"";
    }

  if (this->GetNumberOfROIs() > 0)
    {
    of << " ROI=\"";
    for (int idx = 0; idx < this->GetNumberOfROIs(); idx++)
      {
      if (this->GetNthROINode(idx) != NULL)
        {
        of << "\n";
        this->GetNthROINode(idx)->WriteXML(of, nIndent);
        }
      }
    of << "\"";
    }
  return;
}

//----------------------------------------------------------------------------
void vtkMRMLROIListNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "TextScale")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TextScale;
      }
    else if (!strcmp(attName, "Visibility")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Visibility;
      }
    else if (!strcmp(attName, "color")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Color[0];
      ss >> this->Color[1];
      ss >> this->Color[2];
      }
    else if (!strcmp(attName, "selectedcolor"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SelectedColor[0];
      ss >> this->SelectedColor[1];
      ss >> this->SelectedColor[2];
      }
    else if (!strcmp(attName, "ambient")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Ambient;
      }
    else if (!strcmp(attName, "diffuse")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Diffuse;
      }
    else if (!strcmp(attName, "specular")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Specular;
      }
    else if (!strcmp(attName, "power")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Power;
      }
    else if (!strcmp(attName, "opacity")) 
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Opacity;
      }
    else if (!strcmp(attName, "VolumeNodeID")) 
      {
      std::stringstream ss;
      ss << attValue;
      this->SetVolumeNodeID(attValue);
      }
    else if (!strcmp(attName, "ROI"))
      {
      vtkDebugMacro("ReadXMLAttributes: found a ROI list: " << (char*)attValue << endl);
      // need to add ROI and parse out the list of ROI points
      // assume ID is first, extract that part of the attValue
      char *ROI = (char *)attValue;
      char *IDPtr;
      IDPtr = strstr (ROI,"ROINodeID ");
      vtkDebugMacro( "ReadXMLAttributes: Starting to parse out the ROI list, setting it up for tokenisation\n");
      while (IDPtr != NULL)
        {
        vtkDebugMacro( "current ID pt = " << IDPtr << endl);

        // find the end of this point, new line or end quote
        IDPtr = strstr (IDPtr," ROINodeID");
        if (IDPtr != NULL)
          {
          // replace the space with a carriage return
          IDPtr = strncpy(IDPtr, "\nROINodeID", 1);
          }
        }
      // now parse the string into tokens by the newline
      IDPtr = strtok(ROI, "\n");
      vtkDebugMacro( "\nGetting tokens from the list, to make new points.\n");

      while (IDPtr != NULL)
        {
        vtkDebugMacro( "got a token, adding a ROI for: " << IDPtr << endl);
        // now make a new point
        int ROIIndex = this->AddROI();
        vtkDebugMacro( "new ROI index = " << ROIIndex << endl);
        vtkMRMLROINode *newROINode = this->GetNthROINode(ROIIndex);

        if (newROINode != NULL)
          {
          // now pass it the stuff to parse out and set itself from
          vtkDebugMacro( "ReadXMLAttributes: passing the text pointer for ROI index " << ROIIndex <<  " to the new ROINode: " << IDPtr << endl);
          newROINode->ReadXMLString(IDPtr);
          } else {
            vtkErrorMacro ("ERROR making a new MRML ROINode!\n");
          }
        newROINode = NULL;
        IDPtr = strtok(NULL, "\n");
        }
      }

    else
      {
      vtkDebugMacro("ReadXMLAttributes: Unknown attribute name " << attName);
      }
    }
  vtkDebugMacro("Finished reading in xml attributes, list id = " << this->GetID() << " and name = " << this->GetName() << endl);
  return;
}

//----------------------------------------------------------------------------
void vtkMRMLROIListNode::Copy(vtkMRMLNode *anode)
{
  //Fill in
  return;
}

//----------------------------------------------------------------------------
void vtkMRMLROIListNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //int idx
  Superclass::PrintSelf(os,indent);
  //Fill in
}

//-----------------------------------------------------------
void vtkMRMLROIListNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
  return;
}

//-----------------------------------------------------------
void vtkMRMLROIListNode::UpdateReferences()
{
  Superclass::UpdateReferences();
  return;
}

//----------------------------------------------------------------------------
int vtkMRMLROIListNode::GetNumberOfROIs()
{
    return this->ROIList->vtkCollection::GetNumberOfItems();
}

//----------------------------------------------------------------------------
int vtkMRMLROIListNode::AddROI()
{
  if ( !this->Scene ) 
    {
    vtkErrorMacro ( << "Attempt to add ROI, but no scene set yet");
    return (-1);
    }

  // create a vtkMRMLROI and return the ROI number for later
  // access
  vtkMRMLROINode * ROINode = vtkMRMLROINode::New();

  // give the point a unique name based on the list name
  std::stringstream ss;
  ss << this->GetName();
  ss << "-P";
  std::string nameString;
  ss >> nameString;
  ROINode->SetID(this->GetScene()->GetUniqueNameByString(nameString.c_str()));
  // use the same for the label text for now
  ROINode->SetLabelText(ROINode->GetID());

  // add it to the collection
  this->ROIList->vtkCollection::AddItem(ROINode);
  int itemIndex = this->ROIList->vtkCollection::IsItemPresent(ROINode);
  // decrement the index, because GetNthROI needs a 0 based array
  // index, IsItemPresent returns a 1 based array index
  itemIndex--;

  // then delete it, the collection has registered it and will keep track of
  // it
  ROINode->Delete();
  ROINode = NULL;

  // this list is now modified...
  this->Modified();

  // return an index for use in getting the item again via GetNthROI
  vtkDebugMacro("AddROI: added a ROI to the list at index " << itemIndex << endl);
  return itemIndex;
}

//----------------------------------------------------------------------------
void vtkMRMLROIListNode::RemoveROI(int i)
{
  this->ROIList->vtkCollection::RemoveItem(i);
  this->InvokeEvent(vtkMRMLScene::NodeRemovedEvent, NULL);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLROIListNode::RemoveAllROIs()
{
  int numPoints = this->GetNumberOfROIs();
  for (int r = numPoints - 1; r >= 0; r--)
    {
    // as remove them from the end of the list, the size of the list
    // will shrink as the iterator f reduces
    vtkMRMLROIListNode *ROINode = vtkMRMLROIListNode::SafeDownCast(this->ROIList->vtkCollection::GetItemAsObject(r));
    this->ROIList->vtkCollection::RemoveItem(r);
    if (ROINode != NULL)
      {
      ROINode = NULL;
      }
    this->InvokeEvent(vtkMRMLScene::NodeRemovedEvent, NULL);
    }
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLROIListNode::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, 
                                            void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkMRMLROIListNode::SetColor(double r, double g, double b)
{
  if (this->Color[0] == r &&
    this->Color[1] == g &&
    this->Color[2] == b)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Color to " << r << " " << g << " " << b); 
  this->Color[0] = r;
  this->Color[1] = g;
  this->Color[2] = b;

  // invoke a modified event
  this->Modified();

  // invoke a display modified event
  this->InvokeEvent(vtkMRMLROIListNode::DisplayModifiedEvent);
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLROIListNode::SetSelectedColor(double r, double g, double b)
{
  if (this->SelectedColor[0] == r &&
    this->SelectedColor[1] == g &&
    this->SelectedColor[2] == b)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting SelectedColor to " << r << " " << g << " " << b); 
  this->SelectedColor[0] = r;
  this->SelectedColor[1] = g;
  this->SelectedColor[2] = b;

  // invoke a modified event
  this->Modified();

  // invoke a display modified event
  this->InvokeEvent(vtkMRMLROIListNode::DisplayModifiedEvent);
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLROIListNode::SetColor(double c[3])
{
  // set the colour
  this->SetColor(c[0], c[1], c[2]);
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLROIListNode::SetSelectedColor(double c[3])
{
  // set the selected colour
  this->SetSelectedColor(c[0], c[1], c[2]);
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLROIListNode::SetTextScale(double scale)
{
  if (this->TextScale == scale)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting TextScale to " << scale);
  this->TextScale = scale;

  // invoke a modified event
  this->Modified();

  // invoke a display modified event
  this->InvokeEvent(vtkMRMLROIListNode::DisplayModifiedEvent);
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLROIListNode::SetVisibility(int visible)
{
  if (this->Visibility == visible)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Visibility to " << visible);
  this->Visibility = visible;

  // invoke a modified event
  this->Modified();

  // invoke a display modified event
  this->InvokeEvent(vtkMRMLROIListNode::DisplayModifiedEvent);
}

//---------------------------------------------------------------------------
void vtkMRMLROIListNode::SetOpacity(double opacity)
{
  if (this->Opacity == opacity)
    {
    return;
    }
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Opacity to " << opacity);
  this->Opacity = opacity;

  // invoke a modified event
  this->Modified();

  // invoke a display modified event
  this->InvokeEvent(vtkMRMLROIListNode::DisplayModifiedEvent);
}

//----------------------------------------------------------------------------
int vtkMRMLROIListNode::SetNthROIXYZ(int n, float x, float y, float z)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node == NULL)
    {
    vtkErrorMacro("Unable to get ROI number " << n);
    return 1;
    }
  node->SetXYZ(x,y,z);

  //Update IJK
  if (this->VolumeNodeID != NULL)
    {
    vtkMRMLVolumeNode *VolumeNode  = vtkMRMLVolumeNode::SafeDownCast(this->Scene->GetNodeByID(this->VolumeNodeID));
    if (VolumeNode)
      {
      double rasPoint[4] = { x, y, z, 1.0 };
      double ijkPoint[4];
      vtkMatrix4x4* rasToijk = vtkMatrix4x4::New();
      VolumeNode->GetRASToIJKMatrix(rasToijk);
      rasToijk->MultiplyPoint(rasPoint, ijkPoint);
      rasToijk->Delete();
      
      int* dims = new int[3];
      VolumeNode->GetImageData()->GetDimensions(dims);
      ijkPoint[0] = ijkPoint[0] >= 0 ? ijkPoint[0] : 0;
      ijkPoint[0] = ijkPoint[0] < dims[0] ? ijkPoint[0] : dims[0];
      ijkPoint[1] = ijkPoint[1] >= 0 ? ijkPoint[1] : 0;
      ijkPoint[1] = ijkPoint[1] < dims[1] ? ijkPoint[1] : dims[1];
      ijkPoint[2] = ijkPoint[2] >= 0 ? ijkPoint[2] : 0;
      ijkPoint[2] = ijkPoint[2] < dims[2] ? ijkPoint[2] : dims[2];
      delete [] dims;
      
      node->SetIJK(ijkPoint[0], ijkPoint[1], ijkPoint[2]);
      }
    else
      {
      vtkDebugMacro("No volume selected ...\n");
      }
    }


  // the list contents have been modified
  node = NULL;
  this->InvokeEvent(vtkMRMLROIListNode::ROIModifiedEvent, NULL);
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLROIListNode::SetNthROIIJK(int n, float i, float j, float k)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node == NULL)
    {
    vtkErrorMacro("Unable to get ROI number " << n);
    return 1;
    }
  if (this->VolumeNodeID != NULL)
    {
    vtkMRMLVolumeNode *VolumeNode  = vtkMRMLVolumeNode::SafeDownCast(this->Scene->GetNodeByID(this->VolumeNodeID));
    if (VolumeNode)
      {

      int* dims = new int[3];
      VolumeNode->GetImageData()->GetDimensions(dims);
      i = i >= 0 ? i : 0;
      i = i < dims[0] ? i : dims[0];
      j = j >= 0 ? j : 0;
      j = j < dims[1] ? j : dims[1];
      k = k >= 0 ? k : 0;
      k = k < dims[2] ? k : dims[2];
      delete [] dims;

      node->SetIJK(i,j,k);

      //Update XYZ
      double rasPoint[4]; 
      double ijkPoint[4]= { i, j, k, 1.0 };
      vtkMatrix4x4* ijkToras = vtkMatrix4x4::New();
      VolumeNode->GetIJKToRASMatrix(ijkToras);
      ijkToras->MultiplyPoint(ijkPoint,rasPoint);
      ijkToras->Delete();
      node->SetXYZ(rasPoint[0], rasPoint[1], rasPoint[2]);
      }
    else
      {
      vtkDebugMacro("No volume selected ...\n");
      }
    }

  // the list contents have been modified
  node = NULL;
  this->InvokeEvent(vtkMRMLROIListNode::ROIModifiedEvent, NULL);
  return 0;
}

//----------------------------------------------------------------------------
float * vtkMRMLROIListNode::GetNthROIXYZ(int n)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node != NULL)
    {
    float * xyz = node->GetXYZ();
    node = NULL;
    return xyz;
    }
  else
    {
    return NULL;
    }
}

//----------------------------------------------------------------------------
float * vtkMRMLROIListNode::GetNthROIIJK(int n)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node != NULL)
    {
    float * ijk = node->GetIJK();
    node = NULL;
    return ijk;
    }
  else
    {
    return NULL;
    }
}

//----------------------------------------------------------------------------
int vtkMRMLROIListNode::SetNthROIRadiusXYZ(int n, float Radiusx, float Radiusy, float Radiusz)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node == NULL)
    {
    vtkErrorMacro("Unable to get ROI number " << n);
    return 1;
    }
  node->SetRadiusXYZ(Radiusx,Radiusy,Radiusz);

   //Update RadiusIJK
  if (this->VolumeNodeID != NULL)
    {
    vtkMRMLVolumeNode *VolumeNode  = vtkMRMLVolumeNode::SafeDownCast(this->Scene->GetNodeByID(this->VolumeNodeID));
    if (VolumeNode)
      {
      double* spacing = VolumeNode->GetSpacing();
      node->SetRadiusIJK(Radiusx / spacing[0], Radiusy / spacing[1], Radiusz / spacing[2]);
      }
    else
      {
      vtkDebugMacro("No volume selected ...\n");
      }
    }

  // the list contents have been modified
  node = NULL;
  this->InvokeEvent(vtkMRMLROIListNode::ROIModifiedEvent, NULL);
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLROIListNode::SetNthROIRadiusIJK(int n, float Radiusi, float Radiusj, float Radiusk)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node == NULL)
    {
    vtkErrorMacro("Unable to get ROI number " << n);
    return 1;
    }
  node->SetRadiusIJK(Radiusi,Radiusj,Radiusk);

   //Update RadiusXYZ
  if (this->VolumeNodeID != NULL)
    {
    vtkMRMLVolumeNode *VolumeNode  = vtkMRMLVolumeNode::SafeDownCast(this->Scene->GetNodeByID(this->VolumeNodeID));
    if (VolumeNode)
      {
      double* spacing = VolumeNode->GetSpacing();
      node->SetRadiusXYZ(Radiusi * spacing[0], Radiusj * spacing[1], Radiusk * spacing[2]);
      }
    else
      {
      vtkDebugMacro("No volume selected ...\n");
      }
    }


  // the list contents have been modified
  node = NULL;
  this->InvokeEvent(vtkMRMLROIListNode::ROIModifiedEvent, NULL);
  return 0;
}

//----------------------------------------------------------------------------
float * vtkMRMLROIListNode::GetNthROIRadiusXYZ(int n)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node != NULL)
    {
    float * Radiusxyz = node->GetRadiusXYZ();
    node = NULL;
    return Radiusxyz;
    }
  else
    {
    return NULL;
    }
}

//----------------------------------------------------------------------------
float * vtkMRMLROIListNode::GetNthROIRadiusIJK(int n)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node != NULL)
    {
    float * Radiusijk = node->GetRadiusIJK();
    node = NULL;
    return Radiusijk;
    }
  else
    {
    return NULL;
    }
}

//----------------------------------------------------------------------------
int vtkMRMLROIListNode::SetNthROILabelText(int n, const char *text)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node == NULL)
    {
    vtkErrorMacro("Unable to get ROI number " << n);
    return 1;
    }
  node->SetLabelText(text);
  node = NULL;
  // the list contents have been modified
  this->InvokeEvent(vtkMRMLROIListNode::ROIModifiedEvent, NULL);
  return 0;
}

//----------------------------------------------------------------------------
const char *vtkMRMLROIListNode::GetNthROILabelText(int n)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node != NULL)
    {
    const char *txt = node->GetLabelText();
    node = NULL;
    return txt;
    }
  else
    {
    return "(none)";
    }
}

//----------------------------------------------------------------------------
int vtkMRMLROIListNode::SetNthROISelected(int n, int flag)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node == NULL)
    {
    vtkErrorMacro("Unable to get ROI number " << n);
    return 1;
    }
  node->SetSelected((flag == 0 ? false : true));
  node = NULL;
  // the list contents have been modified
  this->InvokeEvent(vtkMRMLROIListNode::ROIModifiedEvent, NULL);
  return 0;
}

//----------------------------------------------------------------------------
int vtkMRMLROIListNode::GetNthROISelected(int n)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node != NULL)
    {
    int selected = node->GetSelected();
    node = NULL;
    return (selected ? 1 : 0);
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
int vtkMRMLROIListNode::SetNthROIID(int n, const char *id)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node == NULL)
    {
    vtkErrorMacro("Unable to get ROI number " << n);
    return 1;
    }
  node->SetID(id);
  node = NULL;
  // the list contents have been modified
  this->InvokeEvent(vtkMRMLROIListNode::ROIModifiedEvent, NULL);
  return 0;
}

//----------------------------------------------------------------------------
const char *vtkMRMLROIListNode::GetNthROIID(int n)
{
  vtkMRMLROINode *node = this->GetNthROINode(n);
  if (node != NULL)
    {
    const char *id = node->GetID();
    node = NULL;
    return id;
    }
  else
    {
    return "(none)";
    }
}

//----------------------------------------------------------------------------
vtkMRMLROINode* vtkMRMLROIListNode::GetNthROINode(int n)
{
  vtkDebugMacro("GetNthROINode: getting item by index number: " << n);
  if (this->ROIList == NULL)
    {
    vtkErrorMacro ("GetNthROINode: ERROR: ROI list is null\n");
    return NULL;
    }
  if (n < 0 || n >= this->ROIList->GetNumberOfItems()) 
    {
    vtkErrorMacro ("vtkMRMLROIListNode::GetNthROI: index out of bounds, " << n << " is less than zero or more than the number of items: " << this->ROIList->GetNumberOfItems() << endl);
    return NULL;
    }
  else 
    {
    return (vtkMRMLROINode*)this->ROIList->GetItemAsObject(n);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLROIListNode::SetAllVolumeNodeID()
{
  int numROIs = this->GetNumberOfROIs();
  for (int n = 0; n < numROIs; ++n)
    {
    // Set the Volume ID for each ROI
     vtkMRMLROINode *node = this->GetNthROINode(n);
     node->SetVolumeNodeID(this->VolumeNodeID);
    }
  // the list contents have been modified
  this->InvokeEvent(vtkMRMLROIListNode::ROIModifiedEvent, NULL);
  this->Modified();
  return;
}

//----------------------------------------------------------------------------
void vtkMRMLROIListNode::UpdateIJK()  
{
  int numROIs = this->GetNumberOfROIs();
  vtkMRMLVolumeNode *VolumeNode  = vtkMRMLVolumeNode::SafeDownCast(this->Scene->GetNodeByID(this->VolumeNodeID));

  if(VolumeNode != NULL)
    {
    for (int n = 0; n < numROIs; ++n)
      {
      float *xyz = this->GetNthROIXYZ(n);
      this->SetNthROIXYZ(n, xyz[0], xyz[1], xyz[2]);
      float *Radiusxzy = this->GetNthROIRadiusXYZ(n);
      this->SetNthROIRadiusXYZ(n, Radiusxzy[0], Radiusxzy[1], Radiusxzy[2]);
      }
    }
  return;
}
