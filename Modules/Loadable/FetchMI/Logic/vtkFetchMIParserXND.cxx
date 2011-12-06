#include "vtkObjectFactory.h"

#include "vtkFetchMIParserXND.h"
#include "vtkXNDTagTable.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIParserXND );
vtkCxxRevisionMacro ( vtkFetchMIParserXND, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIParserXND::vtkFetchMIParserXND ( )
{
  this->Parser = vtkXMLDataParser::New();
}


//---------------------------------------------------------------------------
vtkFetchMIParserXND::~vtkFetchMIParserXND ( )
{
  if ( this->Parser )
    {
    this->Parser->Delete();
    this->Parser = NULL;
    }
}



//---------------------------------------------------------------------------
void vtkFetchMIParserXND::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    if ( this->Parser )
      {      
      os << indent << "Parser: " << this->GetParser ( ) << "\n";
      }
    else
      {
      os << indent << "Parser: NULL\n";
      }
}



//---------------------------------------------------------------------------
int vtkFetchMIParserXND::ParseMetadataQueryResponse(const char *filename )
{

  
  if ( filename == NULL || !(strcmp(filename, "") ) )
    {
    vtkErrorMacro ( "vtkFetchMIParserXND::ParseMetadataQueryResponse: response filename for parsing is NULL.");
    return 0;
    }
  vtkDebugMacro ("--------------------Parsing metadata query response.");
  if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIParserXND: FetchMINode is NULL.");
    return 0;
    }
  if ( this->FetchMINode->GetTagTableCollection() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIParserXND: FetchMINode's TagTableCollection is NULL.");
    return 0;
    }
  if ( this->Parser == NULL )
    {
    this->Parser = vtkXMLDataParser::New();
    }

  this->ClearMetadataInformation ( );
  this->FetchMINode->SetErrorMessage ( NULL );

  //--- look at xml file to see if there's an error message
  //--- parse the error and fill the node's error message.
  //--- if no, parse response into table.
  vtkXNDTagTable *t = vtkXNDTagTable::SafeDownCast ( this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XND" ));
  if ( t != NULL )
    {
    //--- check for file.
    this->Parser->SetFileName ( filename );
    this->Parser->SetIgnoreCharacterData ( 0 );
    int retval = this->Parser->Parse();
    if ( retval == 0 )
      {
      this->FetchMINode->SetErrorMessage ("Unable to parse tag query response.");
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      return 0;
      }

    //--- first check for errors
    this->NumberOfElements = 0;
    vtkXMLDataElement *root = this->Parser->GetRootElement();
    this->ErrorChecking = 1;
    this->ErrorFlag = 0;
    if ( root != NULL )
      {
      this->GetNumberOfXMLElements( root );
      if ( this->NumberOfElements > 0 )
        {
        root = this->Parser->GetRootElement();
        this->GetXMLElement ( root );
        }
      this->ErrorChecking = 0;
      if ( this->ErrorFlag )
        {
        this->FetchMINode->SetErrorMessage ("The server returned an error response to the query for all known values for its tags.");
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
        this->ErrorFlag = 0;
        return 0;
        }
      }

    //--- If no need to keep tag selection state around, then
    //--- clear out mrml tag table to be refilled by this query.
    if ( t->GetRestoreSelectionState() == 0 )
      {
      t->ClearTagTable();
      }
      
    //--- Set the response type to help in pulling out XML elements properly.
    this->ResponseType = vtkFetchMIParser::TagAttributesQueryResponse;

    //---
    this->NumberOfElements = 0;
    root = this->Parser->GetRootElement();
    if ( root != NULL )
      {
      this->GetNumberOfXMLElements( root );
      if ( this->NumberOfElements > 0 )
        {
        //--- If response is good, parse response into table.
        root = this->Parser->GetRootElement();
        this->GetXMLElement ( root );
        }
      }
    else
      {
      // let user know the parsing didn't go well.
      this->FetchMINode->SetErrorMessage ("Unable to parse tag query response. Displayed list of tags has not been updated.");
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
      return 0;
      }
    }
  return 1;

}

//---------------------------------------------------------------------------
int vtkFetchMIParserXND::ParseMetadataPostResponse( const char *filename )
{

  if ( filename == NULL || !(strcmp(filename, "" )) )
    {
    vtkErrorMacro ( "vtkFetchMIParserXND::ParseMetadataPostResponse: got null or empty response filename." );
    return 0;
    }

  std::ifstream pfile;
  std::string line;
#ifdef _WIN32
  pfile.open ( filename, ios::binary | ios::in );
#else
  pfile.open ( filename, ios::in );
#endif
  
  if ( !pfile.is_open() )
    {
    vtkErrorMacro ( "vtkFetchMIParserXND::ParseMetadataPostResponse: error opening file to parse." );
    return 0;
    }

  int found = 0;
  size_t pos;
  while ( !pfile.eof() )
    {
    line.clear();
    pfile >> line;
    //--- look for uri created and reserved for the resource to be posted.
    //--- XND webservices returns this uri on its own line...
    //--- get lines and search each one.
    //--- when found, set found to 1.    
    if ( ( pos = line.find ( "http://", 0) ) != std::string::npos )
      {
      if ( pos == 0 )
        {
        found = 1;
        break;
        }
      }
    }

  pfile.close();

  if ( found )
    {
    this->SetResponse(line.c_str());
    return 1;
    }
  return 0;
}


//---------------------------------------------------------------------------
int vtkFetchMIParserXND::ParseValuesForAttributeQueryResponse(const char *filename, const char *att )
{
    

  if ( filename == NULL || !(strcmp(filename, "" )) )
    {
    vtkErrorMacro ( "vtkFetchMIParserXND::ParseMetadataValuesQueryResponse: got null or empty response filename." );
    return 0;
    }
  if ( this->FetchMINode == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIParserXND::ParseMetadataValuesQueryResponse: got null FetchMINode.");
    return 0;
    }
  if ( this->Parser == NULL )
    {
    this->Parser = vtkXMLDataParser::New();
    }

  this->FetchMINode->SetErrorMessage ( NULL );
  this->ClearValuesForAttribute( att );

  //--- check for file.
  this->Parser->SetFileName ( filename );
  this->Parser->SetIgnoreCharacterData ( 0 );
  int retval = this->Parser->Parse();
  //--- throw an error event if parsing failed.
  if ( retval == 0 )
    {
    this->FetchMINode->SetErrorMessage ("Unable to parse tag value query response.");
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
    return 0;
    }

  //--- first check for errors
  this->NumberOfElements = 0;
  vtkXMLDataElement *root = this->Parser->GetRootElement();
  this->ErrorChecking = 1;
  this->ErrorFlag = 0;
  if ( root != NULL )
    {
    this->GetNumberOfXMLElements( root );
    if ( this->NumberOfElements > 0 )
      {
      root = this->Parser->GetRootElement();
      this->GetXMLElement ( root );
      }
    this->ErrorChecking = 0;
    if ( this->ErrorFlag )
      {
      this->FetchMINode->SetErrorMessage ("The server returned an error response to the query for its tags.");
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
      this->ErrorFlag = 0;
      return 0;
      }
    }

  //--- Set response type for correct parsing.
  this->ResponseType = vtkFetchMIParser::TagValuesQueryResponse;
  //---
  this->NumberOfElements = 0;
  root = this->Parser->GetRootElement();
  if ( root != NULL )
    {
    this->GetNumberOfXMLElements( root );
    if ( this->NumberOfElements > 0 )
      {
      root = this->Parser->GetRootElement();
      this->GetXMLElement ( root );
      }
    }
  else
    {
    // let user know the parsing didn't go well.
    this->FetchMINode->SetErrorMessage ("Unable to parse tag query response. Displayed list of tags has not been updated.");
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
    }

  return 1;
}


//---------------------------------------------------------------------------
int vtkFetchMIParserXND::ParseResourceQueryResponse(const char *filename )
{
  
    if ( this->GetFetchMINode() == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIParserXND::ParseResourceQueryResponse: FetchMINode is NULL.");
    return 0;
    }
  if ( this->Parser == NULL )
    {
    this->Parser = vtkXMLDataParser::New();
    }
  if ( filename == NULL || !(strcmp(filename, "")) )
    {
    vtkErrorMacro ( "vtkFetchMIParserXND::ParseResourceQueryResponse: No XMLResponse filename is set." );
    return 0;
    }

  this->FetchMINode->SetErrorMessage(NULL );
  if  (this->FetchMINode->GetResourceDescription() != NULL) 
    {
    //--- check for file.
    this->Parser->SetFileName ( filename );
    this->Parser->SetIgnoreCharacterData ( 0 );
    int retval = this->Parser->Parse();
    if ( retval == 0 )
      {
      this->FetchMINode->SetErrorMessage("Unable to parse resource query response.");
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );
      return 0;
      }

    //--- first check for errors
    this->NumberOfElements = 0;
    vtkXMLDataElement *root = this->Parser->GetRootElement();
    this->ErrorChecking = 1;
    this->ErrorFlag = 0;
    if ( root != NULL )
      {
      this->GetNumberOfXMLElements( root );
      if ( this->NumberOfElements > 0 )
        {
        root = this->Parser->GetRootElement();
        this->GetXMLElement ( root );
        }
      this->ErrorChecking = 0;
      if ( this->ErrorFlag )
        {
        this->FetchMINode->SetErrorMessage ("The server returned an error response to the query for matching resources.");
        this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
        this->ErrorFlag = 0;
        return 0;
        }
      }

    //--- make sure ResponseType is set so parsing is done correctly.
    this->ResponseType = vtkFetchMIParser::ResourceQueryResponse;

    this->NumberOfElements = 0;
    root = this->Parser->GetRootElement();
    if ( root != NULL )
      {
      this->GetNumberOfXMLElements( root );
      if ( this->NumberOfElements > 0 )
        {
        //--- If response is good, parse response into table.
        root = this->Parser->GetRootElement();
        this->GetXMLElement ( root );
        }
      }
    else
      {
      // let user know the parsing didn't go well.
      this->FetchMINode->SetErrorMessage("Unable to parse tag resource response.");
      this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::RemoteIOErrorEvent );          
      return 0;
      }
    
    this->FetchMINode->InvokeEvent ( vtkMRMLFetchMINode::ResourceResponseReadyEvent );
    }
  return 1;
}



//---------------------------------------------------------------------------
int vtkFetchMIParserXND::ParseResourcePostResponse(const char *vtkNotUsed(filename))
{
  return 0;
}


//---------------------------------------------------------------------------
int vtkFetchMIParserXND::ParseResourceDownloadResponse(const char *vtkNotUsed(filename))
{
  return 0;
}

//---------------------------------------------------------------------------
int vtkFetchMIParserXND::ParseMetatdataDeleteResponse(const char *vtkNotUsed(filename))
{
  return 0;
}

//---------------------------------------------------------------------------
int vtkFetchMIParserXND::ParseResourceDeleteResponse(const char *vtkNotUsed(filename))
{
  return 0;
}

//---------------------------------------------------------------------------
int vtkFetchMIParserXND::ParseForErrors (const char *vtkNotUsed(val) )
{
  return 0;
}

//---------------------------------------------------------------------------
void vtkFetchMIParserXND::GetXMLEntry ( vtkXMLDataElement *element )
{
    if (element ==  NULL )
    {
    vtkErrorMacro ( "vtkFetchMIParserXND::GetXMLEntry: called with null vtkXMLDataElement.");
    return;
    }

  const char *name = element->GetName();

  if ( name == NULL )
    {
    return;
    }

  const char *attName;
  int numAtts;
  const char *value;
  const char *dtype = "unknown";
  const char *tagValue;
  vtkXMLDataElement *nestedElement;
  std::string currentURI;


  // check to see if this is an item of interest.
  // process appropriately, based on what elements we find.


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
        this->ErrorFlag = 1;
        }
      target = "Error";
      if ( found != std::string::npos)
        {
        this->ErrorFlag = 1;
        }
      target = "ERROR";
      if ( found != std::string::npos)
        {
        this->ErrorFlag = 1;
        }
      }
    }

  
  //---
  // RESOURCES have nested uris and tags (attribute/value pairs)
  // ONLY get these if the ResourceQuery flag is on. Otherwise query was
  // for tags or tag values, so ignore.
  //---
  if ( this->ResponseType == vtkFetchMIParser::ResourceQueryResponse )
    {    
    if ((!(strcmp(name, "resource" ))) ||
        (!(strcmp(name, "Resource" ))) ||
        (!(strcmp(name, "RESOURCE" ))) )
      {
      // parse the xml file and put result into ResourceDescription table.
      // looking for the SlicerDataType to pair with uri
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
            currentURI.clear();
            currentURI = value;
            this->FetchMINode->GetResourceDescription()->AddOrUpdateTag ( currentURI.c_str(), dtype, 0 );        
            }
          }

        //--- get data type
        if (((!(strcmp(nestedElement->GetName(), "tag" ))) ||
             (!(strcmp(nestedElement->GetName(), "Tag" ))) ||
             (!(strcmp(nestedElement->GetName(), "TAG" )))) &&
            ( currentURI.c_str() != NULL  && (strcmp(currentURI.c_str(), "" )) ))
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
                      //now pair it up with currentURI and add to ResourceDescription.
                      this->FetchMINode->GetResourceDescription()->AddOrUpdateTag ( currentURI.c_str(), dtype, 0 );
                      currentURI.clear();
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
  if (  this->ResponseType == vtkFetchMIParser::TagAttributesQueryResponse )
    {
    if ( (!(strcmp(name, "Label" ))) ||
         (!(strcmp(name, "label" ))) )
      {
      // parse the xml file and put result into node's XND tag table.
      // and into logic's CurrentWebServiceMetadata map.
      value = element->GetCharacterData();
      vtkXNDTagTable* t = vtkXNDTagTable::SafeDownCast (this->FetchMINode->GetTagTableCollection()->FindTagTableByName ( "XND" ));
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
        //--- If a NEW server has been selected, then t->RestoreSelectionState = 0
        //--- and we should just call AddOrUpdate with the new tag.
        //---
        //--- NOTE: this behavior may be friendly or annoying;
        //--- not sure. If we don't want to restore the tag's
        //--- selected state, then just get rid of the check on
        //--- t->RestoreSelectionState and always call AddOrUpdateTag
        //---
        if ( t->GetRestoreSelectionState() )
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
  if ( this->ResponseType == vtkFetchMIParser::TagValuesQueryResponse )
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





