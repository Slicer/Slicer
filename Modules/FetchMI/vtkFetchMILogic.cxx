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

//----------------------------------------------------------------------------
//--- a word about language:
//--- Methods and vars in this module assume that:
//--- "Tag" means a metadata element comprised of an "attribute" (or "keyword") and "value".
//--- Tags may have an attribute with many possible values.
//--- Sometimes "Tag" is used to mean "attribute".
//--- we'll change this eventually to be "Tagname"
//----------------------------------------------------------------------------


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
  this->CurrentURI = NULL;
  this->CurrentQuery = NULL;
  this->ResourceQuery = 0;
  this->TagAttributesQuery = 0;
  this->TagValuesQuery = 0;
  this->ErrorChecking = 0;
  this->ParsingError = 0;
  this->RestoreTagSelectionState = 0;

  // Temporary vars used for parsing xml.
  this->NumberOfElements = 0;
  this->NumberOfTagsOnServer = 0;
//  this->DebugOn();

}




//----------------------------------------------------------------------------
vtkFetchMILogic::~vtkFetchMILogic()
{ 
  this->ClearExistingTagsForQuery();
  this->SetFetchMINode(NULL);
  this->SceneTags->Delete();
  this->ResourceQuery = 0;
  this->TagAttributesQuery = 0;
  this->TagValuesQuery = 0;
  this->ErrorChecking = 0;
  this->ParsingError = 0;
  this->RestoreTagSelectionState = 0;  

  if ( this->CurrentURI != NULL )
    {
    delete [] this->CurrentURI;
    this->CurrentURI = NULL;
    }
  if ( this->CurrentQuery != NULL )
    {
    delete [] this->CurrentQuery;
    this->CurrentQuery = NULL;
    }
  
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "XMLDirName: " << this->XMLDirName.c_str() << "\n";
  os << indent << "HTTPResponseFileName: " << this->HTTPResponseFileName.c_str() << "\n";
  os << indent << "XMLUploadFileName: " << this->XMLUploadFileName.c_str() << "\n";
  os << indent << "XMLDocumentDeclarationFileName: " << this->DocumentDeclarationFileName.c_str() << "\n";
  os << indent << "HeaderFileName: " << this->HeaderFileName.c_str() << "\n";
  os << indent << "TemporaryResponseFileName: " << this->TemporaryResponseFileName.c_str() << "\n";
  os << indent << "MRMLCacheFileName: " << this->MRMLCacheFileName.c_str() << "\n";
  os << indent << "DownloadDirName: " << this->DownloadDirName.c_str() << "\n";
  os << indent << "ResourceQuery: " << this->GetResourceQuery() << "\n";
  os << indent << "TagAttributesQuery: " << this->GetTagAttributesQuery() << "\n";
  os << indent << "TagValuesQuery: " << this->GetTagValuesQuery() << "\n";
  os << indent << "CurrentQuery: " << (this->GetCurrentQuery() == NULL ? "null" : this->GetCurrentQuery()) << "\n";
  os << indent << "CurrentURI: " << (this->GetCurrentURI() == NULL ? "null" : this->GetCurrentURI()) << "\n";
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
const char* vtkFetchMILogic::GetXMLHeaderFileName ( )
{
  return ( this->HeaderFileName.c_str() );
}



//----------------------------------------------------------------------------
const char* vtkFetchMILogic::GetXMLDocumentDeclarationFileName ( )
{
  return ( this->DocumentDeclarationFileName.c_str() );
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

  vtkDebugMacro ("--------------------Querying server for tags.");
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
          
  //--- Always reset the current Query to empty string before query.
  this->SetCurrentQuery(NULL);
  //--- Always reset the number of known tags on the server to 0 before query.
  this->NumberOfTagsOnServer = 0;

  //--- do the right thing based on which kind of webservice:
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
          //--- clear out container holding all tags and their values on server.
          this->ClearExistingTagsForQuery ( );
          //--- then parse new tags into the container.
          this->ParseTagQueryResponse ( );
          }
        }
      else
        {
        vtkErrorMacro ( "XML Response dir not set." );
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIODirectoryErrorEvent );
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::QueryServerForTagValues ( )
{

  vtkDebugMacro ("--------------------Querying server for tag values.");
  //--- Get the selected service.
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }
  
  if ( this->GetHTTPResponseFileName( ) == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: ResponseFileName is not set.");
    return;
    }
  
  const char *svr = this->GetFetchMINode()->GetSelectedServer();
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( (svr == NULL) || (svctype== NULL ))
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype" );
    return;
    }


  //--- Always reset the current Query to empty string before query
  this->SetCurrentQuery(NULL);
  
  if ( !(strcmp ("HID", svctype )) )
    {
    //--- HID
    vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
    if ( h )
      {
      // TODO: find out how to query HID webservices for tags.
      // h->QueryServer ( uri, responseFile );
      }
    }
  else if ( !(strcmp ("XND", svctype)) )
    {
    //--- XND
    vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XNDHandler" ));
    // for each tag attribute in the tag table, get all values represented in the DB.
    if (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ) != NULL)
      {
      vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
      if ( t != NULL )
        {
        const char *att;
        int i;
        // For each tag, get the attribute, and then
        // Ask server for all values for that attribute.
        // Parse the response from the server.
        // Invoke an event on the node that will be caught by GUI
        // In GUI, the value menus for each attribute will be updated
        // to show all current values for each tag in the DB.
        for (i=0; i < this->GetNumberOfTagsOnServer(); i++ )
          {
          att = t->GetTagAttribute(i);
          std::stringstream q;
          q << svr;
          q << "/search??";
          q << att;
          const char *errorString = h->QueryServer ( q.str().c_str(), this->GetHTTPResponseFileName() );
          if ( !strcmp(errorString, "OK" ))
            {
            //--- clear out the container for values for this tagname.
            this->ClearExistingValuesForTag ( att );
            this->ParseTagValuesQueryResponse ( );
            //--- and Update MRML's tagtable. make sure
            //--- each tag's value is in the logic's new list.
            //--- if so, leave it selected. otherwise, reset
            //--- the value to the first value in logic's list
            this->UpdateMRMLQueryTags();
            }
          else
            {
            vtkErrorMacro ( "XML Response dir not set." );
            this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIODirectoryErrorEvent );
            break;
            }
          }
        //--- invoke an event that updates GUI once we have queried for values for all tags.
        vtkDebugMacro ("--------------------Invoking TagResponseReadyEvent.");
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::TagResponseReadyEvent );          
        }
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::UpdateMRMLQueryTags()
{

  const char *svr = this->GetFetchMINode()->GetSelectedServer();
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( (svr == NULL) || (svctype== NULL ))
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype" );
    return;
    }
  //--
    //--- Make sure MRML tag table is compatible with logic's
    //--- list of tags on server.
    //--- For each tag in MRML's tagtable, get the old value.
    //--- Find that tag in the logic's AllValuesForAllTagsOnServer.
    //--- if it's present, then keep its value in MRML.
    //--- if it's not present, then replace the tag's value in MRML
    //--- with the first value in the logic's list.
  //--

  if ( !(strcmp ("HID", svctype )) )
    {
    //--- HID
    vtkHIDTagTable* t = vtkHIDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HIDTags" ));
    if ( t != NULL )
      {
      //--- TODO: not sure how this will work with HID yet.
      }
    }
  else if ( !(strcmp ("XND", svctype)) )
    {
    //--- XND
    vtkXNDTagTable* t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
    std::map<std::string, std::vector<std::string> >::iterator iter;
    const char *att;
    const char *val;
    if ( t != NULL )
      {
      int numTagsInMRML = t->GetNumberOfTags();
      //--- for each tag in mrml...
      for (int i=0; i < numTagsInMRML; i++ )
        {

        //--- find matching tag in AllValuesForTag
        att = t->GetTagAttribute ( i );
        int foundAtt = 0;
        for ( iter = this->AllValuesForAllTagsOnServer.begin();
              iter != this->AllValuesForAllTagsOnServer.end();
              iter++ )
          {
          if ( !strcmp(iter->first.c_str(),att ))
            {
            //--- found tag in logic. see if value is present.
            foundAtt = 1;
            val = t->GetTagValue ( i );
            int foundVal = 0;
            for ( unsigned int j=0; j < iter->second.size(); j++ )
              {
              if ( !(strcmp(iter->second[j].c_str(), val) ))
                {
                foundVal = 1;
                break;
                }
              }
            if ( !foundVal )
              {
              //--- tag value in MRML is no longer available. clear out.
              //--- and deselect.
              if ( iter->second.size() == 0 )
                {
                t->AddOrUpdateTag ( att, "no values found", 0 );
                }
              else
                {
                t->AddOrUpdateTag ( att, iter->second[0].c_str(), 0 );
                }
              }
            }
          if ( foundAtt )
            {
            break;
            }
          } //end iter
        }
      }
    }
}


//----------------------------------------------------------------------------
int vtkFetchMILogic::DeleteResourceFromServer ( const char *uri )
{
  int retval = 1;

  // check for which server
  // send delete request.
  const char *svr = this->GetFetchMINode()->GetSelectedServer();
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return 0;
    }

  if ( (svr == NULL) || (svctype== NULL ))
    {
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype. Set server before querying." );
    return 0;
    }
  // put up wait dialog...
  // now update the resource list in the GUI.
  if ( !(strcmp ("HID", svctype )) )
    {
    //---
    //--- HID
    //---
    vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HIDHandler" ));
    vtkHIDTagTable* t = vtkHIDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HIDTags" ));
    if ( h && t )
      {
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
        retval = h->DeleteResource ( uri, this->GetHTTPResponseFileName() );
        }
      }
    }
  return ( retval );
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
    vtkErrorMacro ("vtkFetchMILogic: Null server or servicetype. Set server before querying." );
    return;
    }


  std::string att;
  std::string val;
  std::stringstream q;
  
  //--- Always reset the current Query to empty string
  this->SetCurrentQuery(NULL);
  
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
        //--- TODO: develop all HID logic. Now, not finished.
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
        //--- and a value, and both are not "", "NULL" or "no values found"
        //--- etc.
        int num = t->GetNumberOfTags();

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
                     (strcmp(val.c_str(), "no values found")) &&
                     (strcmp(val.c_str(), "No values found")) &&
                     (strcmp(val.c_str(), "" )))
                  {
                  q << att;
                  q << "=";
                  q << val;
                  q << "&";
                  }
                }
              }
            }
          }
        //---
        //--- TODO: trim off the last ampersand in the query string!!!
        //---
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
  int numAtts;
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
    numAtts = element->GetNumberOfAttributes();
    for ( int i=0; i < numAtts; i++ )
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
                            this->SetCurrentURI(value);
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

  if ( name == NULL )
    {
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }

  const char *attName;
  int numAtts;
  const char *value;
  const char *dtype = "unknown";
  const char *tagValue;
  vtkXMLDataElement *nestedElement;


  // check to see if this is an item of interest.
  // process appropriately, based on what elements we find.

  //---
  // THE QUERY -- not all things produce a query attribute in the xml... get in case we want it...
  //---
  if ( !(strcmp(name, "SearchResults" )) )
    {
    // parse the xml file and put the Query into this->CurrentQuery
    // NOTE: we don't yet use this because some queries don't return an XML element named
    // "SearchResults", like the query for tags. If all responses contained this element, then
    // we could use it to parse in more targetted way.
    numAtts = element->GetNumberOfAttributes();
    for ( int j=0; j < numAtts; j++ )
      {
      attName = element->GetAttributeName(j);        
      value = element->GetAttributeValue(j);
      if ( attName != NULL && value != NULL )
        {
        if ( !strcmp (attName, "Query"))
          {
          this->SetCurrentQuery (value);
          }
        }
      }
    }

  //---
  //--- ERROR CHECKING
  //---
  if ( this->ErrorChecking)
    {
    if ((!(strcmp(name, "title" ))) ||
        (!(strcmp(name, "Title" ))) )
      {
      std::string errorString = element->GetCharacterData();
      std::string target = "error";
      size_t found = errorString.find ( target );
      if ( found != std::string::npos)
        {
        this->ParsingError = 1;
        }
      target = "Error";
      if ( found != std::string::npos)
        {
        this->ParsingError = 1;
        }
      target = "ERROR";
      if ( found != std::string::npos)
        {
        this->ParsingError = 1;
        }
      }
    }

  
  //---
  // RESOURCES have nested uris and tags (attribute/value pairs)
  // ONLY get these if the ResourceQuery flag is on. Otherwise query was
  // for tags or tag values, so ignore.
  //---
  if ( this->ResourceQuery )
    {    
    if ((!(strcmp(name, "resource" ))) ||
        (!(strcmp(name, "Resource" ))) ||
        (!(strcmp(name, "RESOURCE" ))) )
      {
      // parse the xml file and put result into ResourceDescription table.
      // looking for the SlicerDataType to pair with uri
      this->SetCurrentURI (NULL);
      int nnested = element->GetNumberOfNestedElements();

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
            this->SetCurrentURI(value);
            this->FetchMINode->GetResourceDescription()->AddOrUpdateTag ( this->CurrentURI, dtype, 0 );        
            }
          }

        //--- get data type
        if (((!(strcmp(nestedElement->GetName(), "tag" ))) ||
             (!(strcmp(nestedElement->GetName(), "Tag" ))) ||
             (!(strcmp(nestedElement->GetName(), "TAG" )))) &&
            ( this->CurrentURI != NULL ) )
          {
          // see if the attribute in this tag is called Label, and if its value is SlicerDataType
          numAtts = nestedElement->GetNumberOfAttributes();
          for ( int j=0; j < numAtts; j++ )
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
                      this->SetCurrentURI (NULL);
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


  //---
  // TAGS -- expect to find a list of elements called label containing tag attribute names.
  //---
  if (  this->TagAttributesQuery )
    {
    if ( (!(strcmp(name, "Label" ))) ||
         (!(strcmp(name, "label" ))) )
      {
      // parse the xml file and put result into node's XND tag table.
      // and into logic's AllValuesForAllTags map.
      value = element->GetCharacterData();
      vtkXNDTagTable* t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
      //--- we are choosing to set SlicerDataType a default
      //--- value of 'MRML' in the XNDTagTable.
      //--- This promotes tagging of scenes with a
      //--- consistent attribute/value, and searching for scenes
      //--- with that consistent tag.
      if ( t != NULL && value != NULL )
        {
        //--- add to logic's map of tag attributes and all values
        this->AddUniqueTag ( value );
        //--- Add to node's tag table of tag attributes.
        //--- Keep selection status if we are just refreshing the
        //--- SAME server's metadata, and tag's already present.
        //--- If a NEW server has been selected, then RestoreTagSelectionState = 0
        //--- and we should just call AddOrUpdate with the new tag.
        //---
        //--- NOTE: this behavior may be friendly or annoying;
        //--- not sure. If we don't want to restore the tag's
        //--- selected state, then just get rid of the check on
        //--- RestoreTagSelectionState and always call AddOrUpdateTag
        //---
        if ( this->GetRestoreTagSelectionState() )
          {
//            if ( t->IsTagSelected(value) <= 0 )            
          if ( t->CheckTableForTag(value) < 0 )
            {
            //--- no need to update tag value or selected state.
            t->AddOrUpdateTag ( value, "no values found", 0 );
            }
          }
        else
          {
          //--- otherwise leave tag attribute, value, selected state as is.
          t->AddOrUpdateTag ( value, "no values found", 0 );
          }
        }
      }
    }

  
    //---
    // TAG VALUES -- expect long list of resources with nested
    // uris that we ignore here, and then the target list of tag
    // VALUEs for a given ATTRIBUTE at end.
    //---
    //--- Look for this guy:
    //---<Tag Label="ATTRIBUTE">
    //---<Value>VALUE_1</Value>
    //---<Value>VALUE_2</Value>
    //---</Tag>
    //---
  if ( this->TagValuesQuery )
    {
    if (((!(strcmp(name, "tag" ))) ||
         (!(strcmp(name, "Tag" ))) ||
         (!(strcmp(name, "TAG" )))) )
      {
      numAtts = element->GetNumberOfAttributes();
      for ( int j=0; j < numAtts; j++ )
        {
        attName = element->GetAttributeName(j);        
        value = element->GetAttributeValue(j);
        if ( attName != NULL && value != NULL )
          {
          if ( !strcmp(attName, "Label" ))
            {
            //--- get all nested values.
            int numnested = element->GetNumberOfNestedElements();
            vtkXMLDataElement *nestedElement;
            //--- get tag value
            for ( int i=0; i < numnested; i++)
              {
              nestedElement = element->GetNestedElement ( i );
              if (nestedElement != NULL)
                {
                if (!(strcmp(nestedElement->GetName(), "Value")) ||
                    !(strcmp(nestedElement->GetName(), "value")) )
                  {
                  tagValue = nestedElement->GetCharacterData();
                  this->AddUniqueValueForTag ( value, tagValue );
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
void vtkFetchMILogic::ParseTagValuesQueryResponse ( )
{
    
  vtkDebugMacro ("--------------------Parsing tag values query response.");
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
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  if ( !(strcmp ("HID", svctype )) )
    {
    //--- HID
    }
  else if ( !(strcmp ("XND", svctype)) )
    {
    //--- XND
    if ( this->GetHTTPResponseFileName() != NULL )
      {
      //--- check for file.
      vtkXMLDataParser *parser = vtkXMLDataParser::New();
      parser->SetFileName ( this->GetHTTPResponseFileName() );
      parser->SetIgnoreCharacterData ( 0 );
      int retval = parser->Parse();
      //--- throw an error event if parsing failed.
      if ( retval == 0 )
        {
        if ( parser )
          {
          parser->Delete();
          }
        this->FetchMINode->SetErrorMessage ("Unable to parse tag value query response.");
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
        return;
        }

      //--- first check for errors
      this->NumberOfElements = 0;
      vtkXMLDataElement *root = parser->GetRootElement();
      this->ErrorChecking = 1;
      this->ParsingError = 0;
      if ( root != NULL )
        {
        this->GetNumberOfXMLElements( root );
        if ( this->NumberOfElements > 0 )
          {
          root = parser->GetRootElement();
          this->GetXMLElement ( root );
          }
        this->ErrorChecking = 0;
        if ( this->ParsingError )
          {
          this->FetchMINode->SetErrorMessage ("The server returned an error response to the query for its tags.");
          this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
          if ( parser )
            {
            parser->Delete();
            }
          this->ParsingError = 0;
          return;
          }
        }

      //--- Turn TagValuesQuery ON so parser knows what to parse
      this->TagValuesQuery = 1;
      //---
      this->NumberOfElements = 0;
      root = parser->GetRootElement();
      if ( root != NULL )
        {
        this->GetNumberOfXMLElements( root );
        if ( this->NumberOfElements > 0 )
          {
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

      //--- Turn TagValuesQuery OFF
      this->TagValuesQuery = 0;
      //---

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
void vtkFetchMILogic::ParseTagQueryResponse ( )
{

  vtkDebugMacro ("--------------------Parsing tag query response.");
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }

  if ( this->FetchMINode->GetTagTableCollection() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode's TagTableCollection is NULL.");
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
    vtkHIDTagTable *t = vtkHIDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "HIDTags" ));
    //--- if no need to keep tag selection state around, then
    //--- clear out mrml tag table to be refilled by this query.
    if ( t != NULL )
      {
      if ( this->GetRestoreTagSelectionState() == 0 )
        {
          t->ClearTagTable();
          }
      }

    }
  else if ( !(strcmp ("XND", svctype)) )
    {
    //--- XND
    //--- look at xml file to see if there's an error message
    //--- parse the error and fill the node's error message.
    //--- if no, parse response into table.
    vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));

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

      //--- first check for errors
      this->NumberOfElements = 0;
      vtkXMLDataElement *root = parser->GetRootElement();
      this->ErrorChecking = 1;
      this->ParsingError = 0;
      if ( root != NULL )
        {
        this->GetNumberOfXMLElements( root );
        if ( this->NumberOfElements > 0 )
          {
          root = parser->GetRootElement();
          this->GetXMLElement ( root );
          }
        this->ErrorChecking = 0;
        if ( this->ParsingError )
          {
          this->FetchMINode->SetErrorMessage ("The server returned an error response to the query for all known values for its tags.");
          this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
          if ( parser )
            {
            parser->Delete();
            }
          this->ParsingError = 0;
          return;
          }
        }

      //--- If no need to keep tag selection state around, then
      //--- clear out mrml tag table to be refilled by this query.
      if ( this->GetRestoreTagSelectionState() == 0 )
        {
        t->ClearTagTable();
        }
      
      //--- Turn TagAttributesQuery ON to help in pulling out XML elements.
      this->TagAttributesQuery = 1;
      //---

      this->NumberOfElements = 0;
      root = parser->GetRootElement();
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

      //--- Turn TagAttributesQuery OFF
      this->TagAttributesQuery = 0;
      //---

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

    //--- first check for errors
    this->NumberOfElements = 0;
    vtkXMLDataElement *root = parser->GetRootElement();
    this->ErrorChecking = 1;
    this->ParsingError = 0;
    if ( root != NULL )
      {
      this->GetNumberOfXMLElements( root );
      if ( this->NumberOfElements > 0 )
        {
        root = parser->GetRootElement();
        this->GetXMLElement ( root );
        }
      this->ErrorChecking = 0;
      if ( this->ParsingError )
        {
        this->FetchMINode->SetErrorMessage ("The server returned an error response to the query for matching resources.");
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
        if ( parser )
          {
          parser->Delete();
          }
        this->ParsingError = 0;
        return;
        }
      }

    //--- make sure ResourceQuery is ON to enable parsing of URIs.
    this->ResourceQuery = 1;
    //---

    this->NumberOfElements = 0;
    root = parser->GetRootElement();
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

    //--- make sure ResourceQuery is OFF 
    this->ResourceQuery = 0;
    //---
    
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
int vtkFetchMILogic::PostMetadataToXND ( vtkXNDHandler *handler, const char *dataFilename )
{
  // return 1 if OK, 0 if not.
  int returnval = 0;
  
  if ( handler == NULL )
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND got a NULL handler" );
    return returnval;
    }
  if (this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND got a NULL FetchMINode" );
    return returnval;
    }
  if (this->GetXMLUploadFileName() == NULL)
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND Got NULL file for Metadata upload.");
    return returnval;
    }
  if ( this->GetXMLHeaderFileName() == NULL )
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND Got NULL file for Metadata upload.");
    return returnval;    
    }
  if (this->GetTemporaryResponseFileName() == NULL)
    {
    vtkErrorMacro ( "FetchMILogic: PostMetadataToXND got NULL file for Metadata upload.");
    return returnval;
    }

  // set particular XND host in the XNDhandler
  std::string svr = this->GetFetchMINode()->GetSelectedServer();
  std::string::size_type index =  svr.find("://", 0);
  if ( index  != std::string::npos)
    {
    std::string hostname = svr.substr( index+3, std::string::npos );
    handler->SetHostName(hostname.c_str());
    }
     
  
  // set the post destination for data being described
  std::stringstream ss;
  ss << svr.c_str();
  ss << "/data";
  returnval = handler->PostMetadata ( ss.str().c_str(), this->GetXMLHeaderFileName(), dataFilename, this->GetXMLUploadFileName(), this->GetTemporaryResponseFileName() );
  return (returnval);
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
void vtkFetchMILogic::ClearExistingTagsForQuery ( )
{

  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->AllValuesForAllTagsOnServer.begin();
        iter != this->AllValuesForAllTagsOnServer.end();
        iter++ )
    {
    iter->second.clear();
    }
  this->AllValuesForAllTagsOnServer.clear();
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::ClearExistingValuesForTag ( const char *tagname )
{

  //--- first find the tag, then clear all the values.
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->AllValuesForAllTagsOnServer.begin();
        iter != this->AllValuesForAllTagsOnServer.end();
        iter++ )
    {
    if ( !(strcmp (iter->first.c_str(), tagname) ) )
      {
      iter->second.clear();
      break;
      }
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::AddUniqueTag ( const char *tagname )
{
  //--- search thru vector of strings to see if we can find the tag.
  int unique = 1;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->AllValuesForAllTagsOnServer.begin();
        iter != this->AllValuesForAllTagsOnServer.end();
        iter++ )
    {
    if ( ! (strcmp (iter->first.c_str(), tagname ) ) )
      {
      unique = 0;
      break;
      }
    }

  //--- add tag if it's not already in list.
  if ( unique )
    {
    // TODO: check to see if this works
    // create and initialize a vector of strings 
    std::vector<std::string> v;
    v.clear();
    // create an entry for it in the map.
    this->AllValuesForAllTagsOnServer.insert ( std::make_pair (tagname, v ));
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::AddUniqueValueForTag ( const char *tagname, const char *val )
{
  //--- search thru vector of strings for tag to see if we can find the tag.
  int tagFound = 0;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->AllValuesForAllTagsOnServer.begin();
        iter != this->AllValuesForAllTagsOnServer.end();
        iter++ )
    {
    if ( ! (strcmp (iter->first.c_str(), tagname ) ) )
      {
      tagFound=1;
      break;
      }
    }

  //--- add value for tag if it's not already in list.
  if ( tagFound )
    {
    int valFound = 0;
    for (int i=0; i < iter->second.size(); i++ )
      {
      if ( ! (strcmp(iter->second[i].c_str(), val)) )
        {
        valFound = 1;
        break;
        }
      }
    if ( !valFound )
      {
      iter->second.push_back ( val );
      }
    }
}




//----------------------------------------------------------------------------
int vtkFetchMILogic::GetNumberOfTagValues( const char *tagname)
{

  int numValues = 0;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->AllValuesForAllTagsOnServer.begin();
        iter != this->AllValuesForAllTagsOnServer.end();
        iter++ )
    {
    if ( ! (strcmp (iter->first.c_str(), tagname ) ) )
      {
      int numValues = iter->second.size();
      break;
      }
    }
  if ( numValues <= 0 )
    {
    return 0;
    }
  else
    {
    return (numValues);
    }
}


//----------------------------------------------------------------------------
int vtkFetchMILogic::GetNumberOfTagsOnServer ( )
{

  int numTags = this->AllValuesForAllTagsOnServer.size();
  return ( numTags );

}



//----------------------------------------------------------------------------
const char * vtkFetchMILogic::GetNthTagAttribute ( int n)
{

  int cnt = 0;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->AllValuesForAllTagsOnServer.begin();
        iter != this->AllValuesForAllTagsOnServer.end();
        iter++ )
    {
    if ( cnt < n )
      {
      cnt++;
      }
    else
      {
      return ( iter->first.c_str() );
      }
    }
  return ( NULL );

}


//----------------------------------------------------------------------------
const char * vtkFetchMILogic::GetNthValueForTag(const char *tagname, int n)
{

  int numValues = 0;
  std::map<std::string, std::vector<std::string> >::iterator iter;
  for ( iter = this->AllValuesForAllTagsOnServer.begin();
        iter != this->AllValuesForAllTagsOnServer.end();
        iter++ )
    {
    if ( ! (strcmp (iter->first.c_str(), tagname ) ) )
      {
      if ( n <= iter->second.size()-1 )
        {
        return ( iter->second[n].c_str() );
        }
      else
        {
        return NULL;
        }
      }
    }
  return NULL;
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

//---
//--- TODO: right now only allow MRML scene downloads. Generalize this.
//---
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
  

  //--- TODO: make this work!
  //--- make sure we have a data type we know how to handle.
/*
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



//----------------------------------------------------------------------------
int vtkFetchMILogic::CheckStorageNodeFileNames()
{
  // Before upload, check to make sure all storable nodes have
  // set storage nodes with filenames

  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::CheckStorageNodeFileNames: Null scene. ");
    return(0);
    }

  vtkMRMLNode *node;
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    node = this->GetMRMLScene()->GetNodeByID( this->SelectedStorableNodeIDs[n] );
    if (node->GetModifiedSinceRead())
      {
      vtkErrorMacro("vtkFetchMILogic::CheckStorageNodeFileNames: error, node " << this->SelectedStorableNodeIDs[n] << " has been modified. Please save all unsaved nodes first");
      return (0);
      }
    }
  return (1);
  
}

//----------------------------------------------------------------------------
void vtkFetchMILogic::SetCacheFileNamesAndXNDHandler(vtkXNDHandler *handler)
{
  //---
  // Get all selected storable nodes from this->SelectedStorableNodeIDs;
  // (This vector of strings is populated by the GUI when upload button is
  // selected)
  //---

  if ( handler == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::SetCacheFileNamesAndURIHandler: Null URIHandler. ");
    return;
    }
    if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::SetCacheFileNamesAndURIHandler: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::SetCacheFileNamesAndURIHandler: Null FetchMI node. ");
    return;
    }

  // get the cache dir
  std::vector<std::string> pathComponents;
  vtksys::SystemTools::SplitPath( this->GetMRMLScene()->GetCacheManager()->GetRemoteCacheDirectory(), pathComponents);

  //--- scene
  if ( this->SceneSelected )
    {
    vtksys_stl::string sceneFileName = this->GetMRMLScene()->GetURL();
    vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName (sceneFileName );
    const char *mrmlFileName = vtkFileName.c_str();
    if ( !mrmlFileName || (!strcmp(mrmlFileName, "" )) )
      {
      mrmlFileName = "SlicerScene1.mrml";
      }
    
    // addthe mrml file
    pathComponents.push_back(mrmlFileName);
    // set the new url
    vtksys_stl::string tmp = vtksys::SystemTools::JoinPath(pathComponents);
    mrmlFileName = tmp.c_str();
    vtkDebugMacro("RequestResourceUploadToXND: setting scene url to " << mrmlFileName);
    this->GetMRMLScene()->SetURL(mrmlFileName);
    }


  //--- storable nodes
  vtkMRMLStorableNode *storableNode;
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
      vtksys_stl::string nodeFileName = storageNode->GetFileName();
      vtksys_stl::string vtkFileName = vtksys::SystemTools::GetFilenameName (nodeFileName );
      const char *filename = vtkFileName.c_str();
     
      // add the file name to the cache dir
      pathComponents.pop_back();
      pathComponents.push_back(filename);
      // set it
      vtkDebugMacro("SetCacheFileNamesAndURIHandler: setting file name " << vtksys::SystemTools::JoinPath(pathComponents).c_str() << " to storage node " << storageNode->GetID());
      vtksys_stl::string tmp = vtksys::SystemTools::JoinPath(pathComponents);
      storageNode->SetFileName( tmp.c_str() );
      //--- If the node is a multivolume, set the filename and all FileListMembers to
      //--- corresponding cachedir/filenames using AddFileName() method.
      // make up a vector of the new file names
      std::vector<std::string> CacheFileNameList;
      for (int filenum = 0; filenum < storageNode->GetNumberOfFileNames(); filenum++)
        {
        vtksys_stl::string nthFilename = storageNode->GetNthFileName(filenum);
        vtkFileName = vtksys::SystemTools::GetFilenameName(nthFilename);
        nthFilename =  vtkFileName.c_str();
        pathComponents.pop_back();
        pathComponents.push_back(nthFilename);
        vtkDebugMacro("SetCacheFileNamesAndURIHandler: adding file name " << vtksys::SystemTools::JoinPath(pathComponents).c_str() << " to list of new file names");
        vtksys_stl::string ttmp = vtksys::SystemTools::JoinPath(pathComponents);
        CacheFileNameList.push_back(ttmp.c_str());
        }
      // reset the file list
      storageNode->ResetFileNameList();
      // now add the new ones back in
      for (int filenum = 0; filenum < CacheFileNameList.size(); filenum++)
        {
        vtkDebugMacro("SetCacheFileNamesAndURIHandler: adding file name " << CacheFileNameList[filenum] << " to storage node " << storageNode->GetID());
        storageNode->AddFileName(CacheFileNameList[filenum].c_str());
        }
      //--- Write the file (or multivolume set of files) to cache.
      //--- USE GetNumberOfFileNames to get the number of FileListMembers.
      //--- USE GetNthFileName to get each, probably SetNthFileName to set each.
      //--- If this fails, error message and return.
      
      //--- Set the URIHandler on the storage node
      vtkDebugMacro("SetCacheFileNamesAndURIHandler: setting handler on storage node " << (storageNode->GetID() == NULL ? "(null)" : storageNode->GetID()));
      storageNode->SetURIHandler(handler);
      // *NOTE: make sure to see that DataIOManagerLogic (or whatever) checks to see
      // if the URIHandler is set before calling CanHandleURI() on all scene handlers,
      // and if not, fix this logic to use the storage node's handler as set.
      // } // end LOOP THRU NODES.
      }
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::PostStorableNodesToXND()
{
  // Get all selected storable nodes from this->SelectedStorableNodeIDs;
  // FOR EACH FILENAME & FILELISTMEMBER IN EACH SELECTED NODE with valid uri:

  vtkMRMLStorableNode *storableNode;
  vtkMRMLStorageNode *storageNode;
  
  for (unsigned int n = 0; n < this->SelectedStorableNodeIDs.size(); n++)
    {
    std::string nodeID = this->SelectedStorableNodeIDs[n];
    storableNode = vtkMRMLStorableNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID( nodeID.c_str() ));
    // for each storage node
    int numStorageNodes = storableNode->GetNumberOfStorageNodes();

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
}



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------




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

            //--- for now, create temporary xml document description in
            //--- cache dir.
            pathComponents.pop_back();
            pathComponents.push_back("FetchMI_DocumentDeclaration.xml");
            this->DocumentDeclarationFileName =  vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "XMLDocumentDeclarationFileName = " << this->DocumentDeclarationFileName );
            
            //--- for now, create temporary xml header in
            //--- cache dir.
            pathComponents.pop_back();
            pathComponents.push_back("FetchMI_UploadHeader.xml");
            this->HeaderFileName =  vtksys::SystemTools::JoinPath(pathComponents);
            vtkDebugMacro ( "XMLHeaderFileName = " << this->HeaderFileName );

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


