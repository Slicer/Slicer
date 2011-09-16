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
// .NAME vtkImageMultiLevelSets
// The filter computers the transform between log odds and probabilities 
#ifndef __vtkImageMultiLevelSets_h
#define __vtkImageMultiLevelSets_h 
  
#include <vtkEMSegment.h> 
#include "vtkImageMultipleInputFilter.h"
#include "vtkImageLevelSets.h" 
#include "assert.h"


class VTK_EMSEGMENT_EXPORT vtkImageMultiLevelSets : public vtkImageMultipleInputFilter
{
  public:
  // -----------------------------------------------------
  // Genral Functions for the filter
  // -----------------------------------------------------
  static vtkImageMultiLevelSets *New();
  vtkTypeMacro(vtkImageMultiLevelSets,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetprobCondWeightMin(float init){ this->probCondWeightMin = init;}
  // Inside is greater 0
  void SetLogCondIntensityInsideBright() { this->logCondIntensityInside= -1; }
  // Inside is smaller 0 
  void SetLogCondIntensityInsideDark()   { this->logCondIntensityInside= 1; }
  void SetNumberOfCurves(int initNumCurves);


  void SetCurve(int initID, vtkImageLevelSets  *initWithoutLogOdds, vtkImageData  *initInitialCurve, 
        vtkImageData *initLogCondIntImage, float initLogCondIntCoeff, float initLogCurveCouplingCoeff, vtkImageData  *initFinalCurve);

  int  InitParam();
  int  InitEvolution();
  int  Iterate();
  void EndEvolution();

  void SetLogCondTerm(int ID, vtkImageData* data) {this->CurvesOutputJustLogOdds[ID]   = (float*)data->GetScalarPointer();}

//BTX
  // For Threading 
  void DefineThread(int& first, int& last, const int id, const int total);
  void ComputeLogOddsComponent(const int first, const int last);
  void ComputeLogOddsComponent_IPMICorrect(const int first, const int last);

  enum {
    IPMI_ORIG = 0,
    IPMI_CORRECT,
    JOURNAL
  };
//ETX
  void ComputeLogOddsComponent_journal(const int first, const int last) ;
 
  void SetMultiLevelVersion(int init) {this->MultiLevelVersion = init; }
  int  GetMultiLevelVersion() { return this->MultiLevelVersion; }

protected:
  vtkImageMultiLevelSets();
  vtkImageMultiLevelSets(const vtkImageMultiLevelSets&) {};
  ~vtkImageMultiLevelSets(); 
 
  int CheckInput(vtkImageData *InData);

  // These are disabled ! to fit in structure of vtkImageLevelSets
  vtkImageData** GetOutputs() {return NULL;} 

  // Disable this function
  vtkImageData* GetOutput() {return NULL;}
 
  void operator=(const vtkImageMultiLevelSets&) {};
 
  // Do not execute right now
  void ExecuteData(vtkDataObject *);   

  void ThreadedExecute(vtkImageData **inData, vtkImageData *outData,int outExt[6], int id) {assert(0);} 

  // -------------------------------
  // Dimension of probability space
  int NumberOfCurves; 
  // To check consisency
  int XDim;
  int YDim;
  int ZDim;
  int ImgDim;

  // Level Set Parameters 
  float *logCondIntensityCoefficient;
  float *logCurveCouplingCoefficient;
  vtkImageLevelSets **Curves;
  vtkImageData **CurvesOutputWithoutLogOdds;
  float **CurvesOutputJustLogOdds;
  vtkImageData **CurvesOutputComplete;

  float probCondWeightMin;
  float logCondIntensityInside;

  vtkMultiThreader* Threader;

  int MultiLevelVersion;
};
#endif











