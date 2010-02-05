#include "vtkObjectFactory.h"

#include "vtkFetchMIWriterXND.h"
#include <string>
#include <iostream>
#include <sstream>
#include <vtksys/SystemTools.hxx>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkFetchMIWriterXND );
vtkCxxRevisionMacro ( vtkFetchMIWriterXND, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkFetchMIWriterXND::vtkFetchMIWriterXND ( )
{
  this->SetName ("XNDWriter");
}


//---------------------------------------------------------------------------
vtkFetchMIWriterXND::~vtkFetchMIWriterXND ( )
{
}

//---------------------------------------------------------------------------
void vtkFetchMIWriterXND::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf ( os, indent );
}


//---------------------------------------------------------------------------
void vtkFetchMIWriterXND::WriteDocumentDeclaration()
{
  std::string s = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  // s.c_str() constains string with null terminating character
  // s.data() contains the string without any null terminating character.
  // not sure which will work, so try both.
  
  if ( this->DocumentDeclarationFilename == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteDocumentDeclaration: can't write dtd, DocumentDeclarationFilename is NULL.");
    return;
    }

  FILE *fptr = fopen ( this->DocumentDeclarationFilename, "w");
  if ( fptr == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteDocumentDeclaration: failed to open file for writing Document Declaration");
    return;
    }

  size_t count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
  if ( count != (size_t)s.size() )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteDocumentDeclaration: incomplete write of Document Declaration");
    }
  fclose( fptr);
}



//---------------------------------------------------------------------------
void vtkFetchMIWriterXND::WriteXMLHeader ( const char *dataFilename)
{

  
  if ( this->HeaderFilename == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteXMLHeader: got NULL header filename" );
    return;
    }
  if ( dataFilename == NULL || !(strcmp(dataFilename, "" )) )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteXMLHeader: got NULL data filename" );
    return;
    }
  
  FILE *fptr = fopen ( this->HeaderFilename, "w");
  if ( fptr == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteXMLHeader: failed to open header file for writing" );
    return;
    }
  
  std::stringstream ss1;
  ss1 << "Content-Type: application/x-xnat-metadata+xml\n";
  std::string s = ss1.str();
  size_t count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
  if ( count != (size_t)s.size() )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteXMLHeader: incomplete write of header file.");
    fclose(fptr);
    return;
    }
  
  std::stringstream ss2;
  ss2 << "Content-Disposition: x-xnat-metadata; filename=\"";
  ss2 << dataFilename;
  ss2 << "\"\n";
  s.clear();
  s = ss2.str();
  count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
  if ( count != (size_t)s.size() )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteXMLHeader: incomplete write of header file.");
    }
  fclose( fptr);

}

//---------------------------------------------------------------------------
void vtkFetchMIWriterXND::WriteMetadataForNode ( const char *nodeID, vtkMRMLScene *scene )
{
  FILE *fptr = NULL;
  long int lSize;
  size_t count;
  char *dtdBuffer = NULL;
  
  if ( scene == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: got NULL MRMLScene.");
    return;
    }
  if ( nodeID == NULL || !(strcmp(nodeID, "" )))
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: got NULL or empty nodeID." );
    return;
    }
  if ( this->DocumentDeclarationFilename == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: DocumentDeclarationFilename is NULL" );
    return;
    }
  if ( this->MetadataFilename == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: MetadataFilename is NULL" );
    return;
    }
  
  vtkMRMLStorableNode *node = vtkMRMLStorableNode::SafeDownCast (scene->GetNodeByID(nodeID) );
  if ( node == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: got NULL node." );
    return;
    }
  vtkTagTable *tt = node->GetUserTagTable();
  if ( tt == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: got NULL UserTagTable for node." );
    return;    
    }


  //--- open document declaration
  fptr = fopen ( this->DocumentDeclarationFilename, "rb");
  if ( fptr == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: failed to open Document Declaration File for reading.");
    return;
    }

  //--- get file size
  fseek ( fptr, 0, SEEK_END);
  lSize = (ftell ( fptr )) - 1;
  rewind ( fptr );
  //--- read and close document declaration  
  dtdBuffer = (char*) malloc (sizeof(char)*lSize );
  if ( dtdBuffer == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: unable to allocate buffer to store Document Declaration.");
    fclose (fptr);
    return;
    }
  count = fread(dtdBuffer, 1, lSize, fptr);
  if ( count != (size_t)lSize )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: incomplete read of the Document Declaration.");
    fclose (fptr);
    return;
    }
  fclose(fptr);

  fptr = fopen ( this->MetadataFilename, "w");
  if ( fptr == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: error opening MetadataFilename." );
    free (dtdBuffer);
    return;
    }
  //--- add the DtdBuffer
  count = fwrite ( dtdBuffer, 1, (size_t)lSize, fptr );
  if ( count != (size_t)lSize )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: incomplete write of Document Declaration  to Metadata file.");
    free(dtdBuffer);
    fclose (fptr);
    return;
    }

  //-- Start writing the metadata description and add the namespace
  std::string nspace = "xmlns=\"http://nrg.wustl.edu/xe\"";
  std::stringstream ss;
  ss << "<Metadata ";
  ss << nspace;
  ss << ">\n";
  std::string s = ss.str();

  //--- write stuff
  count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
  if ( count != (size_t)s.size() )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: incomplete write of file to metadata file.");
    free(dtdBuffer);
    fclose (fptr);
    return;
    }

  free (dtdBuffer);

  int num = tt->GetNumberOfTags();
  int found = 0;
  std::string att;
  std::string val;

  //--- Make sure SlicerDataType is set.
  //--- Until we have a CONSISTENT way of describing formats as metadata,
  //--- and a robust way of mapping formats to MRML node types,
  //--- we are enforcing a SlicerDataType attribute that will ensure
  //--- the correct node is associated to the data next time it is loaded.
  for (int i = 0; i < num; i++ )
    {
    att = tt->GetTagAttribute(i);
    if ( !(strcmp( att.c_str(), "SlicerDataType")))
      {
      found=1;
      break;
      }
    }
  if (! found )
    {
    tt->AddOrUpdateTag ( "SlicerDataType", "unknown");
    }

  //--- now add all tags.
  for (int i=0; i < num; i++ )
    {
    //--- write tag open and keword
    att.clear();
    val.clear();
    s.clear();
    ss.str(std::string() );
    att = tt->GetTagAttribute(i);
    val = tt->GetTagValue(i);
    ss << "<Tag Label=\"";
    ss << att.c_str();
    ss << "\">\n";
    s = ss.str();
    //--- write stuff
    count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
    if ( count != (size_t)s.size() )
      {
      vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: incomplete write of metadata to Metadata file.");
      fclose (fptr);
      return;
      }
    
    //--- write value
    s.clear();
    ss.str(std::string() );
    ss << "<Value>";
    ss << val.c_str();
    ss << "</Value>\n";
    s = ss.str();
    //--- write stuff
    count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
    if ( count != (size_t)s.size() )
      {
      vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: incomplete write of metadata to Metadata file.");
      fclose (fptr);
      return;
      }

    //-- close tag
    s.clear();
    ss.str(std::string() );
    ss << "</Tag>\n";
    s = ss.str();
    //--- write stuff
    count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
    if ( count != (size_t)s.size() )
      {
      vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: incomplete write of metadata to Metadata file.");
      fclose (fptr);
      return;
      }
    }

  //--- now close the metadata description
  s.clear();
  ss.str(std::string() );
  ss << "</Metadata>\n";
  s = ss.str();
  count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
  if ( count != (size_t)s.size() )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: incomplete write of metadata to Metadata file.");
    fclose (fptr);
    return;
    }

  fclose( fptr);
}

//---------------------------------------------------------------------------
void vtkFetchMIWriterXND::WriteMetadataForScene ( vtkMRMLScene *scene )
{
  FILE *fptr = NULL;
  long int lSize;
  size_t count;
  char *dtdBuffer = NULL;

  if ( scene == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: MRMLScene is NULL" );
    return;
    }
  if ( this->DocumentDeclarationFilename == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: DocumentDeclarationFilename is NULL." );
    return;    
    }

  fptr = fopen ( this->MetadataFilename, "w");
  if ( fptr == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: MetadataFilename is NULL" );
    return;
    }

  //--- open document declaration
  fptr = fopen ( this->DocumentDeclarationFilename, "rb");
  if ( fptr == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: failed to open Document Declaration file for reading.");
    return;
    }

  //--- get file size
  fseek ( fptr, 0, SEEK_END);
  lSize = (ftell ( fptr )) - 1;
  rewind ( fptr );
  //--- read and close document declaration  
  dtdBuffer = (char*) malloc (sizeof(char)*lSize );
  if ( dtdBuffer == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: unable to allocate buffer to store Document Declaration.");
    fclose (fptr);
    return;
    }
  count = fread(dtdBuffer, 1, lSize, fptr);
  if ( count != (size_t)lSize )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: incomplete read of the Document Declaration.");
    fclose (fptr);
    return;
    }
  fclose(fptr);

  fptr = fopen ( this->MetadataFilename, "w");
  if ( fptr == NULL )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: error opening MetadataFilename." );
    free (dtdBuffer);
    return;
    }
  //--- add the DtdBuffer
  count = fwrite ( dtdBuffer, 1, (size_t)lSize, fptr );
  if ( count != (size_t)lSize )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: incomplete write of Document Declaration  to Metadata file.");
    free(dtdBuffer);
    fclose (fptr);
    return;
    }
  free (dtdBuffer);
  
  //-- Start writing the metadata description and add the namespace
  std::string nspace = "xmlns=\"http://nrg.wustl.edu/xe\"";
  std::stringstream ss;
  ss << "<Metadata ";
  ss << nspace;
  ss << ">";
  std::string s = ss.str();
  count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
  if ( count != (size_t)s.size() )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: incomplete write of file to Metadata file.");
    free(dtdBuffer);
    fclose (fptr);
    return;
    }

  //--- get the metadata and make sure SlicerDataType is defined for the scene.
  vtkTagTable *tt = scene->GetUserTagTable();
  tt->AddOrUpdateTag ( "SlicerDataType", "MRML");
  int num = tt->GetNumberOfTags();
  std::string att;
  std::string val;

  //--- write all the metadata.
  //--- now add all tags.
  for (int i=0; i < num; i++ )
    {
    //--- write tag open and keword
    att.clear();
    val.clear();
    s.clear();
    ss.str(std::string());
    att = tt->GetTagAttribute(i);
    val = tt->GetTagValue(i);
    ss << "<Tag Label=\"";
    ss << att.c_str();
    ss << "\">";
    s = ss.str();
    //--- write stuff
    count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
    if ( count != (size_t)s.size() )
      {
      vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: incomplete write of metadata to Metadata file.");
      fclose (fptr);
      return;
      }
    
    //--- write value
    s.clear();
    ss.str(std::string());
    ss << "<Value>";
    ss << val.c_str();
    ss << "</Value>";
    s = ss.str();
    //--- write stuff
    count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
    if ( count != (size_t)s.size() )
      {
      vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: incomplete write of metadata to Metadata file.");
      fclose (fptr);
      return;
      }

    //-- close tag
    s.clear();
    ss.str(std::string());
    ss << "</Tag>";
    s = ss.str();
    //--- write stuff
    count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
    if ( count != (size_t)s.size() )
      {
      vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForNode: incomplete write of metadata to Metadata file.");
      fclose (fptr);
      return;
      }
    }

  //--- now close the metadata description
  s.clear();
  ss.str(std::string());
  ss << "</Metadata>";
  s = ss.str();
  count = fwrite ( s.c_str(), 1, (size_t)s.size(), fptr );
  if ( count != (size_t)s.size() )
    {
    vtkErrorMacro ( "vtkFetchMIWriterXND::WriteMetadataForScene: incomplete write of metadata to Metadata file.");
    fclose (fptr);
    return;
    }

  fclose( fptr);
}







