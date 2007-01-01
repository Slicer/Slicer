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

// Sets up and defines the parameter for the MF Approximation in the E-Step
template <class T> void EMLocalAlgorithm<T>::RegularizeWeightsWithMeanField(int iter) {

  int   LabelMapMFADifferenceAbsolut  =  0;    
  float LabelMapMFADifferencePercent  =  0.0; 

  float WeightsMFADifferenceAbsolut   =  0.0;    
  float WeightsMFADifferencePercent   =  0.0;    
  int   MFAStopFlag                   =  0;
  int   regiter                       =  0;

  if (this->PrintMFALabelMapConvergence) {
    this->LabelMapMFADifferenceFile = this->OpenTextFile("MFALabelMapConvergence",0,0,1,iter,1,"MFA LabelMap Convergence parameters will be written to: " );
    fprintf(this->LabelMapMFADifferenceFile, "%% Absolut Percent \n");      
  }
  
  if (this->PrintMFAWeightsConvergence) {
    WeightsMFADifferenceFile = this->OpenTextFile("MFAWeightsConvergence",0,0,1,iter,1, "MFA LabelMap Convergence parameters will be written to: ");
    fprintf(this->WeightsMFADifferenceFile, "%% Absolut Percent \n");      
  }

  //--------------------------------------------------------------------
  // Mean Field / Regularization
  //--------------------------------------------------------------------
  for (;;) {
    // -----------------------------------------------------------
    // Initialization
    regiter ++;
    cout << "EMLocalAlgorithm: "<< regiter << ". EM - MF Iteration" << endl;

    this->w_m_inputPtr  = (regiter%2 ? w_mPtr  : w_mCopy);
    this->w_m_outputPtr = (regiter%2 ? w_mCopy : w_mPtr);

    // -----------------------------------------------------------
    // Calculation of MF
    this->E_Step_ExecuteMultiThread(); 
 
    // -----------------------------------------------------------
    // Determine and Print Convergence Factor
    if (StopMFAType || PrintMFALabelMapConvergence || PrintMFAWeightsConvergence) { 
      this->DifferenceMeassure(StopMFAType, PrintMFALabelMapConvergence, PrintMFAWeightsConvergence, 2,CurrentMFALabelMap,w_m_outputPtr, 
                   LabelMapMFADifferenceAbsolut, LabelMapMFADifferencePercent,CurrentMFAWeights, WeightsMFADifferenceAbsolut, 
                   WeightsMFADifferencePercent, this->actSupCl->GetStopMFAValue(),MFAStopFlag);
      if (WeightsMFADifferenceFile)  
    fprintf(WeightsMFADifferenceFile, "%f %f \n", WeightsMFADifferenceAbsolut,  WeightsMFADifferencePercent);
      if (LabelMapMFADifferenceFile) 
    fprintf(LabelMapMFADifferenceFile,"%d %f \n", LabelMapMFADifferenceAbsolut, LabelMapMFADifferencePercent);
    }

    // -----------------------------------------------------------
    // Check Stopping condition
    if (regiter >= this->NumRegIter || MFAStopFlag) break;
  }
  // -----------------------------------------------------------
  // Clean up
  // -----------------------------------------------------------
  if (LabelMapMFADifferenceFile != NULL) {
    fprintf(LabelMapMFADifferenceFile,"%% Number Of Iterations: %d \n", regiter);
    fprintf(LabelMapMFADifferenceFile,"%% Maximum Iteration Border: %d \n", NumRegIter);
    fflush(LabelMapMFADifferenceFile);
    fclose(LabelMapMFADifferenceFile);   
  }
  
  if (WeightsMFADifferenceFile != NULL) {
    fprintf(WeightsMFADifferenceFile,"%% Number Of Iterations: %d \n", regiter);
    fprintf(WeightsMFADifferenceFile,"%% Maximum Iteration Border: %d \n", NumRegIter);
    fflush(WeightsMFADifferenceFile);
    fclose(WeightsMFADifferenceFile);   
  }

  if ((NumRegIter%2 && !MFAStopFlag) || (regiter%2 && MFAStopFlag) ) {
    assert(w_mCopy);
    for (int j=0; j < this->NumTotalTypeCLASS; j++) memcpy(this->w_mPtr[j],w_mCopy[j],sizeof(float)*this->ImageProd);
  } 
}


  // -----------------------------------------------------------
  // neighbouring field values
  // -----------------------------------------------------------
  // The different Matrisses are defined as follow:
  // Matrix[i] = Neighbour in a certain direction
  // In Tcl TK defined in this order: West North Up East South Down 
  // this->MRFParams[i] :  
  // i = 3 pixel and east neighbour
  // i = 0 pixel and west neighbour
  // i = 4 pixel and south neighbour
  // i = 1 pixel and north neighbour
  // i = 2 pixel and previous neighbour (up)
  // i = 5 pixel and next neighbour (down)

template  <class T> inline double EMLocalAlgorithm<T>::NeighberhoodEnergy(float **w_m_input, unsigned char MapVector, int CurrentClass) {

  if (MapVector&EMSEGMENT_NOTROI) return 1.0;

  int  JumpHorizontal  = this->BoundaryMaxX;
  int  JumpSlice       = this->imgXY;
  

  float wxp, wxn, wyn, wyp, wzn, wzp;
  wxp = wxn= wyn= wyp= wzn = wzp=0;
  int ClassIndex = 0;

  // -----------------------------------------------------------
  // Calculate Neighbouring Tissue Relationships
  // -----------------------------------------------------------  
  if (MapVector) {
    for (int k=0;k< this->NumClasses ;k++){
      for (int l=0;l< this->NumChildClasses[k];l++){
    // f(j,l,h-1)
    if (MapVector&EMSEGMENT_WEST)   wxn += (*w_m_input[ClassIndex])*(float)this->MRFParams[3][k][CurrentClass]; 
    else                            wxn += w_m_input[ClassIndex][-JumpHorizontal]*(float) this->MRFParams[3][k][CurrentClass];
    // f(j,l,h+1)
    if (MapVector&EMSEGMENT_EAST)   wxp += (*w_m_input[ClassIndex])*(float)this->MRFParams[0][k][CurrentClass];
    else                            wxp += w_m_input[ClassIndex][JumpHorizontal]*(float)this->MRFParams[0][k][CurrentClass];
    //  Remember: The picture is upside down:
    // Therefore I had to switch the MRF parameters 1 (South) and 4(North)
    // f(j,l-1,h)
    if (MapVector&EMSEGMENT_NORTH)  wyn += (*w_m_input[ClassIndex])*(float)this->MRFParams[1][k][CurrentClass];                       
    else                            wyn += w_m_input[ClassIndex][-1]*(float)this->MRFParams[1][k][CurrentClass]; 
    // f(j,l+1,h)
    if (MapVector&EMSEGMENT_SOUTH)  wyp += (*w_m_input[ClassIndex])*(float)this->MRFParams[4][k][CurrentClass];
    else                            wyp += w_m_input[ClassIndex][1]*(float)this->MRFParams[4][k][CurrentClass];
    // f(j-1,l,h)
    if (MapVector&EMSEGMENT_FIRST) wzn += (*w_m_input[ClassIndex])*(float)this->MRFParams[5][k][CurrentClass];  
    else                            wzn += w_m_input[ClassIndex][-JumpSlice]*(float)this->MRFParams[5][k][CurrentClass]; 
    // f(j+1,l,h)
    if (MapVector&EMSEGMENT_LAST)  wzp += (*w_m_input[ClassIndex])*(float)this->MRFParams[2][k][CurrentClass]; 
    else                            wzp += w_m_input[ClassIndex][JumpSlice]*(float)this->MRFParams[2][k][CurrentClass]; 

    ClassIndex ++;
      }
    }
  } else {
    for (int k=0;k < this->NumClasses ;k++){
      for (int l=0;l< this->NumChildClasses[k];l++){
    // f(j,l,h-1)
    wxn += w_m_input[ClassIndex][-JumpHorizontal]*(float)this->MRFParams[3][k][CurrentClass];
    // f(j,l,h+1)
    wxp += w_m_input[ClassIndex][JumpHorizontal]*(float)this->MRFParams[0][k][CurrentClass];
    // f(j,l-1,h)
    wyn += w_m_input[ClassIndex][-1]*(float)this->MRFParams[4][k][CurrentClass]; 
    // f(j,l+1,h)
    wyp += w_m_input[ClassIndex][1]*(float)this->MRFParams[1][k][CurrentClass];
    // f(j-1,l,h)
    wzn += w_m_input[ClassIndex][-JumpSlice]*(float)this->MRFParams[5][k][CurrentClass]; 
    // f(j+1,l,h)
    wzp += w_m_input[ClassIndex][JumpSlice]*(float)this->MRFParams[2][k][CurrentClass]; 
    ClassIndex ++;
      }
    }
  }

  // Kilian: March 06
  //  Old Definition: double(1-this->Alpha+this->Alpha*exp(wxp + wxn + wyp + wyn + wzp + wzn));
  //  Problem:  The  NeighborhoodEnergy and condIntensity  are not properly scalled so that the method is difficult to fine tune 
  //            Thus changing alpha from 0 to 0.01 has a great effect but the difference between alpha 0.01 and 1.0 is very small 
  // 
  //  Solution: Define the influence of the components based in a parameter similar to SpatialTissueDistribution depends on ProbDataSpatialWeight (PDSW)
  //            where the STD of a class (not subclass) \in [PDSW, 1] * TrainingSamples. 
  //            TrainingSamples is a scalar ascross structures so it does not have an impact on the segmentation results.
  // 
  //            - NeighborhoodEnergy: Maximum is e^(6)  => in order for alpha to have any meanining set it to 1 - alpha  + alpha * (e^w -1) / (e^6 -1)
  //                                  => NE(alpha) \in [\alpha , 1] 
  //                                  => alpha = 1.0: NE = (e^w -1) so that the product solely on the neighboorhod energy 
  //                                                  ((e^6 -1) is a structure independent scalar - ignore it)                         
  //                                     alpha = 0.5 the NE \in [0.5 , 1] => impact of neighborhood relationship is reduced  
  //                                     alpha = 0.0 then NE = 1 and is ignored from w_m
 
  float Energy = exp(wxp + wxn + wyp + wyn + wzp + wzn);
  return double(1-this->Alpha+this->Alpha*( Energy - 1) * EMSEGMENT_INVERSE_NEIGHBORHOOD_ENERGY); 
}

