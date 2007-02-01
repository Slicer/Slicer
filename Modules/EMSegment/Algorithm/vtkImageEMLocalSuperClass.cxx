/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMLocalSuperClass.cxx,v $
  Date:      $Date: 2006/12/08 23:28:23 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
#include "vtkImageEMLocalSuperClass.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"

//------------------------------------------------------------------------
vtkImageEMLocalSuperClass* vtkImageEMLocalSuperClass::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageEMLocalSuperClass");
  if(ret)
  {
    return (vtkImageEMLocalSuperClass*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageEMLocalSuperClass;

}


//------------------------------------------------------------------------------
// Define Procedures for vtkImageEMLocalSuperClass
//------------------------------------------------------------------------------
void vtkImageEMLocalSuperClass::CreateVariables() {
  this->NumClasses          = 0;  
  this->ClassList           = NULL;
  this->ClassListType       = NULL;
  this->MrfParams           = NULL;
  this->ParentClass         = NULL;
  this->PrintFrequency      = 0;
  this->PrintBias           = 0;
  this->PrintLabelMap       = 0;
  this->PrintEMLabelMapConvergence  = 0;
  this->PrintEMWeightsConvergence   = 0;
  this->PrintShapeSimularityMeasure = 0;

  this->StopEMType          = EMSEGMENT_STOP_FIXED;
  this->StopEMValue         = 0.0; 
  this->StopEMMaxIter       = 0; 

  this->PrintMFALabelMapConvergence  = 0;
  this->PrintMFAWeightsConvergence   = 0;
  this->StopMFAType          = EMSEGMENT_STOP_FIXED;
  this->StopMFAValue         = 0.0; 
  this->StopMFAMaxIter       = 0; 
  this->StopBiasCalculation  = -1;
 
  this->RegistrationType  = 0 ;
  this->GenerateBackgroundProbability = 0;

  this->PCAShapeModelType = EMSEGMENT_PCASHAPE_INDEPENDENT;
  
  this->RegistrationIndependentSubClassFlag =0;
}

//------------------------------------------------------------------------------
void vtkImageEMLocalSuperClass::DeleteSuperClassVariables() {
  int z,y;
  if (this->MrfParams) {
    for (z=0; z< 6; z ++) { 
      for (y=0; y < this->NumClasses; y++) delete[] this->MrfParams[z][y];
      delete[] this->MrfParams[z];
    }
    delete[] this->MrfParams;
  }

  //
  // delete elements of class list
  //
  if (this->ClassList)
    {
    for (int i = 0; i < this->NumClasses; ++i)
      {
      if (this->ClassList[i])
        {
        // we must do this since the data are referenced by void*
        // pointers and we must decrease the reference count
        vtkObjectBase* objectPointer = 
          static_cast<vtkObjectBase*>(this->ClassList[i]);
        if (objectPointer != 0)
          {
          objectPointer->Delete();
          }
        }
      }
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
void vtkImageEMLocalSuperClass::AddSubClass(void* ClassData, classType initType, int index)
{
  if (index < 0) {
    vtkEMAddErrorMessage("Index is not set correctly");
    return;
  }
  // Only if input images are changing  - automatically updated works  
  // this->Modified();

  if (this->NumClasses <= index) {
    vtkImageEMLocalSuperClass *parent = this->ParentClass;
    classType*  oldClassListType = NULL;
    void**      oldClassList = NULL;
    int         oldNumClasses = this->NumClasses;
    // Delete and remeber old class list
    if (oldNumClasses) 
      {
      oldClassListType = new classType[oldNumClasses];
      oldClassList     = new void*[oldNumClasses];
      for (int i = 0; i <oldNumClasses; i++) 
        {
        // legacy code issue: we have to manually register a new
        // reference to each object---otherwise the objects will be
        // deleted in the DeleteSuperClassVariables function
        vtkObjectBase* oldDataAsVTKObject = 
          static_cast<vtkObjectBase*>(this->ClassList[i]);    
        if (oldDataAsVTKObject != 0)
          {
          oldDataAsVTKObject->Register(this);
          }

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

  //
  // legacy code issue: we need to do some fancy footwork because we
  // need to update the reference count but we only have a void*
  // pointer.
  //
  vtkObjectBase* dataAsVTKObject = static_cast<vtkObjectBase*>(ClassData);
  if (dataAsVTKObject == 0)
    {
    vtkErrorMacro("AddSubClass: could not cast to vtk object from void*.");
    return;    
    }

  this->ClassListType[index] = initType;

  if (this->ClassList[index] == ClassData)
    {
    return;
    }

  if (this->ClassList[index])
    {
    vtkObjectBase* oldDataAsVTKObject = 
      static_cast<vtkObjectBase*>(this->ClassList[index]);    
    if (oldDataAsVTKObject != 0)
      {
      oldDataAsVTKObject->Delete();
      }
    }

  dataAsVTKObject->Register(this);
  this->ClassList[index] = ClassData;
}

//------------------------------------------------------------------------------
void vtkImageEMLocalSuperClass::SetMarkovMatrix(double value, int k, int j, int i) {
  if ((j<0) || (j >= this->NumClasses) ||
      (i<0) || (i >= this->NumClasses) ||
      (k<0) || (k > 5) || (value < 0) || (value > 1)) {
    vtkEMAddErrorMessage("Error:vtkImageEMLocalSuperClass::SetMarkovMatrix Incorrect input: " << value << " " << k << " " <<  j << " " << i << " " << this->NumClasses);
    return;
  }
  this->MrfParams[k][j][i] = value;
}

//------------------------------------------------------------------------------
// if flag is set => includes subclasses of type SUPERCLASS
int vtkImageEMLocalSuperClass::GetTotalNumberOfClasses(bool flag) {
  int result = 0;
  for (int i=0;  i< this->NumClasses; i++) {
    if (this->ClassListType[i] == SUPERCLASS) {
      result += ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetTotalNumberOfClasses(flag); 
      if (flag == true) result ++;
    }  else { result ++;}
  }
  return result;
}

//------------------------------------------------------------------------------
// Returns a list of all labels (excluding the once with -1 and double once) in numerical order ,e.g. 0 1 4 5  
// Make sure LabelList is  with 0 -> otherwise won't get right list 
int vtkImageEMLocalSuperClass::GetAllLabels(short *LabelList, int result, int Max) {
  int i,j,k, label;
  for (i=0;  i< this->NumClasses; i++) {
    if (this->ClassListType[i] == SUPERCLASS) {
      result = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetAllLabels(LabelList,result,Max);
    } else {
      j= 0;
      label = ((vtkImageEMLocalClass*) this->ClassList[i])->GetLabel();
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
int vtkImageEMLocalSuperClass::LabelAllSuperClasses(short *TakenLabelList, int Result, int Max) {
  int i,j,k;
  vtkNotUsed(short label=0;);
  // You have to have atleast one label defined
  assert(Result && Result <= Max);

  for (i=0;  i < this->NumClasses; i++) {
    if (this->ClassListType[i] == SUPERCLASS) {
      Result = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->LabelAllSuperClasses(TakenLabelList,Result,Max);
      // Kilian Jan06: There used to be a bug in this code - for example if TakenLabelList label list consists of (e.g. 0 1 2 3 0 0) => Max = 5
      //               then the old code woud it assign the label 4 at position 5 => it is not part of taken label anymore and the TakenLabelList 
      //               is not in order.
      // Just add to end of list 
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
      ((vtkImageEMLocalSuperClass*) this->ClassList[i])->Label =  PreLabelID;
      Result ++;
    }
  }
  return Result;
}


//------------------------------------------------------------------------------
int vtkImageEMLocalSuperClass::GetTotalNumberOfProbDataPtr() {
  // If a superclass has a probability data defined than we assign to each subclass the probability of the superclass
  if (this->ProbImageData) {
    return this->GetTotalNumberOfClasses(0);
  } else {
    int result = 0;
    for (int i=0;  i< this->NumClasses; i++) {
      if (this->ClassListType[i] == SUPERCLASS) {
    result += ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetTotalNumberOfProbDataPtr(); 
      } else { 
    if (((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataWeight() >  0.0) result++;
      }
    }
    return result;
  }
}

//------------------------------------------------------------------------------
int vtkImageEMLocalSuperClass::GetTotalNumberOfEigenModes() {
  int result = 0;
  for (int i=0;  i< this->NumClasses; i++) {
    if (this->ClassListType[i] == SUPERCLASS) {
      result += ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetTotalNumberOfEigenModes(); 
    } else { 
      result += (((vtkImageEMLocalClass*) this->ClassList[i])->GetPCANumberOfEigenModes()); 
    }
  }
  return result;
}

int vtkImageEMLocalSuperClass::GetPCANumberOfEigenModesList(int *NumberOfEigenModesList, int index) {
  for (int i=0;  i< this->NumClasses; i++) {
    if (this->ClassListType[i] == SUPERCLASS) {
      index= ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetPCANumberOfEigenModesList(NumberOfEigenModesList,index); 
    } else { 
      NumberOfEigenModesList[index] = (((vtkImageEMLocalClass*) this->ClassList[i])->GetPCANumberOfEigenModes()); 
      index ++;
    }
  }
  return index;
}

void vtkImageEMLocalSuperClass::GetPCANumberOfEigenModes(int *NumberOfEigenModesList) {
  this->GetPCANumberOfEigenModesList(NumberOfEigenModesList,0);
}


//------------------------------------------------------------------------------
int vtkImageEMLocalSuperClass::GetProbDataPtrFlag() {
  if (this->ProbImageData) {
    return 1;
  } 
  for (int i = 0; i < this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) {
      if (((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataPtr(0)) return 1;
    } else {
      if (((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetProbDataPtrFlag()) return 1;
    }
  }
  return 0;
}



//------------------------------------------------------------------------------
int vtkImageEMLocalSuperClass::GetPCAPtrFlag() {
  for (int i = 0; i < this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) {
      if (((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAMeanShapePtr(0)) return 1;
    } else {
      if (((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetPCAPtrFlag()) return 1;
    }
  }
  return 0;
}




//------------------------------------------------------------------------------
int vtkImageEMLocalSuperClass::GetPCAParametersPtr(void** PCAMeanShapePtr, void*** PCAEigenVectorPtr, int index, int BoundaryType) {
  // cout << "Start vtkImageEMLocalSuperClass::GetPCAParametersPtr" << endl;
  for (int i = 0; i <this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) {
      PCAMeanShapePtr[index]    = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAMeanShapePtr(BoundaryType);

      int NumOfEigenVectors = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCANumberOfEigenModes();
      for (int j = 0 ; j < NumOfEigenVectors; j++) {
    PCAEigenVectorPtr[index][j] = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAEigenVectorPtr(j,BoundaryType);
      }
      index ++;
    } else index = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetPCAParametersPtr(PCAMeanShapePtr, PCAEigenVectorPtr, index,BoundaryType);
  }
  // cout << "End vtkImageEMLocalSuperClass::GetPCAParametersPtr" << endl;
  return index;
}
// BoundaryType = do we want to consider the segmentation boundary or not 
//------------------------------------------------------------------------------
int vtkImageEMLocalSuperClass::GetPCAParameters(float **ShapeParametersList, int *PCAMeanShapeIncY, int *PCAMeanShapeIncZ, 
                                                  int **PCAEigenVectorIncY, int **PCAEigenVectorIncZ, double **PCAEigenValues,  float *PCALogisticSlope, 
                                                  float *PCALogisticBoundary, float *PCALogisticMin, float *PCALogisticMax, int index, int BoundaryType) {
  for (int i = 0; i <this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) {
      ShapeParametersList[index]    = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAShapeParameters();
      PCAMeanShapeIncY[index]       = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAMeanShapeIncY(BoundaryType); 
      PCAMeanShapeIncZ[index]       = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAMeanShapeIncZ(BoundaryType); 
      int NumOfEigenVectors = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCANumberOfEigenModes();
      for (int j = 0; j < NumOfEigenVectors ; j++) {
    PCAEigenVectorIncY[index][j] = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAEigenVectorIncY(j,BoundaryType); 
    PCAEigenVectorIncZ[index][j] = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAEigenVectorIncY(j,BoundaryType);
      }
      PCAEigenValues[index]         = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAEigenValues(); 
      PCALogisticSlope[index]       = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCALogisticSlope();
      PCALogisticBoundary[index]    = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCALogisticBoundary();
      PCALogisticMin[index]         = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCALogisticMin();
      PCALogisticMax[index]         = ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCALogisticMax();
      index ++;
    } else index = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetPCAParameters(ShapeParametersList,PCAMeanShapeIncY,PCAMeanShapeIncZ, PCAEigenVectorIncY, PCAEigenVectorIncZ,PCAEigenValues, PCALogisticSlope, PCALogisticBoundary, PCALogisticMin, PCALogisticMax,index, BoundaryType); 
  }
  return index;
}

//------------------------------------------------------------------------------
int vtkImageEMLocalSuperClass::GetProbDataPtrList(void **PointerList, int index, int BoundaryType) {
  int i;
  // if the super class has prob data defined than we just use that 
  if (this->ProbImageData) {
    int Num = this->GetTotalNumberOfClasses(0);
    void *Pointer = this->GetProbDataPtr(BoundaryType);
    for (i = 0; i < Num; i++) {
      PointerList[index] = Pointer;
      index ++;
    }
  } else {
    for (i = 0; i <this->NumClasses; i++) {
      if (this->ClassListType[i] == CLASS) {
    PointerList[index] = ((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataPtr(BoundaryType);
    index ++;
      } else index = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetProbDataPtrList(PointerList,index,BoundaryType); 
    }
  }
  return index;
} 

// just indicate the first probdata of all subclasses if it was defined by the super class - this is necessary so that 
// for shape and registration we can properly normalize !
 
int vtkImageEMLocalSuperClass::GetProbImageDataCount(char *list, int index) {
  int i;
  // if the super class has prob data defined than we just use that 
  if (this->ProbImageData) {
    int Num = this->GetTotalNumberOfClasses(0);
    for (i = 0; i < Num; i++) {
      list[index] = (i > 0 ? 0 : 1);
      index ++;
    }
  } else {
    for (i = 0; i <this->NumClasses; i++) {
      if (this->ClassListType[i] == CLASS) {
    list[index] = (((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataPtr(0) ? 1 : 0) ;
    index ++;
      } else index = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetProbImageDataCount(list, index); 
    }
  }
  return index;
} 

//------------------------------------------------------------------------------
int vtkImageEMLocalSuperClass::GetProbDataIncYandZ(int* ProbDataIncY,int* ProbDataIncZ,int index, int BoundaryType) {
  int i;
  // if the super class has prob data defined than we just use that 
  if (this->ProbImageData) {
    int Num = this->GetTotalNumberOfClasses(0);
    int DataIncY = this->GetProbDataIncY(BoundaryType);
    int DataIncZ = this->GetProbDataIncZ(BoundaryType);
    for (i = 0; i < Num; i++) {
      ProbDataIncY[index] =  DataIncY;
      ProbDataIncZ[index] =  DataIncZ;
      index ++;
    }
  } else {
    for (i = 0; i <this->NumClasses; i++) {
      if (this->ClassListType[i] == CLASS) {
    if (((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataWeight() > 0.0) {
      ProbDataIncY[index] = ((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataIncY(BoundaryType);
      ProbDataIncZ[index] = ((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataIncZ(BoundaryType);
    } else {
      ProbDataIncY[index] = ProbDataIncZ[index] = 0; 
    }
    index ++;
      } else index = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetProbDataIncYandZ(ProbDataIncY,ProbDataIncZ,index,BoundaryType); 
    }
  }
  return index;
} 
//------------------------------------------------------------------------------
void vtkImageEMLocalSuperClass::GetRegistrationClassSpecificParameterList(int *RegistrationIndependentSubClassFlag, int *RegistrationClassSpecificRegistrationFlag,int &NumParaSets) {

  if (this->RegistrationType != EMSEGMENT_REGISTRATION_CLASS_ONLY) NumParaSets = 1; 
  else NumParaSets = 0;

  for (int i = 0; i < this->NumClasses; i++) {
    if  (this->ClassListType[i] == SUPERCLASS) {
      RegistrationIndependentSubClassFlag[i] = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetRegistrationIndependentSubClassFlag();
      cout << "Registration Indepdent Sub Class Flag  " << i << ": " << ( (RegistrationIndependentSubClassFlag[i] == 1) ? "On" : "Off" ) << endl;
      RegistrationClassSpecificRegistrationFlag[i] = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetRegistrationClassSpecificRegistrationFlag();
    } else {
      RegistrationIndependentSubClassFlag[i] = 0;
      RegistrationClassSpecificRegistrationFlag[i] = ((vtkImageEMLocalClass*) this->ClassList[i])->GetRegistrationClassSpecificRegistrationFlag();
    }
    // Only add a new parameter set if the registration includes strucutre specific registration and structure specific registration is activated 
    if (RegistrationType > EMSEGMENT_REGISTRATION_GLOBAL_ONLY) {
      cout << "Registration Class Specific Registration Flag " << i << ": "  ;
      if (RegistrationClassSpecificRegistrationFlag[i] && (i || !GenerateBackgroundProbability)) {
    NumParaSets ++;
    cout << "On " << endl;
      } else {
    cout << "Off " << endl;
      }
    }
  }
} 



//------------------------------------------------------------------------------
int vtkImageEMLocalSuperClass::GetTissueDefinition(int *LabelList,double** LogMu, double ***LogCov, int index) {
  int i;
  for (i = 0; i < this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) {
      LogMu[index]         = ((vtkImageEMLocalClass*) this->ClassList[i])->GetLogMu();
      LogCov[index]        = ((vtkImageEMLocalClass*) this->ClassList[i])->GetLogCovariance();
      LabelList[index]     = ((vtkImageEMLocalClass*) this->ClassList[i])->GetLabel();
      index ++;
    } else index = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetTissueDefinition(LabelList,LogMu, LogCov,index);
  }
  return index;
} 

//------------------------------------------------------------------------------
classType vtkImageEMLocalSuperClass::GetClassType(void* active) {
  int i = 0; 
  while ((i < this->NumClasses) && (active != this->ClassList[i])) i++;
  if (i == this->NumClasses) return CLASS;
  return this->ClassListType[i];                     
}

//------------------------------------------------------------------------------
void vtkImageEMLocalSuperClass::PrintSelf(ostream& os,vtkIndent indent) {
  os << indent << "---------------------------------------- SUPERCLASS ----------------------------------------------" << endl;
  this->vtkImageEMLocalGenericClass::PrintSelf(os,indent); 
  os << indent << "NumClasses:                    " << this->NumClasses << endl;
  os << indent << "PrintFrequency:                " << this->PrintFrequency << endl;
  os << indent << "PrintBias:                     " << this->PrintBias<< endl;
  os << indent << "PrintLabelMap:                 " << this->PrintLabelMap << endl;
  os << indent << "PrintEMLabelMapConvergence:    " << this->PrintEMLabelMapConvergence << endl;
  os << indent << "PrintEMWeightsConvergence:     " << this->PrintEMWeightsConvergence  << endl;
  os << indent << "PrintMFALabelMapConvergence:   " << this->PrintMFALabelMapConvergence << endl;
  os << indent << "PrintMFAWeightsConvergence:    " << this->PrintMFAWeightsConvergence  << endl;
  os << indent << "PrintShapeSimularityMeasure:   " << this->PrintShapeSimularityMeasure << endl;
  os << indent << "StopEMType:                    " << this->StopEMType  << endl;
  os << indent << "StopEMValue:                   " << this->StopEMValue << endl;
  os << indent << "StopEMMaxIter:                 " << this->StopEMMaxIter << endl;
  os << indent << "StopMFAType:                   " << this->StopMFAType  << endl;
  os << indent << "StopMFAValue:                  " << this->StopMFAValue << endl;
  os << indent << "StopMFAMaxIter:                " << this->StopMFAMaxIter << endl;
  os << indent << "StopBiasCalculation:           " << this->StopBiasCalculation << endl;
  os << indent << "RegistrationType:              " << this->RegistrationType << endl;
  os << indent << "GenerateBackgroundProbability: " << this->GenerateBackgroundProbability << endl;
  os << indent << "RegistrationIndependentSubClassFlag " << this->RegistrationIndependentSubClassFlag << endl; 
  os << indent << "PCAShapeModelType:             " << this->PCAShapeModelType  << endl;

  // No need of expensive call to new for a simple array
  static const char * const Directions[] = {
    "West ",
    "North",
    "Up   ",
    "East ",
    "South",
    "Down "
  };
  os << indent << "MrfParams:                    " << endl;
  for (int z=0; z < 6; z++) { 
    os << indent << "   " << Directions[z] << ":    ";   
    for (int y=0;y < this->NumClasses; y ++) {
      if (y) os << "| ";
      for (int x=0;x < this->NumClasses; x ++) os << this->MrfParams[z][y][x] << " " ;
    }
    os << endl;
  }
  for (int i =0; i < this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) ((vtkImageEMLocalClass*)this->ClassList[i])->PrintSelf(os,indent.GetNextIndent());
    else ((vtkImageEMLocalSuperClass*)this->ClassList[i])->PrintSelf(os,indent.GetNextIndent());
  }
}


//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageEMLocalSuperClass::ExecuteData(vtkDataObject *)
{
  // cout << "start vtkImageEMLocalSuperClass::ExecuteData" << endl; 
   vtkDebugMacro(<<"ExecuteData()"); 
   // ==================================================
   // Check existence of subclasses and update them 
   this->ResetWarningMessage();
   if (this->GetErrorFlag()) 
     vtkEMAddWarningMessage("The error flag for this module was set with the following messages (the error messages will be reset now:\n"<<this->GetErrorMessages());

   this->ResetErrorMessage();
   this->vtkImageEMLocalGenericClass::ExecuteData(NULL);
   // Error Occured
   // Kilian change this 
   if (this->GetErrorFlag()) return;

   if (!this->NumClasses) {
     vtkEMAddErrorMessage("No sub classes defined!");
     return;
   }

   
   if (this->ProbImageData == NULL) {
     int ProbDataPtrIndex = -1; 
     int PCAPtrIndex      = -1; 

     for (int i = 0; i <this->NumClasses; i++) {
       if (!this->ClassList[i]) {
         vtkEMAddErrorMessage("Class with index "<< i <<" is not defined!");
         return;
       }

       if (this->ClassListType[i] == CLASS) {
         ((vtkImageEMLocalClass*) this->ClassList[i])->Update();
         // Add Messages to own errror messages 
         if (((vtkImageEMLocalClass*) this->ClassList[i])->GetErrorFlag()) {
       vtkEMJustAddErrorMessage(((vtkImageEMLocalClass*) this->ClassList[i])->GetErrorMessages());
       return;
         }
     if ((ProbDataPtrIndex < 0) && ((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataPtr(0))  ProbDataPtrIndex = i;
     if ((PCAPtrIndex < 0)      && ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAMeanShapePtr(0)) PCAPtrIndex  = i;

       } else {
         ((vtkImageEMLocalSuperClass*) this->ClassList[i])->Update();
         // Add Messages to own errror messages 
         if (((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetErrorFlag()) {
       vtkEMJustAddErrorMessage(((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetErrorMessages());
       return;
         }
     if ((ProbDataPtrIndex < 0) &&  ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetProbDataPtrFlag()) ProbDataPtrIndex = i;
     if ((PCAPtrIndex  < 0)     &&  ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetPCAPtrFlag())  PCAPtrIndex  = i;
       }
     }
     // Kilian : Currently the name of ProbDataWeight is not good bc it also defines the influence for  PCAPtr
     if ((ProbDataPtrIndex < 0) && (PCAPtrIndex < 0) && (this->ProbDataWeight > 0.0) ) { 
       this->ProbDataWeight = 0.0; 
       vtkEMAddWarningMessage("No PropDataPtr or PCAPtr defined for any sub classes  => ProbDataWeight is set to 0! "); 
     }

     // ==================================================
     // Set values 
     if ((ProbDataPtrIndex > -1) || (PCAPtrIndex > -1)) {
       int index = (ProbDataPtrIndex > -1 ? ProbDataPtrIndex : PCAPtrIndex);
       if (this->ClassListType[index] == CLASS) {
     memcpy(this->SegmentationBoundaryMax,((vtkImageEMLocalClass*) this->ClassList[index])->GetSegmentationBoundaryMax(),sizeof(int)*3);
     memcpy(this->SegmentationBoundaryMin,((vtkImageEMLocalClass*) this->ClassList[index])->GetSegmentationBoundaryMin(),sizeof(int)*3);
     memcpy(this->DataDim,((vtkImageEMLocalClass*) this->ClassList[index])->GetDataDim(),sizeof(int)*3);
     memcpy(this->DataSpacing,((vtkImageEMLocalClass*) this->ClassList[index])->GetDataSpacing(),sizeof(float)*3);
       }
       else {
     memcpy(this->SegmentationBoundaryMax,((vtkImageEMLocalSuperClass*) this->ClassList[index])->GetSegmentationBoundaryMax(),sizeof(int)*3);
     memcpy(this->SegmentationBoundaryMin,((vtkImageEMLocalSuperClass*) this->ClassList[index])->GetSegmentationBoundaryMin(),sizeof(int)*3);
     memcpy(this->DataDim,((vtkImageEMLocalSuperClass*) this->ClassList[index])->GetDataDim(),sizeof(int)*3);
     memcpy(this->DataSpacing,((vtkImageEMLocalSuperClass*) this->ClassList[index])->GetDataSpacing(),sizeof(float)*3);
       }
     }
     // Look for the first ProbData entry and then define scalar type accordingly
     for (int i = 0; i <this->NumClasses; i++) {
       if (this->ClassListType[i] == CLASS) {
     this->ProbDataScalarType = ((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataScalarType(); 
     if (this->ProbDataScalarType > -1) i = this->NumClasses;
       } else {
     this->ProbDataScalarType = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetProbDataScalarType(); 
     if (this->ProbDataScalarType > -1) i = this->NumClasses;
       }
     }
   } else {
     cout << "Warning:: Probability Data of SuperClass activated - Class specific probability maps are overwritten!" << endl; 
     // Kilian: Currently we also disreagard the the SHAPE model - change this later  
     if (this->GetTotalNumberOfEigenModes()) {
       cout << "Error::vtkImageEMLocalSuperClass:: SuperClass has Probability Data but sub classes have PCAShape model activated - Conflict of interest !" << endl;
       exit(1);
     }
     
   }
   // ==================================================
   // Check own values
   for (int i=0;i <  this->NumClasses; i++) {
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
   for (int i = 0; i <this->NumClasses; i++) {
     if (this->ClassListType[i] == CLASS) {
       if (((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataPtr(0)  || ((vtkImageEMLocalClass*) this->ClassList[i])->GetPCAMeanShapePtr(0)) {
         // Check if any input data was defined 
     
     if ((((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataScalarType() > -1) && (this->ProbDataScalarType != ((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataScalarType())) {
       vtkEMAddErrorMessage( "ProbDataScalarType of  class "<< i << " is of type "<< ((vtkImageEMLocalClass*) this->ClassList[i])->GetProbDataScalarType() 
               << ", which is inconsistent with this super class' scalar type "<< this->ProbDataScalarType << "! Note: VTK_FLOAT="<< VTK_FLOAT <<", VTK_SHORT=" << VTK_SHORT);
       return;
     }

     int* max = ((vtkImageEMLocalClass*) this->ClassList[i])->GetSegmentationBoundaryMax();
     if (memcmp(this->SegmentationBoundaryMax,max,sizeof(int)*3)) {
       vtkEMAddErrorMessage( "SegmentationBoundaryMax of  class "<< i << "( "<< max[0] << "," << max[1] << "," << max[2] 
                 << ") is inconsistent with this super class' ones ("<< this->SegmentationBoundaryMax[0] <<","<<this->SegmentationBoundaryMax[1] 
                 << "," << this->SegmentationBoundaryMax[2] <<")");
       return; 
     }

     int* min = ((vtkImageEMLocalClass*) this->ClassList[i])->GetSegmentationBoundaryMin();
     if (memcmp(this->SegmentationBoundaryMin,min,sizeof(int)*3)) {
       vtkEMAddErrorMessage( "SegmentationBoundaryMin of  class "<< i << "( "<< min[0] << "," << min[1] << "," << min[2] 
                 << ") is inconsistent with this super class' ones ("<< this->SegmentationBoundaryMin[0] <<","<<this->SegmentationBoundaryMin[1] 
                 << "," << this->SegmentationBoundaryMin[2] <<")");
       return; 
     }

     int* dim = ((vtkImageEMLocalClass*) this->ClassList[i])->GetDataDim();
     if (memcmp(this->DataDim,dim,sizeof(int)*3)) {
       vtkEMAddErrorMessage("DataDim of class "<< i << "( "<< dim[0] << "," << dim[1] << "," << dim[2] 
                << ") is inconsistent with this super class' ones ("<< this->DataDim[0] <<","<<this->DataDim[1] << "," << this->DataDim[2] <<")");
       return; 
     }
     float* spacing = ((vtkImageEMLocalClass*) this->ClassList[i])->GetDataSpacing();
     if (memcmp(this->DataSpacing,spacing,sizeof(float)*3)) {
       vtkEMAddErrorMessage("DataSpacing of class "<< i << " ("<< spacing[0] << "," << spacing[1] << "," << spacing[2] 
                << ") is inconsistent with this super class' ones ("<< this->DataSpacing[0] <<","<<this->DataSpacing[1] << "," << this->DataSpacing[2] <<")");
       return; 
     }
       } 
     } else {
       // If data is par t of the subclasses - check dimension
       if (    ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetProbDataPtrFlag() 
            || ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetPCAPtrFlag()) {

     if ((((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetProbDataScalarType() > -1) 
            && (this->ProbDataScalarType != ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetProbDataScalarType())) {
       vtkEMAddErrorMessage( "ProbDataScalarType of class "<< i << " is of type "<< ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetProbDataScalarType() 
                 << ", which is inconsistent with this super class' scalar type "<< this->ProbDataScalarType 
                                 << "! Note: VTK_FLOAT="<< VTK_FLOAT <<", VTK_SHORT=" << VTK_SHORT);
       return;
     }


     int* max = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetSegmentationBoundaryMax();
     if (memcmp(this->SegmentationBoundaryMax,max,sizeof(int)*3)) {
       vtkEMAddErrorMessage( "SegmentationBoundaryMax of class "<< i << "( "<< max[0] << "," << max[1] << "," << max[2] 
                       << ") is inconsistent with this super class' ones ("<< this->SegmentationBoundaryMax[0] <<","
                       << this->SegmentationBoundaryMax[1] << "," << this->SegmentationBoundaryMax[2] <<")");
       return; 
     }

     int* min = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetSegmentationBoundaryMin();
     if (memcmp(this->SegmentationBoundaryMin,min,sizeof(int)*3)) {
       vtkEMAddErrorMessage( "SegmentationBoundaryMin of  class "<< i << "( "<< min[0] << "," << min[1] << "," << min[2] 
                 << ") is inconsistent with this super class' ones ("<< this->SegmentationBoundaryMin[0] <<","
                 << this->SegmentationBoundaryMin[1] << "," << this->SegmentationBoundaryMin[2] <<")");
       return; 
     }

     int* dim = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetDataDim();
     if (memcmp(this->DataDim,dim,sizeof(int)*3)) {
       vtkEMAddErrorMessage( "DataDim of class "<< i << "( "<< dim[0] << "," << dim[1] << "," << dim[2] 
                       << ") is inconsistent with this super class' ones ("<< this->DataDim[0] <<","<<this->DataDim[1] << "," << this->DataDim[2] <<")");
       return; 
     }

     float* spacing = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetDataSpacing();
     if (memcmp(this->DataSpacing,spacing,sizeof(float)*3)) {
       vtkEMAddErrorMessage( "DataSpacing of class "<< i << " ("<< spacing[0] << "," << spacing[1] << "," << spacing[2] 
                       << ") is inconsistent with this super class' ones ("<< this->DataSpacing[0] <<","<<this->DataSpacing[1] << "," << this->DataSpacing[2] <<")");
       return; 
     }

       } 
     }
   }
   // cout << "End vtkImageEMLocalSuperClass::ExecuteData" << endl; 
}
