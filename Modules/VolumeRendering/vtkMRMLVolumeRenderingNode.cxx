#include "vtkMRMLVolumeRenderingNode.h"
#include "vtkMRMLNode.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkPiecewiseFunction.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <string>


vtkMRMLVolumeRenderingNode* vtkMRMLVolumeRenderingNode::New()
{
        // First try to create the object from the vtkObjectFactory
        vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeRenderingNode");
        if(ret)
        {
                return (vtkMRMLVolumeRenderingNode*)ret;
        }
        // If the factory was unable to create the object, then create it here.
        return new vtkMRMLVolumeRenderingNode;
}

vtkMRMLNode* vtkMRMLVolumeRenderingNode::CreateNodeInstance(void)
{
        // First try to create the object from the vtkObjectFactory
        vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLVolumeRenderingNode");
        if(ret)
        {
                return (vtkMRMLVolumeRenderingNode*)ret;
        }
        // If the factory was unable to create the object, then create it here.
        return new vtkMRMLVolumeRenderingNode;
}

vtkMRMLVolumeRenderingNode::vtkMRMLVolumeRenderingNode(void)
{   
    Buffer=NULL;
    this->DebugOn();
    this->VolumeProperty=vtkVolumeProperty::New();
    //this->VolumeProperty->SetGradientOpacity(vtkPiecewiseFunction::New());
    //this->VolumeProperty->SetScalarOpacity(vtkPiecewiseFunction::New());
    //this->VolumeProperty->SetColor(vtkColorTransferFunction::New());

    //Standard is 3D-Volume Texture Mapper
    this->Mapper=vtkVolumeTextureMapper3D::New();

}

vtkMRMLVolumeRenderingNode::~vtkMRMLVolumeRenderingNode(void)
{
}
void vtkMRMLVolumeRenderingNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);
  
  //vtkIndent indent(nIndent);
  
  of << " scalarOpacity=\"" << this->getPiecewiseFunctionString(this->VolumeProperty->GetScalarOpacity()) << "\"";
  of << " gradientOpacity=\"" <<this->getPiecewiseFunctionString(this->VolumeProperty->GetGradientOpacity())<< "\"";
  of << " colorTransfer=\"" <<this->getColorTransferFunctionString(this->VolumeProperty->GetRGBTransferFunction())<< "\"";
  of << " interpolation=\"" <<this->VolumeProperty->GetInterpolationType()<< "\"";
  of << " shade=\"" <<this->VolumeProperty->GetShade()<< "\"";
  of << " diffuse=\"" <<this->VolumeProperty->GetDiffuse()<< "\"";
  of << " ambient=\"" <<this->VolumeProperty->GetAmbient()<< "\"";
  of << " specular=\"" <<this->VolumeProperty->GetSpecular()<< "\"";
  of << " specularPower=\"" <<this->VolumeProperty->GetSpecularPower()<<"\"";
}

//----------------------------------------------------------------------------
void vtkMRMLVolumeRenderingNode::ReadXMLAttributes(const char** atts)
{
    Superclass::ReadXMLAttributes(atts);

    const char* attName;
    const char* attValue;
    while (*atts!=NULL){
        attName= *(atts++);
        attValue= *(atts++);
        if(!strcmp(attName,"scalarOpacity"))
        {
            vtkPiecewiseFunction *scalarOpacity=vtkPiecewiseFunction::New();
            this->GetPiecewiseFunctionFromString(attValue,scalarOpacity);
            this->VolumeProperty->SetScalarOpacity(scalarOpacity);
        }
        else if(!strcmp(attName,"gradientOpacity"))
        {
            vtkPiecewiseFunction *gradientOpacity=vtkPiecewiseFunction::New();
            this->GetPiecewiseFunctionFromString(attValue,gradientOpacity);
            this->VolumeProperty->SetScalarOpacity(gradientOpacity);
        }
        else if(!strcmp(attName,"colorTransfer"))
        {
            vtkColorTransferFunction *colorTransfer=vtkColorTransferFunction::New();
            this->GetColorTransferFunction(attValue,colorTransfer);
            this->VolumeProperty->SetColor(colorTransfer);
        }
        else if(!strcmp(attName,"interpolation"))
        {
            int interpolation;
            std::stringstream ss;
            ss <<attValue;
            ss>>interpolation;
            this->VolumeProperty->SetInterpolationType(interpolation);
        }
        else if(!strcmp(attName,"shade"))
        {
            int shade;
            std::stringstream ss;
            ss <<attValue;
            ss>>shade;
            this->VolumeProperty->SetShade(shade);
        }
        else if(!strcmp(attName,"diffuse"))
        {
            int diffuse;
            std::stringstream ss;
            ss<<attValue;
            ss>>diffuse;
            this->VolumeProperty->SetDiffuse(diffuse);
        }
        else if(!strcmp(attName,"ambient"))
        {
            int ambient;
            std::stringstream ss;
            ss<<attValue;
            ss>>ambient;
            this->VolumeProperty->SetAmbient(ambient);
        }
        else if(!strcmp(attName,"specular"))
        {
            int specular;
            std::stringstream ss;
            ss<<attValue;
            ss>>specular;
            this->VolumeProperty->SetSpecular(specular);
        }
        else if(!strcmp(attName,"specularPower"))
        {
            int specularPower;
            std::stringstream ss;
            ss<<attValue;
            ss>>specularPower;
            this->VolumeProperty->SetSpecularPower(specularPower);
        }//else if

  }//while
    vtkDebugMacro("Finished reading in xml attributes, list id = " << this->GetID() << " and name = " << this->GetName() << endl);
}

//----------------------------------------------------------------------------
int vtkMRMLVolumeRenderingNode::ReadFile ()
{
  vtkErrorMacro("Subclass has not implemented ReadFile.");
  return 0;
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLVolumeRenderingNode::Copy(vtkMRMLNode *anode)
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
void vtkMRMLVolumeRenderingNode::PrintSelf(ostream& os, vtkIndent indent)
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

void vtkMRMLVolumeRenderingNode::UpdateScene(vtkMRMLScene *scene)
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
void vtkMRMLVolumeRenderingNode::ProcessMRMLEvents ( vtkObject *caller,
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
int vtkMRMLVolumeRenderingNode::GetFirstType()
{
  vtkErrorMacro("Subclass has not over ridden this method");
  return -1;
}

//---------------------------------------------------------------------------
int vtkMRMLVolumeRenderingNode::GetLastType()
{
  vtkErrorMacro("Subclass has not over ridden this method");
  return -1;
}

//---------------------------------------------------------------------------
const char * vtkMRMLVolumeRenderingNode::GetTypeAsString()
{
  vtkErrorMacro("Subclass has not over ridden this method");
  return "(unknown)";
}

const char* vtkMRMLVolumeRenderingNode::getPiecewiseFunctionString(vtkPiecewiseFunction* function)
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
    return resultStream.str().c_str();
    
}
const char*  vtkMRMLVolumeRenderingNode::getColorTransferFunctionString(vtkColorTransferFunction* function)
{
    
    //maybe size*4
    std::ostringstream resultStream;
    //resultStream.str
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
    return resultStream.str().c_str();

}
void vtkMRMLVolumeRenderingNode::GetPiecewiseFunctionFromString(const char* string,vtkPiecewiseFunction* result)
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
  //nothing to fill
  if (size==0)
  {
      return;
  }
  //getPoints

        resultChar = strtok (NULL, "#");
  while (resultChar != NULL)
  {
    dataVector.push_back(strtod(resultChar,NULL));   
      resultChar = strtok (NULL, "#");

  }
   result->FillFromDataPointer(size/2,&dataVector[0]);
}
void vtkMRMLVolumeRenderingNode::GetColorTransferFunction(const char* string, vtkColorTransferFunction* result)
{

    char *newOne=new char[strlen(string)];
    strcpy(newOne,string);
     int size=0;
    std::vector<double> dataVector;
  char * resultChar;
  resultChar = strtok (newOne,"#");
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
  //nothing to fill
  if (size==0)
  {
      return;
  }
  //getPoints
  resultChar = strtok (NULL, "#");
  while (resultChar != NULL)
  {
      
      dataVector.push_back(strtod(resultChar,NULL));
      resultChar = strtok (NULL, "#");
  }
    result->FillFromDataPointer(size,&dataVector[0]);
}


