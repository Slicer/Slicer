#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkSlicerDiffusionEditorLogic.h"

#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkDoubleArray.h"
#include "vtkNRRDReader.h"
#include <string>
#include "vtkTimerLog.h"

vtkCxxRevisionMacro(vtkSlicerDiffusionEditorLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerDiffusionEditorLogic);

//---------------------------------------------------------------------------
vtkSlicerDiffusionEditorLogic::vtkSlicerDiffusionEditorLogic(void)
  {
  this->ActiveDTINode = NULL;
  this->ActiveDWINode = NULL;
  this->StackPosition = 0;
  this->UndoFlag = 0;
  }

//---------------------------------------------------------------------------
vtkSlicerDiffusionEditorLogic::~vtkSlicerDiffusionEditorLogic(void)
  {
  if (this->ActiveDTINode)
    {
    vtkSetMRMLNodeMacro(this->ActiveDTINode, NULL)
    }
  if (this->ActiveDWINode)
    {
    vtkSetMRMLNodeMacro(this->ActiveDWINode, NULL);
    }
  this->ClearStack();
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorLogic::PrintSelf ( ostream& os, vtkIndent indent )
  {
  this->vtkObject::PrintSelf ( os, indent );
  os << indent << "vtkSlicerDiffusionEditorLogic: " << this->GetClassName ( ) << "\n";
  }

//---------------------------------------------------------------------------
int vtkSlicerDiffusionEditorLogic::AddGradients (const char* filename, int numberOfGradients, vtkDoubleArray *newBValue, 
                                                vtkDoubleArray *newGradients)
  {
  // format the filename
  std::string fileString(filename);
  for (unsigned int i = 0; i < fileString.length(); i++)
    {
    if (fileString[i] == '\\') fileString[i] = '/';
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
    std::string suffix(".txt");
    std::string::size_type pos = fileString.find(suffix);
    if(pos == std::string::npos)
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
      while (!file.eof()) //as long as more characters are in the file
        {
        char c;
        file.get(c);
        content<<c;
        }
      return this->ParseGradientsBvaluesToArray(content.str().c_str(), numberOfGradients, newBValue, newGradients);
      }
    return 0;
    }
  storageNode->Delete();
  dwiNode->Delete();
  return 1;
  }

//---------------------------------------------------------------------------
int vtkSlicerDiffusionEditorLogic::StringToDouble(const std::string &s, double &result)
  {
  std::stringstream stream (s);
  if(stream >> result)
    {
    if(stream.eof()) return 1; //if no more characters are in the stream
    }
  return 0;
  }

//---------------------------------------------------------------------------
int vtkSlicerDiffusionEditorLogic::ParseGradientsBvaluesToArray(const char *oldGradients, unsigned int numberOfGradients,
                                                 vtkDoubleArray *newBValues, vtkDoubleArray *newGradients)
  {
  if (oldGradients == NULL)
    {
    vtkErrorMacro(<< this->GetClassName() << ": oldGradients is NULL");
    return 0;
    }

  // read in current gradients 
  std::stringstream grad;
  grad << oldGradients;

  // save all gradient values in a vector
  std::vector<double> vec;
  while(!grad.eof())
    {
    std::string dummy = "";
    double newValue = -1;
    grad >> dummy;
    // check if valid value
    if(StringToDouble(dummy, newValue))  vec.push_back(newValue);
    else
      {
      std::string::size_type pos = dummy.find_first_of("=",0);
      std::string dummy2 = dummy.substr(pos+1);
      if(StringToDouble(dummy2, newValue))  vec.push_back(newValue);
      }
    }

  // exit if not the necessary number of values are input
  if(vec.size() != numberOfGradients*3+1)
    {
    vtkWarningMacro("given values "<<vec.size()<<", needed "<<numberOfGradients*3+1);
    return 0;
    }

  // set number of gradients to new arrays
  newGradients->SetNumberOfComponents(3);
  newGradients->SetNumberOfTuples(numberOfGradients);
  newBValues->SetNumberOfTuples(numberOfGradients);

  // create for bValue calculation
  vtkDoubleArray *gradientNormalized = vtkDoubleArray::New(); 
  double gradientRange[2];

  // set gradients and compute gradientNormalized values for bValue calculation
  for(unsigned int j = 1; j < vec.size(); j=j+3)
    {
    for(unsigned int i=j; i<j+3;i++)
      {
      newGradients->SetValue(i-1,vec[i]);
      }
    gradientNormalized->InsertNextValue(sqrt(vec[j]*vec[j]+vec[j+1]*vec[j+1]+vec[j+2]*vec[j+2]));
    }

  // get range for bValue calculation  
  gradientNormalized->GetRange(gradientRange);

  // compute bValues and set them
  for (unsigned int i=0; i<numberOfGradients;i++)
    {
    newBValues->SetValue(i,vec[0]*gradientNormalized->GetValue(i)/gradientRange[1]);
    }

  gradientNormalized->Delete();
  return 1;
  }

//---------------------------------------------------------------------------
std::string vtkSlicerDiffusionEditorLogic::ParseGradientsBvaluesToString(vtkDoubleArray *BValues, vtkDoubleArray *Gradients)
  {
  std::stringstream output;

  // create for bValue calculation
  double gradientRange[2];
  vtkDoubleArray *gradientNormalized = vtkDoubleArray::New();
  double g[3]; // single gradient

  // compute norm of each gradient 
  for(int i=0; i<Gradients->GetNumberOfTuples();i++)
    {
    Gradients->GetTuple(i,g);
    gradientNormalized->InsertNextValue(sqrt(g[0]*g[0]+g[1]*g[1]+g[2]*g[2]));
    }

  // get gradientRange of norm array
  gradientNormalized->GetRange(gradientRange);

  // compute bValue
  double bValue = -1;
  for(int i = 0; i< BValues->GetSize(); i++)
    {
    double numerator = BValues->GetValue(i)*gradientRange[1];
    double denominator = gradientNormalized->GetValue(i);
    if(!numerator == 0 && !denominator == 0)
      {
      bValue = numerator/denominator;
      break;
      }
    }

  // read in new bValue
  output << "DWMRI_b-value:= " << bValue << endl; 

  // read in new gradients
  // (this->Gradients->GetSize() is not always correct.)
  for(int i=0; i < Gradients->GetNumberOfTuples()*3; i=i+3)
    {
    output << "DWMRI_gradient_" << setfill('0') << setw(4) << i/3 << ":=" << " ";
    for(int j=i; j<i+3; j++)
      {
      output << Gradients->GetValue(j) << " ";
      }
    output << "\n";        
    }

  gradientNormalized->Delete();
  return output.str(); // string with bValue and gradients
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorLogic::SetActiveVolumeNode(vtkMRMLVolumeNode *node)
  {
  //clear stack bevor new node is activ
  this->ClearStack();
  if(node->IsA("vtkMRMLDiffusionWeightedVolumeNode"))
    {
    vtkSetMRMLNodeMacro(this->ActiveDWINode, node);
    vtkSetMRMLNodeMacro(this->ActiveDTINode, NULL); //set other node NULL
    }
  else if(node->IsA("vtkMRMLDiffusionTensorVolumeNode"))
    {
    vtkSetMRMLNodeMacro(this->ActiveDTINode, node);
    vtkSetMRMLNodeMacro(this->ActiveDWINode, NULL); //set other node NULL
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorLogic::SaveStateForUndoRedo()
  {
  //new node comes in, delete nodes in stack that are no longer reachable 
  //meaning: all nodes after current StackPosition
  if(!this->UndoRedoStack.empty() && this->StackPosition != this->UndoRedoStack.size())
    {
    while(this->StackPosition != this->UndoRedoStack.size())
      {
      this->UndoRedoStack.pop_back();
      }
    }
  //if undo mode is on, you don't need to save the current node, because it is already in
  //the stack
  if(!this->UndoFlag)
    {
    //create node and save it in stack
    if(this->ActiveDWINode != NULL)
      {
      vtkMRMLDiffusionWeightedVolumeNode *nodeToSave = vtkMRMLDiffusionWeightedVolumeNode::New();
      nodeToSave->Copy(this->ActiveDWINode);
      this->UndoRedoStack.push_back(nodeToSave);
      }
    else if(this->ActiveDTINode != NULL)
      {
      vtkMRMLDiffusionTensorVolumeNode *nodeToSave = vtkMRMLDiffusionTensorVolumeNode::New();
      nodeToSave->Copy(this->ActiveDTINode);
      this->UndoRedoStack.push_back(nodeToSave);
      }
    this->StackPosition = this->UndoRedoStack.size(); //update StackPosition
    }
  this->UndoFlag = 0; //no longer in undo mode
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorLogic::UpdateActiveVolumeNode(vtkMRMLVolumeNode *node)
  {
  vtkMatrix4x4 *m = vtkMatrix4x4::New();
  if(node->IsA("vtkMRMLDiffusionWeightedVolumeNode"))
    {
    vtkMRMLDiffusionWeightedVolumeNode *dwiNode = 
      vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(node);
    dwiNode->GetMeasurementFrameMatrix(m);
    this->ActiveDWINode->SetMeasurementFrameMatrix(m);
    this->ActiveDWINode->SetDiffusionGradients(dwiNode->GetDiffusionGradients());
    this->ActiveDWINode->SetBValues(dwiNode->GetBValues());
    }
  else if(node->IsA("vtkMRMLDiffusionTensorVolumeNode"))
    {
    vtkMRMLDiffusionTensorVolumeNode *dtiNode = 
      vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(node);
    dtiNode->GetMeasurementFrameMatrix(m);
    this->ActiveDTINode->SetMeasurementFrameMatrix(m);
    }
  m->Delete();
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorLogic::Restore()
  {
  if(!this->UndoRedoStack.empty())
    {
    vtkMRMLVolumeNode *node = this->UndoRedoStack.at(0);
    this->UpdateActiveVolumeNode(node); //display original node
    //delete all previous changes
    this->ClearStack();
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorLogic::Undo()
  {
  //the first time you click undo, save the parmeters
  //necessary for redo to go to the last state
  if(this->StackPosition == this->UndoRedoStack.size() && !this->UndoFlag)
    {
    this->SaveStateForUndoRedo();
    this->UndoFlag = 1; //from now on you are in undo mode
    }

  if(!this->UndoRedoStack.empty() && this->IsUndoable())
    {
    this->StackPosition--; //go to previous node
    vtkMRMLVolumeNode *node = this->UndoRedoStack.at(this->StackPosition-1);
    this->UpdateActiveVolumeNode(node); //display node
    }
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorLogic::Redo()
  {
  if(!this->UndoRedoStack.empty() && this->IsRedoable())
    {
    this->StackPosition++; //go to next node
    vtkMRMLVolumeNode *node;
    node = this->UndoRedoStack.at(this->StackPosition-1);
    this->UpdateActiveVolumeNode(node); //display node
    }
  }

//---------------------------------------------------------------------------
int vtkSlicerDiffusionEditorLogic::IsUndoable()
  {
  if((this->UndoRedoStack.size()+1 > this->StackPosition && this->StackPosition > 1) || !this->UndoFlag) return 1;
  else return 0;
  }

//---------------------------------------------------------------------------
int vtkSlicerDiffusionEditorLogic::IsRedoable()
  {
  if(0 < this->StackPosition && this->StackPosition < this->UndoRedoStack.size()) return 1;
  else return 0;
  }

//---------------------------------------------------------------------------
void vtkSlicerDiffusionEditorLogic::ClearStack()
  {
  if (!this->UndoRedoStack.empty())
    {
    for(unsigned int i = 0; i<this->UndoRedoStack.size(); i++)
      {
      this->UndoRedoStack.at(i)->Delete();
      this->UndoRedoStack.at(i) = NULL;
      }
    }
  this->UndoRedoStack.clear();
  this->StackPosition = 0;
  this->UndoFlag = 0;
  }
