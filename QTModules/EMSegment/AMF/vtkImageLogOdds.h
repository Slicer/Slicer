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
// .NAME vtkImageLogOdds
// The filter computers the transform between log odds and probabilities 
#ifndef __vtkImageLogOdds_h
#define __vtkImageLogOdds_h 
  
#include <vtkEMSegment.h> 
#include "vtkImageMultipleInputFilter.h"
#include <vtkstd/vector> 

// Modes of operating this filter
// Probabilities are turned into LogOdds
#define LOGODDS_PROB2LOG 1 
// Log Odds are turned into probabilities
#define LOGODDS_LOG2PROB 2 

// Log Odds are first mapped into the probabilistic space via the generalized logistic function 
// afterwards they mapped back into the log odds space using the log odds function for binary distributions
// => 0.5 in probability space is mapped back to zero level set in log odds 
#define LOGODDS_LOGNORM  3 

// Turn Log Odds map into a label map by first turning them into probabilities and then performing a max 
#define LOGODDS_LOG2MAP  4 


#define LOGODDS_INSIDE_POSTIVE 1
#define LOGODDS_INSIDE_NEGATIVE -1
 
typedef std::vector<int> labelListType;

class VTK_EMSEGMENT_EXPORT vtkImageLogOdds : public vtkImageMultipleInputFilter
{
  public:
  // -----------------------------------------------------
  // Genral Functions for the filter
  // -----------------------------------------------------
  static vtkImageLogOdds *New();
  vtkTypeMacro(vtkImageLogOdds,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) { };

  // Set the Dimension of the Probability Space
  void SetDimProbSpace(int init);

  // Only use when input is defined through probabilities (Mode = Prob2Log)
  void SetProbabilities(int index, vtkImageData *image);
  // Only use when input is defined through LogOdds (Mode = Log2Prob, LorNorm, Log2Map)
  void SetLogOdds(int index, vtkImageData *image);

  // See earlier explanations about different modes 
  void SetMode_Log2Prob();
  void SetMode_Prob2Log();
  void SetMode_LogNorm();
  void SetMode_Log2Map();

  // Anything voxel with MaxProb MinProb is assigned in the label map to the background ! 
  void SetMapMinProb(float init);

  vtkGetMacro(Mode,int);

  // Only use when output are probabilities (Mode = Log2Prob)
  vtkImageData* GetProbabilities(int index);
  // Only use when output are LogOdds (Mode = LogNorm or Prob2Log)
  vtkImageData* GetLogOdds(int index);

  // Only use when output is a Map (Mode = Log2Map) 
  vtkImageData* GetMap();

  // Only used when mapping LogOdds to Probabilities (LogNorm and Log2Prob): 
  // Defines the steepenes of the slope with alpha > 1 steepens the slope in the probabilitistic space 
  // and alpha < 1 more slowely.  
  vtkSetMacro(Alpha,float);
  vtkGetMacro(Alpha,float);

  // In level set formulation negative values define the inside of an object 
  // where my distance maps define inside by positive values 
  void SetLogOddsInsidePositive() {this->LogOddsType = LOGODDS_INSIDE_POSTIVE;};
  void SetLogOddsInsideNegative() {this->LogOddsType = LOGODDS_INSIDE_NEGATIVE;};


  void SetLabelList(labelListType initList) {this->LabelList = initList;}
 
protected:
  vtkImageLogOdds();
  vtkImageLogOdds(const vtkImageLogOdds&) {};
  ~vtkImageLogOdds(); 
 
  int CheckInput(vtkImageData *InData);

  void InitializeOutputs();

  // Noy a valid function anymore 
  // vtkImageData** GetOutputs() {return (vtkImageData **) this->Outputs;}

  vtkImageData* GetOutput(int index);

  // Disable this function
  vtkImageData* GetOutput() {return NULL;}
 
  void operator=(const vtkImageLogOdds&) {};
  void ExecuteData(vtkDataObject *);   

  void ThreadedExecute(vtkImageData **inData, vtkImageData *outData,int outExt[6], int id);

  // -------------------------------
  // Core Functions
  //  LogOdds -> Probs

  void GeneralizedLogistic(float **inptr, float **outptr); 
  //  Prob -> LogOdds
  void MultiNomialLogOdds(float **inptr, float **outptr);
  // Binary case
  void LogOdds(float *probPtr, float *logPtr);
  // LogOdds -> LogOdds : LogOdds -> Prob via GeneralizedLogistic() and Prob-> LogOdds via LogOdds()
  void LogOddsNorm(float **inptr, float **outptr);

  // LogOdds -> Map : label of LogOdds map is shifted by one (e.g. input 0 => label 1) BG is assigned to 0 !
   void LogOddsMap(float **inptr, short **outptr);

//BTX
  std::vector<vtkImageData*> results;
//ETX

  // -------------------------------
  // Dimension of probability space
  int DimProbSpace;
  int DimOutput;
  int DimInput;
  int Mode;

  // To check consisency
  int XDim;
  int YDim;
  int ZDim;
  int InputScalarType;
  float epsilon;
  float omega;

  float MapMinProb;
  float Alpha;
  int LogOddsType;
  //BTX
  
  labelListType LabelList;
  //ETX
};
#endif











