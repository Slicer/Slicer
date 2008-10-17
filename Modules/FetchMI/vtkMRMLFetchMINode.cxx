#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLFetchMINode.h"
#include "vtkMRMLScene.h"
#include "vtkHIDTagTable.h"
#include "vtkXNDTagTable.h"

//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkMRMLFetchMINode, "$Revision: 1.0 $");

//------------------------------------------------------------------------------
vtkMRMLFetchMINode* vtkMRMLFetchMINode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFetchMINode");
  if(ret)
    {
      return (vtkMRMLFetchMINode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFetchMINode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLFetchMINode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLFetchMINode");
  if(ret)
    {
      return (vtkMRMLFetchMINode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLFetchMINode;
}

//----------------------------------------------------------------------------
vtkMRMLFetchMINode::vtkMRMLFetchMINode()
{
  this->SingletonTag = "vtkMRMLFetchMINode";
   this->HideFromEditors = true;

   this->TagTableCollection = vtkTagTableCollection::New();
   this->SetKnownServers();

   this->SetRequiredTags();

   this->ErrorMessage = NULL;
   this->SelectedServer = NULL; 
   this->SelectedServiceType = NULL; 
   
   this->ResourceDescription = vtkTagTable::New();
   this->SlicerDataTypes = vtkStringArray::New();
   this->SlicerDataTypes->InsertValue ( 0, "MRML");
   this->SlicerDataTypes->InsertValue ( 1, "ScalarVolume");
   this->SlicerDataTypes->InsertValue ( 2, "LabelMap");
   this->SlicerDataTypes->InsertValue ( 3, "VTKModel");
   this->SlicerDataTypes->InsertValue ( 4, "FreeSurferModel");
   this->SlicerDataTypes->InsertValue ( 5, "DTIVolume");
   this->SlicerDataTypes->InsertValue ( 6, "DWIVolume");
   this->SlicerDataTypes->InsertValue ( 7, "UnstructuredGrid");

   //--- initialize tag table with default tags for
   //--- Slicer-friendly services
   //--- fBIRN HID web services
   vtkHIDTagTable *hid_tt = vtkHIDTagTable::New();
   hid_tt->Initialize();
   this->TagTableCollection->AddTableByName ( hid_tt, "HIDTags" );
   hid_tt->Delete();
   //--- XNAT Desktop web services
   vtkXNDTagTable *xnd_tt = vtkXNDTagTable::New();
   xnd_tt->Initialize();
   this->TagTableCollection->AddTableByName (xnd_tt, "XNDTags" );
   xnd_tt->Delete();
}


//----------------------------------------------------------------------------
vtkMRMLFetchMINode::~vtkMRMLFetchMINode()
{
  
  if ( this->SlicerDataTypes)
    {
    this->SlicerDataTypes->Delete();
    this->SlicerDataTypes = NULL;
    }
  if ( this->TagTableCollection != NULL )
    {
    this->TagTableCollection->RemoveAllItems();
    this->TagTableCollection->Delete();
    this->TagTableCollection = NULL;
    }
  if ( this->ResourceDescription )
    {
    this->ResourceDescription->Delete();
    this->ResourceDescription = NULL;
    }
  if ( this->SelectedServer != NULL )
    {
    delete [] this->SelectedServer;
    this->SelectedServer = NULL;
    }
  if ( this->SelectedServiceType != NULL )
    {
    delete [] this->SelectedServiceType;
    this->SelectedServiceType = NULL;
    }
  if ( this->ErrorMessage != NULL )
    {
    delete [] this->ErrorMessage;
    this->ErrorMessage = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);


}


//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLFetchMINode::ReadXMLAttributes(const char** atts)" << endl;
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "FETCHMI"))
      {
      }
    }
}






//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::SetRequiredTags ( )
{
  //--- for xnat desktop
  this->AddRequiredXNDTag ( "Experiment" );
  this->AddRequiredXNDTag ( "Project" );
  this->AddRequiredXNDTag ( "Scan" );
  this->AddRequiredXNDTag ( "Subject" );  
  this->AddRequiredXNDTag ( "Modality" );
  this->AddRequiredXNDTag ( "SlicerDataType" );
}





//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::SetKnownServers ( )
{
   //--- add all known servers
  this->AddNewServer ( "http://bobby.bwh.harvard.edu:8000" );
  this->AddNewServer ( "http://localhost:8081" );
  this->AddNewServer ( "https://loci.ucsd.edu/hid" );
  this->InvokeEvent (vtkMRMLFetchMINode::KnownServersModifiedEvent );
}



//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::SetServer ( const char *s)
{
  if (s == NULL)
    {
    vtkErrorMacro("SetServer: can't select a null server.");
    return;
    }
  this->SetSelectedServer (s);
  if ( !(strcmp(s, "https://loci.ucsd.edu/hid" ) ))
    {
    this->SetServiceType ( "HID" );
    }
  else
    {
    //--- for now assume it's xnd if not hid.
    this->SetServiceType ( "XND" );
    }

  this->InvokeEvent ( vtkMRMLFetchMINode::SelectedServerModifiedEvent );
}


//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::SetServiceType ( const char *s)
{
  this->SetSelectedServiceType (s);
}


//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::AddRequiredXNDTag ( const char *tag )
{
  int unique = 1;
  std::string s;

  if (tag == NULL)
    {
    vtkErrorMacro("AddRequiredXNDTag: can't add  a null tag!");
    return;
    }
  
  int n = this->RequiredXNDTags.size();
  for ( int i = 0; i < n; i++ )
    {
    s = RequiredXNDTags[i];
    if ( !(strcmp ( s.c_str(), tag)) )
      {
      unique = 0;
      break;
      }
    }
    //--- add the server tag if it's not already here.
    if ( unique )
      {
      this->RequiredXNDTags.push_back ( std::string ( tag ) );
      }

}



//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::AddNewServer ( const char *name )
{

  int unique = 1;
  std::string s;

  if (name == NULL)
    {
    vtkErrorMacro("AddNewServer: can't add a null server name.");
    return;
    }
  
  int n = this->KnownServers.size();
  for ( int i = 0; i < n; i++ )
    {
    s = KnownServers[i];
    if ( !(strcmp ( s.c_str(), name)) )
      {
      unique = 0;
      break;
      }
    }
    //--- add the server name if it's not already here.
    if ( unique )
      {
      this->KnownServers.push_back ( std::string ( name ) );
      this->InvokeEvent ( vtkMRMLFetchMINode::KnownServersModifiedEvent );
      }
}



//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::RaiseErrorEvent ()
{
  this->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
}



//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::RaiseErrorChoice ()
{
  this->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorChoiceEvent );
}




//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLFetchMINode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLFetchMINode *node = (vtkMRMLFetchMINode *) anode;

  this->SetSelectedServer(node->GetSelectedServer());
  this->SetSelectedServiceType(node->GetSelectedServiceType());

  // TODO: loop through known servers
}

//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "TagTableCollection: " << this->GetTagTableCollection() << "\n";
  int n = this->KnownServers.size();
  for ( int i = 0; i < n; i++ )
    {
    os << indent << "KnownServers[" << i << "] = " << this->KnownServers[i] << "\n";
    }
  os << indent << "SelectedServer: " << (this->SelectedServer == NULL ? "null" :  this->SelectedServer) << "\n";
  os << indent << "SelectedServiceType: " << (this->SelectedServiceType == NULL ? "null" : this->SelectedServiceType) << "\n";
  os << indent << "ErrorMessage: " << this->GetErrorMessage() << "\n";
}



