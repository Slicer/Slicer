/*=auto=========================================================================

(c) Copyright 2001 Massachusetts Institute of Technology

Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for any purpose, 
provided that the above copyright notice and the following three paragraphs 
appear on all copies of this software.  Use of this software constitutes 
acceptance of these terms and conditions.

IN NO EVENT SHALL MIT BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, 
INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE 
AND ITS DOCUMENTATION, EVEN IF MIT HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

MIT SPECIFICALLY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTIES INCLUDING, 
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED "AS IS."  MIT HAS NO OBLIGATION TO PROVIDE 
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================auto=*/

#include "vtkImageWriter.h"

// --------------------------------------------------------------------------------------------------------------------------
//  Registration Functions 
// --------------------------------------------------------------------------------------------------------------------------
inline void EMLocalAlgorithm_TransfereTranRotSca_ToRegistrationParameter(double *Translation, double *Rotation, double *Scale, double *FinalParameters, 
                                           EMLocalRegistrationCostFunction  *RegistrationParameters) {
  if (RegistrationParameters->GetTwoDFlag()) {
    FinalParameters[0] = float(Translation[0]); FinalParameters[1] = float(Translation[1]);
    FinalParameters[2] = float(Rotation[2]);
    if (!RegistrationParameters->GetRigidFlag()) {
      FinalParameters[3] = float(Scale[0]);       FinalParameters[4] = float(Scale[1]);
    }
  } else {
    FinalParameters[0] = float(Translation[0]); FinalParameters[1] = float(Translation[1]); FinalParameters[2] = float(Translation[2]);
    FinalParameters[3] = float(Rotation[0]);    FinalParameters[4] = float(Rotation[1]);    FinalParameters[5] = float(Rotation[2]);
    if (!RegistrationParameters->GetRigidFlag()) {
      FinalParameters[6] = float(Scale[0]);       FinalParameters[7] = float(Scale[1]);       FinalParameters[8] = float(Scale[2]);
    }
  }
}


//------------------------------------------------------------------------------
// Print Functions  
//------------------------------------------------------------------------------

void* EMLocalAlgorithm_GetPointerToVtkImageData(vtkImageData *Image, int DataType, int Ext[6]) {
 Image->SetWholeExtent(Ext);
 Image->SetExtent(Ext); 
 Image->SetNumberOfScalarComponents(1);
 Image->SetScalarType(DataType); 
 Image->AllocateScalars(); 
 return Image->GetScalarPointerForExtent(Ext);
}

template <class T>
void EMLocalAlgorithm_PrintVector(T* parameters, int Min,  int Max) {
  for (int i = Min; i <= Max ; i++)  cout << parameters[i] << " ";
  cout << endl;
}

void EMLocalAlgorithm_PrintVectorPrecise(float *parameters, int Min,  int Max) {
  for (int i = Min; i <= Max ; i++)  fprintf(stdout,"%12f ",parameters[i]);
  cout << endl;
}

template <class T>
void EMLocalAlgorithm_PrintMatrix(T**mat,int xMin, int yMin, int xMax,int yMax) {
  for (int y = yMin; y <= yMax; y++) EMLocalAlgorithm_PrintVector(mat[y],xMin,xMax); 
}

template <class T>
FILE* EMLocalAlgorithm<T>::OpenTextFile(const char FileName[], int Label, int LabelFlag, int LevelNameFlag, int iter, int IterFlag, const char FileSucessMessage[]) {
  char OpenFileName[1000];
  FILE *OpenFile = vtkImageEMGeneral::OpenTextFile(this->PrintDir, FileName, Label, LabelFlag, this->LevelName, LevelNameFlag, iter, IterFlag, FileSucessMessage, OpenFileName);
  if (OpenFile == NULL) vtkEMAddErrorMessage("Could not open file :" << OpenFileName);
  return OpenFile;
}

//----------------------------------------------------------------------------
// Transferes the data wrom the working extent (that is only the area defined by the SegmentationBounday)
// to the Extent of the output - if SliceNum > 0 then it will just print out one slice
 
template <class T, class TIn, class TOut>
static void EMLocalAlgorithm_TransfereDataToOutputExtension(EMLocalAlgorithm<T> *selfPtr,TIn* inputExtension_Vector, TOut* outputExtension_Ptr, int outputExtent_Inc[3], int SliceNum) {
  // -----------------------------------------------------
  // 1.) Setup Parameteres 
  // -----------------------------------------------------
  // Get increments to march through data
  int outputExtension[6];
  memcpy(outputExtension,selfPtr->GetExtent(), sizeof(int)*6);

  int SegmentationBoundaryMin[3];
  memcpy(SegmentationBoundaryMin,selfPtr->GetSegmentationBoundaryMin(), sizeof(int)*3);

  int DimensionX = selfPtr->GetBoundaryMaxX();
  int DimensionY = selfPtr->GetBoundaryMaxY();
  int DimensionZ = selfPtr->GetBoundaryMaxZ();

  if (SliceNum > 0) {
    outputExtension[4] += SliceNum-1 + SegmentationBoundaryMin[2] -1;
    outputExtension[5] = outputExtension[4];
    SegmentationBoundaryMin[2] = 1;
    DimensionZ = 1;
  }

  int idxR, idxY, idxZ;

  // We assume output is 1 scalar 
  TOut* outputExtensionStart_Ptr = outputExtension_Ptr; 

  int LengthOfXDim = outputExtension[1] - outputExtension[0] +1  + outputExtent_Inc[1];
  int LengthOfYDim = LengthOfXDim*(outputExtension[3] - outputExtension[2] + 1) +  outputExtent_Inc[2];  
  // Location of the first voxel 
  outputExtensionStart_Ptr += (SegmentationBoundaryMin[0] - 1) + (SegmentationBoundaryMin[1] - 1) * LengthOfXDim
         + LengthOfYDim *(SegmentationBoundaryMin[2] - 1);

  int BoundaryDataIncY = LengthOfXDim - DimensionX;
  int BoundaryDataIncZ = LengthOfYDim - DimensionY *LengthOfXDim;

  // -----------------------------------------------------
  // 2.) Set output to zero 
  // -----------------------------------------------------

  // Initializa output by setting it to zero
  for (idxZ = outputExtension[4]; idxZ <= outputExtension[5]; idxZ++) {
    for (idxY = outputExtension[2]; idxY <= outputExtension[3]; idxY++) {
      memset(outputExtension_Ptr,0,sizeof(TOut)*(outputExtension[1] - outputExtension[0] + 1));
      outputExtension_Ptr +=  LengthOfXDim;
    }
  }
  outputExtension_Ptr = outputExtensionStart_Ptr ;  


  // -----------------------------------------------------
  // 3.) Define output in working Extension 
  // -----------------------------------------------------
  int index = 0;
  // 3.) Write result in outPtr
  for (idxZ = 0; idxZ < DimensionZ; idxZ++) {
    for (idxY = 0; idxY < DimensionY; idxY++) {
      for (idxR = 0; idxR < DimensionX; idxR++) {
        *outputExtension_Ptr = (TOut) inputExtension_Vector[index];
    index ++; 
        outputExtension_Ptr++;
      }
      outputExtension_Ptr +=  BoundaryDataIncY;
    }
    outputExtension_Ptr +=  BoundaryDataIncZ;
  }
}


//----------------------------------------------------------------------------
int EMLocalAlgorithm_GEImageWriter(vtkImageData *Volume, char *FileName,int PrintFlag) {
  if (PrintFlag) cout << "Write to file " <<  FileName << endl;

#ifdef _WIN32 
  // Double or Float is not correctly printed out in windwos 
  if (Volume->GetScalarType() == VTK_DOUBLE || Volume->GetScalarType() == VTK_FLOAT) {
    int *Extent =Volume->GetExtent();
    void* VolumeDataPtr = Volume->GetScalarPointerForExtent(Extent);
    int ImageX = Extent[1] - Extent[0] +1; 
    int ImageY = Extent[3] - Extent[2] +1; 
    int ImageXY = ImageX * ImageY;

    vtkIdType outIncX, OutIncY, outIncZ;
    Volume->GetContinuousIncrements(Extent, outIncX, OutIncY, outIncZ);

    if (OutIncY != 0 || outIncZ != 0 ) return 0;
    
    char *SliceFileName = new char[int(strlen(FileName)) + 6];
    for (int i = Extent[4]; i <= Extent[5]; i++) {
      sprintf(SliceFileName,"%s.%03d",FileName,i);
      switch (Volume->GetScalarType()) {
        vtkTemplateMacro5(vtkFileOps_WriteToFlippedGEFile,SliceFileName,(VTK_TT*)  VolumeDataPtr, ImageX, ImageY, ImageXY);
      }
    }
    delete []SliceFileName;
    return 1;
  }
#endif

  vtkImageWriter *Write=vtkImageWriter::New();
  Write->SetInput(Volume);
  Write->SetFilePrefix(FileName);
  Write->SetFilePattern("%s.%03d");
  Write->Write();
  Write->Delete();
  return 1;
}


//----------------------------------------------------------------------------
// Transferes the data from the  working extent (that is only the area defined by the SegmentationBounday)
// to the Extent of the output and prints it out . 
// SliceNum > 0              => it will just print out that one Slice
// OriginalExtensionFlag = 1 => inputExtension_Vector already has extension defined by self->GetExtent()

template <class T,class TIn>
static void EMLocalAlgorithm_PrintDataToOutputExtension(EMLocalAlgorithm<T> *selfPtr,TIn* inputExtension_Vector, int outputScalar, char* FileName ,int SliceNum, int OriginalExtensionFlag, int PrintOutputFlag){
  vtkImageData *OriginalExtension_Data = vtkImageData::New(); 
  int ChangedExtent[6];
  memcpy(ChangedExtent, selfPtr->GetExtent(), sizeof(int)*6); 
  // Just move it one up - this is just a shortcut - fix it later 
  // Whatever extent [4] and [5] is set to that will the numbering of the slice number int the print out 
  if (!ChangedExtent[4]) {
    ChangedExtent[4] ++;
    ChangedExtent[5] ++;
  }
  if (SliceNum > 0) {
    ChangedExtent[4] += SliceNum-1 + selfPtr->GetSegmentationBoundaryMin()[2] -1 ;
    ChangedExtent[5] = ChangedExtent[4];
  } 
  TIn* OriginalExtension_DataPtr = (TIn*) EMLocalAlgorithm_GetPointerToVtkImageData(OriginalExtension_Data,outputScalar,ChangedExtent);
  vtkIdType OutIncX, OutIncY, OutIncZ;
  OriginalExtension_Data->GetContinuousIncrements(selfPtr->GetExtent(), OutIncX, OutIncY, OutIncZ);
  if ( OriginalExtensionFlag) {
    int NumX = ChangedExtent[1] -ChangedExtent[0] + 1;
    for(int z = 0; z <= ChangedExtent[5] -ChangedExtent[4]; z++) {
      for(int y = 0; y <= ChangedExtent[3] -ChangedExtent[2]; y++) {
    memcpy(OriginalExtension_DataPtr, inputExtension_Vector,sizeof(TIn)*NumX);
    inputExtension_Vector += NumX;
    OriginalExtension_DataPtr += NumX+OutIncY;
      }
      OriginalExtension_DataPtr += OutIncZ;
    }
  } else {
    int outInc[3] = {OutIncX, OutIncY, OutIncZ};

    EMLocalAlgorithm_TransfereDataToOutputExtension(selfPtr,inputExtension_Vector,OriginalExtension_DataPtr ,outInc,SliceNum);
  } 
  EMLocalAlgorithm_GEImageWriter(OriginalExtension_Data,FileName,PrintOutputFlag);
  OriginalExtension_Data->Delete();    
}

// -----------------------------------------------------------
// E-Step Print Function
// -----------------------------------------------------------
template <class T> void EMLocalAlgorithm<T>::Print_E_StepResultsToFile(int iter) {
  // Check if anybody wants to print out weights

  // -----------------------------------------------------------
  // Print out Weights
  // -----------------------------------------------------------

  int WeightFlag  = 0; 
  { 
    int c = 0 ;
    while ((c < this->NumClasses) && (!WeightFlag)) {
      if (((this->ClassListType[c] == CLASS)      && (((vtkImageEMLocalClass*) this->ClassList[c])->GetPrintWeights())) ||
      ((this->ClassListType[c] == SUPERCLASS) && (((vtkImageEMLocalSuperClass*) this->ClassList[c])->GetPrintWeights())) ) WeightFlag = 1;
      c++;
    }
  }
  if (WeightFlag) {
    char *FileName = new char [1000];
    // Just a dummy so it does not cut of the last directory ! => will create this->PrintDir if needed  
    sprintf(FileName,"%s/Weights/blub",this->PrintDir);
    
    if (vtkFileOps::makeDirectoryIfNeeded(FileName) == -1) {
      vtkEMAddErrorMessage( "Could not create the follwoing directory :" << this->PrintDir << "/weights");
      return;
    } 

    sprintf(FileName,"%s/Weights/iter%02d/blub",this->PrintDir,iter);    
    if (vtkFileOps::makeDirectoryIfNeeded(FileName) == -1) {
      vtkEMAddErrorMessage( "Could not create the follwoing directory :" << this->PrintDir << "/weights/iter"<< iter);
      return;
    } 

    int index =0;
    float *outputWeight = NULL;
    int PrintClassWeight;
    float *sumWeight = NULL;
    for ( int c = 0 ; c < this->NumClasses; c++) {
      PrintClassWeight = 0;
      // Define outputWeight
      if (this->ClassListType[c] == CLASS) {
    if (((vtkImageEMLocalClass*) this->ClassList[c])->GetPrintWeights()) {
      outputWeight = this->w_mPtr[index]; 
      PrintClassWeight = 1;
    }
    index ++;
      } else {
    if (((vtkImageEMLocalSuperClass*) this->ClassList[c])->GetPrintWeights() ) {
      PrintClassWeight = 1;
      // Add the weights of all the substructures together
      sumWeight= new float[this->ImageProd]; 
      memcpy(sumWeight,this->w_mPtr[index],sizeof(float)*this->ImageProd); 
      index ++;
      for (int i = 1 ; i < this->NumChildClasses[c]; i++) {
        for (int x = 0 ; x < this->ImageProd; x++) sumWeight[x] += this->w_mPtr[index][x];
        index ++;
      }
      outputWeight = sumWeight; 
    } else {
      index += this->NumChildClasses[c];
    }
      }
      // Define Filename 
      sprintf(FileName,"%s/Weights/iter%02d/EMWeightL%sC%d",this->PrintDir,iter, this->LevelName,c);
      // Print out data 
      switch (((vtkImageEMLocalClass*) this->ClassList[c])->GetPrintWeights()) {
         case 1:  
       EMLocalAlgorithm_PrintDataToOutputExtension(this,outputWeight,VTK_FLOAT,FileName,0,0,0);
       break;
         case 2:
       short *outputShortWeight = new short[this->ImageProd];
       for (int i = 0 ; i < this->ImageProd; i++) outputShortWeight[i] = short(1000.0 *outputWeight[i]); 
       EMLocalAlgorithm_PrintDataToOutputExtension(this,outputShortWeight,VTK_SHORT,FileName,0,0,1);
       delete[] outputShortWeight;
       break;
      }
      if (sumWeight) {
    delete[] sumWeight;
    sumWeight = NULL;
      }
    }
    delete[] FileName;
  }

  // -----------------------------------------------------------
  // Define Label Map 
  // -----------------------------------------------------------
  vtkImageData *LabelMap_WorkingExtension_Data  = NULL;
  short* LabelMap_WorkingExtension_Ptr = NULL;

  // Generate LabelMap if necessary
  if (this->actSupCl->GetPrintLabelMap() || this->QualityFile || this->PCAFile) {
    // Define Label Map with the exetensions defined by the SegmentationBoundary
    int Ext[6] = { 0, this->GetDimensionX()-1, 0, this->GetDimensionY()-1, 0, this->GetDimensionZ()-1};
    LabelMap_WorkingExtension_Data = vtkImageData::New(); 
    LabelMap_WorkingExtension_Ptr = (short*) EMLocalAlgorithm_GetPointerToVtkImageData(LabelMap_WorkingExtension_Data ,VTK_SHORT,Ext);    
    this->DetermineLabelMap(LabelMap_WorkingExtension_Ptr);
  }
  // -----------------------------------------------------------
  //  Print Label Map 
  // -----------------------------------------------------------
  if (this->actSupCl->GetPrintLabelMap()) {
    // Generate LabelMap Directory 
    char FileName[1000];
    sprintf(FileName,"%s/Labelmaps/blub",this->PrintDir); 
    if (vtkFileOps::makeDirectoryIfNeeded(FileName) == -1) {
      vtkEMAddErrorMessage( "Could not create the directory :" << this->PrintDir << "/LabelMaps");
      return;
    }

    // Transfere it to output Extension - Do not use the same extension as for Dice measure bc of different extensions !
    vtkImageData *OriginalExtension_Data = vtkImageData::New();
    int ChangedExtent[6];

    memcpy(ChangedExtent, this->Extent, sizeof(int)*6); 
    if (!ChangedExtent[4]) {
      ChangedExtent[4] ++;
      ChangedExtent[5] ++;
    }
    short* OriginalExtension_DataPtr = (short*) EMLocalAlgorithm_GetPointerToVtkImageData(OriginalExtension_Data,VTK_SHORT,ChangedExtent);
    vtkIdType outIncX, outIncY, outIncZ;
    OriginalExtension_Data->GetContinuousIncrements(ChangedExtent, outIncX, outIncY, outIncZ);
    int outInc[3] = {outIncX, outIncY, outIncZ};
    EMLocalAlgorithm_TransfereDataToOutputExtension(this,LabelMap_WorkingExtension_Ptr,OriginalExtension_DataPtr ,outInc,0);

    // Save Files 
    sprintf(FileName,"%s/Labelmaps/iter%02d/EMLabelMapL%s",this->PrintDir,iter, this->LevelName); 
    if (vtkFileOps::makeDirectoryIfNeeded(FileName) == -1) {
      vtkEMAddErrorMessage( "Could not create the directory :" << this->PrintDir << "/LabelMaps/iter"<<iter);
      OriginalExtension_Data->Delete();
      return;
    }
    EMLocalAlgorithm_GEImageWriter(OriginalExtension_Data,FileName,0);
    OriginalExtension_Data->Delete();
  }
  // -----------------------------------------------------------
  //  Print Quality Measure 
  // -----------------------------------------------------------
  if (this->QualityFile) {
    // -------------------------------------------------
    // Transfere it to output Extension
    vtkImageData *OriginalExtension_Data = vtkImageData::New();
    short *OriginalExtension_DataPtr = (short*) EMLocalAlgorithm_GetPointerToVtkImageData(OriginalExtension_Data,VTK_SHORT,this->Extent);
    vtkIdType outIncX, outIncY, outIncZ;
    OriginalExtension_Data->GetContinuousIncrements(this->Extent, outIncX, outIncY, outIncZ);
    int outInc[3] = {outIncX, outIncY, outIncZ};
    EMLocalAlgorithm_TransfereDataToOutputExtension(this,LabelMap_WorkingExtension_Ptr,OriginalExtension_DataPtr ,outInc,0);

    // -------------------------------------------------
    // Measure Quality Meatric
    cout << "===================================================" << endl;
    int index = 0;
    for ( int c = 0 ; c < this->NumClasses; c++) {
      if (this->QualityFlagList[c]) {
    vtkImageData *QualityReference = ((vtkImageEMLocalClass*) this->ClassList[c])->GetReferenceStandard();

    if   ((this->QualityFile[this->QualityFlagList[c] -1]) &&  QualityReference) {     
      switch (this->QualityFlagList[c]) {
       case 1: 
         {
           int MinExtent[3] = {this->SegmentationBoundaryMin[0] -1, this->SegmentationBoundaryMin[1] -1, this->SegmentationBoundaryMin[2] -1};
           int MaxExtent[3] = {this->SegmentationBoundaryMax[0] -1, this->SegmentationBoundaryMax[1] -1, this->SegmentationBoundaryMax[2] -1};
           fprintf(this->QualityFile[this->QualityFlagList[c] -1],"%10f ",vtkImageEMGeneral::CalcSimularityMeasure(OriginalExtension_Data,QualityReference, this->LabelList[index],1, MinExtent, MaxExtent)); 

           break;
         }
        // Fill in other quality standards if needed
      }
      cout << endl;

    }
    // Fill the other spaces 
        index ++;
      } else {
    index += this->NumChildClasses[c];
      }
    }
    cout << "===================================================" << endl;
    // -------------------------------------------------
    // Print To file and clean up
    for (int i = 0; i < EMSEGMENT_NUM_OF_QUALITY_MEASURE ; i++) if (this->QualityFile[i]) {fprintf(this->QualityFile[i],"\n"); fflush(this->QualityFile[i]);}
    OriginalExtension_Data->Delete();
  }
                                 
  if (LabelMap_WorkingExtension_Data) LabelMap_WorkingExtension_Data->Delete();


  // -----------------------------------------------------------
  // Print convergence parameters 
  // -----------------------------------------------------------
  if (this->WeightsEMDifferenceFile && (iter > 1))  fprintf(this->WeightsEMDifferenceFile, "%f %f \n", this->WeightsEMDifferenceAbsolut,  this->WeightsEMDifferencePercent);
  if (this->LabelMapEMDifferenceFile && (iter > 1)) fprintf(this->LabelMapEMDifferenceFile,"%d %f \n", this->LabelMapEMDifferenceAbsolut, this->LabelMapEMDifferencePercent);
  
  // -----------------------------------------------------------
  // Just for debugging 
  // -----------------------------------------------------------

  if (0) {

    // Generate Binary map 
    short *Blub = new short[this->ImageProd]; 
    for (int i = 0 ; i < this->ImageProd; i++) Blub[i] = (this->OutputVectorPtr[i] & EMSEGMENT_INCORRECT_MODEL ? 1 : 0);

    // Print out 
    vtkImageData *output_OriginalExtension_Data = vtkImageData::New();  
    short* output_OriginalExtension_DataPtr = (short*) EMLocalAlgorithm_GetPointerToVtkImageData(output_OriginalExtension_Data,VTK_SHORT,this->Extent);
    vtkIdType outIncX, outIncY, outIncZ;
    output_OriginalExtension_Data->GetContinuousIncrements(this->Extent, outIncX, outIncY, outIncZ);


    int outInc[3] = {outIncX, outIncY, outIncZ};
    EMLocalAlgorithm_TransfereDataToOutputExtension(this,Blub,output_OriginalExtension_DataPtr ,outInc,0);

    char FileName[1000];
    sprintf(FileName,"%s/output/outputL%sI%d",this->PrintDir, this->LevelName,iter); 
    if (vtkFileOps::makeDirectoryIfNeeded(FileName) == -1) {
      vtkEMAddErrorMessage( "Could not create the directory :" << this->PrintDir << "/output");
      return;
    }
    EMLocalAlgorithm_GEImageWriter(output_OriginalExtension_Data,FileName,1);
    output_OriginalExtension_Data->Delete();
    delete[] Blub;
  }
}



//----------------------------------------------------------------------------
// M-Step Printing Functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Registration

void EMLocalAlgorithm_PrintRegistrationParameters(FILE* ParameterFile, double *Translation, double *Rotation, double *Scale, float cost, int TwoDFlag) {
  if (ParameterFile) {
    if (TwoDFlag) 
      fprintf(ParameterFile,"%12f %12f %12f %12f %12f %12f %12f %12f 1.0\n", Translation[0], Translation[1], Translation[2], Rotation[0], Rotation[1], Rotation[2], Scale[0], Scale[1]);
    else 
      fprintf(ParameterFile,"%12f %12f %12f %12f %12f %12f %12f %12f %12f\n", Translation[0], Translation[1], Translation[2], Rotation[0], Rotation[1], Rotation[2], Scale[0], Scale[1], Scale[2]);
    fprintf(ParameterFile,"Cost: %12f\n",cost);
    fflush(ParameterFile); 
  }
}

//----------------------------------------------------------------------------
void EMLocalAlgorithm_PrintRegistrationParameters(FILE** ParameterFile, double **Translation, double **Rotation, double **Scale, float cost, float costClassSpecific,
                                                       EMLocalRegistrationCostFunction *RegistrationParameters ) {
  if (!ParameterFile) return;
  for (int i = 0; i < RegistrationParameters->GetNumberOfParameterSets(); i++) {
    if ((RegistrationParameters->GetRegistrationType() == EMSEGMENT_REGISTRATION_SEQUENTIAL) && i) 
       EMLocalAlgorithm_PrintRegistrationParameters(ParameterFile[i], Translation[i], Rotation[i], Scale[i],costClassSpecific,RegistrationParameters->GetTwoDFlag());
    else EMLocalAlgorithm_PrintRegistrationParameters(ParameterFile[i], Translation[i], Rotation[i], Scale[i],cost, RegistrationParameters->GetTwoDFlag());
  }
}


//----------------------------------------------------------------------------
template <class T>
void EMLocalAlgorithm<T>::PrintRegistrationData(int SimularityFlag, double **SimplexTranslation, double **SimplexRotation,  double **SimplexScale, int iter) {
  if (this->RegistrationType == EMSEGMENT_REGISTRATION_DISABLED) return; 

  //-----------------------------------------------
  // Setup cost function
  if (SimularityFlag) {
    this->RegistrationParameters->SpatialCostFunctionOn(); 
  } else if (!this->RegistrationParameterFile) return;

  int NumParaSets   = this->RegistrationParameters->GetNumberOfParameterSets();
  int NumParaPerSet = this->RegistrationParameters->GetNumberOfParameterPerSet();
  double*  FinalParameters = new double[NumParaPerSet*NumParaSets];
  
  for (int i = 0; i < NumParaSets; i++) EMLocalAlgorithm_TransfereTranRotSca_ToRegistrationParameter(SimplexTranslation[i], SimplexRotation[i], SimplexScale[i], 
                                                                  &FinalParameters[i*NumParaPerSet],RegistrationParameters);
  // EMLocalAlgorithm_PrintVector(FinalParameters, 0, NumParaPerSet*NumParaSets -1); 

  //-----------------------------------------------
  // Run cost function
  float cost;
  float costClassSpecific = 0;

  // if (RegistrationParameters->MAPAlgorithm == EMSEGMENT_REGISTRATION_SIMPLEX) FinalParameters --;
  if (this->RegistrationType < EMSEGMENT_REGISTRATION_SEQUENTIAL) {
    // cout << "Debug " << endl;
    // FinalParameters[0] = 1;
    cost = this->RegistrationParameters->ComputeCostFunction(FinalParameters);
    if (!iter) cout << "Initial Cost:       " << cost << endl;   
  } else {
    this->RegistrationParameters->SetRegistrationType(EMSEGMENT_REGISTRATION_GLOBAL_ONLY);
    int OrigNumberOfParaSets = this->RegistrationParameters->GetNumberOfParameterSets();
    this->RegistrationParameters->SetNumberOfParameterSets(1);
    cost = this->RegistrationParameters->ComputeCostFunction(FinalParameters);
    if (SimularityFlag) {
      char FileName[1000];
      if (iter) sprintf(FileName,"%s/Registration/SimularityMeasureGlobalL%sI%d",this->PrintDir, this->LevelName,iter);
      else sprintf(FileName,"%s/Registration/SimularityMeasureGlobalL%sInit",this->PrintDir, this->LevelName);
      if (RegistrationParameters->GetDebug()) sprintf(FileName,"%s/Registration/SimularityMeasureGlobalDebug",this->PrintDir);
      EMLocalAlgorithm_PrintDataToOutputExtension(this,RegistrationParameters->GetSpatialCostFunction(),VTK_DOUBLE,FileName,0,0,0);
    } 
    if (!iter) {
      cout << "Initial Cost:       " << cost << endl;
      // this is not the cleanest way but the fastest right now 
      // if (RegistrationParameters->MAPAlgorithm == EMSEGMENT_REGISTRATION_SIMPLEX) FinalParameters ++;
      delete[] FinalParameters;
      this->RegistrationParameters->SetRegistrationType(EMSEGMENT_REGISTRATION_SEQUENTIAL);
      this->RegistrationParameters->SetNumberOfParameterSets(OrigNumberOfParaSets);
      return ;
    }

    this->RegistrationParameters->SetRegistrationType(EMSEGMENT_REGISTRATION_CLASS_ONLY);
    this->RegistrationParameters->SetNumberOfParameterSets(OrigNumberOfParaSets -1);

    FinalParameters += this->RegistrationParameters->GetNumberOfParameterPerSet();
    costClassSpecific = this->RegistrationParameters->ComputeCostFunction(FinalParameters);


    FinalParameters -= this->RegistrationParameters->GetNumberOfParameterPerSet();
    // Kilian: Change this later very complicated way of doing things
    this->RegistrationParameters->SetNumberOfParameterSets(OrigNumberOfParaSets);
    this->RegistrationParameters->SetRegistrationType(EMSEGMENT_REGISTRATION_SEQUENTIAL);
  }
  // if (RegistrationParameters->MAPAlgorithm == EMSEGMENT_REGISTRATION_SIMPLEX) FinalParameters ++;

  //-----------------------------------------------
  // Print out parameters
  EMLocalAlgorithm_PrintRegistrationParameters(this->RegistrationParameterFile, SimplexTranslation, SimplexRotation, SimplexScale, cost, costClassSpecific, 
                         this->RegistrationParameters); 

  if (SimularityFlag) {
    char FileName[1000];
    if (this->RegistrationType != EMSEGMENT_REGISTRATION_SEQUENTIAL) {
      if (iter) sprintf(FileName,"%s/Registration/SimularityMeasureL%sI%d",this->PrintDir,this->LevelName,iter);
      else sprintf(FileName,"%s/Registration/SimularityMeasureL%sInit",this->PrintDir, this->LevelName);
      if (RegistrationParameters->GetDebug()) sprintf(FileName,"%s/Registration/SimularityMeasureDebug",this->PrintDir);
    } else {
      if (iter) sprintf(FileName,"%s/Registration/SimularityMeasureClassL%sI%d",this->PrintDir,this->LevelName,iter);
      if (RegistrationParameters->GetDebug()) sprintf(FileName,"%s/Registration/SimularityMeasureClassDebug",this->PrintDir);
    }

    EMLocalAlgorithm_PrintDataToOutputExtension(this,this->RegistrationParameters->GetSpatialCostFunction(),VTK_DOUBLE,FileName,0,0,0);
    RegistrationParameters->SpatialCostFunctionOff();
  } 

  //-----------------------------------------------
  // Clean up
  delete[] FinalParameters;
}

template <class T> int EMLocalAlgorithm<T>::DefinePrintRegistrationParameters(int NumParaSets) {
  // Open Files         
  for (int c = 0; c < NumParaSets; c++) this->RegistrationParameterFile[c] = NULL; 
  int ParaSetIndex = 0; 

  if (this->RegistrationType != EMSEGMENT_REGISTRATION_CLASS_ONLY) {
    this->RegistrationParameterFile[0] = this->OpenTextFile("Registration/GlobalParameters",0,0,1,0,0,NULL);
    ParaSetIndex ++ ;
  }
  
  if (this->RegistrationType != EMSEGMENT_REGISTRATION_GLOBAL_ONLY) {
    for (int c = this->GenerateBackgroundProbability; c < this->NumClasses; c++) {
      if (this->RegistrationClassSpecificRegistrationFlag[c]) {
    if ((this->ClassListType[c]== CLASS) ? ((vtkImageEMLocalClass*) this->ClassList[c])->GetPrintRegistrationParameters() : ((vtkImageEMLocalSuperClass*) this->ClassList[c])->GetPrintRegistrationParameters())
      this->RegistrationParameterFile[ParaSetIndex] = this->OpenTextFile("Registration/ClassParameters",c,1,1,0,0,NULL);
    ParaSetIndex ++;
      }
    }
  }
  return 1;
}

template <class T>
void EMLocalAlgorithm<T>::Print_M_StepRegistrationToFile(int iter, float RegistrationCost, float RegistrationClassSpecificCost) {
  if (this->RegistrationType <= EMSEGMENT_REGISTRATION_APPLY) return;
   
  // Print Simularity Measure
  if (this->actSupCl->GetPrintRegistrationSimularityMeasure()) { 
    this->PrintRegistrationData(1, this->RegistrationTranslation, this->RegistrationRotation, this->RegistrationScale, iter);
  }
  else {
    EMLocalAlgorithm_PrintRegistrationParameters(this->RegistrationParameterFile,  this->RegistrationTranslation, this->RegistrationRotation, 
                           this->RegistrationScale, RegistrationCost, RegistrationClassSpecificCost, this->RegistrationParameters);
  }
 
}



//----------------------------------------------------------------------------
// Shape 
template <class T>
float EMLocalAlgorithm<T>::PrintShapeData(float **PCAShapeParameters, int iter , int PrintSimulatingFlag) {
  if (this->ShapeParameters->PCAShapeModelType == EMSEGMENT_PCASHAPE_APPLY) return 0.0;
  // cout << "Total " << this->PCATotalNumOfShapeParameters << endl;
  double *parameters = new double[this->PCATotalNumOfShapeParameters];
  int ShapeIndex = 0;
  // cout << "Start Printing Shape Data " << endl;
  for (int i = 0 ; i < this->NumTotalTypeCLASS; i++) {
    for (int k = 0 ; k < this->PCANumberOfEigenModes[i]; k++) {
      parameters[ShapeIndex] = double(PCAShapeParameters[i][k]);
      ShapeIndex ++;
    }
    if (this->PCAShapeModelType == EMSEGMENT_PCASHAPE_DEPENDENT) ShapeIndex = 0;
  }

  if (PrintSimulatingFlag) this->ShapeParameters->SpatialCostFunctionOn();

  //cout << "Super Debug " << endl;
  //parameters[0] = 2.0; 
  // cout << "Start Threading" << endl;
  float cost = this->ShapeParameters->ComputeCostFunction(parameters);

  if (PrintSimulatingFlag) {
    char FileName[1000];

    if (iter) sprintf(FileName,"%s/Shape/iter%02d/SimularityMeasureL%s",this->PrintDir,iter,this->LevelName);
    else sprintf(FileName,"%s/Shape/init/SimularityMeasureL%s",this->PrintDir, this->LevelName);
    if (vtkFileOps::makeDirectoryIfNeeded(FileName) == -1) {
      vtkEMAddErrorMessage( "Could not create the directory for:" << FileName);
      return cost;
    }
    EMLocalAlgorithm_PrintDataToOutputExtension(this,this->ShapeParameters->GetSpatialCostFunction(),VTK_FLOAT,FileName,0,0,0);
    this->ShapeParameters->SpatialCostFunctionOff();
  }
  delete[] parameters;
  return cost;
}

void EMLocalAlgorithm_PrintPCAParameters(EMLocalShapeCostFunction *ShapeParameters, FILE **PCAFile, float **PCAShapeParameters, int*  LabelList, float PCACost) {
  if (ShapeParameters->PCAShapeModelType == EMSEGMENT_PCASHAPE_APPLY) return;

  int index = 0;
  int NumClasses                = ShapeParameters->GetNumClasses();
  int *NumChildClasses          = ShapeParameters->GetNumChildClasses();
  float **PCAInverseEigenValues = ShapeParameters->PCAInverseEigenValues;

  cout << "---------- PCA -----------" << endl;   
  cout << "Gaussian Peanlity: " << ShapeParameters->GetGaussianPenalty() << endl;
  cout << "Image Peanlity:    " << ShapeParameters->GetImagePenalty() << endl;
  cout << "Total Cost:        " << PCACost << endl;
  for (int c = 0; c < NumClasses; c++) {
    if (PCAFile[c]) {
      for (int i = 0 ; i <  NumChildClasses[c]; i++) {
    cout << "Parameters "<< c << " with Label" << LabelList[index] << " (B Value):  ";
    for (int k = 0 ; k < ShapeParameters->GetPCANumberOfEigenModes(index); k++) {
      cout << PCAShapeParameters[index][k] << " (" << PCAShapeParameters[index][k] * sqrt(PCAInverseEigenValues[index][k])<< ") | ";
      fprintf(PCAFile[c],"%12f ",PCAShapeParameters[index][k] *sqrt(PCAInverseEigenValues[index][k]));
    }
    fprintf(PCAFile[c],"\n");
    fprintf(PCAFile[c],"Cost: %f \n",PCACost);

    cout << endl;
    index ++;
      }
    } else {
      index += NumChildClasses[c];
    }
  }
 
  // Generate the ShapeParameters from the labelmap 
  // float  **PCAMeanShapePtr       = new float*[NumTotalTypeCLASS]; 
  // for (int i = 0; i < NumTotalTypeCLASS; i++) PCAMeanShapePtr[i] = ShapeParameters.PCAMeanShapePtr[i];
  // int NumTotalTypeCLASS         = ShapeParameters.EMHierarchyParameters->NumTotalTypeCLASS;
  // float **PCAValues = new float*[NumTotalTypeCLASS];
  // for (int i=0; i< NumTotalTypeCLASS; i++) {
  //   if (PCANumberOfEigenModes[i]) PCAValues[i] = new float[PCANumberOfEigenModes[i]];
  //   else PCAValues[i] = NULL;
  // }
  // // Calculate PCA parameters of labelmap 
  // vtkImageEMLocalSegmentGeneratePCAWeightsFromTissueWeights(this, PCAValues, NumTotalTypeCLASS, PCANumberOfEigenModes, LabelList,  PCAMeanShapePtr, 
  //                                   PCAEigenVectorsPtr, PCAEigenValues, LabelMap_WorkingExtension_Data); 
  // 
  // // Print them out 
  // index = 0 ;
  // for ( int c = 0 ; c < NumClasses; c++) {
  //   if (PCAFile[c]) {
  //     cout << "PCAShapeParameters (B Value) " << c << " :  ";
  //     for (int l = 0; l < PCANumberOfEigenModes[index]; l++) cout <<  PCAValues[index][l] * sqrt(PCAEigenValues[index][l]) << " ("<<  PCAValues[index][l] << ") |";
  //     cout << endl;     
  //   }
  //   index += NumChildClasses[c];
  // }
  // 
  // // Cleanup Mess
  // for (int i=0; i< NumTotalTypeCLASS; i++) {
  //   if (PCAValues[i]) delete[] PCAValues[i]; 
  // }
  // delete[] PCAValues;

  cout << "---------------------" << endl;    
}

template <class T> void EMLocalAlgorithm<T>::Print_M_StepShapeToFile(int iter, float PCACost) {
  // Print Simularity Measure
  if (this->actSupCl->GetPrintShapeSimularityMeasure()) this->PrintShapeData(this->PCAShapeParameters,iter,1);
  if (this->PCAFile) EMLocalAlgorithm_PrintPCAParameters(this->ShapeParameters, this->PCAFile, this->PCAShapeParameters, this->LabelList, PCACost);
}

// -----------------------------------------------------------
// Main M-Step Function
template <class T>
void EMLocalAlgorithm<T>::Print_M_StepResultsToFile(int iter, float PCACost, float RegistrationCost, float RegistrationClassSpecificCost, 
                                                   EMTriVolume& iv_m, EMVolume *r_m, float *cY_M)  {
  this->Print_M_StepRegistrationToFile(iter, RegistrationCost, RegistrationClassSpecificCost);
  this->Print_M_StepShapeToFile(iter, PCACost);
  // Just run it one more time to print out bias
  if (this->BiasPrint && (!this->ROIPtr)) this->IntensityCorrection(1, iter, iv_m, r_m, cY_M);
}


