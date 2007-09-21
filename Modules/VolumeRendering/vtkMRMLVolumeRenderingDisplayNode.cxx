#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkMRMLNode.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkPiecewiseFunction.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>


vtkMRMLVolumeRenderingDisplayNode* vtkMRMLVolumeRenderingDisplayNode::New()
{
        // First try to create the object from the vtkObjectFactory
        vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeRenderingDisplayNode");
        if(ret)
        {
                return (vtkMRMLVolumeRenderingDisplayNode*)ret;
        }
        // If the factory was unable to create the object, then create it here.
        return new vtkMRMLVolumeRenderingDisplayNode;
}

vtkMRMLNode* vtkMRMLVolumeRenderingDisplayNode::CreateNodeInstance(void)
{
        // First try to create the object from the vtkObjectFactory
        vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeRenderingDisplayNode");
        if(ret)
        {
                return (vtkMRMLVolumeRenderingDisplayNode*)ret;
        }
        // If the factory was unable to create the object, then create it here.
        return new vtkMRMLVolumeRenderingDisplayNode;
}

vtkMRMLVolumeRenderingDisplayNode::vtkMRMLVolumeRenderingDisplayNode(void)
{   
    Buffer=NULL;
    this->DebugOn();

}

vtkMRMLVolumeRenderingDisplayNode::~vtkMRMLVolumeRenderingDisplayNode(void)
{
}
void vtkMRMLVolumeRenderingDisplayNode::WriteXML(ostream& of, int nIndent)
{
  //// Write all attributes not equal to their defaults
  //
  //Superclass::WriteXML(of, nIndent);
  //
  //vtkIndent indent(nIndent);
  //
  //of << " type=\"" << this->GetType() << "\"";

  //if (this->FileName != NULL)
  //  {
  //  of << " filename=\"" << this->FileName << "\"";
  //  }
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::ReadXMLAttributes(const char** atts)
{

  //Superclass::ReadXMLAttributes(atts);

  //const char* attName;
  //const char* attValue;
  //while (*atts != NULL) 
  //  {
  //  attName = *(atts++);
  //  attValue = *(atts++);
  //  if (!strcmp(attName, "name"))
  //    {
  //    this->SetName(attValue);
  //    }
  //  else if (!strcmp(attName, "type")) 
  //    {
  //    int type;
  //    std::stringstream ss;
  //    ss << attValue;
  //    ss >> type;
  //    this->SetType(type);
  //    }
  //  else if (!strcmp(attName, "filename"))
  //    {
  //    this->SetFileName(attValue);
  //    // read in the file with the colours
  //    std::cout << "Reading file " << this->FileName << endl;
  //    this->ReadFile();
  //    }
  //  }
  //vtkDebugMacro("Finished reading in xml attributes, list id = " << this->GetID() << " and name = " << this->GetName() << endl);
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayNode::ReadFile ()
{
  vtkErrorMacro("Subclass has not implemented ReadFile.");
  return 0;
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLVolumeRenderingDisplayNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLColorNode *node = (vtkMRMLColorNode *) anode;

  //if (node->Type != -1)
  //  {
  //  // not using SetType, as that will basically recreate a new color node,
  //  // very slow
  //  this->Type = node->Type;
  //  }
  //this->SetFileName(node->FileName);
  //this->SetNoName(node->NoName);

  //// copy names
  //this->Names = node->Names;
  //
  //this->NamesInitialised = node->NamesInitialised;
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
 /* Superclass::PrintSelf(os,indent);

  os << indent << "Name: " <<
      (this->Name ? this->Name : "(none)") << "\n";
  

  os << indent << "Type: (" << this->GetTypeAsString() << ")\n";

  os << indent << "NoName = " <<
    (this->NoName ? this->NoName : "(not set)") <<  "\n";

  os << indent << "Names array initialised: " << (this->GetNamesInitialised() ? "true" : "false") << "\n";
  
  if (this->Names.size() > 0)
    {
    os << indent << "Color Names:\n";
    for (unsigned int i = 0; (int)i < this->Names.size(); i++)
      {
      os << indent << indent << i << " " << this->GetColorName(i) << endl;
      }
    }*/
}

//-----------------------------------------------------------

void vtkMRMLVolumeRenderingDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
    Superclass::UpdateScene(scene);
    /*
    if (this->GetStorageNodeID() == NULL) 
    {
        //vtkErrorMacro("No reference StorageNodeID found");
        return;
    }

    vtkMRMLNode* mnode = scene->GetNodeByID(this->StorageNodeID);
    if (mnode) 
    {
        vtkMRMLStorageNode *node  = dynamic_cast < vtkMRMLStorageNode *>(mnode);
        node->ReadData(this);
        //this->SetAndObservePolyData(this->GetPolyData());
    }
    */
}


//---------------------------------------------------------------------------
void vtkMRMLVolumeRenderingDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
/*
  vtkMRMLColorDisplayNode *dnode = this->GetDisplayNode();
  if (dnode != NULL && dnode == vtkMRMLColorDisplayNode::SafeDownCast(caller) &&
      event ==  vtkCommand::ModifiedEvent)
    {
        this->InvokeEvent(vtkMRMLColorNode::DisplayModifiedEvent, NULL);
    }
*/
  return;
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayNode::GetFirstType()
{
  vtkErrorMacro("Subclass has not over ridden this method");
  return -1;
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingDisplayNode::GetLastType()
{
  vtkErrorMacro("Subclass has not over ridden this method");
  return -1;
}

//---------------------------------------------------------------------------
const char * vtkMRMLVolumeRenderingDisplayNode::GetTypeAsString()
{
  vtkErrorMacro("Subclass has not over ridden this method");
  return "(unknown)";
}

char* vtkMRMLVolumeRenderingDisplayNode::getPiecewiseFunctionString(vtkPiecewiseFunction* function)
{
    std::ostringstream resultStream;
    int arraysize=function->GetSize()*2;
    double *data=function->GetDataPointer();
    double *it=data;
    //write header
    resultStream<<"vtkPiecewiseFunction#"<<arraysize;
    for (int i=0;i<arraysize;i++)
    {
        resultStream<<"#"<<*it;
        it++;

    }
    //Delete existing buffer
    if(Buffer)
    {
        delete []Buffer;
    }
    //Convert to char
    this->Buffer= new char[resultStream.str().length()+1];
    resultStream.str().copy(this->Buffer,std::string::npos);
    this->Buffer[resultStream.str().length()]=0;
    return this->Buffer;
    
}
char*  vtkMRMLVolumeRenderingDisplayNode::getColorTransferFunctionString(vtkColorTransferFunction* function)
{
    //maybe size*4
    std::ostringstream resultStream;
    int arraysize=function->GetSize()*4;
    double *data=function->GetDataPointer();
    double *it=data;
    //write header
    resultStream<<"vtkColorTransferFunction "<<arraysize<<"#";
    for (int i=0;i<arraysize;i++)
    {
        resultStream<<*it<<"#";
        it++;

    }
    //Delete existing buffer
    if(Buffer)
    {
        delete []Buffer;
    }
    //Convert to char
    this->Buffer= new char[resultStream.str().length()+1];
    resultStream.str().copy(this->Buffer,std::string::npos);
    this->Buffer[resultStream.str().length()]=0;
    return this->Buffer;

}
void vtkMRMLVolumeRenderingDisplayNode::GetPiecewiseFunctionFromString(char* string,vtkPiecewiseFunction* result)
{
    
    char *newOne=new char[strlen(string)];
    strcpy(newOne,string);
    int size=0;
    std::vector<double> dataVector;
  char * resultChar;
  resultChar = strtok (newOne,"#");
  if(strcmp(resultChar,"vtkPiecewiseFunction")!=0)
  {
    return;
  }
  else
  {
      resultChar = strtok (NULL,"#");
  }
  //get size
  size=(int)(strtod(resultChar,NULL));
  //getPoints

        resultChar = strtok (NULL, "#");
  while (resultChar != NULL)
  {
    dataVector.push_back(strtod(resultChar,NULL));   
      resultChar = strtok (NULL, "#");

  }
   result->FillFromDataPointer(size/2,&dataVector[0]);
}
void vtkMRMLVolumeRenderingDisplayNode::GetColorTransferFunction(char* string, vtkColorTransferFunction* result)
{
    char *newOne=new char[strlen(string)];
    strcpy(newOne,string);
     int size=0;
    std::vector<double> dataVector;
  char * resultChar;
  resultChar = strtok (string,"#");
  if(strcmp(resultChar,"vtkColorTransferFunction")!=0)
  {
    return;
  }
  else
  {
      resultChar = strtok (NULL,"#");
  }
  //get size
  size=(int)(strtod(resultChar,NULL));
  //getPoints
  resultChar = strtok (NULL, "#");
  while (resultChar != NULL)
  {
      
      dataVector.push_back(strtod(resultChar,NULL));
      resultChar = strtok (NULL, "#");
  }
    result->FillFromDataPointer(size,&dataVector[0]);
}

void vtkMRMLVolumeRenderingDisplayNode::InitializePipeline(vtkMRMLDisplayNode *node)
{

    //Instantiate mapper and volumeProperty with "default objects"
    this->Mapper=vtkVolumeTextureMapper3D::New();
    this->Mapper->SetInput(node->GetImageData());
    this->VolumeProperty=vtkVolumeProperty::New();
    this->VolumeProperty->SetScalarOpacity(vtkPiecewiseFunction::New());
    this->VolumeProperty->SetGradientOpacity(vtkPiecewiseFunction::New());
    this->VolumeProperty->SetColor(vtkColorTransferFunction::New());
    this->PipelineInitializedOn();
    /*set ::VR($this,pfed_hist) [vtkKWHistogram New]
        puts "pfed_hist: $$::VR($this,pfed_hist)"
        $::VR($this,pfed_hist) BuildHistogram [[$::VR($this,aImageData) GetPointData] GetScalars] 0*/
}


