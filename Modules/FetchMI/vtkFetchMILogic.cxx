#include <string>
#include <iostream>
#include <sstream>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

#include "vtkObjectFactory.h"

#include "vtkFetchMILogic.h"
#include "vtkFetchMI.h"

#include "vtkHTTPHandler.h"
#include "vtkHIDHandler.h"
#include "vtkXNDHandler.h"
#include "vtkXMLDataParser.h"

#include "vtkMRMLScene.h"
#include "vtkCacheManager.h"
#include "vtkSlicerApplication.h"

#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorableNode.h"

#include "vtkKWMessageDialog.h"


vtkFetchMILogic* vtkFetchMILogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkFetchMILogic");
  if(ret)
    {
      return (vtkFetchMILogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkFetchMILogic;
}


//----------------------------------------------------------------------------
vtkFetchMILogic::vtkFetchMILogic()
{
  this->FetchMINode = NULL;
  this->SceneTags = vtkTagTable::New();
  this->SaveSceneDescription = 1;
  this->SceneSelected = 1;
  // used for parsing xml.
  this->NumberOfElements = 0;
  this->CurrentURI = NULL;

}




//----------------------------------------------------------------------------
vtkFetchMILogic::~vtkFetchMILogic()
{ 
  this->SetFetchMINode(NULL);
  this->SceneTags->Delete();
  this->CurrentURI  = NULL;
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "XMLDirName: " << this->XMLDirName.c_str() << "\n";
  os << indent << "HTTPResponseFileName: " << this->HTTPResponseFileName.c_str() << "\n";
  os << indent << "XMLUploadFileName: " << this->XMLUploadFileName.c_str() << "\n";
  os << indent << "TemporaryResponseFileName: " << this->TemporaryResponseFileName.c_str() << "\n";
  os << indent << "MRMLCacheFileName: " << this->MRMLCacheFileName.c_str() << "\n";
  os << indent << "DownloadDirName: " << this->DownloadDirName.c_str() << "\n";
  os << indent << "Selected Storable Node IDs:\n";
  for (unsigned int i = 0; i < this->SelectedStorableNodeIDs.size(); i++)
    {
    os << indent.GetNextIndent() << i << ": " << this->SelectedStorableNodeIDs[i].c_str() << "\n";
    }
  os << indent << "Modified Node IDs:\n";
  for (unsigned int i = 0; i < this->ModifiedNodes.size(); i++)
    {
    os << indent.GetNextIndent() << i << ": " << this->ModifiedNodes[i].c_str() << "\n";
    }  
  os << indent << "SaveSceneDsecription: " << this->SaveSceneDescription << "\n";
  os << indent << "SceneSelected: " << this->SceneSelected << "\n";

  os << indent << "SceneTags: (TBD)\n";

  os << indent << "NumberOfElements: " << this->NumberOfElements << "\n";
  os << indent << "CurrentURI: " << (this->CurrentURI == NULL ? "null" : this->CurrentURI) << "\n";
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::DeselectNode( const char *nodeID)
{
  std::string s;
  std::vector<std::string>tmp;
  int n = this->SelectedStorableNodeIDs.size();
  // copy all elements but one marked for delete to a temporary vector
  for ( int i = 0; i < n; i++ )
    {
    s = this->SelectedStorableNodeIDs[i];
    if ( (strcmp ( s.c_str(), nodeID )) )
      {
      tmp.push_back (std::string (nodeID ));
      }
    }

  // clear the vector and copy in new elements
  n = tmp.size();
  this->SelectedStorableNodeIDs.clear();
  for ( int i = 0; i < n; i++ )
    {
    this->SelectedStorableNodeIDs.push_back ( tmp[i] );
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::SelectNode( const char *nodeID)
{
  int unique = 1;
  std::string s;

  int n = this->SelectedStorableNodeIDs.size();
  for ( int i = 0; i < n; i++ )
    {
    s = this->SelectedStorableNodeIDs[i];
    if ( !(strcmp ( s.c_str(), nodeID )) )
      {
      unique = 0;
      break;
      }
    }
    //--- add the server name if it's not already here.
    if ( unique )
      {
      this->SelectedStorableNodeIDs.push_back ( std::string ( nodeID ) );
      }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::SelectScene()
{
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }
  this->SceneSelected = 1;

}


//----------------------------------------------------------------------------
void vtkFetchMILogic::DeselectScene()
{
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }
  this->SceneSelected = 0;

}







//----------------------------------------------------------------------------
const char *vtkFetchMILogic::GetHTTPResponseFileName ( )
{
  return ( this->HTTPResponseFileName.c_str() );
}




//----------------------------------------------------------------------------
const char *vtkFetchMILogic::GetXMLUploadFileName ( )
{
  return ( this->XMLUploadFileName.c_str() );
}


//----------------------------------------------------------------------------
const char *vtkFetchMILogic::GetTemporaryResponseFileName ( )
{
  return ( this->TemporaryResponseFileName.c_str() );
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::QueryServerForTags ( )
{
  //--- Get the selected service.
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }
  
  const char *svr = this->GetFetchMINode()->GetSelectedServer();
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( (svr == NULL) || (svctype== NULL ))
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype" );
    return;
    }

  if ( !(strcmp ("HID", svctype )) )
    {
    //--- HID
    vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
    if ( h )
      {
      // for now assume we can't query for tags.
      // just fill the table up with default ones.
      // TODO: find out how to query HID webservices for tags.
      // h->QueryServer ( uri, responseFile );
      }
    }
  else if ( !(strcmp ("XND", svctype)) )
    {
    //--- XND
    vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XNDHandler" ));
    if ( h )
      {
      //--- Check local file to receive response.
      if ( this->GetHTTPResponseFileName( ) )
        {
        //--- Build query
        std::stringstream q;
        q << svr;
        q << "/tags";
        const char *errorString = h->QueryServer ( q.str().c_str(), this->GetHTTPResponseFileName() );
        if ( !strcmp(errorString, "OK" ))
          {
          this->ParseTagQueryResponse ( );
          }
        }
      else
        {
        vtkErrorMacro ( "XML Response dir not set." );
        }
      }
    }
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::QueryServerForResources ( )
{
  //--- get selected tags
  //--- for now, use SlicerDataType=MRML
  //--- and form a uri.
  //--- Get the selected service.
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }
  
  const char *svr = this->GetFetchMINode()->GetSelectedServer();
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( (svr == NULL) || (svctype== NULL ))
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype" );
    return;
    }


  std::string att;
  std::string val;
  std::stringstream q;
  
  if ( !(strcmp ("HID", svctype )) )
    {
    //---
    //--- HID
    //---
    vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
    vtkHIDTagTable* t = vtkHIDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HIDTags" ));
    if ( h && t )
      {
      //--- Check local file to receive response.
      if ( this->GetHTTPResponseFileName( ) )
        {
        //--- for now only these queries are supported:
        //--- find a mrml file
        //curl -k "https://loci.ucsd.edu/hid/search?exp=fBIRNPhaseII__0010&subject=000670986943&file_type=MRML" 
        //--- query for subjects by experiment
        //curl -k https://loci.ucsd.edu/hid/experiments/subjects
        //--- query for experiments
        //curl -k https://loci.ucsd.edu/hid/experiments 

        // h->QueryServer ( uri, responseFile );
        //--- Limited query capability, so just query for mrml now.
        //--- Look thru tag table for values for Experiment
        //--- and values for Subject.
        //--- if we don't find them, note as much to the user,
        //--- and don't make the query.

        //--- do a case insensitive find on the attributes
        int subjectIndex = t->CheckTableForTag("Subject");
        int experimentIndex = t->CheckTableForTag("Experiment");
        int filetypeIndex = t->CheckTableForTag("File_type");

        //--- if any of the key tags are missing.... report to user and abort query.
        if ( subjectIndex < 0 || experimentIndex < 0 || filetypeIndex < 0 )
          {
          vtkErrorMacro ( "vtkFetchMILogic: Not all required tags are on data");
          return;
          }

        //--- otherwise assemble the hid query.
        q << svr << "/search?";

        att =  t->GetTagAttribute(experimentIndex);
        if ( att.c_str() != NULL )
          {
          if ( t->IsTagSelected(att.c_str()) )
            {
            val = t->GetTagValue(experimentIndex);
            }
          }
        if ( att.c_str() != NULL && val.c_str() != NULL )
          {
          q << "exp=";
          q << val;
          q << "&";
          }

        att =  t->GetTagAttribute(subjectIndex);
        if ( att.c_str() != NULL )
          {
          if ( t->IsTagSelected(att.c_str()) )
            {
            val = t->GetTagValue(subjectIndex);
            }
          }
        if ( att.c_str() != NULL && val.c_str() != NULL )
          {
          q << "subject=";
          q << val;
          q << "&";
          }

        att =  t->GetTagAttribute(filetypeIndex);
        if ( att.c_str() != NULL )
          {
          if ( t->IsTagSelected(att.c_str()) )
            {
            val = t->GetTagValue(filetypeIndex);
            }
          }
        if ( att.c_str() != NULL && val.c_str() != NULL )
          {
          q << "file_type=";
          q << val;
          }

        const char *errorString = h->QueryServer ( q.str().c_str(), this->GetHTTPResponseFileName() );
        if ( !strcmp(errorString, "OK" ))
          {
          this->ParseResourceQueryResponse ( );
          }
        }
      else
        {
        vtkErrorMacro ( "XML Response dir not set." );
        }
      }
    }

  else if ( !(strcmp ("XND", svctype)) )
    {
    //---
    //--- XND
    //---
    vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XNDHandler" ));
    vtkXNDTagTable* t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
    if ( h && t )
      {
      //--- Check local file to receive response.
      if ( this->GetHTTPResponseFileName( ) )
        {
        //--- Build query
        q << svr << "/search?";

        //--- TODO: is there a better way to filter out weird tags?
        //--- Append all selected XND tags that are reasonably
        //--- formed. That means they have both an attribute
        //--- and a value, and both are not "", "NULL" or "none"
        //--- etc.
        int num = t->GetNumberOfTags();
        int firsttag = 1;
        for ( int i =0; i<num; i++)
          {
          att = t->GetTagAttribute(i);
          val = t->GetTagValue(i);
          if ( t->IsTagSelected(att.c_str()))
            {
            //--- do some checking...
            if ( att.c_str() != NULL && val.c_str() != NULL )
              {
              if ( (strcmp(att.c_str(), "NULL")) &&
                   (strcmp(att.c_str(), "null")) &&
                   (strcmp(att.c_str(), "none")) &&
                   (strcmp(att.c_str(), "None")) &&
                   (strcmp(att.c_str(), "" )))
                {
                if ( (strcmp(val.c_str(), "NULL")) &&
                     (strcmp(val.c_str(), "null")) &&
                     (strcmp(val.c_str(), "none")) &&
                     (strcmp(val.c_str(), "None")) &&
                     (strcmp(val.c_str(), "" )))
                  {
                  if ( firsttag )
                    {
                    q << "&";
                    firsttag = 0;
                    }
                  q << att;
                  q << "=";
                  q << val;
                  }
                }
              }
            }
          }
        const char *errorString = h->QueryServer ( q.str().c_str(), this->GetHTTPResponseFileName() );
        if ( !strcmp(errorString, "OK" ))
          {
          this->ParseResourceQueryResponse ( );
          }
        }
      else
        {
        vtkErrorMacro ( "XML Response dir not set." );
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::GetNumberOfXMLElements( vtkXMLDataElement *element )
{
  if (element ==  NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: GetNumberOfXMLElements called with null vtkXMLDataElement.");
    return;
    }

  this->NumberOfElements++;
  int nnested = element->GetNumberOfNestedElements();
  for ( int i=0; i < nnested; i++ )
    {
    vtkXMLDataElement *e = element->GetNestedElement ( i );
    if ( e != NULL )
      {
      this->GetNumberOfXMLElements( e );
      }
    }
}





//----------------------------------------------------------------------------
void vtkFetchMILogic::GetXMLElement( vtkXMLDataElement *element )
{
  if (element ==  NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: GetXMLElement called with null vtkXMLDataElement.");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }

  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( !(strcmp (svctype, "XND")))
    {
    this->GetXNDXMLEntry ( element );
    }
  else if ( !(strcmp(svctype, "HID")))
    {
    this->GetHIDXMLEntry ( element );
    }
  
  int nnested = element->GetNumberOfNestedElements();
  for ( int i=0; i < nnested; i++)
    {
    vtkXMLDataElement *nestedElement = element->GetNestedElement ( i );
    if (nestedElement != NULL )
      {
      this->GetXMLElement (nestedElement);
      }
    }

}




//----------------------------------------------------------------------------
void vtkFetchMILogic::GetHIDXMLEntry( vtkXMLDataElement *element )
{

  if (element ==  NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: GetXMLEntry called with null vtkXMLDataElement.");
    return;
    }

  const char *name = element->GetName();
  const char *attName;
  const char *value;
  const char *dtype = "unknown";

  if ( name == NULL )
    {
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }

  // check to see if this is an item of interest.
  // do different things based on what elements we find.
  //---
  // TAGS
  //---
  if ( (!(strcmp(name, "ul" ))) ||
       (!(strcmp(name, "UL" ))) )
    {
    // see if the attribute in this tag is called Label, and if its value is SlicerDataType
    int numatts = element->GetNumberOfAttributes();
    for ( int i=0; i < numatts; i++ )
      {
      attName = element->GetAttributeName(i);        
      value = element->GetAttributeValue(i);
      if ( attName != NULL && value != NULL )
        {            
        if ( (!strcmp(value, "mrml" )) ||
             (!strcmp(value, "MRML")) )
          {
          //=== if this says 1, then good. if 3, then we can simplify ths metod.
          int nnested = element->GetNumberOfNestedElements();
          vtkXMLDataElement *nnestedElement = element;
          for ( int j=0; j < nnested; j++)
            {
            nnestedElement = nnestedElement->GetNestedElement ( j );
            if ( nnestedElement == NULL )
              {
              break;
              }
            name = nnestedElement->GetName();
            if ( name != NULL )
              {
              if ( !(strcmp (name, "li" )) ||
                   !(strcmp (name, "LI" )) )
                {
                //--- go in deeper.
                int nnnested = nnestedElement->GetNumberOfNestedElements();
                vtkXMLDataElement *nnnestedElement = nnestedElement;
                for ( int k=0; k < nnnested; k++ )
                  {
                  nnnestedElement = nnnestedElement->GetNestedElement ( k );
                  if ( nnnestedElement == NULL )
                    {
                    break;
                    }
                  name = nnnestedElement->GetName();
                  int numatts2 = nnnestedElement->GetNumberOfAttributes();
                  for (int n=0; n<numatts2; n++)
                    {
                    if ( name != NULL )
                      {
                      if ( !(strcmp (name, "a" )) ||
                           !(strcmp (name, "A" )) )
                        {
                        attName = nnnestedElement->GetAttributeName(n);        
                        value = nnnestedElement->GetAttributeValue(n);
                        if ( attName != NULL && value != NULL )
                          {
                          if ( !(strcmp(attName, "href")) ||
                               !(strcmp(attName, "HREF")))
                            {
                            this->CurrentURI = value;
                            this->FetchMINode->GetResourceDescription()->AddOrUpdateTag ( this->CurrentURI, dtype, 0 );
                            break;
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
}



//----------------------------------------------------------------------------
int vtkFetchMILogic::CheckValidSlicerDataType ( const char *dtype)
{
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: Got NULL FetchMINode pointer.");
    return -1;
    }

  vtksys_stl::string s = dtype;
  vtksys_stl::string tmp;

  int num = this->FetchMINode->GetSlicerDataTypes()->GetNumberOfValues();
  for ( int i=0; i < num; i++ )
    {
    tmp = this->FetchMINode->GetSlicerDataTypes()->GetValue(i);
    if ( tmp.c_str() != NULL )
      {
      if ( tmp == s )
        {
        return (i);
        }
      }
    }

  return -1;
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::GetXNDXMLEntry( vtkXMLDataElement *element )
{

  if (element ==  NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: GetXMLEntry called with null vtkXMLDataElement.");
    return;
    }

  const char *name = element->GetName();
  const char *attName;
  const char *value;
  const char *dtype = "unknown";

  if ( name == NULL )
    {
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }

  // check to see if this is an item of interest.
  // do different things based on what elements we find.


  //---
  // TAGS
  //---
  if ( (!(strcmp(name, "Label" ))) ||
       (!(strcmp(name, "label" ))) )
    {
    // parse the xml file and put result into node's XND tag table.
    //--- we are filling up the Query tag table here.
    //--- tag values should be set by user.
    value = element->GetCharacterData();
    vtkXNDTagTable* t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
    //--- make sure to keep SlicerDataType a default
    //--- value of 'MRML' in the XNDTagTable.
    //--- This promotes tagging of scenes with a
    //--- consistent attribute/value, and searching for scenes
    //--- with that consistent tag.
    if ( value != NULL )
      {
      if ( !(strcmp (value, "SlicerDataType" )))
        {
        t->AddOrUpdateTag ( value, "MRML", 0 );        
        }
      else
        {
        t->AddOrUpdateTag ( value, "none", 0 );
        }
      }
    }

  //---
  // RESOURCES
  //---
  if ((!(strcmp(name, "resource" ))) ||
      (!(strcmp(name, "Resource" ))) ||
      (!(strcmp(name, "RESOURCE" ))) )
    {
    // parse the xml file and put result into ResourceDescription table.
    // looking for the SlicerDataType to pair with uri
    this->CurrentURI = "";
    int nnested = element->GetNumberOfNestedElements();
    vtkXMLDataElement *nestedElement;
    //--- get uri
    for ( int i=0; i < nnested; i++)
      {
      nestedElement = element->GetNestedElement ( i );
      if (nestedElement == NULL)
        {
        break;
        }

      if (!(strcmp(nestedElement->GetName(), "uri")) ||
          !(strcmp(nestedElement->GetName(), "Uri")) ||
          !(strcmp(nestedElement->GetName(), "URI")) )            
        {
        value = nestedElement->GetCharacterData();
        if ( value != NULL )
          {
          this->CurrentURI = value;
          this->FetchMINode->GetResourceDescription()->AddOrUpdateTag ( this->CurrentURI, dtype, 0 );        
          }
        }

      //--- get data type
      if (((!(strcmp(nestedElement->GetName(), "tag" ))) ||
           (!(strcmp(nestedElement->GetName(), "Tag" ))) ||
           (!(strcmp(nestedElement->GetName(), "TAG" )))) &&
          ( (strcmp(this->CurrentURI, ""))))
        {
        // see if the attribute in this tag is called Label, and if its value is SlicerDataType
        int numatts = nestedElement->GetNumberOfAttributes();
        for ( int j=0; j < numatts; j++ )
          {
          attName = nestedElement->GetAttributeName(j);        
          value = nestedElement->GetAttributeValue(j);
          if ( attName != NULL && value != NULL )
            {            
            if ( !strcmp(value, "SlicerDataType" ) )
              {
              // if yes, then get the nested Value Element          
              int nnested2 = nestedElement->GetNumberOfNestedElements();
              vtkXMLDataElement *nested2Element = nestedElement;
              for ( int k=0; k < nnested2; k++)
                {
                nested2Element = nestedElement->GetNestedElement ( k );
                if ( nested2Element == NULL )
                  {
                  break;
                  }
                value = nested2Element->GetName();
                if ( value != NULL )
                  {
                  if ( !(strcmp (value, "Value" )) ||
                       !(strcmp (value, "value" )) )
                    {
                    dtype = nested2Element->GetCharacterData();              
                    //now pair it up with CurrentURI and add to ResourceDescription.
                    this->FetchMINode->GetResourceDescription()->AddOrUpdateTag ( this->CurrentURI, dtype, 0 );
                    this->CurrentURI = "";
                    break;              
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::ParseTagQueryResponse ( )
{
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }

  this->FetchMINode->SetErrorMessage ( NULL );
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( svctype == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype" );
    return;
    }
    
  if ( this->GetHTTPResponseFileName() != NULL )
    {
    //--- TODOCheck document namespace and make sure it
    //--- agrees with the current svctype.
    }
  else
    {
    vtkErrorMacro ( "vtkFetchMILogic: No XMLResponse filename is set." );
    return;
    }
    
  //--- TODO: distinguish these better.... 
  if ( !(strcmp ("HID", svctype )) )
    {
    //--- HID
    //--- look at xml file to see if there's an error message
    //--- parse the error and fill the node's error message.
    //--- if no, parse response into table.
    }
  else if ( !(strcmp ("XND", svctype)) )
    {
    //--- XND
    vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
    if ( t != NULL && this->GetHTTPResponseFileName() != NULL )
      {
      //--- check for file.
      vtkXMLDataParser *parser = vtkXMLDataParser::New();
      parser->SetFileName ( this->GetHTTPResponseFileName() );
      parser->SetIgnoreCharacterData ( 0 );
      int retval = parser->Parse();
      if ( retval == 0 )
        {
        parser->Delete();
        this->FetchMINode->SetErrorMessage ("Unable to parse tag query response.");
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        return;
        }

      this->NumberOfElements = 0;
      vtkXMLDataElement *root = parser->GetRootElement();
      if ( root != NULL )
        {
        this->GetNumberOfXMLElements( root );
        if ( this->NumberOfElements > 0 )
          {
          //--- If response is good, parse response into table.
          root = parser->GetRootElement();
          this->GetXMLElement ( root );
          }
        }
      else
        {
        // let user know the parsing didn't go well.
        this->FetchMINode->SetErrorMessage ("Unable to parse tag query response. Displayed list of tags has not been updated.");
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
        }
      if ( parser )
        {
        parser->Delete();
        }

      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::TagResponseReadyEvent );
      //--- clear response file contents: better way?
      FILE *fptr = fopen(this->GetHTTPResponseFileName(), "w");    
      if ( fptr)
        {
        fclose ( fptr);
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::ParseResourceQueryResponse ( )
{
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }

  this->FetchMINode->SetErrorMessage(NULL );
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( svctype == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype" );
    return;
    }

  if ( this->GetHTTPResponseFileName() != NULL )
    {
    //--- TODOCheck document namespace and make sure it
    //--- agrees with the current svctype.
    }
  else
    {
    vtkErrorMacro ( "vtkFetchMILogic: No XMLResponse filename is set." );
    return;
    }

  if ( (this->FetchMINode->GetResourceDescription() != NULL) && (this->GetHTTPResponseFileName() != NULL) )
    {
    //--- check for file.
    vtkXMLDataParser *parser = vtkXMLDataParser::New();
    parser->SetFileName ( this->GetHTTPResponseFileName() );
    parser->SetIgnoreCharacterData ( 0 );
    int retval = parser->Parse();
    if ( retval == 0 )
      {
      parser->Delete();
      this->FetchMINode->SetErrorMessage("Unable to parse resource query response.");
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      return;
      }
    this->NumberOfElements = 0;
    vtkXMLDataElement *root = parser->GetRootElement();
    if ( root != NULL )
      {
      this->GetNumberOfXMLElements( root );
      if ( this->NumberOfElements > 0 )
        {
        //--- If response is good, parse response into table.
        root = parser->GetRootElement();
        this->GetXMLElement ( root );
        }
      }
    else
      {
      // let user know the parsing didn't go well.
      this->FetchMINode->SetErrorMessage("Unable to parse tag resource response.");
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
      }
    if ( parser )
      {
      parser->Delete();
      }

    //--- clear response file contents: better way?
    FILE *fptr = fopen(this->GetHTTPResponseFileName(), "w");    
    if ( fptr)
      {
      fclose ( fptr);
      }
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::ResourceResponseReadyEvent );
    }
}



//----------------------------------------------------------------------------
const char * vtkFetchMILogic::PostMetadata ( vtkXNDHandler *handler, const char *filename )
{
  const char *returnval = NULL;
  
  if (this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadata got a NULL FetchMINode" );
    return returnval;
    }
  if (this->GetXMLUploadFileName() == NULL)
    {
    vtkErrorMacro ( "FetchMILogic: Got NULL file for Metadata upload.");
    return returnval;
    }
  if (this->GetTemporaryResponseFileName() == NULL)
    {
    vtkErrorMacro ( "FetchMILogic: Got NULL file for Metadata upload.");
    return returnval;
    }

  vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName (filename);
  
  const char *strippedFilename = vtkFileName.c_str();

  std::string svr = this->GetFetchMINode()->GetSelectedServer();
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();  
  if ( svctype == NULL )
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadata got a NULL Web Service Type");
    return returnval;
    }
  if ( !(strcmp(svctype, "HID")))
    {
    // no op
    }
  if ( !(strcmp(svctype, "XND")))
    {
    // SafeDownCast in FindURIHandlerByName was causing a crash
    //vtkXNDHandler *handler = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XNDHandler" ));
    if ( handler == NULL )
      {
      vtkErrorMacro ( "vtkFetchMILogic: PostMetadata got a null XNDHandler." );
      return returnval;
      }
    std::string::size_type index =  svr.find("://", 0);
    if ( index  != std::string::npos)
      {
      std::string hostname = svr.substr( index+3, std::string::npos );
      // do a synchronous dl
      handler->SetHostName(hostname.c_str());
      }
     
    std::stringstream ss;
    ss << svr.c_str();
    ss << "/data";
    // returnval = handler->PostMetadata ( this->GetXMLUploadFileName() );
    returnval = handler->PostMetadata ( ss.str().c_str(), this->GetXMLUploadFileName(), strippedFilename, this->GetTemporaryResponseFileName() );
    return returnval;
    }

  vtkErrorMacro("WriteMetadataForUpload: unknown service type " << svctype);
  return returnval;
}





//----------------------------------------------------------------------------
const char * vtkFetchMILogic::ParsePostMetadataResponse ( const char *response)
{
  // check to see if response starts with http://, if so, return it as is
  if (response == NULL )
    {
    return ( NULL );
    }
  if (!strncmp(response, "http://", 7))
    {
//    response = this->Script("string trim %s", response );
    return response;
    }
  else
    {
    return ( NULL );
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::GetCurrentListOfStorableNodes ( )
{
}


//----------------------------------------------------------------------------
int vtkFetchMILogic::CheckStorableNodesForTags ( )
{
  return 1;
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::TagStorableNodes ( )
{
}



// filename is a dataset's filename or mrmlscene filename.
// ID is a nodeID or is the text "MRML".
//----------------------------------------------------------------------------
int vtkFetchMILogic::WriteMetadataForUpload ( const char *filename, const char *ID, vtkXNDHandler *handler)
{

  if ( this->FetchMINode == NULL) 
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return 0;
    }
  if (this->GetXMLUploadFileName() == NULL)
    {
    vtkErrorMacro ( "vtkFetchMILogic: XMLUploadFileName is not set." );
    return 0;
    }
  if (this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload has null MRMLScene." );
    return 0;        
    }  

  ofstream file;
  // open the file for writing.
#ifdef _WIN32
  file.open(this->GetXMLUploadFileName(), std::ios::out | std::ios::binary);
#else
  file.open(this->GetXMLUploadFileName(), std::ios::out);
#endif
  if ( file.fail() )
    {
    vtkErrorMacro ("WriteMetadataForUpload: could not open file " << this->GetXMLUploadFileName() );
    return 0;
    }


//  const char *svr = this->GetFetchMINode()->GetSelectedServer();
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  const char *att;
  const char *val;

  if ( svctype == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype" );
    return 0;
    }

  if ( !(strcmp(svctype, "XND" )))
    {
    // doing a SafeDownCast in FindURIHandlerByName  was causing a crash...
/*    vtkURIHandler *uriHandler = this->GetMRMLScene()->FindURIHandlerByName ( "XNDHandler" );
    if (uriHandler == NULL)
      {
      vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload got a null URIHandler." );
      return 0;
      }
    vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (uriHandler);
*/
    if ( handler == NULL )
      {
      vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload got a null XNDHandler." );
      return 0;
      }

    //--- Check to see if ID == MRML.
    if ( !(strcmp(ID, "MRML" ) ) )
      {
      vtkTagTable *t = this->GetMRMLScene()->GetUserTagTable();
      if ( t == NULL )
        {
      vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload got a null tag table for the scene." );
      return 0;
        }
      // make sure SlicerDataType tag is set to 'MRML'
      t->AddOrUpdateTag ( "SlicerDataType", "MRML" );
      int num = t->GetNumberOfTags();
      //------ Write XML for the scene file using this->SceneTags

      file << handler->GetXMLDeclaration();
      file << "\n";
      file << "<Metadata ";
      file << handler->GetNameSpace();
      file << ">";
      file << "\n";
        
      for ( int i=0; i < num; i++ )
        {
        att = t->GetTagAttribute(i);
        val = t->GetTagValue(i);
//        if ( t->IsTagSelected(att))
        // use all tags for now
        if ( 1 )
          {
          file << "<Tag Label=";
          file << "\"";
          file << att;
          file << "\"";
          file << ">\n";
          file << "<Value>";
          file << val;
          file << "</Value>\n";
          file << "</Tag>\n";
          }
        }
      file << "</Metadata>\n";
      }
    else
      {
      //------ Check to see if the ID is a storable node in scene.
      //------ if NO, return 0.
      //------ Check to see if the node is tagged for upload
      //------ if not, return 0.
      //------ Write the xml description for each filename
      //------ into file with filename=this->XMLUploadFileName
      //------ return 1 if file writes ok, else return 0

      vtkMRMLStorableNode *n = vtkMRMLStorableNode::SafeDownCast(this->MRMLScene->GetNodeByID ( ID ));
      if ( n == NULL )
        {
        vtkErrorMacro ( "vtkFetchMILogic: WriteMetadataForUpload got a null node to write." );
        return 0;        
        }
      
      // Set up a temporary tag table in case the node doesn't have one.
      vtkTagTable *tmpTags= vtkTagTable::New();
      vtkTagTable *t = n->GetUserTagTable();
      if ( t == NULL)
        {
        t = tmpTags;
        }

      // make sure there's a SlicerDataType tag.
      int num = t->GetNumberOfTags();
      int found = 0;
      for ( int i=0; i < num; i++ )
        {
        att = t->GetTagAttribute(i);
        if ( !strcmp (att, "SlicerDataType" ))
          {
          found = 1;
          break;
          }
        }
      if ( !found )
        {
        t->AddOrUpdateTag ( "SlicerDataType", "unknown" );
        }

      //------ Write XML for the scene file using this->SceneTags
      num = t->GetNumberOfTags();
      file << handler->GetXMLDeclaration();
      file << "\n";
      file << "<Metadata ";
      file << handler->GetNameSpace();
      file << ">";
      file << "\n";

      for (int i=0; i < num; i++ )
        {
        att = t->GetTagAttribute(i);
        val = t->GetTagValue(i);
//        if ( t->IsTagSelected(att))
        // use all tags for now.
        if ( 1 )
          {
          file << "<Tag Label=";
          file << "\"";
          file << att;
          file << "\"";
          file << ">\n";
          file << "<Value>";
          file << val;
          file << "</Value>\n";
          file << "</Tag>\n";
          }
        }
      file << "</Metadata>\n";

      tmpTags->Delete();
      }
    }
  else
    {
    //no-op
    }

  file.close();
  return (1);
}




//----------------------------------------------------------------------------
int vtkFetchMILogic::TestForRequiredTags ( )
{

  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null FetchMINode" );
    return 0;
    }

  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( svctype == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype" );
    return 0;
    }
  
  if ( !(strcmp ("HID", svctype )) )
    {
    //no-op
    }
  if ( !(strcmp ("XND", svctype )) )
    {
    //--- check scene and all selected nodes.
    //---Required tags: experiment, project, subject, scan, modality, SlicerDataType.
    vtkTagTable *t;
    if ( this->SceneSelected )
      {
      if ( this->MRMLScene == NULL )
        {
        vtkErrorMacro ("vtkFetchMILogic: Null MRMLScene." );
        return 0;        
        }
      //--- look at scene tags
      t = this->MRMLScene->GetUserTagTable();
      if ( t == NULL)
        {
        return 0;
        }
      for ( int i=0; i < this->FetchMINode->RequiredXNDTags.size(); i++ )
        {
        if ( t->CheckTableForTag (this->FetchMINode->RequiredXNDTags[i].c_str()) < 0 )
          {
          return 0;
          }
        }
      }

    //--- and look at each node.
    for ( int n=0; n < this->SelectedStorableNodeIDs.size(); n++)
      {
      const char *nodeID = this->SelectedStorableNodeIDs[n].c_str();
      vtkMRMLStorableNode *node = vtkMRMLStorableNode::SafeDownCast ( this->MRMLScene->GetNodeByID (nodeID));
      if ( node != NULL )
        {
        t = this->FetchMINode->GetTagTableCollection()->FindTagTableByName ("XNDTags");
        if ( t == NULL )
          {
          return 0;
          }
        for ( int i=0; i < this->FetchMINode->RequiredXNDTags.size(); i++ )
          {
          if ( t->CheckTableForTag (this->FetchMINode->RequiredXNDTags[i].c_str()) < 0 )
            {
            return 0;
            }
          }
        }
      }
    }

  return 1;
}





//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceUpload ( )
{
  
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }

  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( svctype == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype" );
    return;
    }
  
  if ( !(strcmp ("HID", svctype )) )
    {
    //no-op
    vtkWarningMacro("RequestResourceUpload: HID upload not implemented yet.");
    }
  if ( !(strcmp ("XND", svctype )) )
    {
    if ( 1 || this->TestForRequiredTags() > 0 )
      {
      this->RequestResourceUploadToXND();
      }
      else
      {
      this->FetchMINode->SetErrorMessage ("Some or all items selected are not described by all tags required by XNAT Desktop and Slicer. Please include values for: Project, Experiment, Subject, Scan, Modality, and SlicerDataType.");
      this->FetchMINode->InvokeEvent(vtkMRMLFetchMINode::RemoteIOErrorEvent );
      vtkWarningMacro("Some or all items selected are not described by all tags required by XNAT Desktop and Slicer.");
      }
    }
}



//--- This method has to figure out whether we are downloading
//--- a scene or a dataset then has to figure out which service is queried
//--- and call the correct method to download a scene or resource
//----------------------------------------------------------------------------
int vtkFetchMILogic::RequestResourceDownload ( const char *uri, const char *slicerDataType)
{

  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return -1;
    }

  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( svctype == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype" );
    return -1;
    }
  
  if ( uri == NULL || slicerDataType == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic: Null uri or slicerDataType." );
    return -1;
    }
  
/*
  //--- make sure we have a data type we know how to handle.
  if ( (strcmp (slicerDataType, "VTKModel")) &&
       (strcmp (slicerDataType, "FreeSurferModel")) &&
       (strcmp (slicerDataType, "ScalarVolume")) &&
       (strcmp (slicerDataType, "MRML" )))
    {
    //--- retval reports an unknown slicerDataType.
    return 0;    
    }
*/

  //--- FOR NOW, ONLY mrml...
  if ( (strcmp (slicerDataType, "MRML")))
    {
    //--- retval reports an unknown slicerDataType.
    return 0;    
    }


  //--- handle scene with separate set of methods.
  if ( !(strcmp(slicerDataType, "MRML")))
    {
    //--- check all known webservices to see which is selected
    if ( !(strcmp ("XND", svctype )) )
      {
      this->RequestSceneDownloadFromXND( uri );
      }
    if ( !(strcmp ("HID", svctype )) )
      {
      this->RequestSceneDownloadFromHID( uri );
      }
    }
  else 
    {
    //--- check all known webservices to see which is selected
    if ( !(strcmp ("XND", svctype )) )
      {
      this->RequestResourceDownloadFromXND( uri, slicerDataType );
      }
    if ( !(strcmp ("HID", svctype )) )
      {
      this->RequestResourceDownloadFromHID( uri, slicerDataType );
      }
    }

  //--- retval means good as far as we can tell.
  return 1;
}





//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceDownloadFromHID ( const char *uri, const char *slicerDataType )
{
  // This method will download resource with uri
  // from the currently selected remote repository.
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null FetchMI node. ");
    return;
    }

  vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
  if ( h == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null URIHandler. ");
    return;
    }

  if ( !(strcmp(slicerDataType, "ScalarVolume" )))
    {
    //--- create the node
    //--- set its URIhandler
    //--- download to cache and load.
    }
  if ( !(strcmp(slicerDataType, "VTKModel" )))
    {
    //--- create the node
    //--- set its URIhandler
    //--- download to cache and load.
    }
  if ( !(strcmp(slicerDataType, "FreeSurferModel" )))
    {
    //--- create the node
    //--- set its URIhandler
    //--- download to cache and load.
    }
  
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceDownloadFromXND ( const char *uri, const char *slicerDataType )
{
  // This method will download resource with uri
  // from the currently selected remote repository.

  if (uri == NULL || slicerDataType == NULL)
    {
    vtkErrorMacro("vtkFetchMILogic::RequestResourceDownloadFromXND: uri isn't set or slicerDataType isn't set. uri = " << (uri == NULL ? "null" : uri) << ", slicerDataType = " << (slicerDataType == NULL ? "null" : slicerDataType));
    return;
    }
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromXND: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromXND: Null FetchMI node. ");
    return;
    }

  
  vtkXNDHandler *handler = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XNDHandler" ));
  if ( handler == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromXND: Null URIHandler. ");
    return;
    }

  // for the storage nodes to find the right handler, use xnd:// instead of
  // http
  std::string::size_type index;
  std::string uriString(uri);
  std::string suffix;
  // get all charactersup to the ://
  if ((index = uriString.find("://", 0) ) != std::string::npos)
    {
    suffix = uriString.substr(index, std::string::npos);
    uriString = std::string("xnd") + suffix;
    vtkDebugMacro("vtkFetchMILogic::RequestResourceDownloadFromXND: new uri string = " << uriString.c_str());
    }
    
  if ( !(strcmp(slicerDataType, "ScalarVolume" )))
    {
    // is it a label map?
    /*
    vtkSlicerVolumesLogic *volumesLogic = vtkSlicerVolumesGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Volumes"))->GetLogic();
    if (volumesLogic)
      {
      int loadingOptions = 0 || 8;
      volumesLogic->AddArchetypeVolume(uriString.c_str(), "XNDVolume", loadingOptions);
      }
    */
    //--- create the node, assume scalar
    vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
    vtkMRMLScalarVolumeDisplayNode  *sdisplayNode = NULL;
    vtkMRMLVolumeArchetypeStorageNode *storageNode = vtkMRMLVolumeArchetypeStorageNode::New();
    // set the volume name
    std::string volumeName;
    volumeName = vtksys::SystemTools::GetFilenameName(uriString);
    scalarNode->SetName(volumeName.c_str());
    // set the uri
    storageNode->SetURI(uriString.c_str());
     // need to set the scene on the storage node so that it can test looking
     // for file handlers, but setting ti explicitly
    storageNode->SetScene(this->GetMRMLScene());

    //--- set its URIhandler
    storageNode->SetURIHandler(handler);
    // set other flags on the storage node
    /*
    storageNode->SetCenterImage(centerImage);
    storageNode->SetSingleFile(singleFile);
    storageNode->SetUseOrientationFromFile(useOrientationFromFile);
    */
    this->GetMRMLScene()->SaveStateForUndo();
   
    // add storage nodes to the scene so can observe them
    this->GetMRMLScene()->AddNodeNoNotify(storageNode);
    this->GetMRMLScene()->AddNodeNoNotify(scalarNode);
    scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());
    this->GetMRMLScene()->AddNodeNoNotify(sdisplayNode);
    scalarNode->SetAndObserveDisplayNodeID(sdisplayNode->GetID());

    //--- download to cache and load.
    storageNode->ReadData(scalarNode);

    // since added the node w/o notification, let the scene know now that it
    // has a new node
    this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent, scalarNode);

    // clean up
    if (scalarNode)
      {
      scalarNode->Delete();
      }
    if (storageNode)
      {
      storageNode->Delete();
      }
    if (sdisplayNode)
      {
      sdisplayNode->Delete();
      }

    }
  
  // get the models logic
  /*
    if ( !(strcmp(slicerDataType, "VTKModel" ) ||
    !(strcmp(slicerDataType, "FreeSurferModel" )) )
    {
    vtkSlicerModelsLogic *modelsLogic = vtkSlicerModelsGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Models"))->GetLogic();
    if (modelsLogic)
    {
    modelsLogic->AddNode(uriString.c_str());
    }
    }
  */
  if ( !(strcmp(slicerDataType, "VTKModel" )))
    {
    //--- create the node
    vtkMRMLModelNode *modelNode = vtkMRMLModelNode::New();
    vtkMRMLModelDisplayNode *displayNode = vtkMRMLModelDisplayNode::New();
    vtkMRMLModelStorageNode *mStorageNode = vtkMRMLModelStorageNode::New();

    // set the uri
    mStorageNode->SetURI(uriString.c_str());
    modelNode->SetName("XNDModel");

    //--- set its URIhandler
    mStorageNode->SetURIHandler(handler);

    //--- download to cache and load.
    this->GetMRMLScene()->SaveStateForUndo();

    modelNode->SetScene(this->GetMRMLScene());
    mStorageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene()); 

    this->GetMRMLScene()->AddNodeNoNotify(mStorageNode);  
    this->GetMRMLScene()->AddNodeNoNotify(displayNode);
    modelNode->SetAndObserveStorageNodeID(mStorageNode->GetID());
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());  
    displayNode->SetPolyData(modelNode->GetPolyData());
    
    this->GetMRMLScene()->AddNode(modelNode);  
    // the scene points to it still
    modelNode->Delete();

    int retval = mStorageNode->ReadData(modelNode);
    if (retval != 1)
      {
      vtkErrorMacro("AddModel: error reading " << uriString.c_str());
      this->GetMRMLScene()->RemoveNode(modelNode);
      this->GetMRMLScene()->RemoveNode(mStorageNode);
      this->GetMRMLScene()->RemoveNode(displayNode);
      }
    mStorageNode->Delete();
    displayNode->Delete();
    }

  if ( !(strcmp(slicerDataType, "FreeSurferModel" )))
    {
    //--- create the node, assuming a free surfer model for now
    vtkMRMLModelNode *modelNode = vtkMRMLModelNode::New();
    vtkMRMLModelDisplayNode *displayNode = vtkMRMLModelDisplayNode::New();
    vtkMRMLFreeSurferModelStorageNode *fsmStorageNode = vtkMRMLFreeSurferModelStorageNode::New();
    fsmStorageNode->SetUseStripper(0);  // turn off stripping by default (breaks some pickers)

    // set the uri
    fsmStorageNode->SetURI(uriString.c_str());
    modelNode->SetName("XNDFSModel");
    
    //--- set its URIhandler
    fsmStorageNode->SetURIHandler(handler);
    
    //--- download to cache and load.
    this->GetMRMLScene()->SaveStateForUndo();

    modelNode->SetScene(this->GetMRMLScene());
    fsmStorageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene()); 

    this->GetMRMLScene()->AddNodeNoNotify(fsmStorageNode);  
    this->GetMRMLScene()->AddNodeNoNotify(displayNode);
    modelNode->SetAndObserveStorageNodeID(fsmStorageNode->GetID());
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());  
    displayNode->SetPolyData(modelNode->GetPolyData());
    
    this->GetMRMLScene()->AddNode(modelNode);  
    // the scene points to it still
    modelNode->Delete();

    int retval = fsmStorageNode->ReadData(modelNode);
    if (retval != 1)
      {
      vtkErrorMacro("AddModel: error reading " << uriString.c_str());
      this->GetMRMLScene()->RemoveNode(modelNode);
      this->GetMRMLScene()->RemoveNode(fsmStorageNode);
      this->GetMRMLScene()->RemoveNode(displayNode);
      }
    fsmStorageNode->Delete();
    displayNode->Delete();
    }
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestSceneDownloadFromHID ( const char *uri)
{
  // This method will a resource with uri
  // from the currently selected remote repository.
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null FetchMI node. ");
    return;
    }

  vtkHIDHandler *handler = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
  if ( handler == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceDownloadFromHID: Null URIHandler. ");
    return;
    }

  std::string localURL = this->GetMRMLScene()->GetCacheManager()->GetFilenameFromURI(uri);
  std::string remoteURL = uri;

  // currently have to pass a host name
  // TODO: test this!
  std::string svr = this->GetFetchMINode()->GetSelectedServer();
  std::string hostname;
  std::string::size_type index;
  if ( (index = svr.find("://", 0)) != std::string::npos)
    {
    hostname = svr.substr( index+3, std::string::npos );
    // do a synchronous dl
    handler->SetHostName(hostname.c_str());
    handler->StageFileRead(remoteURL.c_str(), localURL.c_str());
    
    // now override the mrml scene's url to point to file on disk
    this->GetMRMLScene()->SetURL(localURL.c_str());


    //--- load the remote scene
    this->MRMLScene->Connect();

    }
}






//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestSceneDownloadFromXND ( const char *uri )
{
  // This method will download a resource with uri
  // from the currently selected remote repository.
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestSceneDownloadFromXND: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestSceneDownloadFromXND: Null FetchMI node. ");
    return;
    }

  vtkXNDHandler *handler = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XNDHandler" ));
  if ( handler == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestSceneDownloadFromXND: Null URIHandler. ");
    return;
    } 

  std::string localURL = this->GetMRMLScene()->GetCacheManager()->GetFilenameFromURI(uri);
  std::string remoteURL = uri;

  // currently have to pass a host name
  // TODO: test this!
  std::string svr = this->GetFetchMINode()->GetSelectedServer();
  std::string hostname;
  std::string::size_type index;
  index = svr.find("://", 0);
  if ( index != std::string::npos)
    {
    hostname = svr.substr( index+3, std::string::npos );
    // do a synchronous dl
    handler->SetHostName(hostname.c_str());
    handler->StageFileRead(remoteURL.c_str(), localURL.c_str());
    
    // now override the mrml scene's url to point to file on disk
    this->GetMRMLScene()->SetURL(localURL.c_str());

    //--- load the remote scene
    //--- TODO:
    //--- this downloads the scene file, but looks
    //--- like its referenced data don't download
    //--- (at least they don't show up in the cachedir).
    //--- Not sure what's going on.
    //--- Scene shows up with empty nodes created.
    this->MRMLScene->Connect();
    }
}





//----------------------------------------------------------------------------
void vtkFetchMILogic::ClearModifiedNodes()
{
  this->ModifiedNodes.clear();
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::AddModifiedNode( const char *nodeID)
{
  int z = this->ModifiedNodes.size();
  int unique = 1;

  for (int i=0; i < z; i++ )
    {
    if ( !(strcmp(this->ModifiedNodes[i].c_str(), nodeID )))
      {
      unique = 0;
      }
    }


  if ( unique )
    {
    this->ModifiedNodes.push_back(nodeID);
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::RemoveModifiedNode( const char *nodeID)
{
  int z = this->ModifiedNodes.size();
  for (int i=0; i < z; i++ )
    {
    if ( !(strcmp(this->ModifiedNodes[i].c_str(), nodeID )))
      {
      this->ModifiedNodes.erase(this->ModifiedNodes.begin()+i);
      }
    }
}




//----------------------------------------------------------------------------
void vtkFetchMILogic::ClearSelectedStorableNodes ()
{
  this->SelectedStorableNodeIDs.clear();
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::AddSelectedStorableNode( const char *nodeID)
{
  int z = this->SelectedStorableNodeIDs.size();
  int unique = 1;

  for (int i=0; i < z; i++ )
    {
    if ( !(strcmp(this->SelectedStorableNodeIDs[i].c_str(), nodeID )))
      {
      unique = 0;
      }
    }

  if ( unique )
    {
    this->SelectedStorableNodeIDs.push_back(nodeID);
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::RemoveSelectedStorableNode( const char *nodeID)
{
  int z = this->SelectedStorableNodeIDs.size();
  for (int i=0; i < z; i++ )
    {
    if ( !(strcmp(this->SelectedStorableNodeIDs[i].c_str(), nodeID )))
      {
      this->SelectedStorableNodeIDs.erase(this->SelectedStorableNodeIDs.begin()+i);
      }
    }
}




//--- Nicole: this is the serious chunk.
//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceUploadToXND (  )
{
//  this->DebugOn();

  //-------------------------
  // This method sets up the upload of each storable node
  // in the scene (in this->SelectedStorableNodeIDs)
  // and the scene if this->SceneSelected is selected.
  // For all resources, it:
  // * generates metadata
  // * uploads metadata,
  // * uploads data,
  // * generates metadata for scene
  // * uploads metadata for scene
  // * uploads scene.
  // Needs to recover well if for some reason some upload fails,
  // particularly if failure happens within a set of URIListMember uploads.
  //-------------------------
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToXND: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToXND: Null FetchMI node. ");
    return;
    }

  vtkXNDHandler *handler = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XNDHandler" ));
  if ( handler == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToXND: Null URIHandler. ");
    return;
    }
  

  // PASS #0
  // check to make sure all storable nodes have storage nodes with filenames
  // set
  vtkMRMLNode *node;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    node = this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] );
    if (node->GetModifiedSinceRead())
      {
      vtkErrorMacro("vtkFetchMILogic::RequestResourceUploadToXND: error, node " << this->SelectedStorableNodeIDs[n] << " has been modified. Please save all unsaved nodes first");
      return;
      }
    }
  //
  // PASS#1: SET CACHE FILENAMES AND SET URI HANDLER.
  //
  // Get all selected storable nodes from this->SelectedStorableNodeIDs;
  // (This vector of strings is populated by the GUI when upload button is

  // selected)
  vtkMRMLStorableNode *storableNode;
  // get the cache dir
  std::vector<std::string> pathComponents;
  vtksys::SystemTools::SplitPath( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory(), pathComponents);
  
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] ));
    // for each storage node
    int numStorageNodes = storableNode->GetNumberOfStorageNodes();
    vtkMRMLStorageNode *storageNode;
    for (int i = 0; i < numStorageNodes; i++)
      {
      storageNode = storableNode->GetNthStorageNode(i);
      
      // FOR EACH FILENAME & FILELISTMEMBER IN EACH NODE:
      // {
      //--- Set Filename to be cachedir/filename
      // get out the file name
      const char *nodeFileName = storageNode->GetFileName();
      vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName (nodeFileName );
      const char *filename = vtkFileName.c_str();
     
      // add the file name to the cache dir
      pathComponents.push_back(filename);
      // set it
      vtkDebugMacro("RequestResourceUploadToXND: setting file name " << vtksys::SystemTools::JoinPath(pathComponents).c_str() << " to storage node " << storageNode->GetID());
      storageNode->SetFileName(vtksys::SystemTools::JoinPath(pathComponents).c_str());
      //--- If the node is a multivolume, set the filename and all FileListMembers to
      //--- corresponding cachedir/filenames using AddFileName() method.
      // make up a vector of the new file names
      std::vector<std::string> CacheFileNameList;
      for (int filenum = 0; filenum < storageNode->GetNumberOfFileNames(); filenum++)
        {
        const char *nthFilename = storageNode->GetNthFileName(filenum);
        vtkFileName = vtksys::SystemTools::GetFilenameName(nthFilename);
        nthFilename =  vtkFileName.c_str();
        pathComponents.pop_back();
        pathComponents.push_back(nthFilename);
        vtkDebugMacro("RequestResourceUploadToXND: adding file name " << vtksys::SystemTools::JoinPath(pathComponents).c_str() << " to list of new file names");
        CacheFileNameList.push_back(vtksys::SystemTools::JoinPath(pathComponents).c_str());
        }
      // reset the file list
      storageNode->ResetFileNameList();
      // now add the new ones back in
      for (int filenum = 0; filenum < CacheFileNameList.size(); filenum++)
        {
        vtkDebugMacro("RequestResourceUploadToXND: adding file name " << CacheFileNameList[filenum] << " to storage node " << storageNode->GetID());
        storageNode->AddFileName(CacheFileNameList[filenum].c_str());
        }
      //--- Write the file (or multivolume set of files) to cache.
      //--- USE GetNumberOfFileNames to get the number of FileListMembers.
      //--- USE GetNthFileName to get each, probably SetNthFileName to set each.
      //--- If this fails, error message and return.
      
      //--- Set the URIHandler on the storage node
      vtkDebugMacro("RequestResourceUploadToXND: setting handler on storage node " << (storageNode->GetID() == NULL ? "(null)" : storageNode->GetID()));
      storageNode->SetURIHandler(handler);
      // *NOTE: make sure to see that DataIOManagerLogic (or whatever) checks to see
      // if the URIHandler is set before calling CanHandleURI() on all scene handlers,
      // and if not, fix this logic to use the storage node's handler as set.
      // } // end LOOP THRU NODES.
      }
    }
  
  //
  // PASS#2: GENERATE METADATA, UPLOAD METADATA, and SET URIS
  //
  // Loop thru again to generate metadata and upload.
  // Get all selected storable nodes from this->SelectedStorableNodeIDs;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    std::string nodeID = this->SelectedStorableNodeIDs[n];
    vtkDebugMacro("RequestResourceUploadToXND: generating metadata for selected storable node " << nodeID.c_str());
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( nodeID.c_str() ));
    // for each storage node
    int numStorageNodes = storableNode->GetNumberOfStorageNodes();
    vtkMRMLStorageNode *storageNode;
    for (int i = 0; i < numStorageNodes; i++)
      {
      storageNode = storableNode->GetNthStorageNode(i);
      vtkDebugMacro("RequestResourceUploadToXND: have storage node " << i << ", calling write metadata for upload with id " << nodeID.c_str() << " and file name " << storageNode->GetFileName());
      // FOR EACH FILENAME & FILELISTMEMBER IN EACH NODE:
      // {
      //--- call this->WriteMetadataForUpload( filename, nodeID ) on the node
      //--- CHECK RETURN VALUE.
      int retval = this->WriteMetadataForUpload(storageNode->GetFileName(), nodeID.c_str(), handler );
      if (retval == 0)
        {
        vtkErrorMacro("vtkFetchMILogic::RequestResourceUploadToXND: error writing xml for upload, filename = " << storageNode->GetFileName() << ", id = " << nodeID.c_str());
        return;
        }
      //------ if OK:
      //------ call the PostMetadata() method.
      vtkDebugMacro("RequestResourceUploadToXND: calling post meta data.");
      const char *metadataResponse = this->PostMetadata(handler, storageNode->GetFileName());

      vtkDebugMacro("RequestResourceUploadToXND: response from posting = " << (metadataResponse == NULL ? "null" : metadataResponse));
      // parse the return, it's a uri
      const char *uri = this->ParsePostMetadataResponse(metadataResponse);
      //------ Handle bad posts which return NULL -- if the uri is null, abort for the node,
      //------ and set node's URI (and all URIListMembers) to NULL so they won't be staged.
      //------ and deselect the node in the FetchMINode's list
      //------ this->DeselectNode ( nodeID)
      if (uri == NULL)
        {
        vtkErrorMacro("vtkFetchMILogic::RequestResourceUploadToXND: error parsing uri from post meta data call, aborting this storage node " << storageNode->GetID()); // , response = " << metadataResponse);
        storageNode->SetURI(NULL);
        storageNode->ResetURIList();
        this->DeselectNode(nodeID.c_str());
        // bail out of the rest of the storage nodes
        i = numStorageNodes;
        // for now, decrement the node number, since DeselectNode removes an
        // element from the list we're iterating over
        n--;
        }
      else
        {
        vtkDebugMacro("vtkFetchMILogic::RequestResourceUploadToXND: parsed out a return metadatauri : " << uri);
        // then save it in the storage node
        storageNode->SetURI(uri);
        
        // now deal with the rest of the files in the storage node
        int numFiles = storageNode->GetNumberOfFileNames();
        for (int filenum = 0; filenum < numFiles; filenum++)
          {
          if (this->WriteMetadataForUpload(storageNode->GetNthFileName(filenum), nodeID.c_str(), handler) != 1)
            {
            vtkErrorMacro("vtkFetchMILogic::RequestResourceUploadToXND: error writing xml for upload, nth filename = " << storageNode->GetNthFileName(filenum) << ", id = " << nodeID.c_str());
            return;
            }
          //------ if OK:
          //------ call the PostMetadata() method.
          const char *metadataResponse = this->PostMetadata( handler, storageNode->GetNthFileName(filenum) );
          // parse the return, it's a uri
          const char *uri = this->ParsePostMetadataResponse(metadataResponse);
          if (uri == NULL)
            {
            //------ if NOT OK for any individual filename:
            //------ then abort for the node,
            //------ and set node's URI (and all URIListMembers) to NULL so they won't be staged.
            //------ pop up error message giving user chance to continue saving other data, or abort altogether.
            //------ IF CONTINUE
            //------ and deselect the node in the FetchMINode's list this->FetchMINode->DeselectNode ( nodeID)
            //------ ELSE return
            //------ (NOTE: some nodes will have filename changed to cache now. should we
            //------ keep original filenames around and switch them back if
            //------ upload is aborted?)
           
            //---
            //--- Here, instead of bugging user mid-upload, let's
            //--- keep a list of things that failed, and then
            //--- tell user at end.
            //--- Can do this using:
            //--- this->FetchMINode->SetErrorMessage( "the following datasets didn't properly upload: nodeIDA, nodeiDB...");
            //--- this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
            //--- that will trigger the GUI to post the message.
            //--- Can do this here, or at the end of the method...
            //---

            vtkErrorMacro("vtkFetchMILogic::RequestResourceUploadToXND: error parsing uri from post meta data call for file # " << filenum); //, response = " << metadataResponse);
            storageNode->SetURI(NULL);
            storageNode->ResetURIList();
            // ask user
            vtkKWMessageDialog *message = vtkKWMessageDialog::New();
            // TODO: figure out which parent to use
//            message->SetParent ( this->GetParent() );
//            message->SetMasterWindow ( this->SaveDialog );
            message->SetStyleToYesNo();
            std::string msg = "File " + std::string(storageNode->GetNthFileName(filenum)) + " unable to upload to remote host.\nDo you want to continue saving data?";
            message->SetText(msg.c_str());
            message->Create();
            int response = message->Invoke();
            if (response)
              {
              this->DeselectNode(nodeID.c_str());
              // for now, decrement the node number, since DeselectNode removes an
              // element from the list we're iterating over
              n--;
              
              // bail out of the rest of the files
              filenum = numFiles;
              }
            else
              {
              // bail out of the rest of the storage nodes
              i = numStorageNodes;
              }
            }
          else
            {
            vtkDebugMacro("vtkFetchMILogic::RequestResourceUploadToXND: parsed out a return metadatauri : " << uri << ", adding it to storage node " << storageNode->GetID());
            // then save it in the storage node
            storageNode->AddURI(uri);
            }         
          }
        }
      }
    
}
  //
  // PASS#3: STAGE WRITE OF ALL DATA.
  //
  // Now uris are set on all storable nodes marked for save, unless there was a problem
  // with writing or posting their metadata.
  // Get all selected storable nodes from this->SelectedStorableNodeIDs;
  // FOR EACH FILENAME & FILELISTMEMBER IN EACH SELECTED NODE with valid uri:
  // {
  //--- Call StageFileWrite (filename, uri ) and upload.
  // } // end LOOP THRU NODES.
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    std::string nodeID = this->SelectedStorableNodeIDs[n];
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( nodeID.c_str() ));
    // for each storage node
    int numStorageNodes = storableNode->GetNumberOfStorageNodes();
    vtkMRMLStorageNode *storageNode;
    for (int i = 0; i < numStorageNodes; i++)
      {
      storageNode = storableNode->GetNthStorageNode(i);
      if (storageNode->GetURIHandler() == NULL)
        {
        //  we set it to null on error above
        vtkWarningMacro("RequestResourceUploadToXND: not writing on storage node " << i);
        }
      else
        {
        vtkDebugMacro("RequestResourceUploadToXND: caling write data on storage node " << i << ": " << storageNode->GetID());
        if (!storageNode->WriteData(storableNode))
          {
          vtkErrorMacro("RequestResourceUploadToXND: WriteData call failed on storage node " << storageNode->GetID() << " for node " << storableNode->GetName());
          }
        }
      }
    }
  

  //
  // LAST STEP: WRITE SCENE XML AND UPLOAD.
  //
  // Check to see if save scene is selected this->SceneSelected
  if (this->SceneSelected)
    {
    vtkDebugMacro("RequestResourceUploadToXND: uploading scene...");
    // If so write mrml file to cache, include all nodes that have uris AND are selected for upload.
    // (get all selected storable nodes from this->SelectedStorableNodeIDs)
    // --- for now, assume the scene just contains selected nodes for upload

    // set the file name to write the mrml file to cache

    const char *sceneFileName = this->GetMRMLScene()->GetURL();
    vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName (sceneFileName );
    const char *mrmlFileName = vtkFileName.c_str();
    // take the last file off of the path
    pathComponents.pop_back();
    // addthe mrml file
    pathComponents.push_back(mrmlFileName);
    // set the new url
    mrmlFileName = vtksys::SystemTools::JoinPath(pathComponents).c_str();
    vtkDebugMacro("RequestResourceUploadToXND: setting scene url to " << mrmlFileName);
    this->GetMRMLScene()->SetURL(mrmlFileName);
    
    // call this->WriteMetadataForUpload( filename, "MRML") to generate metadata
    //
    int retval = this->WriteMetadataForUpload(mrmlFileName, "MRML", handler);
    if (retval == 1)
      {
      // If return is successful,
      //--- Call handler's PostMetadata() method which returns uri for MRML
      //file.
      const char *response = this->PostMetadata( handler, mrmlFileName );
      const char *uri =  this->ParsePostMetadataResponse(response);
      if (uri != NULL)
        {
        //--- Set scene's URI
        vtkDebugMacro("RequestResourceUploadToXND: setting mrml scene url to " << uri);
        this->GetMRMLScene()->SetURL(uri);
        
        // now upload it
        vtkDebugMacro("RequestResourceUploadToXND: uploading mrml file");
        // hope the hostname has been set...
        handler->StageFileWrite(uri, mrmlFileName);
        }
      else
        {
        vtkErrorMacro("RequestResourceUploadToXND: unable to parse out response from posting metadata for mrml scene, uri is null. Response = " << (response == NULL ? "null" : response));
        return;
        }
      }
    else
      {
      // Otherwise:
      //--- pop up error message
      vtkErrorMacro("RequestResourceUploadToXND: Error writing MRML scene to : " << this->GetMRMLScene()->GetURL());
      return;
      }
    }
//  this->DebugOff();
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceUploadToHID ( )
{
  // no upload capabilities available from HID for now.
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::CreateTemporaryFiles ( )
{
  // This method creates three temporary filenames, using the cache dir, one
  // for holding http from server responses to queries, one
  // for holding xml metadata created for XNAT Desktop uploads,
  // and one for holding mrml scenes downloaded to cache.

  if ( this->GetMRMLScene() != NULL )
    {
    if ( this->GetMRMLScene()->GetCacheManager() != NULL )
      {
      if ( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory() != NULL )
        {
        this->XMLDirName = this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory();
        if ( this->XMLDirName.c_str() != NULL )
          {
          if (vtksys::SystemTools::FileIsDirectory( this->XMLDirName.c_str() ))
            {
            //--- for now, create temporary query response file in cache dir.
            std::vector<std::string> pathComponents;
            vtksys::SystemTools::SplitPath( this->XMLDirName.c_str(), pathComponents);
            // now add the new file name to the end of the path
            pathComponents.push_back("FetchMI_QueryResponse.xml");
            this->HTTPResponseFileName = vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "HTTPResponseFileName = " << this->HTTPResponseFileName );

            //--- for now, create temporary xml metadata description file in
            //--- cache dir.
            pathComponents.pop_back();
            pathComponents.push_back("FetchMI_UploadMetadata.xml");
            this->XMLUploadFileName =  vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "XMLUploadFileName = " << this->XMLUploadFileName );

            //--- for now, create temporary xml response file in
            //--- cache dir.
            pathComponents.pop_back();
            pathComponents.push_back("FetchMI_TemporaryResponse.xml");
            this->TemporaryResponseFileName =  vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "TemporaryResponseFileName = " << this->TemporaryResponseFileName );

            //--- for now, create temporary cache file where downloaded mrml files go.
            pathComponents.pop_back();
            pathComponents.push_back("FetchMI_MRMLCache.xml");
            this->MRMLCacheFileName = vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "MRMLCacheFileName = " << this->MRMLCacheFileName );

            // Wendy: do we need to touch these files, or just create the file names?
            }
          }
        }
      else
        {
        vtkWarningMacro ("RemoteCacheDirectory is not set! Creating files in current dir");
        this->HTTPResponseFileName = std::string("FetchMI_QueryResponse.xml");
        this->XMLUploadFileName = std::string("FetchMI_UploadMetadata.xml");
        this->MRMLCacheFileName = std::string("FetchMI_MRMLCache.xml");
        this->TemporaryResponseFileName = std::string ("FetchMI_TemporaryResponse.xml");
        }
      }
    }
}


