/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMAtlasSuperClass.cxx,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
#include "vtkImageEMAtlasSuperClass.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "assert.h"
//------------------------------------------------------------------------
vtkImageEMAtlasSuperClass* vtkImageEMAtlasSuperClass::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageEMAtlasSuperClass");
  if(ret)
  {
    return (vtkImageEMAtlasSuperClass*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageEMAtlasSuperClass;

}


//------------------------------------------------------------------------------
// Define Procedures for vtkImageEMAtlasSuperClass
//------------------------------------------------------------------------------
void vtkImageEMAtlasSuperClass::CreateVariables() {
  this->NumClasses          = 0;  
  this->ClassList           = NULL;
  this->ClassListType       = NULL;
  this->MrfParams           = NULL;
  this->ParentClass         = NULL;
  this->PrintFrequency      = 0;
  this->PrintBias           = 0;
  this->PrintLabelMap       = 0;

  this->StopEMMaxIter       = 0; 
  this->StopMFAMaxIter       = 0; 

  this->InitialBiasFilePrefix = NULL;
  this->PredefinedLabelMapPrefix = NULL; 
 
}

//------------------------------------------------------------------------------
void vtkImageEMAtlasSuperClass::DeleteSuperClassVariables() {
  int z,y;
  if (this->MrfParams) {
    for (z=0; z< 6; z ++) { 
      for (y=0; y < this->NumClasses; y++) delete[] this->MrfParams[z][y];
      delete[] this->MrfParams[z];
    }
    delete[] this->MrfParams;
  }
  if (this->ClassList)     delete[] this->ClassList; 
  if (this->ClassListType) delete[] this->ClassListType;  

  this->MrfParams           = NULL;
  this->ClassList           = NULL;
  this->ClassListType       = NULL;
  this->ParentClass         = NULL;
  this->NumClasses    = 0;
}
//------------------------------------------------------------------------------
void vtkImageEMAtlasSuperClass::AddSubClass(void* ClassData, classType initType, int index)
{
  if (index < 0) {
    vtkEMAddErrorMessage("Index is not set correctly");
    return;
  }
  // Only if input images are changing  - automatically updated works  
  // this->Modified();

  if (this->NumClasses <= index) {
    vtkImageEMAtlasSuperClass *parent = this->ParentClass;
    classType*  oldClassListType;
    void**      oldClassList;
    int         oldNumClasses = this->NumClasses;
    // Delete and remeber old class list
    if (oldNumClasses) {
      oldClassListType = new classType[oldNumClasses];
      oldClassList     = new void*[oldNumClasses];
      for (int i = 0; i <oldNumClasses; i++) {
    oldClassList[i] = this->ClassList[i];
    oldClassListType[i] = this->ClassListType[i];
      }
      this->DeleteSuperClassVariables();
    }
    // Create  New Classes 
    this->ClassListType = new classType[index+1];
    this->ClassList     = new void*[index+1];
    for (int i = 0; i <= index; i++) this->ClassList[i] = NULL; 
    
    int z,y;
    this->MrfParams = new double**[6];
    for (z=0; z < 6; z++) {
      this->MrfParams[z] = new double*[index+1];
      for (y=0;y <= index; y ++) 
    this->MrfParams[z][y] = new double[index+1];
    }

    for (z=0; z < 6; z++) { 
      for (y=0;y <= index; y ++) memset(this->MrfParams[z][y], 0,NumClasses*sizeof(double)); 
    }

 
    // Copy over old values and delete them 
    if (oldNumClasses) {
      // Copy everything
      this->ParentClass = parent;
      for (int i = 0; i < oldNumClasses; i++) {
        this->ClassList[i] =  oldClassList[i];
        this->ClassListType[i] =  oldClassListType[i];
      }
      // Delete 
      delete[]  oldClassList;
      delete[]  oldClassListType;

    }
    this->NumClasses = index + 1;
  }
  this->ClassList[index] = ClassData;
  this->ClassListType[index] = initType;
}

//------------------------------------------------------------------------------
void vtkImageEMAtlasSuperClass::SetMarkovMatrix(double value, int k, int j, int i) {
  if ((j<0) || (j >= this->NumClasses) ||
      (i<0) || (i >= this->NumClasses) ||
      (k<0) || (k > 5) || (value < 0) || (value > 1)) {
    vtkEMAddErrorMessage("Error:vtkImageEMAtlasSuperClass::SetMarkovMatrix Incorrect input: " << value << " " << k << " " <<  j << " " << i << " " << this->NumClasses);
    return;
  }
  this->MrfParams[k][j][i] = value;
}

//------------------------------------------------------------------------------
// if flag is set => includes subclasses of type SUPERCLASS
int vtkImageEMAtlasSuperClass::GetTotalNumberOfClasses(bool flag) {
  int result = 0;
  for (int i=0;  i< this->NumClasses; i++) {
    if (this->ClassListType[i] == SUPERCLASS) {
      result += ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetTotalNumberOfClasses(flag); 
      if (flag == true) result ++;
    }  else { result ++;}
  }
  return result;
}

//------------------------------------------------------------------------------
// Returns a list of all labels (excluding the once with -1 and double once) in numerical order ,e.g. 0 1 4 5  
// Make sure LabelList is  with 0 -> otherwise won't get right list 
int vtkImageEMAtlasSuperClass::GetAllLabels(short *LabelList, int result, int Max) {
  int i,j,k, label;
  for (i=0;  i< this->NumClasses; i++) {
    if (this->ClassListType[i] == SUPERCLASS) {
      result = ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetAllLabels(LabelList,result,Max);
    } else {
      j= 0;
      label = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetLabel();
      if (label > -1) { 
    if (result) while ((j< result) && (LabelList[j] < label)) j++;
    // Make sure not doubles are in it 
    if ((j== result) || LabelList[j] > label) {
      result ++;
      if (j < Max) {
        k = result -1;
        while (k>j) {
          LabelList[k] =  LabelList[k-1];
          k--;
        }
        LabelList[j] = label;
      }
    }
      }
    }
  }
  return result;
}

//------------------------------------------------------------------------------
int vtkImageEMAtlasSuperClass::LabelAllSuperClasses(short *TakenLabelList, int Result,  int Max) {
  int i,j,k;
  short label=0;
  // You have to have atleast one label defined
  assert(Result && Result <= Max);

  for (i=0;  i < this->NumClasses; i++) {
    if (this->ClassListType[i] == SUPERCLASS) {
      Result = ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->LabelAllSuperClasses(TakenLabelList,Result,Max);

      // Kilian Jan06: There used to be a bug in this code - for example if TakenLabelList label list consists of (e.g. 0 1 2 3 0 0) => Max = 5
      //               then the old code woud it assign the label 4 at position 5 => it is not part of taken label anymore and the TakenLabelList 
      //               is not in order.

      // Just add at the end of the list
      int PreLabelID = TakenLabelList[Result -1] + 1;
      j = Result;
      
      // Othierwise Max to small
      assert(j < Max);

      // Need to make an empty space !
      if ( PreLabelID < TakenLabelList[j]) {
         k = Max -1;
         while (k>j) {TakenLabelList[k] =  TakenLabelList[k-1];k--;}
      } 

      TakenLabelList[j] =  PreLabelID;
      ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->Label =  PreLabelID;
      Result ++;
    }
  }
  return Result;
}
//------------------------------------------------------------------------------
int vtkImageEMAtlasSuperClass::GetTotalNumberOfProbDataPtr() {
  int result = 0;
  for (int i=0;  i< this->NumClasses; i++) {
    if (this->ClassListType[i] == SUPERCLASS) {
      result += ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetTotalNumberOfProbDataPtr(); 
    } else { 
      if (((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataWeight() >  0.0) result++;
    }
  }
  return result;
}

//------------------------------------------------------------------------------
int vtkImageEMAtlasSuperClass::GetProbDataPtrFlag() {
  for (int i = 0; i < this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) {
      if (((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataPtr()) return 1;
    } else {
      if (((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetProbDataPtrFlag()) return 1;
    }
  }
  return 0;
}

//------------------------------------------------------------------------------
int vtkImageEMAtlasSuperClass::GetProbDataPtr(void **PointerList, int index) {
 int i;
  for (i = 0; i <this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) {
      PointerList[index] = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataPtr();
      index ++;
    } else index = ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetProbDataPtr(PointerList,index); 
  }
  return index;
} 

//------------------------------------------------------------------------------
int vtkImageEMAtlasSuperClass::GetProbDataIncYandZ(int* ProbDataIncY,int* ProbDataIncZ,int index) {
  int i;
  for (i = 0; i <this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) {
      if (((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataWeight() > 0.0) {
        ProbDataIncY[index] = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataIncY();
        ProbDataIncZ[index] = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataIncZ();
        index ++;
      }
    } else index = ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetProbDataIncYandZ(ProbDataIncY,ProbDataIncZ,index); 
  }
  return index;
} 

//------------------------------------------------------------------------------
int vtkImageEMAtlasSuperClass::GetTissueDefinition(int *LabelList,double** LogMu, double ***LogCov, int index) {
  int i;
  for (i = 0; i < this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) {
      LogMu[index]         = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetLogMu();
      LogCov[index]        = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetLogCovariance();
      LabelList[index]     = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetLabel();
      index ++;
    } else index = ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetTissueDefinition(LabelList,LogMu, LogCov,index);
  }
  return index;
} 

//------------------------------------------------------------------------------
classType vtkImageEMAtlasSuperClass::GetClassType(void* active) {
  int i = 0; 
  while ((i < this->NumClasses) && (active != this->ClassList[i])) i++;
  if (i == this->NumClasses) return CLASS;
  return this->ClassListType[i];                     
}

//------------------------------------------------------------------------------
void vtkImageEMAtlasSuperClass::PrintSelf(ostream& os,vtkIndent indent) {
  os << indent << "---------------------------------------- SUPERCLASS ----------------------------------------------" << endl;
  this->vtkImageEMGenericClass::PrintSelf(os,indent); 
  os << indent << "NumClasses:              " << this->NumClasses << endl;
  os << indent << "PrintFrequency:          " << this->PrintFrequency << endl;
  os << indent << "PrintBias:               " << this->PrintBias<< endl;
  os << indent << "PrintLabelMap:           " << this->PrintLabelMap << endl;
  os << indent << "StopEMMaxIter:                 " << this->StopEMMaxIter << endl;
  os << indent << "StopMFAMaxIter:                " << this->StopMFAMaxIter << endl;

  os << indent << "InitialBiasFilePrefix:     " << (this->InitialBiasFilePrefix ? this->InitialBiasFilePrefix : "(none)") << "\n";
  os << indent << "PredefinedLabelMapPrefix:         " << (this->PredefinedLabelMapPrefix ? this->PredefinedLabelMapPrefix : "(none)") << "\n";

  // No need of expensive call to new for a simple array
  static const char * const Directions[] = {
    "West ",
    "North",
    "Up   ",
    "East ",
    "South",
    "Down "
  };
  os << indent << "MrfParams:               " << endl;
  for (int z=0; z < 6; z++) { 
    os << indent << "   " << Directions[z] << ":    ";   
    for (int y=0;y < this->NumClasses; y ++) {
      if (y) os << "| ";
      for (int x=0;x < this->NumClasses; x ++) os << this->MrfParams[z][y][x] << " " ;
    }
    os << endl;
  }
  for (int i =0; i < this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) ((vtkImageEMAtlasClass*)this->ClassList[i])->PrintSelf(os,indent.GetNextIndent());
    else ((vtkImageEMAtlasSuperClass*)this->ClassList[i])->PrintSelf(os,indent.GetNextIndent());
  }
}


//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageEMAtlasSuperClass::ExecuteData(vtkDataObject *)
{
  // std::cerr << "start vtkImageEMAtlasSuperClass::ExecuteData" << endl; 
   vtkDebugMacro(<<"ExecuteData()"); 
   // ==================================================
   // Check existence of subclasses and update them 
   this->ResetWarningMessage();
   if (this->GetErrorFlag()) 
     vtkEMAddWarningMessage("The error flag for this module was set with the following messages (the error messages will be reset now:\n"<<this->GetErrorMessages());

   this->ResetErrorMessage();
   this->vtkImageEMGenericClass::ExecuteData(NULL);
   // Error Occured
   // Kilian change this 
   if (this->GetErrorFlag()) return;

   if (!this->NumClasses) {
     vtkEMAddErrorMessage("No sub classes defined!");
     return;
   }

   
   {
     int ProbDataPtrIndex = -1; 

     for (int i = 0; i <this->NumClasses; i++) {
       if (!this->ClassList[i]) {
         vtkEMAddErrorMessage("Class with index "<< i <<" is not defined!");
         return;
       }

       if (this->ClassListType[i] == CLASS) {
         ((vtkImageEMAtlasClass*) this->ClassList[i])->Update();
         // Add Messages to own errror messages 
         if (((vtkImageEMAtlasClass*) this->ClassList[i])->GetErrorFlag()) {
       vtkEMJustAddErrorMessage(((vtkImageEMAtlasClass*) this->ClassList[i])->GetErrorMessages());
       return;
         }
     if ((ProbDataPtrIndex < 0) && ((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataPtr())  ProbDataPtrIndex = i;

       } else {
         ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->Update();
         // Add Messages to own errror messages 
         if (((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetErrorFlag()) {
       vtkEMJustAddErrorMessage(((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetErrorMessages());
       return;
         }

     if ((ProbDataPtrIndex < 0) &&  ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetProbDataPtrFlag()) ProbDataPtrIndex = i;
       }
     }
     if ((ProbDataPtrIndex < 0) && (this->ProbDataWeight > 0.0) ) { 
       this->ProbDataWeight = 0.0; 
       vtkEMAddWarningMessage("No PropDataPtr defined for any sub classes  => ProbDataWeight is set to 0! "); 
     }

     // ==================================================
     // Set values 
     if (ProbDataPtrIndex > -1) {
       int index = ProbDataPtrIndex;
       if (this->ClassListType[index] == CLASS) {
     memcpy(this->SegmentationBoundaryMax,((vtkImageEMAtlasClass*) this->ClassList[index])->GetSegmentationBoundaryMax(),sizeof(int)*3);
     memcpy(this->SegmentationBoundaryMin,((vtkImageEMAtlasClass*) this->ClassList[index])->GetSegmentationBoundaryMin(),sizeof(int)*3);
     memcpy(this->DataDim,((vtkImageEMAtlasClass*) this->ClassList[index])->GetDataDim(),sizeof(int)*3);
     memcpy(this->DataSpacing,((vtkImageEMAtlasClass*) this->ClassList[index])->GetDataSpacing(),sizeof(float)*3);
       }
       else {
     memcpy(this->SegmentationBoundaryMax,((vtkImageEMAtlasSuperClass*) this->ClassList[index])->GetSegmentationBoundaryMax(),sizeof(int)*3);
     memcpy(this->SegmentationBoundaryMin,((vtkImageEMAtlasSuperClass*) this->ClassList[index])->GetSegmentationBoundaryMin(),sizeof(int)*3);
     memcpy(this->DataDim,((vtkImageEMAtlasSuperClass*) this->ClassList[index])->GetDataDim(),sizeof(int)*3);
     memcpy(this->DataSpacing,((vtkImageEMAtlasSuperClass*) this->ClassList[index])->GetDataSpacing(),sizeof(float)*3);
       }
     }
   } 
   // Look for the first ProbData entry and then define scalar type accordingly
   int i;
   for (i = 0; i <this->NumClasses; i++) {
     if (this->ClassListType[i] == CLASS) {
       this->ProbDataScalarType = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataScalarType(); 
       if (this->ProbDataScalarType > -1) i = this->NumClasses;
     } else {
       this->ProbDataScalarType = ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetProbDataScalarType(); 
       if (this->ProbDataScalarType > -1) i = this->NumClasses;
     }
   }
     
   // ==================================================
   // Check own values
   for (i=0;i <  this->NumClasses; i++) {
    for (int j = 0; j < this->NumClasses; j++) {
      for (int k = 0; k < 6; k++) {
    if ((this->MrfParams[k][j][i] < 0) || (this->MrfParams[k][j][i] > 1)) {
      vtkEMAddErrorMessage("MrfParams[" << k <<"] [" << j<<"] [" << i <<"] = " << this->MrfParams[k][j][i] << " is not between 0 and 1 !");
      return;
    }
      }
    }
   }

   // ==================================================
   // Check values of subclasses
   for (i = 0; i <this->NumClasses; i++) {
     if (this->ClassListType[i] == CLASS) {
       if (((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataPtr()) {
         // Check if any input data was defined 
     
     if ((((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataScalarType() > -1) && (this->ProbDataScalarType != ((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataScalarType())) {
       vtkEMAddErrorMessage( "ProbDataScalarType of  class "<< i << " is of type "<< ((vtkImageEMAtlasClass*) this->ClassList[i])->GetProbDataScalarType() 
               << ", which is inconsistent with this super class' scalar type "<< this->ProbDataScalarType << "! Note: VTK_FLOAT="<< VTK_FLOAT <<", VTK_SHORT=" << VTK_SHORT);
       return;
     }

     int* max = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetSegmentationBoundaryMax();
     if (memcmp(this->SegmentationBoundaryMax,max,sizeof(int)*3)) {
       vtkEMAddErrorMessage( "SegmentationBoundaryMax of  class "<< i << "( "<< max[0] << "," << max[1] << "," << max[2] 
                 << ") is inconsistent with this super class' ones ("<< this->SegmentationBoundaryMax[0] <<","<<this->SegmentationBoundaryMax[1] 
                 << "," << this->SegmentationBoundaryMax[2] <<")");
       return; 
     }

     int* min = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetSegmentationBoundaryMin();
     if (memcmp(this->SegmentationBoundaryMin,min,sizeof(int)*3)) {
       vtkEMAddErrorMessage( "SegmentationBoundaryMin of  class "<< i << "( "<< min[0] << "," << min[1] << "," << min[2] 
                 << ") is inconsistent with this super class' ones ("<< this->SegmentationBoundaryMin[0] <<","<<this->SegmentationBoundaryMin[1] 
                 << "," << this->SegmentationBoundaryMin[2] <<")");
       return; 
     }

     int* dim = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetDataDim();
     if (memcmp(this->DataDim,dim,sizeof(int)*3)) {
       vtkEMAddErrorMessage("DataDim of class "<< i << "( "<< dim[0] << "," << dim[1] << "," << dim[2] 
                << ") is inconsistent with this super class' ones ("<< this->DataDim[0] <<","<<this->DataDim[1] << "," << this->DataDim[2] <<")");
       return; 
     }
     float* spacing = ((vtkImageEMAtlasClass*) this->ClassList[i])->GetDataSpacing();
     if (memcmp(this->DataSpacing,spacing,sizeof(float)*3)) {
       vtkEMAddErrorMessage("DataSpacing of class "<< i << " ("<< spacing[0] << "," << spacing[1] << "," << spacing[2] 
                << ") is inconsistent with this super class' ones ("<< this->DataSpacing[0] <<","<<this->DataSpacing[1] << "," << this->DataSpacing[2] <<")");
       return; 
     }
       } 
     } else {
       // If data is part of the subclasses - check dimension
       if (    ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetProbDataPtrFlag()) {

     if ((((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetProbDataScalarType() > -1) 
            && (this->ProbDataScalarType != ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetProbDataScalarType())) {
       vtkEMAddErrorMessage( "ProbDataScalarType of class "<< i << " is of type "<< ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetProbDataScalarType() 
                 << ", which is inconsistent with this super class' scalar type "<< this->ProbDataScalarType 
                                 << "! Note: VTK_FLOAT="<< VTK_FLOAT <<", VTK_SHORT=" << VTK_SHORT);
       return;
     }


     int* max = ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetSegmentationBoundaryMax();
     if (memcmp(this->SegmentationBoundaryMax,max,sizeof(int)*3)) {
       vtkEMAddErrorMessage( "SegmentationBoundaryMax of class "<< i << "( "<< max[0] << "," << max[1] << "," << max[2] 
                       << ") is inconsistent with this super class' ones ("<< this->SegmentationBoundaryMax[0] <<","
                       << this->SegmentationBoundaryMax[1] << "," << this->SegmentationBoundaryMax[2] <<")");
       return; 
     }

     int* min = ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetSegmentationBoundaryMin();
     if (memcmp(this->SegmentationBoundaryMin,min,sizeof(int)*3)) {
       vtkEMAddErrorMessage( "SegmentationBoundaryMin of  class "<< i << "( "<< min[0] << "," << min[1] << "," << min[2] 
                 << ") is inconsistent with this super class' ones ("<< this->SegmentationBoundaryMin[0] <<","
                 << this->SegmentationBoundaryMin[1] << "," << this->SegmentationBoundaryMin[2] <<")");
       return; 
     }

     int* dim = ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetDataDim();
     if (memcmp(this->DataDim,dim,sizeof(int)*3)) {
       vtkEMAddErrorMessage( "DataDim of class "<< i << "( "<< dim[0] << "," << dim[1] << "," << dim[2] 
                       << ") is inconsistent with this super class' ones ("<< this->DataDim[0] <<","<<this->DataDim[1] << "," << this->DataDim[2] <<")");
       return; 
     }

     float* spacing = ((vtkImageEMAtlasSuperClass*) this->ClassList[i])->GetDataSpacing();
     if (memcmp(this->DataSpacing,spacing,sizeof(float)*3)) {
       vtkEMAddErrorMessage( "DataSpacing of class "<< i << " ("<< spacing[0] << "," << spacing[1] << "," << spacing[2] 
                       << ") is inconsistent with this super class' ones ("<< this->DataSpacing[0] <<","<<this->DataSpacing[1] << "," << this->DataSpacing[2] <<")");
       return; 
     }

       } 
     }
   }
}
