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
  this->SceneSelected = 1;
  // used for parsing xml.
  this->NumberOfElements = 0;
}




//----------------------------------------------------------------------------
vtkFetchMILogic::~vtkFetchMILogic()
{ 
  this->SetFetchMINode(NULL);
  this->SceneTags->Delete();
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::PrintSelf(ostream& os, vtkIndent indent)
{
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
  this->SceneSelected = 1;
  this->InvokeEvent (vtkMRMLFetchMINode::SaveSelectionEvent );
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::DeselectScene()
{
  this->SceneSelected = 0;
  this->InvokeEvent (vtkMRMLFetchMINode::SaveSelectionEvent );
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
        vtksys_stl::stringstream q;
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
    vtkXNDTagTable* t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
    if ( h && t )
      {
      //--- Check local file to receive response.
      if ( this->GetHTTPResponseFileName( ) )
        {
        //--- Build query
        vtksys_stl::stringstream q;
        q << svr << "/search?";

        //--- TODO: is there a better way to filter out weird tags?
        //--- Append all selected XND tags that are reasonably
        //--- formed. That means they have both an attribute
        //--- and a value, and both are not "", "NULL" or "none"
        //--- etc.
        int num = t->GetNumberOfTags();
        const char *att;
        const char *val;
        for ( int i =0; i<num; i++)
          {
          att = t->GetTagAttribute(i);
          val = t->GetTagValue(i);
          if ( t->IsTagSelected(att))
            {
            //--- do some checking...
            if ( att != NULL && val != NULL )
              {
              if ( (strcmp(att, "NULL")) &&
                   (strcmp(att, "null")) &&
                   (strcmp(att, "<none>")) &&
                   (strcmp(att, "none")) &&
                   (strcmp(att, "" )))
                {
                if ( (strcmp(val, "NULL")) &&
                   (strcmp(val, "null")) &&
                   (strcmp(val, "<none>")) &&
                   (strcmp(val, "none")) &&
                   (strcmp(val, "" )))
                  {
                  q << "&";
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

  this->GetXMLEntry ( element );
  
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
void vtkFetchMILogic::GetXMLEntry( vtkXMLDataElement *element )
{

  if (element ==  NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: GetXMLEntry called with null vtkXMLDataElement.");
    return;
    }

  const char *name = element->GetName();
  const char *attName;
  const char *value;
  const char *svctype;
  const char *dtype = "<unknown>";
;
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

  svctype = this->GetFetchMINode()->GetSelectedServiceType();

  //---
  // TAGS
  //---
  if ( (!(strcmp(name, "Label" ))) ||
       (!(strcmp(name, "label" ))) )
    {

    // parse the xml file and put result into node's XND tag table.
    if ( !(strcmp ("XND", svctype )) )
      {
      //--- we are filling up the Query tag table here.
      //--- tag values should be set by user.
      value = element->GetCharacterData();
      vtkXNDTagTable* t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XNDTags" ));
      t->AddOrUpdateTag ( value, "<none>", 0 );
      }
    else if (!(strcmp ("HID", svctype )) )
      {
      // no-op
      }
    }
  

  //---
  // RESOURCES
  //---
  if ((!(strcmp(name, "uri" ))) ||
      (!(strcmp(name, "Uri" ))) ||
      (!(strcmp(name, "URI" ))) )
    {

    this->CurrentURI = "";
    // parse the xml file for the uri 
    if ( !(strcmp ("XND", svctype )) )
      {
      // uri
      value = element->GetCharacterData();
      if ( value != NULL )
        {
        this->CurrentURI = value;
        this->FetchMINode->GetResourceDescription()->AddOrUpdateTag ( this->CurrentURI, dtype, 0 );        
        }
      }
    else if (!(strcmp ("HID", svctype )) )
      {
      // no-op
      }
    }



  //---
  // SLICER DATA TYPE
  //---
  if ((!(strcmp(name, "tag" ))) ||
      (!(strcmp(name, "Tag" ))) ||
      (!(strcmp(name, "TAG" ))) )
    {
    // parse the xml file and put result into ResourceDescription table.
    // looking for the SlicerDataType to pair with uri
    if ( !(strcmp ("XND", svctype )) )
      {
      // see if the attribute in this tag is called Label, and if its value is SlicerDataType
      int numatts = element->GetNumberOfAttributes();
      for ( int i=0; i < numatts; i++ )
        {
        attName = element->GetAttributeName(i);        
        value = element->GetAttributeValue(i);
        if ( !strcmp(value, "SlicerDataType" ) )
          {
          // if yes, then get the nested Value Element          
          int nnested = element->GetNumberOfNestedElements();
          vtkXMLDataElement *nestedElement = element;
          for ( int j=0; j < nnested; j++)
            {
            nestedElement = nestedElement->GetNestedElement ( j );
            value = nestedElement->GetName();
            if ( !(strcmp (value, "Value" )) ||
                 !(strcmp (value, "value" )) )
              {
              dtype = nestedElement->GetCharacterData();              
              break;              
              }
            }          
          break;
          }
        }
      //now pair it up with CurrentURI and add to ResourceDescription.
      this->FetchMINode->GetResourceDescription()->AddOrUpdateTag ( this->CurrentURI, dtype, 0 );
      this->CurrentURI = "";
      }
    else if (!(strcmp ("HID", svctype )) )
      {
      // no-op
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

  const char *svr = this->GetFetchMINode()->GetSelectedServer();
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
    
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
        this->FetchMINode->ErrorMessage = "Unable to parse tag query response.";
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
        this->FetchMINode->ErrorMessage = "Unable to parse tag query response. Displayed list of tags has not been updated.";
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
        }
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::TagResponseReadyEvent );
      if ( parser )
        {
        parser->Delete();
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

  const char *svr = this->GetFetchMINode()->GetSelectedServer();
  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();

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
    }
  else if ( !(strcmp ("XND", svctype)) )
    {
    //--- XND
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
        this->FetchMINode->ErrorMessage = "Unable to parse tag query response.";
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
        this->FetchMINode->ErrorMessage = "Unable to parse tag query response. Displayed list of tags has not been updated.";
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
        }
      if ( parser )
        {
        parser->Delete();
        }
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::ResourceResponseReadyEvent );
      }
    }
}


//----------------------------------------------------------------------------
void vtkFetchMILogic::PostMetadata ( )
{
}


//----------------------------------------------------------------------------
const char*vtkFetchMILogic::ParsePostMetadataResponse ( )
{
  return ( NULL );
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
// ID is a
//----------------------------------------------------------------------------
int vtkFetchMILogic::WriteXMLForUpload ( const char *filename, const char *ID)
{

  //--- Check to see if ID == MRMLScene.
  //------ if YES:
  //------ check to see if the scene is tagged for upload in
  //------ this->SceneTags (File_Type attribute should be set to MRML
  //------ by default at least). If not, set (File_Type attribute to MRML in this->SceneTags.
  //------ Write XML for the scene file using this->SceneTags
  
  //------ if NO,
  //------ Check to see if the ID is a storable node in scene.
  //------ if NO, return 0.
  //------ Check to see if the node is tagged for upload
  //------ if not, return 0.
  //------ Write the xml description for each filename
  //------ into file with filename=this->XMLUploadFileName
  //------ return 1 if file writes ok, else return 0
  return (1);
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
  //--- we'll have to have separate methods for each
  if ( !(strcmp ("XND", svctype )) )
    {
    this->RequestResourceUploadToXND();
    }
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceDownload ( const char *uri, const char *nodeType)
{

  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }

  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  //--- we'll have to have separate methods for each
  if ( !(strcmp ("XND", svctype )) )
    {
    this->RequestResourceDownloadFromXND( uri, nodeType );
    }
  if ( !(strcmp ("HID", svctype )) )
    {
    this->RequestResourceDownloadFromHID( uri, nodeType );
    }
}





//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceDownloadFromHID ( const char *uri, const char *nodeType )
{
  // This method will download resource with uri
  // from the currently selected remote repository.
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToHID: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToHID: Null FetchMI node. ");
    return;
    }

  vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HID" ));
  if ( h == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToHID: Null URIHandler. ");
    return;
    }

  if ( !(strcmp(nodeType, "Volume" )))
    {
    //--- create the node
    //--- set its URIhandler
    //--- download to cache and load.
    }
  if ( !(strcmp(nodeType, "Model" )))
    {
    //--- create the node
    //--- set its URIhandler
    //--- download to cache and load.
    }
  
}

//---------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------


// Nicole: can you take a stab? Assume we know the nodeType.
// however, if we just have a raw uri, not sure we'll know it's
// nodeType. we may be able to infer from File_Type tag, but
// not sure yet.
//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceDownloadFromXND ( const char *uri, const char *nodeType )
{
  // This method will download resource with uri
  // from the currently selected remote repository.
  
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

  vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XND" ));
  if ( h == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToXND: Null URIHandler. ");
    return;
    }
  
  if ( !(strcmp(nodeType, "Volume" )))
    {
    //--- create the node
    //--- set its URIhandler
    //--- download to cache and load.
    }
  if ( !(strcmp(nodeType, "Model" )))
    {
    //--- create the node
    //--- set its URIhandler
    //--- download to cache and load.
    }
  
}



//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestSceneDownload ( const char *uri )
{

  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMILogic: FetchMINode is NULL.");
    return;
    }

  const char *svctype = this->GetFetchMINode()->GetSelectedServiceType();
  //--- we'll have to have separate methods for each
  if ( !(strcmp ("XND", svctype )) )
    {
    this->RequestSceneDownloadFromXND( uri );
    }
  if ( !(strcmp ("HID", svctype )) )
    {
    this->RequestSceneDownloadFromHID( uri );
    }

}


// Nicole: can you flesh out?
//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestSceneDownloadFromHID ( const char *uri)
{
  // This method will a resource with uri
  // from the currently selected remote repository.
  
  //--- Get the MRML Scene
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToHID: Null scene. ");
    return;
    }
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToHID: Null FetchMI node. ");
    return;
    }

  vtkHIDHandler *h = vtkHIDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "HID" ));
  if ( h == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToHID: Null URIHandler. ");
    return;
    }
  
  //--- we need to add a method on MRMLScene called SetURIHandler()
  //--- that explictly sets the handler to use for downloading scene.
  //--- this->MRMLScene->SetURIHandler ( h );
  //--- this->MRMLScene->SetURI ( uri );
  //---
  //--- then, in MRMLScene->FindURIHandler() we need to first check
  //--- if the scene's uri handler is set and use it if so. Otherwise, call
  //--- CanHandleURI(uri).

  //--- load the remote scene
  //--- this->MRMLScene->Connect();

  //--- finally, reset the MRMLScene's URIHandler.
  //--- this->MRMLScene->SetURIHandler ( NULL );
}



//--- Nicole: this one requires touching vtkmrmlscene
//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestSceneDownloadFromXND ( const char *uri )
{
  // This method will download a resource with uri
  // from the currently selected remote repository.
  
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

  vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XND" ));
  if ( h == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToXND: Null URIHandler. ");
    return;
    }
  
  //--- we need to add a method on MRMLScene called SetURIHandler()
  //--- that explictly sets the handler to use for downloading scene.
  //--- this->MRMLScene->SetURIHandler ( h );
  //--- this->MRMLScene->SetURI ( uri );
  //---
  //--- then, in MRMLScene->FindURIHandler() we need to first check
  //--- if the scene's uri handler is set and use it if so. Otherwise, call
  //--- CanHandleURI(uri).

  //--- load the remote scene
  //--- this->MRMLScene->Connect();

  //--- finally, reset the MRMLScene's URIHandler.
  //--- this->MRMLScene->SetURIHandler ( NULL );
  
}


//--- Nicole: this is the serious chunk.
//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceUploadToXND (  )
{

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

  vtkXNDHandler *h = vtkXNDHandler::SafeDownCast (this->GetMRMLScene()->FindURIHandlerByName ( "XND" ));
  if ( h == NULL )
    {
    vtkErrorMacro ("vtkFetchMILogic::RequestResourceUploadToXND: Null URIHandler. ");
    return;
    }
  
  
  //
  // PASS#1: SET CACHE FILENAMES AND SET URI HANDLER.
  //
  // Get all selected storable nodes from this->SelectedStorableNodeIDs;
  // (This vector of strings is populated by the GUI when upload button is selected)
  // FOR EACH FILENAME & FILELISTMEMBER IN EACH NODE:
  // {
  //--- Set Filename to be cachedir/filename
  //--- If the node is a multivolume, set the filename and all FileListMembers to
  //--- corresponding cachedir/filenames using AddFileName() method.
  //--- Write the file (or multivolume set of files) to cache.
  //--- USE GetNumberOfFileNames to get the number of FileListMembers.
  //--- USE GetNthFileName to get each, probably SetNthFileName to set each.
  //--- If this fails, error message and return.

  //--- Set the URIHandler on the storage node
  // *NOTE: make sure to see that DataIOManagerLogic (or whatever) checks to see
  // if the URIHandler is set before calling CanHandleURI() on all scene handlers,
  // and if not, fix this logic to use the storage node's handler as set.
  // } // end LOOP THRU NODES.

  
  //
  // PASS#2: GENERATE METADATA, UPLOAD METADATA, and SET URIS
  //
  // Loop thru again to generate metadata and upload.
  // Get all selected storable nodes from this->SelectedStorableNodeIDs;
  // FOR EACH FILENAME & FILELISTMEMBER IN EACH NODE:
  // {
  //--- call this->WriteXMLForUpload( filename, nodeID ) on the node
  //--- CHECK RETURN VALUE.
  //------
  //------ if OK:
  //------ call the handler's PostMetadata() method.
  //------ *NOTE: if the node is a multivolume node, then the WriteXMLForUpload() and
  //------ this->PostMetadata() methods have to be called on each individual filename/ListMemeber
  //------ in the node.
  //------ call this->ParseMetadataPostResponse() (returns uri) to get the uri for each filename.
  //------ Handle bad posts which return NULL -- if the uri is null, abort for the node,
  //------ and set node's URI (and all URIListMembers) to NULL so they won't be staged.
  //------ and deselect the node in the FetchMINode's list this->FetchMINode->DeselectNode ( nodeID)
  //------ Set the URI on the first filename, or call AddURI to subsequent for multivolume files.
  //------
  //------ if NOT OK for any individual filename:
  //------ then abort for the node,
  //------ and set node's URI (and all URIListMembers) to NULL so they won't be staged.
  //------ pop up error message giving user chance to continue saving other data, or abort altogether.
  //------ IF CONTINUE
  //------ and deselect the node in the FetchMINode's list this->FetchMINode->DeselectNode ( nodeID)
  //------ ELSE return
  //------ (NOTE: some nodes will have filename changed to cache now. should we
  //------ keep original filenames around and switch them back if upload is aborted?)
  // } // end LOOP THRU NODES.
  

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
  

  //
  // LAST STEP: WRITE SCENE XML AND UPLOAD.
  //
  // Check to see if save scene is selected this->SceneSelected
  // If so write mrml file to cache, include all nodes that have uris AND are selected for upload.
  // (get all selected storable nodes from this->SelectedStorableNodeIDs)
  // call this->WriteXMLForUpload( filename, MRMLScene) to generate metadata
  // If return is successful,
  //--- Call handler's PostMetadata() method which returns uri for MRML file.
  //--- Set scene's URI
  //--- h->StageFileWrite(cachefile, uri);
  // Otherwise:
  //--- pop up error message
  //--- call this->FetchMINode->DeselectScene() 

}


//----------------------------------------------------------------------------
void vtkFetchMILogic::RequestResourceUploadToHID ( )
{
  // no upload capabilities available from HID for now.
}



//--- Nicole: are the file paths going to be well-formed?
//----------------------------------------------------------------------------
void vtkFetchMILogic::CreateTemporaryFiles ( )
{
  // This method creates three temporary files in cache dir, one
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
            this->HTTPResponseFileName = this->XMLDirName;
            this->HTTPResponseFileName += "/";
            this->HTTPResponseFileName += "FetchMI_QueryResponse.xml";
            vtkDebugMacro ( "HTTPResponseFileName = " << this->HTTPResponseFileName );

            //--- for now, create temporary xml metadata description file in cache dir.
            this->XMLUploadFileName = this->XMLDirName;
            this->XMLUploadFileName += "/";
            this->XMLUploadFileName += "FetchMI_UploadMetadata.xml";
            vtkDebugMacro ( "XMLUploadFileName = " << this->XMLUploadFileName );

            //--- for now, create temporary cache file where downloaded mrml files go.
            this->MRMLCacheFileName = this->XMLDirName;
            this->MRMLCacheFileName += "/";
            this->MRMLCacheFileName += "FetchMI_UploadMetadata.xml";
            vtkDebugMacro ( "MRMLCacheFileName = " << this->MRMLCacheFileName );
            }
          }
        }
      else
        {
        vtkDebugMacro ("RemoteCacheDirectory is not set!");
        }
      }
    }
}


