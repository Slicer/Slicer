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
//------------------------------------------------------------------------------

template <class T>
void EMLocalAlgorithm<T>::DetermineLabelMap(short* LabelMap) { 
#if (0) 
    std::cerr << "EMLocalAlgorithm<T>::DetermineLabelMap LabelMap " << LabelMap << " NumTotalTypeCLASS " << this->NumTotalTypeCLASS << " NumChildClasses  " 
         << this->NumChildClasses << " head " << this->actSupCl << " ROI " << this->ROIPtr << " ImageProd " << this->ImageProd  << " w_m  " << this->w_mPtr << endl;
#endif

  short HeadLabel          = this->actSupCl->GetLabel();
  int    idx,l,k,MaxProbIndex, ClassIndex;
  float  MaxProbValue, temp;

  float **w_m = new float*[NumTotalTypeCLASS];
  for (l=0;l< NumTotalTypeCLASS;l++) w_m[l] = this->w_mPtr[l];

  short* ROI = this->ROIPtr;
  for (idx = 0; idx < this->ImageProd ; idx++) {
    if ((ROI == NULL) || (*ROI++ == HeadLabel)) {
      MaxProbValue = -1;
      MaxProbIndex = 0;
      ClassIndex = 0;
      for (l=0; l< this->NumClasses; l++) {
    temp = 0;
        // std::cerr << " | ";
    for (k=0; k < NumChildClasses[l]; k++) {
      temp += *w_m[ClassIndex];
          // std::cerr << *w_m[ClassIndex] << " " ;
          w_m[ClassIndex]++;
          ClassIndex ++;
    }
        // std::cerr << endl;
    if (!(temp == temp)) {
      std::cerr << ":Error: EMLocalAlgorithm::DetermineLabelMap: in index "<< idx << " Produced a nan " << endl;
      exit(1);
    }
          if ( temp > MaxProbValue) {MaxProbValue = temp; MaxProbIndex = l;}
      }  
      *LabelMap++ = this->CurrentLabelList[MaxProbIndex];
    } else {
      *LabelMap++ = 0;
      for (l=0;l< NumTotalTypeCLASS;l++) w_m[l] ++;
    } 
  }


  delete[] w_m;
 
  //   if (DebugImage) std::cerr << " ---------------------------- End of Error index ------------------" << endl;
}

// -----------------------------------------------------------
// Difference measure calculation for iteration purposes 
// -----------------------------------------------------------
template <class T>
void EMLocalAlgorithm<T>::DifferenceMeassure(int StopType, int PrintLabelMapConvergence, int PrintWeightsConvergence, int iter, short *CurrentLabelMap, float** w_m, 
                           int &LabelMapDifferenceAbsolut, float &LabelMapDifferencePercent, float **CurrentWeights, 
                           float &WeightsDifferenceAbsolut, float &WeightsDifferencePercent, float StopValue, int &StopFlag) {

  if (StopType ==  EMSEGMENT_STOP_LABELMAP || PrintLabelMapConvergence) {
    short* LastLabelMap = NULL;
    if (iter >1) {
      LastLabelMap = new short[this->ImageProd];   
      memcpy(LastLabelMap,CurrentLabelMap,sizeof(short) * this->ImageProd);
    }
     this->DetermineLabelMap(CurrentLabelMap); 
    // Leave out the first time - just initialize values 
    if (iter > 1) {
      LabelMapDifferenceAbsolut = 0;
      for (int i= 0 ; i < this->ImageProd; i++) {
    if (LastLabelMap[i] != CurrentLabelMap[i]) LabelMapDifferenceAbsolut ++;
      }  
      if (this->NumROIVoxels) LabelMapDifferencePercent = float(LabelMapDifferenceAbsolut) / float(this->NumROIVoxels); 
      else LabelMapDifferencePercent = 0.0;
      
      delete[] LastLabelMap;
      std::cerr << "LabelMapDifferenceAbsolut: " << LabelMapDifferenceAbsolut << " LabelMapDifferencePercent: " << LabelMapDifferencePercent << endl;
      
    }  else {
      LabelMapDifferenceAbsolut = -1;      
      LabelMapDifferencePercent = 2.0;
    }
  }
  if (StopType == EMSEGMENT_STOP_WEIGHTS || PrintWeightsConvergence) {
    float** LastWeights = NULL;
    if (iter > 1) {
      LastWeights  = new float*[this->NumClasses];
      for (int i=0; i<this->NumClasses; i++) {
    LastWeights[i] = new float[this->ImageProd];
    memcpy(LastWeights[i],CurrentWeights[i],sizeof(float)*this->ImageProd);
      }
    }
    for (int i=0; i<this->NumClasses; i++) memset(CurrentWeights[i],0, sizeof(float)*this->ImageProd);
    WeightsDifferenceAbsolut = 0.0;
    float diff; 
    int index =0;
    for (int j = 0 ; j < this->NumClasses; j++) { 
      for (int k = 0 ; k < this->NumChildClasses[j]; k++) {
    for (int i = 0 ; i < ImageProd; i++) CurrentWeights[j][i] += w_m[index][i]; 
    index ++;
      }
      if (iter > 1) {
    for (int i= 0 ; i < ImageProd; i++) {           
      diff = LastWeights[j][i] - CurrentWeights[j][i];
      WeightsDifferenceAbsolut  += diff * diff;
    }
      }
    }
      
    if (iter > 1) {
      WeightsDifferenceAbsolut  = sqrt(WeightsDifferenceAbsolut);
      if (this->NumROIVoxels) WeightsDifferencePercent = float(WeightsDifferenceAbsolut) / float(this->NumROIVoxels); 
      else WeightsDifferencePercent = 0.0;
      delete[] LastWeights; 
      std::cerr << "WeightsDifferenceAbsolut: " << WeightsDifferenceAbsolut << " WeightsDifferencePercent: " << WeightsDifferencePercent << endl;
    } else {
      WeightsDifferenceAbsolut = -1.0;      
      WeightsDifferencePercent  = 2.0;
    }
  }
  if (iter > 1) {
    if ((StopType == EMSEGMENT_STOP_WEIGHTS) && (WeightsDifferencePercent  <= StopValue)) StopFlag = 1;
    if ((StopType == EMSEGMENT_STOP_LABELMAP) && (LabelMapDifferencePercent <= StopValue)) StopFlag = 1;
  }                      
}


 
