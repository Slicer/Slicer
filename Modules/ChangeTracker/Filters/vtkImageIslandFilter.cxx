/*=========================================================================

  Program:   Slicer
  Language:  C++
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Applications/GUI/Slicer3.cxx $
  Date:      $Date: 2009-04-15 06:29:13 -0400 (Wed, 15 Apr 2009) $
  Version:   $Revision: 9206 $

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "vtkImageIslandFilter.h"
#include "vtkObjectFactory.h"

#define IMAGEISLANDFILTER_DYNAMIC 0
#define IMAGEISLANDFILTER_STATIC 1

#define IMAGEISLANDFILTER_PRINT_DISABLED 0
#define IMAGEISLANDFILTER_PRINT_COMMON 1
#define IMAGEISLANDFILTER_PRINT_COMPREHENSIVE 2

template <class T> void IslandMemory<T>::Print() {
    cout << "ID:         " << this->ID         << endl;
    cout << "StartVoxel: " << this->StartVoxel << endl;
    cout << "Size:       " << this->Size       << endl;
    cout << "Label:      " << this->Label      << endl;
    if (this->Next) this->Next->Print();
}

template <class T> int IslandMemory<T>::PrintLine () {
    cout << "IslandMemory<T>::PrintLine: island " << this->ID  << " has label " << int(this->Label) << " with " << this->Size << " voxels" << endl;
    // cout << this->StartVoxel % 57 << " " << int( this->StartVoxel / 57) << endl;
    if (this->Next) return this->Next->PrintLine() +1;
    return 1;
}

template <class T> void IslandMemory<T>::PrintLine2 () {
    cout << "IslandMemory: island " << this->ID  << " has label " << this->Label << " with " << this->Size << " voxels" << endl;
    if (this->Next) return this->Next->PrintLine2();
}

/* Test run by executing the follwoing 
setenv VTK_BIN_DIR /home/ai2/kpohl/slicer_devel/vtk4.0/VTK-build
setenv LD_LIBRARY_PATH ${VTK_BIN_DIR}/bin:${LD_LIBRARY_PATH}
./GetVolumeIslands -InputImage1 /home/ai2/kpohl/slicer_devel/slicer2_lmi/EMLocalSegment/tests/TestImageEMLocalSegmentResult -IsLittleEndian1 -Numx1 256 -Numy1 256 -Numz1 2 -Spacing1 1.0 1.0 1.0 | sort -k 8 -n -r
./mathImage -fc isl -pr ../tests/TestImageEMLocalSegmentResult -ir 1 2 -va 0
*/

// NewID = -1 is the default => automatic ID will be generated and returned later 
// MaxSize = up to which size do we want to sort the list ! 

template <class T> int IslandMemory<T>::AddIsland(int NewStartVoxel, int NewSize, T NewLabel, int NewID, int MaxSize) {
  // cout << "IslandMemory::AddIsland " <<  NewStartVoxel << " Size: " << NewSize << " Label " << int(NewLabel) << " ID " << NewID << " " << MaxSize << endl;
    if (this->ID == -1) {
      if (NewID > -1) this->ID  = NewID;
      else this->ID = 1;
      this->StartVoxel = NewStartVoxel;
      this->Size       = NewSize;
      this->Label      = NewLabel;       
      return this->ID;
    } 
    IslandMemory<T> *NewIsland = new IslandMemory<T>;
    IslandMemory<T> *Ptr = this;
    // 1. Figure out ID 

    // Very first Island
    // if (NewID > -1) {    
    //   while ((Ptr) && Ptr->ID != NewID) Ptr = Ptr->Next;
    //   // Then island is already in the list
    //   if (Ptr) {
    //     // I am checking this way if an island alredy exists - programmer might use this function - not necessarily an error
    //     return -1;
    //   }
    // } else {
    if (NewID < 0) {
      NewID = 1;
      while (Ptr != NULL) {
         if (Ptr->ID > NewID) NewID = Ptr->ID;
         Ptr  = Ptr->Next;
      }
      NewID ++; 
    }
    // Values have to swapt bc very first Island is smaller then current island
    if (NewSize < this->Size) {
      NewIsland->ID         = this->ID;
      NewIsland->StartVoxel = this->StartVoxel;
      NewIsland->Size       = this->Size;
      NewIsland->Label      = this->Label;
      NewIsland->Next       = this->Next;

      this->ID         = NewID;
      this->StartVoxel = NewStartVoxel;
      this->Size       = NewSize;
      this->Label      = NewLabel; 
      this->Next       = NewIsland; 
      return this->ID ;
    }
    NewIsland->ID         = NewID;
    NewIsland->StartVoxel = NewStartVoxel;
    NewIsland->Size       = NewSize;
    NewIsland->Label      = NewLabel; 
    // 2. Figure out where to enter new island in the list 
    Ptr = this;
    // So we do not have to search through the entire list every time
    if ((MaxSize > -1)  &&  (NewSize > MaxSize)) NewSize = MaxSize; 
    while ((Ptr->Next != NULL) && (Ptr->Next->Size < NewSize)) Ptr = Ptr->Next;
    NewIsland->Next = Ptr->Next; 
    Ptr->Next = NewIsland;
  
    return NewIsland->ID;
}

// Returns pointer to the next valid element - note if we delete first element then special rule applies 
template <class T> IslandMemory<T>*  IslandMemory<T>::DeleteIsland(int DelID) {
  if (this->ID != DelID) { 
    IslandMemory<T> *Ptr = this;
    while  (Ptr->Next && (Ptr->Next->ID != DelID)) Ptr = Ptr->Next;
    //cerr << "IslandMemory::DeleteIsland: Could not delete Island with ID " << DelID << ". An Island with this ID doesn ot exists !" << endl;
    assert(Ptr->Next); 

    IslandMemory<T>  *DelPtr = Ptr->Next;
    Ptr->Next = Ptr->Next->Next;
    DelPtr->Next = NULL;
    delete DelPtr;
    return Ptr->Next;
  }
  if (this->Next) {
    // We copy over so that we do not loose info
    this->ID         = this->Next->ID;
    this->StartVoxel = this->Next->StartVoxel;
    this->Size       = this->Next->Size;
    this->Label      = this->Next->Label;
    IslandMemory<T>  *DelPtr  = this->Next;
    this->Next       = this->Next->Next;
    DelPtr->Next = NULL;
    delete DelPtr;
    return this;   
  }
  // If only one island exists => just initialize
  this->CreateVariables();
  return NULL;
}
template <class T> void IslandMemory<T>::CreateVariables () {
  // cout << "CreateVariables  " << this << endl;
    this->StartVoxel = -1;this->Next =NULL; this->ID = -1; this->Size = 0;
}

template <class T> void IslandMemory<T>::SetSize(int NewSize, IslandMemory<T>* SetIsland, int MaxSize) {
  // cout << "IslandMemory<T>::SetSize " << NewSize << " old size : " << SetIsland->GetSize() << " ID: " << SetIsland->GetID()<< endl;
  int  setSize        = SetIsland->Size;

  if (setSize == NewSize) return;
  // Last Element 

  if ((SetIsland->Next == NULL) && (NewSize > setSize)) {
    SetIsland->Size = NewSize;
    return;
  } 

  // Nothing needs to be changed
  if ((MaxSize > -1) && (NewSize >  MaxSize) && (setSize >  MaxSize)) {
    SetIsland->Size = NewSize;
    return;
  } 

  T    setLabel       = SetIsland->Label;
  int  SetStartVoxel  = SetIsland->StartVoxel;
  int  SetID          = SetIsland->ID;
  IslandMemory<T>* Ptr =  this->DeleteIsland(SetID);
  // Cannot be last element
  assert(Ptr);
  // cout << "ID " << SetID << endl;
  if ((NewSize > setSize) && (SetID > -1)) {
    int result = Ptr->AddIsland(SetStartVoxel, NewSize, setLabel, SetID,MaxSize); 
    //assert(result > -1);
    if (!(result > -1))
      {
      cout << "Result " << result << " not > -1\n";
      return;
      } 
  } else {
    int result = this->AddIsland(SetStartVoxel, NewSize, setLabel, SetID, MaxSize);
    //assert( result > -1);
    if (!(result > -1))
      {
      cout << "Result " << result << " not > -1\n";
      return;
      }
  }
}
//------------------------------------------------------------------------------
template <class T> int IslandMemoryGroup<T>::GetSize() {
  if (!this->List) {
    cout << "ERROR: IslandMemoryGroup<T>::GetSize(): List is not defined" << endl;
    exit(1);
  }
  return this->List->GetSize();
}

template <class T> int IslandMemoryGroup<T>::AddIsland(int NewStartVoxel, int NewSize, T NewLabel, int NewID) {
  // cout << "IslandMemoryGroup<T>::AddIsland " << NewID << endl;
  // always define newid
  assert(NewID > 0);

  int initSize = (NewSize > this->MaxSize ? this->MaxSize: NewSize); 
  assert(initSize > 0);
  if (this->Size < 0) {
    this->Size = initSize;
    this->List = new IslandMemory<T>;
    return this->List->AddIsland(NewStartVoxel, NewSize, NewLabel, NewID, this->MaxSize);
  }

  int result =  (this->AddGroup(NewSize))->List->AddIsland(NewStartVoxel, NewSize, NewLabel, NewID, this->MaxSize);

  // cout << "End of Add Group " << endl;
  return result;

}

template <class T> IslandMemoryGroup<T>* IslandMemoryGroup<T>::AddGroup(int NewSize) {
  int initSize = (NewSize > this->MaxSize ? this->MaxSize: NewSize); 

  if (initSize == this->Size) return this; 
  if (initSize < this->Size) {
    IslandMemoryGroup<T> *NewGroup = new IslandMemoryGroup<T>;
    
    NewGroup->Size       = this->Size;
    NewGroup->List       = this->List;
    NewGroup->Next       = this->Next;
    NewGroup->MaxSize    = this->MaxSize;

    this->Size       = initSize;
    this->List       = new IslandMemory<T>; 
    this->Next       = NewGroup; 
    return this;
  } 

  IslandMemoryGroup<T> *Ptr = this;
  while ((Ptr->Next != NULL) && (Ptr->Next->Size < initSize)) Ptr = Ptr->Next;
  if (Ptr->Next && Ptr->Next->Size == initSize) return Ptr->Next; 

  // Create a Ne Group
  IslandMemoryGroup<T> *NewGroup = new IslandMemoryGroup<T>;
  NewGroup->Size       = initSize;
  NewGroup->List       = new IslandMemory<T>; ;
  NewGroup->Next       = Ptr->Next;
  NewGroup->MaxSize    = this->MaxSize;
  Ptr->Next = NewGroup;

  return  NewGroup; 
}

template <class T> IslandMemory<T>*  IslandMemoryGroup<T>::DeleteIsland(int DelID, int DelSize) {
  // cout << "IslandMemoryGroup: DeleteIsland " << DelID << " " << DelSize << endl;
  assert(DelSize > 0) ;
  DelSize = (this->MaxSize < DelSize ? this->MaxSize : DelSize);

  IslandMemory<T>* result = NULL;
  IslandMemoryGroup<T> *Ptr = this;

  if (this->Size != DelSize) {
    while(Ptr->Next && (Ptr->Next->Size != DelSize)) Ptr= Ptr-> Next;
    assert(Ptr->Next);
    result = Ptr->Next->List->DeleteIsland(DelID);
    if (result) return result;

    // If result == NULL then list could be empty 
    if ( Ptr->Next->GetID() < 0) {
      // If so then delete entire list
      IslandMemoryGroup<T>  *DelPtr = Ptr->Next;
      Ptr->Next = Ptr->Next->Next;
      DelPtr->Next = NULL;
      delete DelPtr;
    } else {
      // Otherwise just jump to next list 
      Ptr = Ptr->Next;
    }
    return (Ptr->Next ? Ptr->Next->List : NULL);  
  }
    
  result = this->List->DeleteIsland(DelID);
  if (result) return result;
  // Just got to end of list - so jump to next line
  if (this->GetID() > -1) return (this->Next ? this->Next->List : NULL);

  // Delete First Element - if next element exists just copy second to first elemnt 
  if (this->Next) {
    // Maxsize does not have to be copied over bc it should be the same accross all elements
    this->Size      = this->Next->Size;
    // first remove list 
    delete this->List;
    // then copy 
    this->List      = this->Next->List;

    IslandMemoryGroup<T>  *DelPtr  = this->Next;
    this->Next       = this->Next->Next;
   
    // Initialize deleting  
    DelPtr->Next = NULL;
    DelPtr->List = NULL;
    delete DelPtr;

    // Finished 
    return this->List;  
  }
  // cout << "Entire list is empty -debug" << endl; 
  // exit(0);
  // Initialize
  
  delete this->List; 
  int oldMaxSize = this->MaxSize;
  this->CreateVariables();
  this->MaxSize = oldMaxSize;

  return NULL;
}

template <class T> IslandMemory<T>*  IslandMemoryGroup<T>::GetIsland(int getID, int getSize) {
     IslandMemoryGroup<T>* ptr = this; 

     // Have to look through everything
     if (getSize < 0) {
       IslandMemory<T>* result = NULL;
       while(ptr && !result) {
     result = ptr->List->GetIsland(getID);
     ptr = ptr->Next;
       }
       return result;
     }
     ptr = this->GetGroup(getSize);
     if (ptr) return ptr->List->GetIsland(getID);
     return NULL;
}

template <class T> void IslandMemoryGroup<T>::SetSize(int NewSize, IslandMemory<T>* SetIsland) {
  // cout << "IslandMemoryGroup<T>::SetSize " << NewSize << " old size : " << SetIsland->GetSize() << " ID: " << SetIsland->GetID()<< endl;

  int  setSize        = SetIsland->GetSize();
  if (setSize == NewSize) return;
  // Last Element 
  if ((NewSize >  this->MaxSize) && (setSize >  this->MaxSize)) {
    SetIsland->SetSize(NewSize,SetIsland,this->MaxSize); 
    return;
  }

  T    setLabel       = SetIsland->GetLabel();
  int  SetStartVoxel  = SetIsland->GetStartVoxel();
  int  SetID          = SetIsland->GetID();
  this->DeleteIsland(SetID,setSize);
  this->AddIsland(SetStartVoxel, NewSize, setLabel, SetID);
}

template <class T> int IslandMemoryGroup<T>::PrintLine () {
  // cout << "---- IslandMemoryGroup Address: " << this << endl;

  if (this->Size == -1) { 
    cout << "No islands" << endl; return 0 ;
  }   
  int result = this->List->PrintLine();
  if (this->Next) result += this->Next->PrintLine();
  return result;
}

//------------------------------------------------------------------------------

template<class T> void EMStack<T>::Print() {
  if (!this->Valid) cout << "Stack is Empty" << endl;
  else {
    cout << this->Entry << endl;
    if (this->Next) this->Next->Print();
  }
}




//------------------------------------------------------------------------------
vtkImageIslandFilter* vtkImageIslandFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageIslandFilter");
  if(ret) return (vtkImageIslandFilter*)ret;
  // If the factory was unable to create the object, then create it here.
  return new vtkImageIslandFilter;
}

//----------------------------------------------------------------------------
vtkImageIslandFilter::vtkImageIslandFilter()
{
  IslandMinSize = 0;        // Smalles size of islands allowed, otherwise will be erased
  IslandInputLabelMin =  -1;
  IslandInputLabelMax =  -1;
  IslandOutputLabel = -1;
  IslandROI = NULL;
  PrintInformation = IMAGEISLANDFILTER_PRINT_DISABLED;
  NeighborhoodDim = IMAGEISLANDFILTER_NEIGHBORHOOD_3D;
  RemoveHoleOnlyFlag = 0;
}

vtkImageIslandFilter::~vtkImageIslandFilter(){ }

//----------------------------------------------------------------------------
void vtkImageIslandFilter::ComputeInputUpdateExtent(int inExt[6], int vtkNotUsed(outExt)[6])
{
  this->GetInput()->GetWholeExtent(inExt);
}


//----------------------------------------------------------------------------
void vtkImageIslandFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

// To chage anything about output us this executed before Thread
//----------------------------------------------------------------------------
void vtkImageIslandFilter::ExecuteInformation(vtkImageData *inData, vtkImageData *outData) 
{
  outData->SetOrigin(inData->GetOrigin());
  outData->SetNumberOfScalarComponents(1);
  outData->SetWholeExtent(inData->GetWholeExtent());
  outData->SetSpacing(inData->GetSpacing());
  outData->SetScalarType(inData->GetScalarType());
}
//----------------------------------------------------------------------------
template <class T>
static void vtkImageIslandFilter_FindIslands(int* Checked,int index,int & IslandSize,T Label, int ID, T* inPtr, int const SizeX, int const SizeY, int const SizeXY, int const SizeZ, EMStack<int> *Stack) {
   if ((index % SizeX)              && (inPtr[index-1]     ==  Label)  && (!Checked[index-1])) {
      Checked[index-1] = ID;
      IslandSize++;
      Stack->Push(index-1);
   }
   if (((index+1) % SizeX)          && (inPtr[index+1]     ==  Label)  && (!Checked[index+1])) {
      Checked[index+1] = ID;
      IslandSize++;
      Stack->Push(index+1);
   }
   if ((int(index/SizeX)%SizeY)    && (inPtr[index-SizeX] ==  Label) && (!Checked[index-SizeX])) {
     Checked[index-SizeX] = ID;
     IslandSize++;
     Stack->Push(index-SizeX);
   }
   if ((int(index/SizeX +1)%SizeY) && (inPtr[index+SizeX] ==  Label) && (!Checked[index+SizeX])) {
     Checked[index+SizeX] = ID;
     IslandSize++;
     Stack->Push(index+SizeX);
   }
   if ((int(index/(SizeXY)))  && (inPtr[index-SizeXY] ==  Label) && (!Checked[index-SizeXY])) {
     Checked[index-(SizeXY)] = ID;
     IslandSize++;
     Stack->Push(index-(SizeXY));
   }
   if ((int(index/(SizeXY)+1) % SizeZ)  && (inPtr[index+SizeXY] ==  Label) && (!Checked[index+(SizeXY)])) {
     Checked[index+(SizeXY)] = ID;
     IslandSize++;
     Stack->Push(index+(SizeXY));
   }
}

//----------------------------------------------------------------------------
template <class T>
static void vtkImageIslandFilter_GetBorder_Island_ImageEdgeFlag(EMStack<int>* OuterBorder,int index, T Label, T* OutPtr, char *Checked, int const SizeX, int const SizeY, 
                                int const SizeXY, int const SizeZ, EMStack<int> *Stack, int &ImageEdgeFlag) {
  if (index % SizeX) {
    if (!Checked[index-1]) {
      Checked[index-1] = 1;
      if (OutPtr[index-1] == Label) Stack->Push(index-1);
      else if (OuterBorder) OuterBorder->Push(index-1);
    }
  } else if (SizeX >2)   ImageEdgeFlag = 1; 

  if ((index+1) % SizeX) {
    if (!Checked[index+1]) {
      Checked[index+1] = 1;
      if (OutPtr[index+1] == Label) Stack->Push(index+1);
      else if (OuterBorder) OuterBorder->Push(index+1);
    }
  } else if (SizeX >2)  ImageEdgeFlag = 1; 

  if (int(index/SizeX)%SizeY) {
    if (!Checked[index-SizeX]) {
      Checked[index-SizeX] = 1;
      if (OutPtr[index-SizeX] == Label) Stack->Push(index-SizeX);
      else if (OuterBorder) OuterBorder->Push(index-SizeX);
    }
  } else if (SizeY >2)  ImageEdgeFlag = 1; 

  if (int(index/SizeX+1)%SizeY) {
    if (!Checked[index+SizeX]) {
      Checked[index+SizeX] = 1;
      if (OutPtr[index+SizeX] == Label) Stack->Push(index+SizeX);
      else if (OuterBorder) OuterBorder->Push(index+SizeX); 
    }
  } else if (SizeY >2)  ImageEdgeFlag = 1; 

  if (int(index/(SizeXY))) {
    if (!Checked[index-SizeXY]) {
      Checked[index-SizeXY] = 1;
      if (OutPtr[index-SizeXY] == Label) Stack->Push(index-SizeXY);
      else if (OuterBorder) OuterBorder->Push(index-SizeXY);
    }
  } else if (SizeZ > 2) ImageEdgeFlag = 1; 

  if (int(index/(SizeXY)+1) % SizeZ) {
    if (!Checked[index+SizeXY]) {
      Checked[index+SizeXY] = 1;
      if (OutPtr[index+SizeXY] == Label) Stack->Push(index+SizeXY);
      else if (OuterBorder) OuterBorder->Push(index+SizeXY);
    }
  } else if (SizeZ > 2)  ImageEdgeFlag = 1; 
}

template <class T>
static int vtkImageIslandFilter_DetermineBorder_Island_ImageEdgeFlag(T* outPtr, const int StartingVoxelOfIsland, IslandMemoryGroup<T> *Mem,int const SizeX, int const SizeY, int const SizeXY, int const SizeZ, const int SizeXYZ, const int *ImageIslandID, EMStack<int> *outOuterBorder,  EMStack<int> * outIslandVoxels,IslandMemory<T>* outBorderIslands, int &ImageEdgeFlag) {
    char *Checked = new char[SizeXYZ];
    memset(Checked,0,sizeof(char)*SizeXYZ);   
    EMStack<int> *IslandVoxelStack = new EMStack<int>;
    int contIndex;
    int BorderVoxelIndex;
    int outNumberOfBorderingIslands = 0;

    EMStack<int> *OuterBorder = new EMStack<int>;

    IslandVoxelStack->Push(StartingVoxelOfIsland);

    ImageEdgeFlag = 0;
    while (IslandVoxelStack->Pop(contIndex)) {
      outIslandVoxels->Push(contIndex);
      vtkImageIslandFilter_GetBorder_Island_ImageEdgeFlag(OuterBorder,contIndex,outPtr[contIndex],outPtr,Checked,SizeX, SizeY, SizeXY, SizeZ,IslandVoxelStack, ImageEdgeFlag);
    }


    while (OuterBorder->Pop(BorderVoxelIndex)) { 
      if (!(outBorderIslands->GetIsland(ImageIslandID[BorderVoxelIndex]))) {
    IslandMemory<T>* BorderIsl = Mem->GetIsland(ImageIslandID[BorderVoxelIndex],-1);
    // cout << "VI:" << BorderVoxelIndex << " SizeXYZ " << SizeXYZ << " ID: " << ImageIslandID[BorderVoxelIndex] << endl;
    // Mem->PrintLine();
    assert(BorderIsl);
    outBorderIslands->AddIsland(BorderIsl);
    outNumberOfBorderingIslands ++;
      }
      outOuterBorder->Push(BorderVoxelIndex);
    }
    delete[] Checked;
    delete IslandVoxelStack;

    return outNumberOfBorderingIslands;
}

//----------------------------------------------------------------------------
// Deletes first island on the memory stack from the island 
// MaxOverallIslandSize is the minimum size of islands in this filter that wont be destroyed anymore. 
template <class T>
static IslandMemory<T>* vtkImageIslandFilter_DeleteIslandFromImage(IslandMemoryGroup<T> *Mem, IslandMemory<T> *DeleteIslandPtr, T* outPtr, int const SizeX, int const SizeY, 
                                   int const SizeXY, int const SizeZ, int SizeXYZ, int *ImageIslandID, int MaxOverallIslandSize, 
                                   const int RemoveHoleOnlyFlag) {
  // Make sure that the island memory stack is not empty
  // cout << "============== vtkImageIslandFilter_DeleteIslandFromImage Start Deleting ID "<< DeleteIslandPtr->GetID() << " ========== " << endl;
  assert(Mem->GetID() > -1);
  
  EMStack<int> *OuterBorder     = new EMStack<int>;
  EMStack<int> *IslandVoxels    = new EMStack<int>;

  // OuterBorder->Print();

  IslandMemory<T>* BorderIslands = new IslandMemory<T> ;
  IslandMemory<T>* BorderIslandsPtr;

  // --------------------------------------------
  // cout << "== Define Border" << endl;
  int ImageEdgeFlag;
  int NumberOfBorderingIslands =
       vtkImageIslandFilter_DetermineBorder_Island_ImageEdgeFlag(outPtr,DeleteIslandPtr->GetStartVoxel(), Mem, SizeX, SizeY, SizeXY, SizeZ, SizeXYZ, ImageIslandID, OuterBorder, 
                                 IslandVoxels,BorderIslands,ImageEdgeFlag);

  // cout << "== Bordering Islands" << endl; 
  // BorderIslands->PrintLine();

  // --------------------------------------------
  // cout << "3. Figure out which Labels to be assigned"<< endl;

  T   MaxLabel; 
  int MaxNumber; 
  int MaxID;
  IslandMemory<T>* MaxIslandPtr = NULL;

  if (1) {
    // What label has the island bordering the removal island
    T *BorderLabelsName     = new T[NumberOfBorderingIslands];
    // How many voxels of that label exists
    int *BorderLabelsNumber = new int[NumberOfBorderingIslands];
    int BorderVoxelIndex;

    // Determine Labels around border and number of voxels with that label
    memset(BorderLabelsNumber,0,sizeof(int)*NumberOfBorderingIslands);  
    
    // Count how many voxels with a certain label are bordering island
    OuterBorder->Pop(BorderVoxelIndex);
    // BorderLabelsName[0] = outPtr[BorderVoxelIndex];
    // BorderLabelsNumber[0] = 1;

    while (OuterBorder->Pop(BorderVoxelIndex)) {
      int i = 0;
      while ((BorderLabelsName[i] != outPtr[BorderVoxelIndex]) && (BorderLabelsNumber[i] > 0)) i++;
      if (BorderLabelsNumber[i] == 0) BorderLabelsName[i] = outPtr[BorderVoxelIndex];
      BorderLabelsNumber[i] ++;
    }
    // Only remove islands that are holes, i.e. encircled by one label
    if (RemoveHoleOnlyFlag) {
      // DeleteIslandPtr->PrintLine();
      // cout << ImageEdgeFlag << endl;
      if (ImageEdgeFlag) {
    delete[] BorderLabelsName;
    delete[] BorderLabelsNumber;
    delete OuterBorder;
        delete IslandVoxels; 
    delete BorderIslands;
    return DeleteIslandPtr->GetNext(); 
      }
      T Label    = BorderLabelsName[0];
      for (int i = 1 ; i < NumberOfBorderingIslands; i++) {
    // Otherwise it is not an island if BorderLabelsNumber[i] == 0 !
    if ((BorderLabelsName[i] != Label) && (BorderLabelsNumber[i])) {
      delete[] BorderLabelsName;
      delete[] BorderLabelsNumber;
      delete OuterBorder;
      delete IslandVoxels; 
      delete BorderIslands;
      return DeleteIslandPtr->GetNext(); 
    }
      }
    } 

    // Determine Maximum Label
    MaxLabel    = BorderLabelsName[0];
    MaxNumber = BorderLabelsNumber[0];

    int i = 0;
    while ((i < NumberOfBorderingIslands) && (BorderLabelsNumber[i] > 0)) {
      if (BorderLabelsNumber[i] >  MaxNumber) {
    MaxNumber = BorderLabelsNumber[i] ;
    MaxLabel  = BorderLabelsName[i];
      }
      i++;
    }

    // Determine Island ID and Ptr 
    BorderIslandsPtr = BorderIslands;
    int MaxIslandNumber = 0;
    MaxID = -1;
      
    while (BorderIslandsPtr) { 
      if (BorderIslandsPtr->GetLabel() == MaxLabel && BorderIslandsPtr->GetSize() > MaxIslandNumber) { 
    MaxIslandNumber = BorderIslandsPtr->GetSize();
    MaxID = BorderIslandsPtr->GetID();
    MaxIslandPtr = BorderIslandsPtr;
      }
      BorderIslandsPtr = BorderIslandsPtr->GetNext();
    }

    delete[] BorderLabelsName;
    delete[] BorderLabelsNumber;
  }

  // --------------------------------------------
  // cout << "4. Generate one big Island" << endl;

  if (1) {
    int StartIndex; 
    char *Checked = new char[SizeXYZ];
    EMStack<int> *IslandVoxelStack = new EMStack<int>;

    BorderIslandsPtr = BorderIslands;
    
    int IslandSize = MaxIslandPtr ->GetSize();
    int InitializeChecked = 1;

    // Do not change order of step a and b otherwise it will take a lot longer bc now every island is attached to every other island 
    // a) Add border islands to the maximum island. They all will be linked as soon as we change label of island in step b) 
    while (BorderIslandsPtr) { 
      if (BorderIslandsPtr->GetLabel() == MaxLabel) {
    // if island is greater MaxOverallIslandSize then the island wont be altered anymore so we can leave the label 
    if ((BorderIslandsPtr->GetID() != MaxID) && (BorderIslandsPtr->GetSize() < MaxOverallIslandSize)) {
      // cout << "Delete Island " << BorderIslandsPtr->GetID() << " with size " << BorderIslandsPtr->GetSize() << endl;
      if (InitializeChecked) {
        InitializeChecked = 0;
        memset(Checked,0,sizeof(char)*SizeXYZ); 
      }

      IslandSize += BorderIslandsPtr->GetSize();
      StartIndex = BorderIslandsPtr->GetStartVoxel();

      Checked[StartIndex] = 1;
      IslandVoxelStack->Push(StartIndex);
      int contIndex;
      int blub;

      while (IslandVoxelStack->Pop(contIndex)) {
        ImageIslandID[contIndex] = MaxID;
        vtkImageIslandFilter_GetBorder_Island_ImageEdgeFlag(NULL, contIndex,outPtr[StartIndex],outPtr,Checked,SizeX, SizeY, SizeXY, SizeZ,IslandVoxelStack,blub);
      }
      if (1) { 
        // cout << "What  " << endl;
        // Mem->PrintLine();
        Mem->DeleteIsland(BorderIslandsPtr->GetID(),BorderIslandsPtr->GetSize());
        // cout << "Why " << endl;
        // Mem->PrintLine();
      }
    } 
      }
      BorderIslandsPtr = BorderIslandsPtr->GetNext();
    }
   
    // b) Change the label for each voxel of the island and delete it 

    IslandSize += DeleteIslandPtr->GetSize();
    int contIndex;
    while (IslandVoxels->Pop(contIndex)) {
      outPtr[contIndex] = MaxLabel;
      ImageIslandID[contIndex] = MaxID;
    }
    // c) Update New Island  only if it is below max size - otherwise it does not have an impact  
    Mem->SetSize(IslandSize,MaxIslandPtr);

    delete[] Checked;
    delete IslandVoxelStack;
  }

  // --------------------------------------------
  // cout << "5. Delete variables" << endl; 

  delete OuterBorder;
  delete BorderIslands;

  // Do not delete until the end so that we can provide a pointer the next valid element in the stack 
  IslandMemory<T>* NEXT = Mem->DeleteIsland(DeleteIslandPtr->GetID(),DeleteIslandPtr->GetSize()); 

  // cout << "============== vtkImageIslandFilter_DeleteIslandFromImage Start ========== " << endl;

  return NEXT;

}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T> static int vtkImageIslandFilter_DefineIsland(int index, EMStack<int> *VoxelStackMem, int *Checked,
                                T  *inPtr,int const SizeX, int const SizeY, int const SizeXY, int const SizeZ,  int IslandID) {

  // Initializing 
  T label  = inPtr[index];
  int IslandSize = 1;
  EMStack<int> *VoxelStack = new EMStack<int>;
  Checked[index] = IslandID;
      
  // Define Island
  if (VoxelStackMem) VoxelStackMem->Push(index);
  // Push in all the neighbors of voxel index that have the same label
  vtkImageIslandFilter_FindIslands(Checked,index,IslandSize,label,IslandID,inPtr,SizeX, SizeY, SizeXY, SizeZ, VoxelStack);

  // Keep on adding voxels with labels to the stack that are neighbors to voxels in current stack until no new ones can be added 
  while (VoxelStack->Pop(index)) {
    if (VoxelStackMem) VoxelStackMem->Push(index);
    vtkImageIslandFilter_FindIslands(Checked,index,IslandSize,label,IslandID,inPtr, SizeX, SizeY, SizeXY, SizeZ, VoxelStack);
  }

  // Clean Up
  delete VoxelStack;

  return IslandSize;
}


template <class T>
static void vtkImageIslandFilterExecute(vtkImageIslandFilter *self, T *inPtr, int inExt[6], short* IslandROIPtr, T *outPtr)
{
  // cout << "vtkImageIslandFilterExecute Start " << endl;

  int index = 0; 
  int IslandCount = 0;

  // find the region to loop over
  int SizeX = inExt[1] - inExt[0] + 1;
  int SizeY = inExt[3] - inExt[2] + 1; 
  int SizeZ = inExt[5] - inExt[4] + 1;

  int SizeXY = SizeX * SizeY;
  int SizeXYZ = SizeXY*SizeZ;
  memcpy(outPtr,inPtr,sizeof(T)*SizeXYZ);

  // The Slices of the images 
  int *Checked = new int[SizeXYZ];
  memset(Checked,0,SizeXYZ*sizeof(int));
  int DeletedIslands = 0;


  // Define type of island removal
  int islandInputLabelMin  = self->GetIslandInputLabelMin();
  int islandInputLabelMax  = self->GetIslandInputLabelMax();
  int IslandInputLabelFlag = ((islandInputLabelMax > -1)  && (islandInputLabelMin > -1));
  int IslandOutputLabel    = self->GetIslandOutputLabel();
  int IslandMinSize        = self->GetIslandMinSize();
  int PrintInformation = self->GetPrintInformation();

  // Only remove holes 
  int RemoveHoleOnlyFlag = self->GetRemoveHoleOnlyFlag();
  
  //Both are defined or not
  int IslandRemovalType = (IslandOutputLabel < 0 ? IMAGEISLANDFILTER_DYNAMIC : IMAGEISLANDFILTER_STATIC);



  // Currently only implemented as static 
  if (IslandRemovalType == IMAGEISLANDFILTER_DYNAMIC) assert(!IslandROIPtr);

  IslandMemoryGroup<T> *Mem = (IslandRemovalType == IMAGEISLANDFILTER_STATIC ? NULL : new IslandMemoryGroup<T>);
  if (Mem) Mem->SetMaxSize(IslandMinSize);

  EMStack<int> *VoxelStackMem = (IslandRemovalType == IMAGEISLANDFILTER_STATIC ? new EMStack<int> : NULL);

  if (PrintInformation == IMAGEISLANDFILTER_PRINT_COMPREHENSIVE) {
    cout << "vtkImageIslandFilterExecute: Detect Islands in the image" << endl;
    cout << "vtkImageIslandFilterExecute: IslandRemovalType ";
    if (IslandRemovalType == IMAGEISLANDFILTER_DYNAMIC) cout << "Dynamic" << endl;
    else if (IslandRemovalType == IMAGEISLANDFILTER_STATIC) cout << "Static" << endl;
    else cout << "Unknown" << endl;

  }
  

  while (index < SizeXYZ) {
    if (!Checked[index]) {
      switch (IslandRemovalType) {
        case IMAGEISLANDFILTER_DYNAMIC: {
          IslandCount ++;

          int NewID = IslandCount; 
          int IslandSize = vtkImageIslandFilter_DefineIsland(index, VoxelStackMem, Checked, inPtr, SizeX, SizeY, SizeXY, SizeZ, NewID);
           // If it is not the same then island must be alread part of it 
      // cout << "Mem->AddIsland(index,IslandSize,inPtr[index],IslandCount) " << index << " " << IslandSize << " " << inPtr[index] << " " << IslandCount << endl;
          int currentIslandCount = Mem->AddIsland(index,IslandSize,inPtr[index],IslandCount);
      //assert(currentIslandCount == IslandCount);
          if (currentIslandCount != IslandCount)
            {
            cout << "Current island count " << currentIslandCount << " is not equal to island count " << IslandCount << endl;
            return;
            }

          break;
        }
        case IMAGEISLANDFILTER_STATIC: {
           // Only delete it if it is a certain lable and if activated - touching the ROI
           if ((IslandInputLabelFlag  && ((T(islandInputLabelMin) >  inPtr[index]) || (T(islandInputLabelMax) <  inPtr[index])))
                     || (IslandROIPtr && !*IslandROIPtr)) break;

           IslandCount ++;
           int IslandSize = vtkImageIslandFilter_DefineIsland(index, VoxelStackMem, Checked, inPtr, SizeX, SizeY, SizeXY, SizeZ, IslandCount);
           int contIndex; 
           // Relabel all of them  if true and otherwise do not do anything
           if (IslandSize < IslandMinSize) {
              DeletedIslands ++;
              while (VoxelStackMem->Pop(contIndex))  outPtr[contIndex] = T(IslandOutputLabel);
           } else while (VoxelStackMem->Pop(contIndex));
            break;
      }
      default :
      cout << "Do not know type " << IslandRemovalType << endl;
      exit(1);
      }
    }

    if (IslandROIPtr) IslandROIPtr ++;
    index ++;
  }

  if (IslandRemovalType == IMAGEISLANDFILTER_STATIC) {
    // Already deleted all Islands 
    if (PrintInformation > IMAGEISLANDFILTER_PRINT_DISABLED) 
    cout << "Deleted " << DeletedIslands << " from " << IslandCount << " between label " << islandInputLabelMin << " and " << islandInputLabelMax <<  endl;
    delete VoxelStackMem;
    delete[] Checked;
    return;
  }

  // Now we define Dynamic Displacement 

  // Have to delete islands now 
  // If you want to print out all islands 
  if (PrintInformation > IMAGEISLANDFILTER_PRINT_DISABLED) {
    if (PrintInformation == IMAGEISLANDFILTER_PRINT_COMPREHENSIVE) Mem->PrintLine();
    cout << "There are currently " << IslandCount << " Islands in the image." << endl; 
  }
 
  // Never take out the very last element = this is the entire image 
  int IslandDeleteCount = 0; 
  IslandMemoryGroup<T> *MemGroupPtr = Mem;
  int IslandCurrentSize = MemGroupPtr->GetSize();

  // Do not delete largest group 
  while ((IslandCurrentSize < IslandMinSize) && MemGroupPtr->GetNextGroup()) {
    // All islands with the same size
    // cout << "========================== Remove " << IslandCurrentSize << endl;
   
    IslandMemory<T> *MemIslandPtr = MemGroupPtr->GetList();
    int GroupSize = MemGroupPtr->GetSize();

    while (MemIslandPtr) {
      if (!IslandInputLabelFlag  || 
          ((T(islandInputLabelMin) <= MemIslandPtr->GetLabel()) && (MemIslandPtr->GetLabel() <=  T(islandInputLabelMax)))) {
        IslandDeleteCount ++;

#ifndef _WIN32  
        if (PrintInformation > IMAGEISLANDFILTER_PRINT_DISABLED && !(IslandDeleteCount % 100)) {  
      cout << MemIslandPtr->GetSize() << " "; 
      cout.flush();
    }
#endif
    MemIslandPtr = vtkImageIslandFilter_DeleteIslandFromImage(Mem, MemIslandPtr, outPtr, SizeX, SizeY, SizeXY, 
                                  SizeZ, SizeXYZ, Checked,IslandMinSize, RemoveHoleOnlyFlag);

    // NULL should never be able to be returned as their is always the entire image as last image.
      } else {
    // You have to delete entries otherwise it is an endless loop at this point
    MemIslandPtr = MemIslandPtr->GetNext(); 
      }
      // This is not necessary so that we can later check if we still have a valid pointer in MemGroupPtr 
      if (MemIslandPtr  && MemIslandPtr->GetSize() != GroupSize) MemIslandPtr = NULL;
    }
    // Detect Next Group to work on - have to start all over bc Group might be deleted !
    MemGroupPtr = Mem;
    while (MemGroupPtr->GetNextGroup() && MemGroupPtr->GetSize() <= IslandCurrentSize) {
      MemGroupPtr = MemGroupPtr->GetNextGroup();

    }
    IslandCurrentSize = MemGroupPtr->GetSize();
  }

  if (PrintInformation > IMAGEISLANDFILTER_PRINT_DISABLED) {
    if (IslandDeleteCount) {
      if (IslandDeleteCount > 99) cout << endl;
      if (PrintInformation == IMAGEISLANDFILTER_PRINT_COMPREHENSIVE) {
    cout << "================== New list of islands ===========" << endl;
    IslandCount = Mem->PrintLine();
      } else {
    IslandCount = Mem->NumberOfIslands();
      }
      cout << "New number of Islands: " << IslandCount << endl; 
    } else {
      cout << "No Islands where deleted " << endl;
    }
  }

  if (Mem) delete Mem;
  delete[] Checked;
  // cout << "vtkImageIslandFilterExecute End " << endl;
}

// Executes vtkImageIslandFilterExecute slice by slice 
template <class T>
static void vtkImageIslandFilterExecuteBySlice(vtkImageIslandFilter *self, T *inPtr, int inExt[6], short* IslandROIPtr, T *outPtr) {
  int Extent2D[6] ={inExt[0],inExt[1],inExt[2],inExt[3],0,0};
  int DimZ = inExt[5] - inExt[4] +1; 
  int VoxelsPerSlice = (inExt[1] - inExt[0] + 1)* (inExt[3] - inExt[2] + 1); 
  for (int z = 0; z < DimZ; z++) {
    if (self->GetPrintInformation() > IMAGEISLANDFILTER_PRINT_DISABLED) 
      cout << "======= Slice " << z << " =============" << endl;
    vtkImageIslandFilterExecute(self, inPtr,Extent2D, IslandROIPtr, outPtr);
    inPtr += VoxelsPerSlice;
    if (IslandROIPtr) IslandROIPtr += VoxelsPerSlice;
    outPtr += VoxelsPerSlice;
  }
}

//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.

// void vtkImageIslandFilter::ThreadedExecute(vtkImageData *inData, vtkImageData *outData,int outExt[6], int id)
void vtkImageIslandFilter::ExecuteData(vtkDataObject *)
{
  void *inPtr;
  void *outPtr;

  int inExt[6];
  vtkIdType inInc[3];
  int outExt[6];
  vtkIdType outInc[3];
  // Necessary  for VTK
  this->ComputeInputUpdateExtent(inExt,outExt);
 // vtk4
  vtkImageData *inData  = this->GetInput();
  vtkImageData *outData = this->GetOutput();
  outData->SetExtent(this->GetOutput()->GetWholeExtent());
  outData->AllocateScalars();
  outData->GetWholeExtent(outExt);
  // vtk4
  vtkDebugMacro(<< "Execute: inData = " << inData << ", outData = " << outData);
 
  if (inData == NULL) {
    vtkErrorMacro(<< "Input " << 0 << " must be specified.");
    return;
  }
  if (inData->GetNumberOfScalarComponents() != 1) {
     vtkErrorMacro(<< "Number Of Scalar Components for Input has to be 1.");
     return;
  }
  inData->GetContinuousIncrements(inExt, inInc[0], inInc[1], inInc[2]);
  outData->GetContinuousIncrements(outExt, outInc[0], outInc[1], outInc[2]);
  if (!((inInc[0] == 0) && (inInc[1] == 0) && (inInc[2] == 0) && (outInc[0] == 0) && (outInc[1] == 0) && (outInc[2] == 0))) {
     vtkErrorMacro(<< "Increments for input and output have to be 0!");
     return;
  }

  // Check IslandROI 
  short *islandROIPtr = NULL;
  if (IslandROI) {
    int islandExt[6];
    vtkIdType islandInc[3];
    this->IslandROI->GetWholeExtent(islandExt);
    this->IslandROI->GetContinuousIncrements(islandExt, islandInc[0], islandInc[1], islandInc[2]);
    assert((islandExt[1] - islandExt[0] == inExt[1] - inExt[0]) && (islandExt[3] - islandExt[2] == inExt[3] - inExt[2]) && (islandExt[5] - islandExt[4] == inExt[5] - inExt[4])); 
    assert(islandInc[0] == 0);
    assert(islandInc[1] == 0);
    assert(islandInc[2] == 0);
    assert(VTK_SHORT == this->IslandROI->GetScalarType());
    islandROIPtr = (short*)this->IslandROI->GetScalarPointerForExtent(islandExt);
  }

  inPtr = inData->GetScalarPointerForExtent(inExt);
  outPtr = outData->GetScalarPointerForExtent(outExt);

  if (PrintInformation > IMAGEISLANDFILTER_PRINT_DISABLED) { 
    cout << "====================================================== " << endl;
    cout << "vtkImageIslandFilter::ExecuteData: Delete Islands smaller " << this->IslandMinSize << " in Mode " << (this->NeighborhoodDim == IMAGEISLANDFILTER_NEIGHBORHOOD_3D ? 3 : 2) << "D" << endl;  
  }


  if (this->NeighborhoodDim == IMAGEISLANDFILTER_NEIGHBORHOOD_3D) {
    switch (inData->GetScalarType()) {
      vtkTemplateMacro(vtkImageIslandFilterExecute(this, (VTK_TT *)(inPtr),inExt, islandROIPtr, (VTK_TT *)(outPtr)));
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
  } else {
      switch (inData->GetScalarType()) {
    vtkTemplateMacro(vtkImageIslandFilterExecuteBySlice(this, (VTK_TT *)(inPtr),inExt, islandROIPtr, (VTK_TT *)(outPtr)));
      default:
    vtkErrorMacro(<< "Execute: Unknown ScalarType");
    return;
      }
  }
  if (PrintInformation > IMAGEISLANDFILTER_PRINT_DISABLED) {
    cout << "vtkImageIslandFilter::ExecuteData: Finished Deleting " << endl;
    cout << "====================================================== " << endl;
  }

}

template <class T>
static void vtkImageIslandFilter_GetMaxIslandSize(T* inPtr, int LabelMin , int LabelMax, int inExt[6], int& maxSize ) {

   int SizeX = inExt[1] - inExt[0] + 1;
   int SizeY = inExt[3] - inExt[2] + 1; 
   int SizeZ = inExt[5] - inExt[4] + 1; 
   int SizeXY = SizeX * SizeY;
   int SizeXYZ = SizeXY*SizeZ;

   int *Checked = new int[SizeXYZ];
   memset(Checked,0,SizeXYZ*sizeof(int));

   int index = 0; 
   int IslandCount = 0;
   maxSize = -1; 

   while (index < SizeXYZ) {
     if (!Checked[index]) {

       if ((T(LabelMin) <=  inPtr[index]) && (inPtr[index] <= T(LabelMax))) {
    IslandCount ++;
    int IslandSize = vtkImageIslandFilter_DefineIsland(index, NULL, Checked, inPtr, SizeX, SizeY, SizeXY, SizeZ, IslandCount);
    if (IslandSize > maxSize ) {maxSize= IslandSize;}
       }
     }
     index ++;
   }
   delete[] Checked;
}


int vtkImageIslandFilter::GetMaxIslandSize(vtkImageData *InputData) {
   int islandInputLabelMin  = this->GetIslandInputLabelMin();
   int islandInputLabelMax  = this->GetIslandInputLabelMax();
   if ((islandInputLabelMax < 0 )  || (islandInputLabelMin < 0 )) {
     vtkErrorMacro(<< "vtkImageIslandFilter::GetMaxIslandSize: Define IslandInputLabelMax and IslandInputLabelMin before calling this function!");
     return -1;
   }

   int inExt[6];
   InputData->GetWholeExtent(inExt);
   void* inPtr = InputData->GetScalarPointerForExtent(inExt);
   int result;
   switch (InputData->GetScalarType()) {
      vtkTemplateMacro(vtkImageIslandFilter_GetMaxIslandSize((VTK_TT *)(inPtr), islandInputLabelMin,  islandInputLabelMax, inExt, result)); 
   default:
    vtkErrorMacro(<< "Execute: Unknown ScalarType");
    return -1;
   }
   return result;

}
