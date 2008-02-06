#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerGradientEditorLogic.h"

#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkDoubleArray.h"
#include "vtkNRRDReader.h"
#include <string.h>

vtkCxxRevisionMacro(vtkSlicerGradientEditorLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerGradientEditorLogic);

//---------------------------------------------------------------------------
vtkSlicerGradientEditorLogic::vtkSlicerGradientEditorLogic(void)
  {
  }

//---------------------------------------------------------------------------
vtkSlicerGradientEditorLogic::~vtkSlicerGradientEditorLogic(void)
  {
  }

//---------------------------------------------------------------------------
void vtkSlicerGradientEditorLogic::PrintSelf ( ostream& os, vtkIndent indent )
  {
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerGradientEditorLogic: " << this->GetClassName ( ) << "\n";
  }

//---------------------------------------------------------------------------
int vtkSlicerGradientEditorLogic::AddGradients (const char* filename, int numberOfGradients, vtkDoubleArray *newBValue, 
      vtkDoubleArray *newGradients)
  {
  // format the filename
  std::string fileString(filename);
  for (unsigned int i = 0; i < fileString.length(); i++)
    {
    if (fileString[i] == '\\')
      {
      fileString[i] = '/';
      }
    }

  // Instanciation of the I/O mechanism
  vtkMRMLNRRDStorageNode *storageNode = vtkMRMLNRRDStorageNode::New();
  vtkMRMLDiffusionWeightedVolumeNode *dwiNode = vtkMRMLDiffusionWeightedVolumeNode::New();
  dwiNode->SetBValues(newBValue);
  dwiNode->SetDiffusionGradients(newGradients);
  storageNode->SetFileName(fileString.c_str());

  if (!storageNode->ReadData(dwiNode))
    {
    storageNode->Delete();
    dwiNode->Delete();
    //check if txt file
    std::string a(".txt");
    std::string::size_type i = fileString.find(a);
    if(i=0)
      {
      //no txt file or valid nhdr
      vtkWarningMacro("no valid file");
      return 0;
      }
     
    ifstream file;
    file.open(fileString.c_str(), ios::in);

    if(file.good())
      {
      std::stringstream content;
      file.seekg(0L, ios::beg);
      while (!file.eof())
        {
        char c;
        file.get(c);
        content<<c;
        }
      vtkWarningMacro("\n"<<content.str().c_str());
      return this->ParseGradients(content.str().c_str(), numberOfGradients, newBValue, newGradients);
      }
    }
   return 0;
  }

//---------------------------------------------------------------------------
int vtkSlicerGradientEditorLogic::StringToDouble(const std::string &s, double &result)
  {
  std::stringstream stream (s);
  if(stream >> result)
    {
    if(stream.eof())
      {
      return 1;
      }
    }
  return 0;
  }

//---------------------------------------------------------------------------
int vtkSlicerGradientEditorLogic::ParseGradients(const char *oldGradients, int numberOfGradients,
                                                  vtkDoubleArray *newBValues, vtkDoubleArray *newGradients)
  {
  if (oldGradients == NULL || oldGradients == "")
    {
    vtkErrorMacro(<< this->GetClassName() << ": oldGradients is NULL");
    return 0;
    }

  // read in current gradients 
  std::stringstream grad;
  grad << oldGradients;

  // save all values in a vector
  std::vector<double> vec;
  while(!grad.eof())
    {
    std::string dummy = "";
    double newValue = -1;
    grad >> dummy;
    if(StringToDouble(dummy, newValue))
      {
      vec.push_back(newValue);
      }      
    }

  // exit if too many or to less values are input
  if(vec.size() != numberOfGradients*3+1)
    {
    vtkWarningMacro("given values "<<vec.size()<<" needed "<<numberOfGradients*3+1);
    return 0;
    }

  vtkDoubleArray *factor = vtkDoubleArray::New();
  newGradients->SetNumberOfComponents(3);
  newGradients->SetNumberOfTuples(numberOfGradients);
  newBValues->SetNumberOfTuples(numberOfGradients);

  // set gradients and factor values
  for(unsigned int j = 1; j < vec.size(); j=j+3)
    {
    for(unsigned int i=j; i<j+3;i++)
      {
      newGradients->SetValue(i-1,vec[i]);
      }
    factor->InsertNextValue(sqrt(vec[j]*vec[j]+vec[j+1]*vec[j+1]+vec[j+2]*vec[j+2]));
    }

  // get range
  double range[2];
  factor->GetRange(range);

  // set BValues
  for (int i=0; i<numberOfGradients;i++)
    {
    newBValues->SetValue(i,vec[0]*factor->GetValue(i)/range[1]);
    }

  factor->Delete();
  return 1;
  }

