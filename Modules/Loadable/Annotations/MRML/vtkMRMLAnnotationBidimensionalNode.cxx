#include <sstream>

#include "vtkMRMLAnnotationBidimensionalNode.h"

#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationBidimensionalNode);


//----------------------------------------------------------------------------
vtkMRMLAnnotationBidimensionalNode::vtkMRMLAnnotationBidimensionalNode()
{
  this->HideFromEditors = false;
  this->AnnotationFormat = nullptr;
  this->SetAnnotationFormat("%.1f");
  this->Resolution = 5;
}
//----------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::Initialize(vtkMRMLScene* mrmlScene)
{

    Superclass::Initialize(mrmlScene);

    /* taken out by rec from Kilian
    if (!mrmlScene)
    {
        vtkErrorMacro("Scene was null!");
        return;
    }

    mrmlScene->AddNode(this);

    this->CreateAnnotationTextDisplayNode();
    this->CreateAnnotationPointDisplayNode();
    this->CreateAnnotationLineDisplayNode();
    */
  // default starting position
    /*
  {
    double pos[3] = {-20.0, 0.0, -20.0};
    this->SetControlPoint(pos, 0);
  }
  {
    double pos[3] = {-20.0, 0.0, 20.0};
    this->SetControlPoint(pos, 1);
  }
  {
    double pos[3] = {0.0, 20.0, 0.0};
    this->SetControlPoint(pos, 2);
  }
  {
    double pos[3] = {0.0, -20.0, 0.0};
    this->SetControlPoint(pos, 3);
  }
*/
    this->AddText(" ",1,1);

    this->InvokeEvent(vtkMRMLAnnotationBidimensionalNode::BidimensionalNodeAddedEvent);
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationBidimensionalNode::~vtkMRMLAnnotationBidimensionalNode()
{
  vtkDebugMacro("Destructing...." << (this->GetID() != nullptr ? this->GetID() : "null id"));
  if (this->AnnotationFormat)
    {
      delete [] this->AnnotationFormat;
      this->AnnotationFormat = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << "AnnotationFormat=\"";
  if (this->AnnotationFormat)
    {
    of << this->AnnotationFormat << "\"";
    }
  else
    {
    of << "\"";
    }
  of << "Resolution=\""<< this->Resolution << "\"";

  if (this->measurement1)
    {
    of << "measurement1=\"" << this->measurement1 << "\"";
    }
  if (this->measurement2)
    {
    of << "measurement2=\"" << this->measurement2 << "\"";
    }
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::ReadXMLAttributes(const char** atts)
{
  // cout << "vtkMRMLAnnotationRulerNode::ReadXMLAttributes start"<< endl;

  int disabledModify = this->StartModify();

  this->ResetAnnotations();

  Superclass::ReadXMLAttributes(atts);


  while (*atts != nullptr)
    {
    const char* attName = *(atts++);
    std::string attValue(*(atts++));


    if (!strcmp(attName, "rulerResolution"))
      {

    std::stringstream ss;
        ss << attValue;
        ss >> this->Resolution;
      }
    else if (!strcmp(attName, "AnnotationFormat"))
      {
      this->SetAnnotationFormat(attValue.c_str());
      }
    else if (!strcmp(attName, "measurement1"))
      {
      std::stringstream ss;
      ss << attValue;
      double biM1;
      ss >> biM1;

      this->measurement1 = biM1;

      }
    else if (!strcmp(attName, "measurement2"))
      {
      std::stringstream ss;
      ss << attValue;
      double biM2;
      ss >> biM2;

      this->measurement2 = biM2;

      }


    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLAnnotationBidimensionalNode *node = (vtkMRMLAnnotationBidimensionalNode *) anode;

  this->SetBidimensionalMeasurement(node->GetBidimensionalMeasurement()[0],node->GetBidimensionalMeasurement()[1]);
}

//-----------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag)
{
  //cout << "vtkMRMLAnnotationRulerNode::PrintAnnotationInfo" << endl;
  if (titleFlag)
    {

      os <<indent << "vtkMRMLAnnotationBidimensionalNode: Annotation Summary";
      if (this->GetName())
    {
      os << " of " << this->GetName();
    }
      os << endl;
    }

  Superclass::PrintAnnotationInfo(os, indent, 0);

  os << indent << "rulerDistanceAnnotationFormat: ";
  if (this->AnnotationFormat)
    {
      os  << this->AnnotationFormat << "\n";
    }
  else
    {
      os  << "(None)" << "\n";
    }
  os << indent << "Resolution: " << this->Resolution << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationBidimensionalNode::SetBidimensionalMeasurement(double val1, double val2)
{
  this->measurement1 = val1;
  this->measurement2 = val2;

  this->InvokeEvent(vtkMRMLAnnotationBidimensionalNode::ValueModifiedEvent);
}

//---------------------------------------------------------------------------
std::vector<double> vtkMRMLAnnotationBidimensionalNode::GetBidimensionalMeasurement()
{

  std::vector<double> measurements;

  measurements.clear();
  measurements.push_back(this->measurement1);
  measurements.push_back(this->measurement2);

  return measurements;
}

//---------------------------------------------------------------------------
int vtkMRMLAnnotationBidimensionalNode::SetControlPoint(double newControl[3], int id)
{
  if (id < 0 || id > 3) {
    return 0;
  }

  int flag = Superclass::SetControlPoint(id, newControl,1,1);
  if (!flag)
  {
    return 0;
  }
  if (this->GetNumberOfControlPoints() < 3)
  {
    return 1;
  }

  if (this->GetNumberOfLines() == 2)
  {
    return 1;
  }

  this->AddLine(0,1,1,1);
  this->AddLine(1,2,1,1);
  return 1;
}
