#ifndef __vtkMRMLPETCTFusionNode_h
#define __vtkMRMLPETCTFusionNode_h

#include <list>

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkPETCTFusion.h"

class vtkImageData;

class VTK_PETCTFUSION_EXPORT vtkMRMLPETCTFusionNode : public vtkMRMLNode
{
  public:
 
  static vtkMRMLPETCTFusionNode *New();
  vtkTypeMacro(vtkMRMLPETCTFusionNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

//BTX
  typedef struct SUVEntry {
    int Label;
    double Max;
    double Mean;
  } SUVEntry;

  std::list <SUVEntry> LabelResults;
//ETX


  // Description:
  // Save PETCTFusion result to text file
  virtual void SaveResultToTextFile( const char *fileName );
 
  // Description:
  // Create instance of a PETCTFusion node.
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "PETCTFusionParameters";};

 
  // Description:
  // Get/Set input grayscale volume MRML Id
  vtkGetStringMacro(InputCTReference);
  vtkSetStringMacro(InputCTReference);
  
  // Description:
  // Get/Set input labelmap volume MRML Id

  vtkGetStringMacro(InputPETReference);
  vtkSetStringMacro(InputPETReference);

  vtkGetStringMacro (InputMask );
  vtkSetStringMacro (InputMask );

  vtkGetStringMacro (MessageText );
  vtkSetStringMacro (MessageText );
  
  vtkGetStringMacro (PETLUT);
  vtkSetStringMacro (PETLUT);
  
  vtkGetStringMacro (CTLUT);
  vtkSetStringMacro (CTLUT);
  
  vtkGetStringMacro (Layout);
  vtkSetStringMacro (Layout);

  vtkGetStringMacro ( WeightUnits );
  vtkSetStringMacro ( WeightUnits );

  vtkGetStringMacro ( DoseRadioactivityUnits );
  vtkSetStringMacro ( DoseRadioactivityUnits );

  vtkGetStringMacro ( TissueRadioactivityUnits );
  vtkSetStringMacro ( TissueRadioactivityUnits );
  
  vtkGetStringMacro ( VolumeUnits );
  vtkSetStringMacro ( VolumeUnits );
  
  vtkGetMacro ( VolumeRendering, int );
  vtkSetMacro ( VolumeRendering, int );

  vtkGetMacro ( NumberOfTemporalPositions, int );
  vtkSetMacro ( NumberOfTemporalPositions, int );

  vtkGetMacro ( InjectedDose, double );
  vtkSetMacro ( InjectedDose, double );
  
  vtkGetMacro ( PatientWeight, double );
  vtkSetMacro ( PatientWeight, double );

  vtkGetStringMacro ( PatientName );
  vtkSetStringMacro ( PatientName );
  
  vtkGetStringMacro ( StudyDate );
  vtkSetStringMacro ( StudyDate );
  
  vtkGetStringMacro ( RadiopharmaceuticalStartTime );
  vtkSetStringMacro ( RadiopharmaceuticalStartTime );

  vtkGetStringMacro (DecayCorrection);
  vtkSetStringMacro (DecayCorrection);
  
  vtkGetStringMacro (DecayFactor );
  vtkSetStringMacro (DecayFactor );
    
  vtkGetStringMacro (FrameReferenceTime );
  vtkSetStringMacro (FrameReferenceTime );

  vtkGetStringMacro (RadionuclideHalfLife );
  vtkSetStringMacro (RadionuclideHalfLife );

  vtkGetStringMacro (SeriesTime );
  vtkSetStringMacro (SeriesTime );
  
  vtkGetStringMacro (PhilipsSUVFactor );
  vtkSetStringMacro (PhilipsSUVFactor );
  
  vtkGetStringMacro (CalibrationFactor );
  vtkSetStringMacro (CalibrationFactor );
  
  vtkGetMacro ( SUVmax_t1, float );
  vtkSetMacro ( SUVmax_t1, float );
  vtkGetMacro ( SUVmax_t2, float );
  vtkSetMacro ( SUVmax_t2, float );

  vtkGetMacro ( SUVmin_t1, float );
  vtkSetMacro ( SUVmin_t1, float );
  vtkGetMacro ( SUVmin_t2, float );
  vtkSetMacro ( SUVmin_t2, float );

  vtkGetMacro ( SUVmean_t1, float );
  vtkSetMacro ( SUVmean_t1, float );
  vtkGetMacro ( SUVmean_t2, float );
  vtkSetMacro ( SUVmean_t2, float );

  vtkGetMacro ( SUVmaxmean_t1, float );
  vtkSetMacro ( SUVmaxmean_t1, float );
  vtkGetMacro ( SUVmaxmean_t2, float );
  vtkSetMacro ( SUVmaxmean_t2, float );

  vtkGetMacro ( SUV_t1, float )
  vtkSetMacro ( SUV_t1, float )
    
  vtkGetMacro ( SUV_t2, float )
  vtkSetMacro ( SUV_t2, float )
    
  vtkGetMacro ( CTRangeMin, double);
  vtkSetMacro ( CTRangeMin, double);

  vtkGetMacro ( CTRangeMax, double);
  vtkSetMacro ( CTRangeMax, double);

  vtkGetMacro ( CTMin, double);
  vtkSetMacro ( CTMin, double);

  vtkGetMacro ( CTMax, double);
  vtkSetMacro ( CTMax, double);
  
  vtkGetMacro ( ColorRangeMin, double );
  vtkSetMacro ( ColorRangeMin, double );  

  vtkGetMacro ( ColorRangeMax, double );
  vtkSetMacro ( ColorRangeMax, double );
  
  vtkGetMacro ( PETMin, double );
  vtkSetMacro ( PETMin, double );

  vtkGetMacro ( PETMax, double );
  vtkSetMacro ( PETMax, double );

  vtkGetMacro ( PETSUVmax, double );
  vtkSetMacro ( PETSUVmax, double );
  
  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);
  virtual void SetColorRange ( double min, double max );
  virtual void SetCTRange ( double min, double max );

//BTX
  enum {
      ErrorEvent = 11000,
      ComputeDoneEvent,
      StartUpdatingDisplayEvent,
      UpdatePETDisplayEvent,
      UpdateCTDisplayEvent,
      DICOMUpdateEvent,
      NonDICOMEvent,
      WaitEvent,
      StatusEvent,
      PlotReadyEvent,
  };
//ETX

 protected:

   vtkMRMLPETCTFusionNode();
  ~vtkMRMLPETCTFusionNode();
  vtkMRMLPETCTFusionNode(const vtkMRMLPETCTFusionNode&);
  void operator=(const vtkMRMLPETCTFusionNode&);

  char* InputCTReference;
  char* InputPETReference;
  char *InputMask;
  char* PETLUT;
  char* CTLUT;
  char* MessageText;
  char* Layout;
  int VolumeRendering;

  //--- MAIN PET PARAMS for simple computation
  int NumberOfTemporalPositions;
  double PatientWeight;
  double InjectedDose;

  //--- other PET params of interest
  //--- Can have values: { kg g lb }
  char* WeightUnits;

  char* PatientName;
  char* StudyDate;

  //--- Can have values: { MBq kBq Bq mBq uBq MCi kCi Ci mCi uCi }
  char* DoseRadioactivityUnits;
  char* TissueRadioactivityUnits;

  //--- Can have values: { ml }
  char* VolumeUnits;
  char *RadiopharmaceuticalStartTime;
  char *DecayCorrection;
  char *DecayFactor;
  char *FrameReferenceTime;
  char *RadionuclideHalfLife;
  char *SeriesTime;
  //--- Philips images with custom tag
  char *PhilipsSUVFactor;
  char *CalibrationFactor;

  //--- result holders
  float SUV_t1;
  float SUV_t2;
  float SUVmax_t1;
  float SUVmin_t1;
  float SUVmean_t1;
  float SUVmaxmean_t1;
  float SUVmax_t2;
  float SUVmin_t2;
  float SUVmean_t2;
  float SUVmaxmean_t2;

  double ColorRangeMin;
  double ColorRangeMax;
  double PETMin;
  double PETMax;
  double PETSUVmax;

  double CTRangeMin;
  double CTRangeMax;
  double CTMin;
  double CTMax;
};

#endif

