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

template  <class T> int EMLocalAlgorithm<T>::Initialize(vtkImageEMLocalSegmenter *vtk_filter, T **initProbDataPtrStart, float** initInputVector, short *initROI, 
                              float **initw_mPtr, char *initLevelName, float initGlobalRegInvRotation[9], float initGlobalRegInvTranslation[3], 
                              int initRegistrationType, int DataType)
{
    int SuccessFlag = 1;
   
    this->InitializeEM(vtk_filter, initLevelName, initRegistrationType, initInputVector, initROI, vtk_filter->GetActiveSuperClass()->GetLabel(), initw_mPtr); 
    if (!this->InitializeClass(vtk_filter->GetActiveSuperClass(), initProbDataPtrStart)) SuccessFlag = 0;
    
    this->InitializeHierarchicalParameters();
    this->InitializeBias();
    this->InitializePrint();

    if (!this->InitializeShape()) SuccessFlag = 0;
    if (!this->InitializeRegistration(initGlobalRegInvRotation, initGlobalRegInvTranslation)) SuccessFlag = 0;

    this->InitializeEStepMultiThreader(DataType);
    return SuccessFlag;
}


template  <class T> void EMLocalAlgorithm<T>::InitializeEM(vtkImageEMLocalSegmenter* vtk_filter, char* initLevelName,int initRegistrationType, 
                                   float** initInputVector, short *initROI, int ROI_Label, float **initw_mPtr) {
  this->ImageProd                = vtk_filter->GetImageProd();
  this->NumInputImages           = vtk_filter->GetNumInputImages();
  this->SegmentationBoundaryMin  = vtk_filter->GetSegmentationBoundaryMin();
  this->SegmentationBoundaryMax  = vtk_filter->GetSegmentationBoundaryMax();
  this->Extent                   = vtk_filter->GetExtent();

  this->BoundaryMaxZ             = vtk_filter->GetDimensionZ();
  this->BoundaryMaxY             = vtk_filter->GetDimensionY();
  this->BoundaryMaxX             = vtk_filter->GetDimensionX();  
  this->imgXY                    = BoundaryMaxY*BoundaryMaxX;

  this->RealMaxX                 = Extent[1] - Extent[0] + 1;
  this->RealMaxY                 = Extent[3] - Extent[2] + 1;
  this->RealMaxZ                 = Extent[5] - Extent[4] + 1;

  this->Alpha                         = vtk_filter->GetAlpha();
  if ((this->Alpha > 0) && (vtk_filter->GetActiveSuperClass()->GetStopMFAMaxIter() < 1)) {
    vtkEMAddWarningMessage("Alpha is set to " <<this->Alpha << " even though StopMFAMaxIter < 1 ! Thus, we disable MeanField and set Alpha to 0" );
    this->Alpha = 0.0;
  }
  this->NumberOfTrainingSamples       = vtk_filter->GetNumberOfTrainingSamples();
  this->RegistrationInterpolationType = vtk_filter->GetRegistrationInterpolationType();

  this->PrintDir                = vtk_filter->GetPrintDir(); 
  this->LevelName               = initLevelName;
  this->RegistrationType        = initRegistrationType;
  this->DisableMultiThreading   = vtk_filter->GetDisableMultiThreading(); 

  this->SmoothingWidth          = vtk_filter->GetSmoothingWidth();
  this->SmoothingSigma          = vtk_filter->GetSmoothingSigma();


  // Should be defined later in EM-Varaible Section but needed for CostFunctionParameters
  this->OutputVectorPtr = new unsigned char[ImageProd];
  unsigned char* OutputVector = this->OutputVectorPtr;
  memset(OutputVector, 0, this->ImageProd*sizeof(unsigned char));

  this->cY_MPtr = new float[this->NumInputImages* this->ImageProd]; 
  memset(this->cY_MPtr, 0, this->NumInputImages * this->ImageProd * sizeof(float));

  this->NumROIVoxels                 =  0;
  this->ROIPtr = initROI;
  this->InputVectorPtr = initInputVector;

  if (this->ROIPtr) {
    // 1.) Check what the region of interest is 
    for (int i=0; i < this->ImageProd; i++) {
      if ((*this->ROIPtr++) != ROI_Label) *OutputVector |= EMSEGMENT_NOTROI; 
      else  NumROIVoxels ++;
      OutputVector ++;
    }
    OutputVector = OutputVectorPtr; this->ROIPtr = initROI;
  } else {
    NumROIVoxels = ImageProd;
  }

  // 2.) Check every voxel if it has a defined neighbor or an edge
  for (int i=0; i < ImageProd; i++) {
    if (*OutputVector < EMSEGMENT_NOTROI) {
      if (i % BoundaryMaxX)  {
         if (OutputVector[-1]&EMSEGMENT_NOTROI)          *OutputVector |= EMSEGMENT_NORTH;
      } else { *OutputVector |= EMSEGMENT_NORTH;}

      if ((i+1) % BoundaryMaxX) {
    if (OutputVector[1]&EMSEGMENT_NOTROI)           *OutputVector |= EMSEGMENT_SOUTH;
      } else { *OutputVector |= EMSEGMENT_SOUTH;}
             
      if (int(i/BoundaryMaxX)%BoundaryMaxY)     {
    if  (OutputVector[-BoundaryMaxX]&EMSEGMENT_NOTROI) *OutputVector |= EMSEGMENT_WEST;
      } else { *OutputVector |= EMSEGMENT_WEST;}
  
      if (int(i/BoundaryMaxX + 1)%BoundaryMaxY) {
    if (OutputVector[BoundaryMaxX]&EMSEGMENT_NOTROI)   *OutputVector |= EMSEGMENT_EAST;
      } else { *OutputVector |= EMSEGMENT_EAST;}

      if (int(i/imgXY))                   {
    if (OutputVector[-imgXY]&EMSEGMENT_NOTROI)      *OutputVector |= EMSEGMENT_FIRST;
      } else { *OutputVector |= EMSEGMENT_FIRST;}

      if (int(i/imgXY +1)% BoundaryMaxZ )    {
    if (OutputVector[+imgXY]&EMSEGMENT_NOTROI)      *OutputVector |= EMSEGMENT_LAST;
      } else { *OutputVector |= EMSEGMENT_LAST;}
    }
    OutputVector ++;
  }

  this->w_mPtr = initw_mPtr;
}

template  <class T> int EMLocalAlgorithm<T>::InitializeClass(vtkImageEMLocalSuperClass* initactSupCl, T** initProbDataPtrStart) {
  // Local Variables 
  float *InputChannelWeights;
  int SuccessFlag = 1 ;

  // Initialize
  this->actSupCl                      = initactSupCl;
  this->ClassList                     = this->actSupCl->GetClassList();
  this->ClassListType                 = this->actSupCl->GetClassListType();  
  this->NumClasses                    = this->actSupCl->GetNumClasses();
  this->NumTotalTypeCLASS             = this->actSupCl->GetTotalNumberOfClasses(false);
  this->GenerateBackgroundProbability = this->actSupCl->GetGenerateBackgroundProbability();
  // GenerateBoundaryProbability has to be 0 or 1  
  assert(GenerateBackgroundProbability > -1 && GenerateBackgroundProbability < 2);

  this->StopBiasCalculation           = this->actSupCl->GetStopBiasCalculation();
 
  this->BiasPrint                     = this->actSupCl->GetPrintBias();
  this->PrintFrequency                = this->actSupCl->GetPrintFrequency();
  this->PrintEMLabelMapConvergence    = this->actSupCl->GetPrintEMLabelMapConvergence(); 
  this->PrintEMWeightsConvergence     = this->actSupCl->GetPrintEMWeightsConvergence(); 
  this->StopEMType                    = this->actSupCl->GetStopEMType();
  this->StopEMValue                   = this->actSupCl->GetStopEMValue();

  this->NumIter                       = this->actSupCl->GetStopEMMaxIter();

  this->PrintMFALabelMapConvergence   =  this->actSupCl->GetPrintMFALabelMapConvergence(); 
  this->PrintMFAWeightsConvergence    =  this->actSupCl->GetPrintMFAWeightsConvergence(); 

  this->NumRegIter                    =  this->actSupCl->GetStopMFAMaxIter();
  this->StopMFAType                   =  this->actSupCl->GetStopMFAType();
  this->MRFParams                     =  this->actSupCl->GetMrfParams(); 

   if (PrintMFALabelMapConvergence || StopMFAType ==  EMSEGMENT_STOP_LABELMAP) 
    CurrentMFALabelMap  = new short[this->ImageProd];
  else 
    this->CurrentMFALabelMap           =  NULL;

  if (PrintMFAWeightsConvergence || StopMFAType == EMSEGMENT_STOP_WEIGHTS) {
    this->CurrentMFAWeights  = new float*[this->NumClasses];
    for (int i=0; i<this->NumClasses; i++) this->CurrentMFAWeights[i] = new float[this->ImageProd];
  } else {
    this->CurrentMFAWeights           =  NULL;
  }

  // After Check some more the registrationTyp 
  this->ProbDataIncZ = new int[this->NumTotalTypeCLASS];           
  this->ProbDataIncY = new int[this->NumTotalTypeCLASS];       

  // Depending if we do registration, the segmentation boundary has to be considered or not 
  this->actSupCl->GetProbDataIncYandZ(this->ProbDataIncY,this->ProbDataIncZ,0,!this->RegistrationType);
  this->TissueProbability        = new double[NumClasses];
  this->ProbDataWeight           = new float[NumClasses];
  this->ProbDataMinusWeight      = new float[NumClasses];

  // if on of the weights is set to 0 the virtual dimension is one minus the real dimensio = NimInputImages -> necessary to calculate gaussian curve 
  // -> otherwise you get different reuslts from segmenting something with weight =(1,0) in comaprison to the equivalent one dimensional input channel segmentation 
  this->VirtualNumInputImages       = new int[NumClasses];

  this->ProbDataPtrStart = new T*[NumTotalTypeCLASS];
  for (int i =0;i<NumTotalTypeCLASS;i++)  this->ProbDataPtrStart[i] = initProbDataPtrStart[i]; 

  this->ExcludeFromIncompleteEStepFlag = new int[NumClasses];

  this->LogMu                        = new double*[NumTotalTypeCLASS];
  this->LogCovariance                = new double**[NumTotalTypeCLASS];
  this->InverseWeightedLogCov        = new double**[NumTotalTypeCLASS];
  this->InvSqrtDetWeightedLogCov     = new double[NumTotalTypeCLASS];
  this->LabelList                    = new int[NumTotalTypeCLASS];
  this->CurrentLabelList             = new int[NumClasses];

  // Define Files that should be printed in 
  for (int i = 0; i < this->NumClasses; i ++) {
    if (this->ClassListType[i] == CLASS) {
      CurrentLabelList[i]               = ((vtkImageEMLocalClass*) this->ClassList[i])->GetLabel();
      ExcludeFromIncompleteEStepFlag[i] = ((vtkImageEMLocalClass*) ClassList[i])->GetExcludeFromIncompleteEStepFlag(); 
    } else {
      CurrentLabelList[i]               = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetLabel();
      ExcludeFromIncompleteEStepFlag[i] = ((vtkImageEMLocalSuperClass*) ClassList[i])->GetExcludeFromIncompleteEStepFlag();
    }                      
    if (ExcludeFromIncompleteEStepFlag[i]) 
      { 
      std::cerr << "Class "<< i << " is excluded from Incomplete E-Step ! " 
                << std::endl;
      }

  }
  
  // ------------------------------------------------------
  // Variables defined by Subclasses
  // ------------------------------------------------------
  for (int i = 0; i < this->NumTotalTypeCLASS; i++) {
    this->InverseWeightedLogCov[i]  = new double*[NumInputImages];
    for (int x = 0; x < NumInputImages; x++)  this->InverseWeightedLogCov[i][x]  = new double[NumInputImages];
  }

  this->actSupCl->GetTissueDefinition(this->LabelList,this->LogMu, this->LogCovariance, 0);

#if (0)
 {
    vtkIndent indent;
    actSupCl->PrintSelf(std::cerr,indent); 
 }
#endif

  // Number of child classes => if class is of Type CLASS => is set to 1
  // otherwise it is ther number of children of TYPE CLASS
  this->NumChildClasses = new int[NumClasses];

  // VirtualOveralInputChannelFlag[i] = 0 => input channel i is not used by any class 
  // VirtualOveralInputChannelFlag[i] = 1 => input channel i is used by at least one class 
  this->VirtualOveralInputChannelFlag = new int[NumInputImages];
  memset(this->VirtualOveralInputChannelFlag, 0, sizeof(int)*this->NumInputImages);    

  // VirtualOveralInputChannelNum defines the number of real input channels analyzed by all tissue classes together 
  // if VirtualOveralInputChannelNum < NumInputChannel => at least one input channel is not used by any tissue class

  int index = 0;

  for (int i=0; i < this->NumClasses; i++) {
    if (this->ClassListType[i] == CLASS) {
      NumChildClasses[i]   = 1;
      this->TissueProbability[i]  = ((vtkImageEMLocalClass*) ClassList[i])->GetTissueProbability();

      this->ProbDataWeight[i]     = ((vtkImageEMLocalClass*) ClassList[i])->GetProbDataWeight();      
      this->ProbDataMinusWeight[i] =  float(NumberOfTrainingSamples)*(1.0 - ProbDataWeight[i]);

      InputChannelWeights = ((vtkImageEMLocalClass*) ClassList[i])->GetInputChannelWeights();
 
      if (this->CalcWeightedCovariance(InverseWeightedLogCov[index], InvSqrtDetWeightedLogCov[index], InputChannelWeights, LogCovariance[index], this->VirtualNumInputImages[i],NumInputImages) == 0) 
        {
        vtkEMAddErrorMessage("vtkImageEMLocalAlgorithm: weighted covariance has a non positive determinante  for class with index "<< index << ". See shell for more specific output!");
        std::cerr << "InvSqrtDetWeightedLogCov: " 
                  << InvSqrtDetWeightedLogCov[index] 
                  << " VirtualNumInputImages: " 
                  << VirtualNumInputImages[i] 
                  << std::endl  
                  << "LogCovariance :";  
        for (int x =0 ; x <NumInputImages; x ++ ) 
          {
          for (int y =0 ; y <NumInputImages; y ++ ) 
            {
            std::cerr << LogCovariance[index][x][y] << " ";
            }
          std::cerr << " | ";
          }
        std::cerr << std::endl << "Weighted LogCovariance : "; 
        for (int x =0 ; x <NumInputImages; x ++ ) 
          {
          for (int y =0 ; y <NumInputImages; y ++ ) 
            {
            std::cerr << LogCovariance[index][x][y]*
              InputChannelWeights[x]*InputChannelWeights[y] << " ";
            }
          std::cerr << " | ";
          }
        
        std::cerr << std::endl;  
        SuccessFlag = 0;
        }

#if (0)
      std::cerr << "=========== "<< index << " =============" << std::endl;
      std::cerr << "InvSqrtDetWeightedLogCov: " 
                << InvSqrtDetWeightedLogCov[index] 
                << " VirtualNumInputImages: " 
                << VirtualNumInputImages[i] 
                << std::endl  
                << "InverseWeightedLogCov :";
      for (int x =0 ; x <NumInputImages; x ++ ) 
        {
        for (int y =0 ; y <NumInputImages; y ++ ) 
          {
          fprintf(stderr, "%8.5f ", InverseWeightedLogCov[index][x][y]);
          }
        std::cerr << " | ";
        }
      std::cerr << std::endl;
#endif
      index ++;

    } 
    else  
      {
      NumChildClasses[i]   = ((vtkImageEMLocalSuperClass*) 
                              ClassList[i])->GetTotalNumberOfClasses(false);
      TissueProbability[i] = ((vtkImageEMLocalSuperClass*) 
                              ClassList[i])->GetTissueProbability();
      ProbDataWeight[i]    = ((vtkImageEMLocalSuperClass*) 
                              ClassList[i])->GetProbDataWeight();
      if (NumChildClasses[i]) 
        {
        ProbDataMinusWeight[i] =  float(NumberOfTrainingSamples)* 
          (1.0 - ProbDataWeight[i])/float(NumChildClasses[i]);
        }
      else 
        {
        ProbDataMinusWeight[i] = 0.0;
        }

      InputChannelWeights = ((vtkImageEMLocalSuperClass*) 
                             ClassList[i])->GetInputChannelWeights();

      for (int k = 0;k < NumChildClasses[i]; k++) 
        {
        if (this->CalcWeightedCovariance(InverseWeightedLogCov[index], 
                                         InvSqrtDetWeightedLogCov[index], 
                                         InputChannelWeights, 
                                         LogCovariance[index], 
                                         VirtualNumInputImages[i],
                                         NumInputImages) == 0) 
          {
          vtkEMAddErrorMessage("vtkImageEMLocalAlgorithm: weighted covariance has a non positive determinante  for class with index "<< index << ". See shell for more specific output!");
          std::cerr << "InvSqrtDetWeightedLogCov: " 
                    << InvSqrtDetWeightedLogCov[index] 
                    << " VirtualNumInputImages: " 
                    <<  VirtualNumInputImages[i] << std::endl  
                    << "LogCovariance :";  
          for (int x =0 ; x <NumInputImages; x ++ ) 
            {
            for (int y =0 ; y <NumInputImages; y ++ ) 
              {
              std::cerr << LogCovariance[index][x][y] << " ";
              }
            std::cerr << " | ";
           }
          std::cerr << std::endl << "Weighted LogCovariance : "; 
          for (int x =0 ; x <NumInputImages; x ++ ) 
            {
            for (int y =0 ; y <NumInputImages; y ++ ) 
              {
              std::cerr << LogCovariance[index][x][y]*InputChannelWeights[x]*
                InputChannelWeights[y] << " ";
              }
            std::cerr << " | ";
            }
           
          std::cerr << std::endl;  
          SuccessFlag = 0;
          }
#if (0)
        std::cerr << "=========== "<< index << " =============" << std::endl;
        std::cerr << "InvSqrtDetWeightedLogCov: " 
                  << InvSqrtDetWeightedLogCov[index] 
                  << " VirtualNumInputImages: " 
                  <<  VirtualNumInputImages[i] 
                  << std::endl 
                  << "InverseWeightedLogCov: ";  
        for (int x =0 ; x <NumInputImages; x ++ ) 
          {
          for (int y =0 ; y <NumInputImages; y ++ ) 
            {
            fprintf(stdout, "%8.5f ", InverseWeightedLogCov[index][x][y]);
            }
          std::cerr << " | ";
          }
        std::cerr << std::endl;
#endif
        index ++; 
        }
      }


    for (int k= 0; k < NumInputImages; k++) 
      { 
      if (InputChannelWeights[k] > 0.0) 
        {
        VirtualOveralInputChannelFlag[k] = 1; 
        }
      }
  } 

  this->VirtualOveralInputChannelNum = 0;
  for (int k= 0; k < NumInputImages; k++) { 
    if (this->VirtualOveralInputChannelFlag[k]) this->VirtualOveralInputChannelNum ++;
  }
  return SuccessFlag;
}

// Kilian: Rename hierarchicalparameters to EMLocalCommonInterfaceParameters
template  <class T> void EMLocalAlgorithm<T>::InitializeHierarchicalParameters() {

  HierarchicalParameters.NumClasses            = this->NumClasses;
  HierarchicalParameters.NumTotalTypeCLASS     = this->NumTotalTypeCLASS;
  HierarchicalParameters.NumChildClasses       = this->NumChildClasses;
  HierarchicalParameters.ProbDataIncY          = this->ProbDataIncY; 
  HierarchicalParameters.ProbDataIncZ          = this->ProbDataIncZ; 
  HierarchicalParameters.ProbDataWeight        = this->ProbDataWeight;
  HierarchicalParameters.ProbDataMinusWeight   = this->ProbDataMinusWeight;
  HierarchicalParameters.ProbDataType          = (this->actSupCl->GetProbDataScalarType() > -1 ? this->actSupCl->GetProbDataScalarType() : VTK_CHAR) ;
  HierarchicalParameters.LogMu                 = this->LogMu;
  HierarchicalParameters.InvLogCov             = this->InverseWeightedLogCov;
  HierarchicalParameters.InvSqrtDetLogCov      = this->InvSqrtDetWeightedLogCov;
  HierarchicalParameters.TissueProbability     = this->TissueProbability;
  HierarchicalParameters.VirtualNumInputImages = this->VirtualNumInputImages;
  HierarchicalParameters.MrfParams             = this->actSupCl->GetMrfParams();
}


// -----------------------------------------------------------
// Variables defined for Bias
// -----------------------------------------------------------

template  <class T> void EMLocalAlgorithm<T>::InitializeBias() {
  int BiasLengthFileName    = 0;
 
  if (this->BiasPrint && this->PrintFrequency) {
    if (this->PrintDir != NULL) BiasLengthFileName = int(strlen(this->PrintDir));
    BiasLengthFileName +=  20 + NumInputImages/10 + int(strlen(this->LevelName));

    char *BiasDirectory = new char[BiasLengthFileName];
    if (this->PrintDir != NULL) sprintf(BiasDirectory,"%s/Bias/blub",this->PrintDir);
    else sprintf(BiasDirectory,"Bias");

    if (vtkFileOps::makeDirectoryIfNeeded(BiasDirectory) == -1) 
      {
      vtkEMAddErrorMessage( "Could not create the directory :" 
                            << this->PrintDir << "/Bias");
      this->BiasPrint = 0;
      } 
    else 
      {
      std::cerr << "vtkImageEMLocalAlgorithm: Print Bias (Type: Float) to " 
                << this->PrintDir << "/Bias" << std::endl;
      }
    delete[] BiasDirectory;
  }
}

// -----------------------------------------------------------
// Print Quality Measure Setup 
// -----------------------------------------------------------

template <class T> void EMLocalAlgorithm<T>::InitializePrint() {
  // This variable is set so that we can exclude classed from the incomplete E-Step 
  int QualityFlag = 0;

  this->QualityFile  = NULL; 
  this->QualityFlagList = new int[NumClasses];
  memset(this->QualityFlagList,0,sizeof(int)*NumClasses);

  if (this->PrintFrequency) {
    int i = 0;
    while ((i < this->NumClasses) && !QualityFlag) {
      if ((this->ClassListType[i]== CLASS) && (((vtkImageEMLocalClass*) this->ClassList[i])->GetPrintQuality()) && 
      ((vtkImageEMLocalClass*) this->ClassList[i])->GetReferenceStandard()) QualityFlag = 1;
      i++;
    }
  }

  if (QualityFlag) {
    std::cerr << "Open Quality File" << std::endl;
    this->QualityFile = new FILE*[EMSEGMENT_NUM_OF_QUALITY_MEASURE];
    for (int i = 0; i < EMSEGMENT_NUM_OF_QUALITY_MEASURE ; i++) this->QualityFile[i] = NULL;
    for (int c = 0; c < this->NumClasses; c++) {
      if (this->ClassListType[c] == CLASS) {
    int PrintQuality = this->QualityFlagList[c] = ((vtkImageEMLocalClass*) this->ClassList[c])->GetPrintQuality();
    
    if  (PrintQuality  && (!QualityFile[PrintQuality -1]) && ((vtkImageEMLocalClass*) ClassList[c])->GetReferenceStandard()) { 
      switch (((vtkImageEMLocalClass*) ClassList[c])->GetPrintQuality ()) {
        case 1 :  this->QualityFile[PrintQuality-1] = this->OpenTextFile("Dice",0,0,1,0,0,NULL); break;
          // Later fill in the names of all the other quality measures 
        default : 
          vtkEMAddWarningMessage("PrintQuality of type "<< ((vtkImageEMLocalClass*) ClassList[c])->GetPrintQuality() << " for class " << c << " is unknown => Deactivated PrintQuality for this class"); 
      }
      int ClassIndex = 0; 

      for (int k= 0 ; k < NumClasses ; k++) 
        if (this->ClassListType[k]== CLASS) {
          if (((vtkImageEMLocalClass*) this->ClassList[k])->GetPrintQuality() == PrintQuality) 
        fprintf(QualityFile[PrintQuality-1],"     %2d    ", LabelList[ClassIndex]);
          ClassIndex ++;
        } else {
          ClassIndex += NumChildClasses[k];
        }
      fprintf(QualityFile[PrintQuality-1],"\n"); 
    }
      }
    }
    std::cerr << "End open Quality File" << std::endl;
  }

  // -----------------------------------------------------------
  // Print Converegence measures  
  // -----------------------------------------------------------
  this->LabelMapEMDifferenceAbsolut  =  0;    
  this->LabelMapEMDifferencePercent  =  0.0;    
  this->CurrentEMLabelMap            =  NULL;
  this->LabelMapEMDifferenceFile     =  NULL;

  this->WeightsEMDifferenceAbsolut   =  0.0;    
  this->WeightsEMDifferencePercent   =  0.0;    
  this->CurrentEMWeights             =  NULL;
  this->WeightsEMDifferenceFile      =  NULL;

  this->EMStopFlag                   =  0;

  if (this->PrintEMLabelMapConvergence || this->StopEMType == EMSEGMENT_STOP_LABELMAP) {
     this->CurrentEMLabelMap  = new short[ImageProd];
     if (this->PrintEMLabelMapConvergence) {
       this->LabelMapEMDifferenceFile = this->OpenTextFile("EMLabelMapConvergence",0,0,1,0,0,"EM Label Convergence Parameters will be written to:" );
       fprintf(this->LabelMapEMDifferenceFile, "%% Absolut Percent \n");      
     }
  }

  if (this->PrintEMWeightsConvergence || this->StopEMType == EMSEGMENT_STOP_WEIGHTS) {
    this->CurrentEMWeights  = new float*[NumClasses];
    for (int i=0; i< this->NumClasses; i++) this->CurrentEMWeights[i] = new float[ImageProd];
    if (this->PrintEMWeightsConvergence) {
      WeightsEMDifferenceFile = this->OpenTextFile("EMWeightsConvergence",0,0,1,0,0,"EM Weights Convergence Parameters will be written to:");
      fprintf(this->WeightsEMDifferenceFile, "%% Absolut Percent \n");      
    }
  }

  // Mean Field Specific Parameters
  this->LabelMapMFADifferenceFile     =  NULL;
  this->WeightsMFADifferenceFile      =  NULL;


}

// ==========================================================
// Definition of Shape Model
// ==========================================================
template <class T> int EMLocalAlgorithm<T>::InitializeShape() {
  int SuccessFlag = 1;
  // -----------------------------------------------------------
  //    Variables Neded for Shape Modelling
  // -----------------------------------------------------------
  this->PCAShapeParameters                = new float*[this->NumTotalTypeCLASS];
  this->PCATotalNumOfShapeParameters      = 0;
  this->PCAParameters                     = new double*[this->NumTotalTypeCLASS];

  this->PCAInverseEigenValues             = new float*[this->NumTotalTypeCLASS];
  this->PCAEigenValues                    = new double*[this->NumTotalTypeCLASS];

  this->PCANumberOfEigenModes             = new int[this->NumTotalTypeCLASS];
  this->actSupCl->GetPCANumberOfEigenModes(PCANumberOfEigenModes);

  this->PCAMeanShapeIncY                  = new int[this->NumTotalTypeCLASS];
  this->PCAMeanShapeIncZ                  = new int[this->NumTotalTypeCLASS];
  this->PCAMeanShapePtrStart              = new float*[this->NumTotalTypeCLASS];

  this->PCAEigenVectorsIncY               = new int*[this->NumTotalTypeCLASS];
  this->PCAEigenVectorsIncZ               = new int*[this->NumTotalTypeCLASS];
  this->PCAEigenVectorsPtrStart           = new float**[NumTotalTypeCLASS];

  for (int i = 0; i < this->NumTotalTypeCLASS; i++) {
    if (this->PCANumberOfEigenModes[i]) {
      this->PCAEigenVectorsIncY[i]    = new int[this->PCANumberOfEigenModes[i]];
      this->PCAEigenVectorsIncZ[i]    = new int[this->PCANumberOfEigenModes[i]];
      this->PCAEigenVectorsPtrStart[i] = new float*[PCANumberOfEigenModes[i]];
    } else {
      this->PCAEigenVectorsIncY[i] = NULL;
      this->PCAEigenVectorsIncZ[i] = NULL;
      this->PCAEigenVectorsPtrStart[i] = NULL;
    }
  }

  this->actSupCl->GetPCAParametersPtr((void**) this->PCAMeanShapePtrStart,(void***) this->PCAEigenVectorsPtrStart, 0, !this->RegistrationType);

  this->PCALogisticSlope                 = new float[NumTotalTypeCLASS];
  this->PCALogisticBoundary              = new float[NumTotalTypeCLASS];
  this->PCALogisticMax                   = new float[NumTotalTypeCLASS];
  
  this->PCALogisticMin                   = new float[NumTotalTypeCLASS];
  
  this->PCA_ROI_Start = NULL;
  this->PCAFile = NULL; 

  // Minimum index which has to be considered by PCA analysis;
  this->PCAMin[0] = this->BoundaryMaxX;  this->PCAMin[1] = this->BoundaryMaxY; this->PCAMin[2] = this->BoundaryMaxZ;
  this->PCAMax[0] = this->PCAMax[1] = this->PCAMax[2] = 0;
  
  this->PCAShapeModelType = actSupCl->GetPCAShapeModelType();

  // -----------------------------------------------------------
  //    Transfere Shape Related Data from CLASS structure 
  //    to EMLocalShapeCostFunction Environment
  // -----------------------------------------------------------

  this->actSupCl->GetPCAParameters(this->PCAShapeParameters,this->PCAMeanShapeIncY, this->PCAMeanShapeIncZ, this->PCAEigenVectorsIncY, this->PCAEigenVectorsIncZ, 
                       this->PCAEigenValues,this->PCALogisticSlope,this->PCALogisticBoundary, this->PCALogisticMin, this->PCALogisticMax,0,
                   (this->RegistrationType == EMSEGMENT_REGISTRATION_DISABLED));

  for (int i = 0; i < this->NumTotalTypeCLASS; i++) {
    if (this->PCANumberOfEigenModes[i]) {
      if (this->Alpha) {
        vtkEMAddWarningMessage("Because PCA Shape Parameters are acitvated we will set Alpha to 0!");
        this->Alpha = 0.0;
      } 
      std::cerr << "Class " << i << " has ShapePtr defined" << std::endl;

      if (this->PCAShapeModelType)  {
    this->PCATotalNumOfShapeParameters         +=  this->PCANumberOfEigenModes[i];
      } else {
    this->PCATotalNumOfShapeParameters         =  this->PCANumberOfEigenModes[i];
      }

      this->PCAInverseEigenValues[i] = new float[this->PCANumberOfEigenModes[i]];
      for (int j = 0 ; j < this->PCANumberOfEigenModes[i]; j++) {
    this->PCAShapeParameters[i][j] = 0.0;

    if (this->PCAEigenValues[i][j] < 0.001)  {
      vtkEMAddErrorMessage("vtkImageEMLocalAlgorithm: Eigenvalue "<< j << " is to smalle (" << this->PCAEigenValues[i][j]<<")!");
      exit(1);
    } 
    this->PCAInverseEigenValues[i][j] = 1.0 / this->PCAEigenValues[i][j];
      }

      // -----------------------------------------------------------------------
      // Pointers to EigenVectors and generate MeanShape
      // Voxels x NumberOfEigenVectors
   
#if (0)
      vtkIndent indent; 
      ((vtkImageEMLocalClass*)ClassList[i])->
        PrintPCAParameters(std::cerr,indent);
      std::cerr << indent << "PCANumberOfEigenModes : " 
                << PCANumberOfEigenModes[i] << std::endl;
      std::cerr << indent << "PCAShapeParameters    : ";
      for (int j = 0 ; j < PCANumberOfEigenModes[i]; j++) 
        {
        std::cerr << PCAShapeParameters[i][j] << " " ;
        }
        std::cerr << endl;
        std::cerr << indent << "PCAInverseEigenValues : ";
        for (int j = 0 ; j < PCANumberOfEigenModes[i]; j++) 
          {
          std::cerr << PCAInverseEigenValues[i][j] << " " ;
          }
        std::cerr << std::endl;
#endif
    } else {
      this->PCAInverseEigenValues[i]      = NULL;
      this->PCAMeanShapePtrStart[i]       = NULL;   
      this->PCAEigenVectorsPtrStart[i]    = NULL;
    }
  }

  // -----------------------------------------------------------
  //    Initialize EMLocalShapeCostFunction Environment
  // -----------------------------------------------------------

  // Cannot define it before bc of PCANumberOfEigenModes
  this->ShapeParameters = new EMLocalShapeCostFunction(&this->HierarchicalParameters,this->PCANumberOfEigenModes, this->DisableMultiThreading); 

  if (PCATotalNumOfShapeParameters) {
    int PCAFlag    = 0; 
    if (this->PCAShapeModelType < EMSEGMENT_PCASHAPE_APPLY) { 
      int c = 0 ;
      while ((c < this->NumClasses) && (!PCAFlag)) {
    if ((this->ClassListType[c] == CLASS) && ((vtkImageEMLocalClass*) this->ClassList[c])->GetPrintPCA()) PCAFlag = 1;
    c++;
      }
    }
    
    if (PCAFlag) {
      char FileName[1000];
      sprintf(FileName,"%s/Shape/blub", this->PrintDir);
      if (vtkFileOps::makeDirectoryIfNeeded(FileName) == -1) {
    vtkEMAddErrorMessage( "Could not create the follwoing directory :" << this->PrintDir << "/shape");
    SuccessFlag = 0;
      } 
      if (SuccessFlag) {
    this->PCAFile = new FILE*[this->NumClasses];
    int index = 0;
    for (int c = 0; c < NumClasses; c++) {
      if (this->ClassListType[c] == CLASS) {
        if (((vtkImageEMLocalClass*) this->ClassList[c])->GetPrintPCA() && PCANumberOfEigenModes[index]) 
          this->PCAFile[c] = this->OpenTextFile("Shape/ShapePara",this->CurrentLabelList[c],1,1,0,0,NULL);
        else 
          this->PCAFile[c] = NULL;
        
        index ++;
      } else {
        index += this->NumChildClasses[c];
        this->PCAFile[c] = NULL;
      }
    }
      }
    }

    actSupCl->GetProbImageDataCount(ShapeParameters->GetIncludeSpatialPriorForNormalization(),0);

    this->PCA_ROIExactVoxelCount = -1;
    ShapeParameters->SetROIExactVoxelCount(&this->PCA_ROIExactVoxelCount);

    ShapeParameters->NumberOfTrainingSamples      = this->NumberOfTrainingSamples;
    ShapeParameters->PCAInverseEigenValues        = this->PCAInverseEigenValues;
    ShapeParameters->PCATotalNumOfShapeParameters = this->PCATotalNumOfShapeParameters;
    ShapeParameters->PCAShapeModelType            = this->PCAShapeModelType;
    std::cerr << "Current Shape Modelling Type : ";
    switch (this->PCAShapeModelType) 
      {
      case EMSEGMENT_PCASHAPE_DEPENDENT    : 
        std::cerr << "Dependent"; break;
      case EMSEGMENT_PCASHAPE_INDEPENDENT  : 
        std::cerr << "Independent"; break;
      case EMSEGMENT_PCASHAPE_APPLY        : 
        std::cerr << "Apply"; break;
      default                              :
        std::cerr << "Unknown"; break;
      }
    std::cerr << endl;
  
    ShapeParameters->PCALogisticSlope    = this->PCALogisticSlope ;
    ShapeParameters->PCALogisticBoundary = this->PCALogisticBoundary;
    ShapeParameters->PCALogisticMin      = this->PCALogisticMin;
    ShapeParameters->PCALogisticMax      = this->PCALogisticMax;

    // Variance of spatial prior defined by shape parameters where the spatial prior is defined as 1/(1+e^(- PCASpatialPriorVariance * DistanceMap of Shape))
    ShapeParameters->PCASpatialPriorVariance = 1.0;
    // Just define those voxels that are interesting for shape analysis

    // Needed bc of registration
    ShapeParameters->InitRegSpecPara(this->RegistrationType,this->RealMaxX, this->RealMaxY, this->RealMaxZ);

    this->PCA_ROI_Start = new unsigned char[ImageProd];
    memset(this->PCA_ROI_Start,0,sizeof(unsigned char)*ImageProd); 
  }
  return SuccessFlag;
}

// ==========================================================
// Definition of Registration Model 
// ==========================================================

template <class T> int EMLocalAlgorithm<T>::InitializeRegistration(float initGlobalRegInvRotation[9], float initGlobalRegInvTranslation[3]) {
  int SuccessFlag = 1;
  // -----------------------------------------------------------
  // Registration Parameter Setup 
  // -----------------------------------------------------------
  // define registration values 
  // Note that the parameters are defined for the not fliped images as done so by Simon resampling program 
  // Thus, if you want to see what you get out of here just undo flipping in resampling and you will be in great shape 
  this->GlobalRegInvRotation = initGlobalRegInvRotation; 
  this->GlobalRegInvTranslation =  initGlobalRegInvTranslation;


  this->SuperClassToAtlasRotationMatrix    = NULL; 
  this->SuperClassToAtlasTranslationVector = NULL; 

  this->ClassToAtlasRotationMatrix         = NULL;
  this->ClassToAtlasTranslationVector      = NULL;

  this->ClassToSuperClassRotationMatrix    = NULL;
  this->ClassToSuperClassTranslationVector = NULL;

  // Should each sub class be considered independent in the registration cost functon of not 
  this->RegistrationIndependentSubClassFlag  = NULL;
  //Should we register class when we do class specific registration
  this->RegistrationClassSpecificRegistrationFlag = NULL;
  this->RegistrationTranslation    = NULL;
  this->RegistrationRotation       = NULL;
  this->RegistrationScale          = NULL;
  this->RegistrationParameterFile  = NULL; 

  // We assume atlas and source have same dimension in terms of voxel size and number 
  // also we start at voxel with number 0 not 1 !
  this->targetmidcol   = 0.0;
  this->targetmidrow   = 0.0;
  this->targetmidslice = 0.0;

  this->TwoDFlag = (this->BoundaryMaxZ > 1 ? 0 : 1);
  this->RigidFlag = ((this->RegistrationType > EMSEGMENT_REGISTRATION_DISABLED) && this->PCATotalNumOfShapeParameters) ;

  
  this->RegistrationParameters = new EMLocalRegistrationCostFunction;

  if (this->RegistrationType > EMSEGMENT_REGISTRATION_DISABLED) {
    this->SuperClassToAtlasRotationMatrix    =  new float[9]; 
    this->SuperClassToAtlasTranslationVector =  new float[3]; 

    this->ClassToAtlasRotationMatrix         = new float*[this->NumClasses];
    this->ClassToAtlasTranslationVector      = new float*[this->NumClasses];
    this->ClassToSuperClassRotationMatrix    = new float*[this->NumClasses];
    this->ClassToSuperClassTranslationVector = new float*[this->NumClasses];

    // we caluclate it alredy before just get rid of it an replace with a function that caluclates also if RegistrationParameters is not defined
    this->RegistrationParameters->SetProbDataPtr((void**) this->ProbDataPtrStart);

    this->RegistrationParameters->SetImage_Length(this->RealMaxX, this->RealMaxY, this->RealMaxZ);
    this->targetmidcol   = this->RegistrationParameters->GetImage_MidX();
    this->targetmidrow   = this->RegistrationParameters->GetImage_MidY();
    this->targetmidslice = this->RegistrationParameters->GetImage_MidZ();

    // In the Real image space index starts at 0 and not at 1
    this->RegistrationParameters->SetBoundary(this->SegmentationBoundaryMin[0] -1, this->SegmentationBoundaryMin[1] -1, this->SegmentationBoundaryMin[2] -1, 
                       this->SegmentationBoundaryMax[0] -1, this->SegmentationBoundaryMax[1] -1, this->SegmentationBoundaryMax[2] -1);

    // Registration parameters for cost function - also needed partly for Shape parameter estimation
    // Kilian: Clean up later - create a structure with paramter that both cost functions need - like RegistrationShapeInterface !  
    this->RegistrationParameters->SetInterpolationType(this->RegistrationInterpolationType);
    this->RegistrationParameters->SetRegistrationType(this->RegistrationType);
    this->RegistrationParameters->SetGenerateBackgroundProbability(this->GenerateBackgroundProbability);
    this->RegistrationParameters->SetNumberOfTrainingSamples(this->NumberOfTrainingSamples);
    this->RegistrationParameters->DebugOff(); 
    this->RegistrationParameters->SetEMHierarchyParameters(this->HierarchicalParameters);       
    this->RegistrationParameters->Setweights(this->w_mPtr);
    this->RegistrationParameters->SetBoundary_ROIVector(this->OutputVectorPtr);
    this->RegistrationParameters->SpatialCostFunctionOff(); 
    this->RegistrationParameters->SetBoundary_NumberOfROIVoxels(this->NumROIVoxels);

    if (RegistrationType > EMSEGMENT_REGISTRATION_APPLY) { 
 
       this->RegistrationIndependentSubClassFlag       = new int[this->NumClasses];
       this->RegistrationClassSpecificRegistrationFlag = new int[this->NumClasses];

       int NumParaSets = 0;


       this->actSupCl->GetRegistrationClassSpecificParameterList(this->RegistrationIndependentSubClassFlag, 
                                 this->RegistrationClassSpecificRegistrationFlag, NumParaSets);

       // At least one set of parameters must be defined otherwisse this does not make sense
       assert(NumParaSets);

       this->RegistrationParameters->SetIndependentSubClassFlag(this->RegistrationIndependentSubClassFlag);
       this->RegistrationParameters->SetClassSpecificRegistrationFlag(this->RegistrationClassSpecificRegistrationFlag);
       std::cerr << "Number Of Parametersets " << NumParaSets << std::endl;
       this->RegistrationParameters->SetDimensionOfParameter(NumParaSets,this->TwoDFlag, this->RigidFlag);
 
       if (!this->DefineGlobalAndStructureRegistrationMatrix()) SuccessFlag = 0; 

       this->RegistrationParameters->SetGlobalToAtlasTranslationVector(this->GlobalRegInvTranslation);
       this->RegistrationParameters->SetGlobalToAtlasRotationMatrix(this->GlobalRegInvRotation);
       this->RegistrationParameters->SetSuperClassToAtlasTranslationVector(SuperClassToAtlasTranslationVector);
       this->RegistrationParameters->SetSuperClassToAtlasRotationMatrix(SuperClassToAtlasRotationMatrix);
       std::cerr << "NumberOfVoxels in the region of interest : " << 
         this->RegistrationParameters->GetBoundary_NumberOfROIVoxels() 
                 << std::endl;
    
       this->RegistrationParameters->ClassInvCovariance_Define(ClassListType,ClassList); 
       this->RegistrationParameters->ClassInvCovariance_Print();

       this->DefineForRegistrationRotTranSca(NumParaSets);

       this->RegistrationParameters->MultiThreadDefine(this->DisableMultiThreading);
       this->RegistrationParameters->DefineRegistrationParametersForThreadedCostFunction(SegmentationBoundaryMin[0] -1, SegmentationBoundaryMin[1] -1, SegmentationBoundaryMin[2] -1, SegmentationBoundaryMax[0] -1, SegmentationBoundaryMax[1] -1, SegmentationBoundaryMax[2] -1);
       if (actSupCl->GetPrintFrequency() && 
       (actSupCl->GetPrintRegistrationParameters() || actSupCl->GetPrintRegistrationSimularityMeasure())) {

     // Make Directory if necessary
     char makedirectory[1000];
     sprintf(makedirectory,"%s/Registration/Blub",this->PrintDir);
     
     if (vtkFileOps::makeDirectoryIfNeeded(makedirectory) == -1) {
       vtkEMAddErrorMessage("Could not create the follwoing directory :" << makedirectory);
       SuccessFlag = 0 ;
     } else if (actSupCl->GetPrintRegistrationParameters()) {
     //std::cerr << "Open Registratation ParameterFiles" << std::endl;
       RegistrationParameterFile = new FILE*[NumParaSets];
       if (!this->DefinePrintRegistrationParameters(NumParaSets))  SuccessFlag = 0;
       // std::cerr << "End" << std::endl;
     }
       }
    } else { 
      // We only apply registration and wont optimize over it 
      if (!this->DefineGlobalAndStructureRegistrationMatrix())  SuccessFlag = 0;
    }
    std::cerr << "Registration Applied to Atlas Space:" << std::endl;
    std::cerr << "Global Matrix: "; 
    EMLocalAlgorithm_PrintVector(GlobalRegInvRotation,0,8); 
    EMLocalAlgorithm_PrintVector(GlobalRegInvTranslation,0,2);
    std::cerr << "Head Parameters: " << std::endl;
    EMLocalAlgorithm_PrintVector(actSupCl->GetRegistrationTranslation(),0,2);
    EMLocalAlgorithm_PrintVector(actSupCl->GetRegistrationRotation(),0,2);
    EMLocalAlgorithm_PrintVector(actSupCl->GetRegistrationScale(),0,2);
  }
 
  // -----------------------------------------------------------
  //       Joint EMLocalShapeCostFunction and EMLocalRegistrationCostFunction 
  //       Environment  
  // -----------------------------------------------------------

  // If necessary baisically create probability maps out of the shape prior so that for the registration approach there is now difference so that there is no difference
  if (PCATotalNumOfShapeParameters && (RegistrationType > EMSEGMENT_REGISTRATION_DISABLED)) 
    {
    std::cerr << "Joint Shape Analysis and Registration" << std::endl;
    ShapeParameters->ClassToAtlasRotationMatrix    =  ClassToAtlasRotationMatrix;
    ShapeParameters->ClassToAtlasTranslationVector =  ClassToAtlasTranslationVector;

    int Real_LengthXYZ             = this->RealMaxX*this->RealMaxY*this->RealMaxZ;
 

    for (int i = 0; i < NumTotalTypeCLASS; i++) 
      {
      if (PCANumberOfEigenModes[i]) 
        {
        if (this->ProbDataPtrStart[i]) 
          {
          std::cerr << "Warning: I am not useing spatial prior for class " 
                    << i << " because shape priors are defined " << std::endl;
          }
        
        this->ProbDataPtrStart[i] = new T[Real_LengthXYZ];
        // std::cerr << " Real_LengthXYZ " << Real_LengthXYZ << std::endl;
        this->ProbDataIncY[i] = ProbDataIncZ[i] = 0;
        
        // define values of spatial prior
        this->Transfere_ShapePara_Into_SpatialPrior(i); 
        }
      }

    // Currently shape model cannot handle background probability
    assert(!this->GenerateBackgroundProbability);
    // This is just for speed reasons - can be easily changed 
    for (int i = 0 ; i <  this->NumTotalTypeCLASS; i++) {
      if (this->PCAMeanShapePtrStart[i]) {
    assert(!this->PCAMeanShapeIncY[i] && !this->PCAMeanShapeIncZ[i]);
    for (int j = 0 ; j < this->PCANumberOfEigenModes[i]; j ++) assert(!this->PCAEigenVectorsIncY[i][j] && !this->PCAEigenVectorsIncZ[i][j]); 
      }
    }
  }   

  if (( this->RegistrationType ==  EMSEGMENT_REGISTRATION_GLOBAL_ONLY) || (this->RegistrationType ==  EMSEGMENT_REGISTRATION_SEQUENTIAL) )  {
    // Region of interest
    this->RegistrationParameters->SetROI_Weight(&this->Registration_ROI_Weight);
    Registration_ROI_Weight.CreateMAP(this->ImageProd);

    // Initialize it once than we can just later use it all the time 
    this->RegistrationParameters->SetROI_ProbData(&Registration_ROI_ProbData);
    EMLocalRegistrationCostFunction_DefineROI_ProbDataValues(RegistrationParameters,this->ProbDataPtrStart); 
    //std::cerr << "Min " << Registration_ROI_ProbData.MinCoord[0] << " " << Registration_ROI_ProbData.MinCoord[1] << " "<< Registration_ROI_ProbData.MinCoord[2] << std::endl;
    //std::cerr << "Max " << Registration_ROI_ProbData.MaxCoord[0] << " " << Registration_ROI_ProbData.MaxCoord[1] << " "<< Registration_ROI_ProbData.MaxCoord[2] << std::endl;
  }
  return SuccessFlag;
}


template <class T> void EMLocalAlgorithm<T>::InitializeEStepMultiThreader(int DataType) {
  this->E_Step_Threader_SelfPointer.self = (void*) this;
  this->E_Step_Threader_SelfPointer.DataType = DataType;

  // Initialize Multithreading
  this->E_Step_Threader_Number = EMLocalInterface_GetDefaultNumberOfThreads(this->DisableMultiThreading);

  // std::cerr << "Threader Number: " << this->E_Step_Threader_Number << std::endl;

  this->E_Step_Threader = vtkMultiThreader::New();
  this->E_Step_Threader->SetNumberOfThreads(this->E_Step_Threader_Number);
  this->E_Step_Threader->SetSingleMethod(EMLocalAlgorithm_E_Step_Threader_Function,((void*) &(this->E_Step_Threader_SelfPointer)));

  this->E_Step_Threader_Parameters = new EMLocalAlgorithm_E_Step_MultiThreaded_Parameters[this->E_Step_Threader_Number];

  int JobSize = this->ImageProd / this->E_Step_Threader_Number;
  int VoxelOffset = 0;
  int VoxelLeftOver;
  for (int i= 0; i < this->E_Step_Threader_Number; i++) 
    {
    // std::cerr << "Thread " << i <<  std::endl;
    int *VoxelStart = this->E_Step_Threader_Parameters[i].VoxelStart;
    VoxelStart[2] = VoxelOffset/this->imgXY;
    VoxelLeftOver = VoxelOffset % this->imgXY;
    VoxelStart[1] = VoxelLeftOver / this->BoundaryMaxX;
    VoxelStart[0] = VoxelLeftOver % this->BoundaryMaxX;
 
     if (i < this->E_Step_Threader_Number -1) this->E_Step_Threader_Parameters[i].NumberOfVoxels = JobSize;
     else this->E_Step_Threader_Parameters[i].NumberOfVoxels = JobSize +  this->ImageProd % this->E_Step_Threader_Number;

     this->E_Step_Threader_Parameters[i].DataJump = EMLocalInterface_DefineMultiThreadJump(VoxelStart,this->BoundaryMaxX,this->BoundaryMaxY,0,0); 

     this->E_Step_Threader_Parameters[i].Registration_ROI_Weight.MAP = this->Registration_ROI_Weight.MAP;
     if (this->E_Step_Threader_Parameters[i].Registration_ROI_Weight.MAP) 
       this->E_Step_Threader_Parameters[i].Registration_ROI_Weight.MAP += this->E_Step_Threader_Parameters[i].DataJump; 

     this->E_Step_Threader_Parameters[i].ProbDataJump      = new int[NumTotalTypeCLASS];
     memset(this->E_Step_Threader_Parameters[i].ProbDataJump,0,sizeof(int)*this->NumTotalTypeCLASS);

     this->E_Step_Threader_Parameters[i].PCAMeanShapeJump   = new int[NumTotalTypeCLASS];
     memset(this->E_Step_Threader_Parameters[i].PCAMeanShapeJump,0,sizeof(int)*this->NumTotalTypeCLASS);

     this->E_Step_Threader_Parameters[i].PCAEigenVectorsJump = new int*[NumTotalTypeCLASS]; 

     //std::cerr << i << " Image: " <<  this->ImageProd << " Job : " << this->E_Step_Threader_Parameters[i].NumberOfVoxels << " VoxelStart :" <<  VoxelStart[0] << " " 
     //      <<  VoxelStart[1] << " " << VoxelStart[2] << " DataJump " << this->E_Step_Threader_Parameters[i].DataJump << " ProbDataJump: " <<  endl; 

     for (int j= 0 ; j < this->NumTotalTypeCLASS; j++) {
       this->E_Step_Threader_Parameters[i].PCAEigenVectorsJump[j] = new int[this->PCANumberOfEigenModes[j]];
       memset(this->E_Step_Threader_Parameters[i].PCAEigenVectorsJump[j],0,sizeof(int)*this->PCANumberOfEigenModes[j]);

       if (this->RegistrationType == EMSEGMENT_REGISTRATION_DISABLED) {
     if (this->ProbDataPtrStart[j]) {
       this->E_Step_Threader_Parameters[i].ProbDataJump[j] = EMLocalInterface_DefineMultiThreadJump(VoxelStart,this->BoundaryMaxX,this->BoundaryMaxY,
                                                    this->ProbDataIncY[j], this->ProbDataIncZ[j]);
     }
     if (this->PCAMeanShapePtrStart[j]) 
       this->E_Step_Threader_Parameters[i].PCAMeanShapeJump[j] = EMLocalInterface_DefineMultiThreadJump(VoxelStart, this->BoundaryMaxX,this->BoundaryMaxY, 
                                                      this->PCAMeanShapeIncY[j], this->PCAMeanShapeIncZ[j]);
     for (int k= 0 ; k < this->PCANumberOfEigenModes[j]; k++) 
       this->E_Step_Threader_Parameters[i].PCAEigenVectorsJump[j][k] =  EMLocalInterface_DefineMultiThreadJump(VoxelStart,this->BoundaryMaxX,this->BoundaryMaxY,
                                                         this->PCAEigenVectorsIncY[j][k],
                                                         this->PCAEigenVectorsIncZ[j][k]);
       }
     }
     VoxelOffset += JobSize;
  }
  // This is actually not necessary for this->E_Step_Threader_Number == 1. 
  // However, we produce very different results using 1 cpu and multi cpu machines. 
  // In the 1 cpu machine the updated weights are getting used in MF calculations 
  // where in Multi CPU the methods uses the weights from the last iteration
  if (this->Alpha > 0.0) {
    // We have to create a copy of w_m as MF read and writes to w_m simultaneously 
    this->w_mCopy   = new float*[this->NumTotalTypeCLASS];
    for (int i = 0; i < this->NumTotalTypeCLASS; i++) this->w_mCopy[i] = new float[this->ImageProd];
  } else {
    this->w_mCopy = NULL;
  }
  this->w_m_inputPtr = this->w_m_outputPtr = this->w_mPtr;

}

