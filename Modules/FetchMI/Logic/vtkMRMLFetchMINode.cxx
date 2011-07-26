
#include "vtkObjectFactory.h"
#include "vtkMRMLFetchMINode.h"
#include "vtkHIDTagTable.h"
#include "vtkXNDTagTable.h"



//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkMRMLFetchMINode, "$Revision: 1.0 $");

//----------------------------------------------------------------------------
//--- a word about language:
//--- Methods and vars in this module assume that:
//--- "Tag" means a metadata element comprised of an "attribute" (or "keyword") and "value".
//--- Tags may have an attribute with many possible values.
//--- Sometimes "Tag" is used to mean "attribute".
//--- we'll change this eventually to be "Tagname"
//----------------------------------------------------------------------------


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
   this->SetSingletonTag("vtkMRMLFetchMINode");
   this->HideFromEditors = true;

   this->TagTableCollection = vtkTagTableCollection::New();

   this->ErrorMessage = NULL;
   this->SelectedServer = NULL; 
   this->SelectedServiceType = NULL;
   this->SelectedTagTable = NULL;

   //--- This contains list of uris and their slice data types.
   //--- NOTE TO DEVELOPERS: add new node types here
   //--- as appropriate.
   this->ResourceDescription = vtkTagTable::New();
   this->ResourceDescription->SetName ( "ResourceDescription");


   
}


//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::AddTagTablesForWebServices ( )
{

  //---
  //--- DEVELOPERS NOTE: extend here as new web services are added.
  //---
  if ( this->GetTagTableCollection() == NULL )
    {
     vtkErrorMacro ( "AddTagTablesForWebServices: Got a null TagTableCollection in FetchMINode." );
     std::string msg = "Unable to create tag tables for Web Services.";
     this->SetErrorMessage (msg.c_str() );
     this->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
     return;
    }

   //--- fBIRN HID web services
  if ( ! this->GetTagTableCollection()->FindTagTableByName ("HID") )
    {
    vtkHIDTagTable *hid_tt = vtkHIDTagTable::New();
    hid_tt->Initialize();
    this->TagTableCollection->AddTableByName ( hid_tt, "HID" );
    hid_tt->Delete();
    }
   
  //--- XNAT Desktop web services
  //--- here we have one table for all instances of xnat desktop
  //--- webservices so that users need not re-define tags to mark up
  //--- data for different remost hosts; just use the same table as a
  //--- metadata dictionary for all.
  if ( ! this->GetTagTableCollection()->FindTagTableByName ("XND") )
    {
    vtkXNDTagTable *xnd_tt = vtkXNDTagTable::New();
    xnd_tt->Initialize();
    this->TagTableCollection->AddTableByName (xnd_tt, "XND" );
    xnd_tt->Delete();
    }
}


//----------------------------------------------------------------------------
vtkMRMLFetchMINode::~vtkMRMLFetchMINode()
{
  
  if ( this->SelectedTagTable )
    {
    this->SetSelectedTagTable ( NULL );
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
void vtkMRMLFetchMINode::WriteXML(ostream& vtkNotUsed(of), int vtkNotUsed(nIndent))
{
/*
  Superclass::WriteXML(of, nIndent);

  // Write all MRML node attributes into output stream
  vtkIndent indent(nIndent);

  if ( this->ErrorMessage != NULL )
    {
    of << " errorMessage=\"" << this->GetErrorMessage() << "\"";
    }
  if ( this->SelectedServer != NULL )
    {
    of << " selectedServer=\"" << this->GetSelectedServer() << "\"";     
    }
  if ( this->SelectedServiceType != NULL )
    {
    of << " selectedServiceType=\"" << this->GetSelectedServiceType() << "\"";     
    }
  if ( this->TagTableCollection != NULL )
    {
    int m;
    int n = this->TagTableCollection->GetNumberOfItems();
    for ( int i=0; i < n; i++ )
      {

      vtkTagTable *t = vtkTagTable::SafeDownCast (this->TagTableCollection->GetItemAsObject ( i));
      if ( t != NULL )
        {
        of << " tagTableName=\"" << this->t->GetName() << "\"";
        m = t->GetNumberOfTags();
        of << " numberOfTags=\"" << m << "\"";
        of << " tags= \"";
        for ( j=0; j < m; j++ )
          {
          // attribute, value, selection state.
          of << t->GetAttributeForTag ( j ) << " " <<  t->GetValueForTag (j ) << " " << t->IsTagSelected ( t->GetAttributeForTag ( j)) << " ";
          }
        of << "\"";        
        }
      }
      }

  if ( this->ResourceDescription != NULL )
    {
    of << " tagTableName=\"" << this->t->GetName() << "\"";
    m = t->GetNumberOfTags();
    of << " numberOfTags=\"" << m << "\"";
    of << " tags= \"";
    for ( j=0; j < m; j++ )
      {
      of << " attribute=\"" << t->GetAttributeForTag ( j ) << "\"";
      of << " value=\"" << t->GetValueForTag (j ) << "\"";
      os << " selected=\"" << t->IsTagSelected ( t->GetAttributeForTag ( j)) << "\"";
      }
    }
*/
}


//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::ReadXMLAttributes(const char** vtkNotUsed(atts))
{
/*
  // cout << "vtkMRMLFetchMINode::ReadXMLAttributes(const char** atts)" << endl;
  vtkMRMLNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "errorMessage"))
      {
      std::stringstream ss;
      std::string s;
      ss << attValue;
      ss >> s;
      vtkDebugMacro ( "Setting the errormessage to " << s.c_str() << "\n");
      this->SetErrorMessage ( s.c_str() );
      }
    else if (!strcmp(attName, "selectedServer"))
      {
      std::stringstream ss;
      std::string s;
      ss << attValue;
      ss >> s;
      vtkDebugMacro ( "Setting the selectedServer to " << s.c_str() << "\n");
      this->SetSelectedServer ( s.c_str() );
      }
    else if (!strcmp(attName, "selectedServiceType"))
      {
      std::stringstream ss;
      std::string s;
      ss << attValue;
      ss >> s;
      vtkDebugMacro ( "Setting the selectedServiceType to " << s.c_str() << "\n");
      this->SetSelectedServiceType ( s.c_str() );
      }
    else if (!strcmp(attName, "tagTableName"))
      {
      std::stringstream ss;
      std::string s;
      std::string att;
      std::string val;
      int selected;
      if (!strcmp(attValue, "ResourceDescription" ))
        {
        if ( this->ResourceDescription )
          {
          ss << attValue;
          ss >> s;
          vtkDebugMacro ( "Setting tagTableName to " << s.c_str() << "\n");
          this->ResourceDescription->SetName ( s.c_str() );
          //--- serialize the tags
          }
        }
      else if (!strcmp(attValue, "" ))
        {
        //--- add to tag table collection
        ss << attValue;
        ss >> s;
        vtkDebugMacro ( "Setting tagTableName to " << s.c_str() << "\n");
        this->ResourceDescription->SetName ( s.c_str() );
        if ( this->GetTagTableCollection() )
          {
          int n = this->GetTagTableCollection()->GetNumberOfItems();
          for ( int i=0; i < n; i++ )
            {
            vtkTagTable *t = vtkTagTable::SafeDownCast( this->GetTagTableCollection()->GetItemAsObject (i));
            if (t != NULL )
              {
              if ( !strcmp (t->GetName(), s.c_str() ))
                {
                //--- serialize the tags into this table.

                }
              }
            }
          }
        }
      }
    }
*/
}






//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::SetServer ( const char *s)
{
  if (s == NULL)
    {
    vtkErrorMacro("SetServer: can't select a null server.");
    this->SetSelectedServer (NULL);
    return;
    }

  //---
  //--- DEVELOPERS NOTE: extend here as new web services are added.
  //---

  this->SetSelectedServer (s);

  if ( !(strcmp(s, "https://loci.ucsd.edu/hid" ) ))
    {
    //--- should we set a handler here too?
    this->SetServiceType ( "HID" );
    this->SetSelectedServer (NULL);
    if ( this->GetTagTableCollection() )
      {
      this->SelectedTagTable = this->GetTagTableCollection()->FindTagTableByName ("HIDTags");
      }
    }
  else
    {
    //--- for now assume it's xnd if not hid.
    //--- should we set a handler here too?
    this->SetServiceType ( "XND" );
    if ( this->GetTagTableCollection() )
      {
      this->SelectedTagTable = this->GetTagTableCollection()->FindTagTableByName ("XND");
      }
    }

  //--- trigger GUI and Logic to update.
  this->InvokeEvent ( vtkMRMLFetchMINode::SelectedServerModifiedEvent );
}



//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::SetServiceType ( const char *s)
{
  this->SetSelectedServiceType (s);
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

  this->SetErrorMessage ( node->GetErrorMessage() );
  this->SetSelectedTagTable ( node->GetSelectedTagTable() );
  this->SetSelectedServer(node->GetSelectedServer());
  this->SetSelectedServiceType(node->GetSelectedServiceType());

  //--- copy resource description.
  if ( node->GetResourceDescription() == NULL )
    {
    }
  if ( this->GetResourceDescription() == NULL )
    {
    //--- try to allocate.
    this->ResourceDescription = vtkTagTable::New();
    this->ResourceDescription->SetName ( "ResourceDescription" );
    }
  if ( this->GetResourceDescription() == NULL )
    {
    vtkErrorMacro( "Copy: got a null Resource description in the node to copy to." );
    return;
    }
  this->ResourceDescription->Copy ( node->GetResourceDescription() );


  //--- copy contents of tag tables to new node.
  if ( node->GetTagTableCollection() == NULL )
    {
    vtkErrorMacro( "Copy: got a null TagTableCollection in the node to copy from." );
    return;
    }
  if ( this->GetTagTableCollection() == NULL)
    {
    //--- try to allocate and set up.
    this->TagTableCollection = vtkTagTableCollection::New();
    this->AddTagTablesForWebServices();
    }
  if ( this->GetTagTableCollection() == NULL )
    {
    vtkErrorMacro( "Copy: got a null TagTableCollection in the node to copy to." );
    return;
    }
  if ( this->GetTagTableCollection()->GetNumberOfItems() !=
       node->GetTagTableCollection()->GetNumberOfItems ( ) )
    {
    vtkErrorMacro( "Copy: got different numbers of tag tables in the copy from and to nodes.");
    return;
    }

  for (int i=0; i < node->GetTagTableCollection()->GetNumberOfItems(); i++ )
    {
    vtkTagTable *destT = vtkTagTable::SafeDownCast ( this->GetTagTableCollection()->GetItemAsObject(i) );
    vtkTagTable *srcT = vtkTagTable::SafeDownCast ( node->GetTagTableCollection()->GetItemAsObject(i) );
    if ( srcT != NULL && destT != NULL )
      {
      //--- copy all metadata from the source table into the dest.
      destT->Copy( srcT );
      }
    }
}


//----------------------------------------------------------------------------
void vtkMRMLFetchMINode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "TagTableCollection: " << this->GetTagTableCollection() << "\n";
  os << indent << "SelectedServer: " << (this->SelectedServer == NULL ? "null" :  this->SelectedServer) << "\n";
  os << indent << "SelectedServiceType: " << (this->SelectedServiceType == NULL ? "null" : this->SelectedServiceType) << "\n";
  os << indent << "ErrorMessage: " << this->GetErrorMessage() << "\n";
}



