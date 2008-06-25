#include "vtkMRMLEMSClassInteractionMatrixNode.h"
#include <sstream>
#include "vtkMRMLScene.h"
#include <algorithm>

#include <vtksys/ios/sstream>

//-----------------------------------------------------------------------------
vtkMRMLEMSClassInteractionMatrixNode* 
vtkMRMLEMSClassInteractionMatrixNode::
New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSClassInteractionMatrixNode");
  if(ret)
    {
    return (vtkMRMLEMSClassInteractionMatrixNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSClassInteractionMatrixNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* 
vtkMRMLEMSClassInteractionMatrixNode::
CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkMRMLEMSClassInteractionMatrixNode");
  if(ret)
    {
    return (vtkMRMLEMSClassInteractionMatrixNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLEMSClassInteractionMatrixNode;
}

//-----------------------------------------------------------------------------
vtkMRMLEMSClassInteractionMatrixNode::vtkMRMLEMSClassInteractionMatrixNode()
{
  this->NumberOfClasses     = 0;
  this->DirectionNames.resize(6);
  this->DirectionNames[DirectionWest] = "West";
  this->DirectionNames[DirectionNorth] = "North";
  this->DirectionNames[DirectionUp] = "Up";
  this->DirectionNames[DirectionEast] = "East";
  this->DirectionNames[DirectionSouth] = "South";
  this->DirectionNames[DirectionDown] = "Down";
  this->Matrices.resize(6);
}

//-----------------------------------------------------------------------------
vtkMRMLEMSClassInteractionMatrixNode::~vtkMRMLEMSClassInteractionMatrixNode()
{
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSClassInteractionMatrixNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);

  for (unsigned int direction = 0; direction < this->DirectionNames.size();
       ++direction)
    {
    of << indent << this->DirectionNames[direction] << "=\"";
    for (unsigned int r = 0; r < this->GetNumberOfClasses(); ++r)
      {
      for (unsigned int c = 0; c < this->GetNumberOfClasses(); ++c)  
        {
        of << this->Matrices[direction][r][c] << " ";
        }
      if (r < this->GetNumberOfClasses() - 1)
        {
        of << "| ";
        }
      }
    of << "\" ";
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSClassInteractionMatrixNode::ReadXMLAttributes(const char** attrs)
{
  Superclass::ReadXMLAttributes(attrs);

  //
  // we assume an even number of attrs
  //
  const char* key;
  const char* val;
  while (*attrs != NULL)
    {
    key = *attrs++;
    val = *attrs++;
    
    vtkstd::vector<vtkstd::string>::iterator directionPosition = 
      vtkstd::find(this->DirectionNames.begin(), this->DirectionNames.end(),
                 key);
    if (directionPosition != this->DirectionNames.end())
      {
      int directionIndex = directionPosition - this->DirectionNames.begin();

      // remove visual row seperators
      std::string valStr(val);
      vtkstd::replace(valStr.begin(), valStr.end(), '|', ' ');

      // read data into a temporary vector
      vtksys_stl::stringstream ss;
      ss << valStr;
      double d;
      vtksys_stl::vector<double> tmpVec;
      while (ss >> d)
        {
        tmpVec.push_back(d);
        }

      // update number of classes
      // assume square matrix
      unsigned int side = (unsigned int) sqrt((double)tmpVec.size());
      if (this->GetNumberOfClasses() != side)
        {
        this->SetNumberOfClasses(side);
        }      

      // copy data
      unsigned int i = 0;
      for (unsigned int r = 0; r < this->GetNumberOfClasses(); ++r)
        {
        for (unsigned int c = 0; c<this->GetNumberOfClasses(); ++c)
          {
          this->Matrices[directionIndex][r][c] = tmpVec[i++];
          }   
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSClassInteractionMatrixNode::Copy(vtkMRMLNode *rhs)
{
  Superclass::Copy(rhs);
  vtkMRMLEMSClassInteractionMatrixNode* node = 
    (vtkMRMLEMSClassInteractionMatrixNode*) rhs;

  this->SetNumberOfClasses(node->GetNumberOfClasses());
  this->Matrices = node->Matrices;
}

//-----------------------------------------------------------------------------
void vtkMRMLEMSClassInteractionMatrixNode::PrintSelf(ostream& os, 
                                                 vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  for (unsigned int direction = 0; direction < this->DirectionNames.size();
       ++direction)
    {
    os << indent << this->DirectionNames[direction] << ": ";
    for (unsigned int r = 0; r < this->GetNumberOfClasses(); ++r)
      {
      for (unsigned int c = 0; c < this->GetNumberOfClasses(); ++c)
        {
        os << this->Matrices[direction][r][c] << " ";
        }
      if (r < this->GetNumberOfClasses() - 1)
        {
        os << "| ";
        }
      }
    os << "\n";
    }
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSClassInteractionMatrixNode::
SetNumberOfClasses(unsigned int n)
{
  if (n != this->GetNumberOfClasses())
    {
    this->NumberOfClasses = n;
      
    // resize matrix, don't preserve data!
    this->Matrices.clear();
    this->Matrices.resize(this->DirectionNames.size());
    for (unsigned int directionIndex = 0; 
         directionIndex < this->DirectionNames.size(); ++directionIndex)
      {
      // each matrix is initialized to the identity
      Matrices[directionIndex].resize(n);
      for (unsigned int i = 0; i < n; ++i)
        {
        this->Matrices[directionIndex][i].resize(n, 0.0);
        this->Matrices[directionIndex][i][i] = 1.0;
        }
      }
    }
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSClassInteractionMatrixNode::
AddClass()
{
  ++this->NumberOfClasses; 
  for (unsigned int directionIndex = 0;
       directionIndex < this->DirectionNames.size(); ++directionIndex)
    {
    // add extra column to current rows
    for (unsigned int i = 0; i < this->NumberOfClasses-1; ++i)
      {
      this->Matrices[directionIndex][i].push_back(0.0);
      }

    // add new row
    this->Matrices[directionIndex].
      push_back(vtkstd::vector<double>(this->NumberOfClasses, 0.0));
    this->Matrices[directionIndex]
      [this->NumberOfClasses-1][this->NumberOfClasses-1] = 1.0;
    }
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSClassInteractionMatrixNode::
RemoveNthClass(int index)
{
  for (unsigned int directionIndex = 0;
       directionIndex < this->DirectionNames.size(); ++directionIndex)
    {
    // remove last column
    for (unsigned int i = 0; i < this->NumberOfClasses; ++i)
      {
      this->Matrices[directionIndex][i].
        erase(this->Matrices[directionIndex][i].begin() + index);
      }
    // remove last row
    this->Matrices[directionIndex].
      erase(this->Matrices[directionIndex].begin() + index);
    }
  --this->NumberOfClasses;
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSClassInteractionMatrixNode::
MoveNthClass(int fromIndex, int toIndex)
{
  for (unsigned int directionIndex = 0;
       directionIndex < this->DirectionNames.size(); ++directionIndex)
    {
    // move column to new spot
    for (unsigned int i = 0; i < this->NumberOfClasses; ++i)
      {
      double movingParam = this->Matrices[directionIndex][i][fromIndex];
      this->Matrices[directionIndex][i].
        erase(this->Matrices[directionIndex][i].begin() + fromIndex);
      this->Matrices[directionIndex][i].
        insert(this->Matrices[directionIndex][i].begin() + toIndex, 
               movingParam);
      }
    
    // move row to new spot
    vtkstd::vector<double> movingVec = 
      this->Matrices[directionIndex][fromIndex];
    this->Matrices[directionIndex].
      erase(this->Matrices[directionIndex].begin() + fromIndex);
    this->Matrices[directionIndex].
      insert(this->Matrices[directionIndex].begin() + toIndex, 
             movingVec);
    }
}

//-----------------------------------------------------------------------------
double
vtkMRMLEMSClassInteractionMatrixNode::
GetClassInteraction(int direction, int row, int column) const
{
  return this->Matrices[direction][row][column];
}

//-----------------------------------------------------------------------------
void
vtkMRMLEMSClassInteractionMatrixNode::
SetClassInteraction(int direction, int row, int column, double value)
{
  this->Matrices[direction][row][column] = value;
}

