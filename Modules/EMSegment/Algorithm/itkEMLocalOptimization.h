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

#ifndef _ITKEMLOCALREOPTIMIZATION_H_INCLUDED
#define _ITKEMLOCALREOPTIMIZATION_H_INCLUDED 1

#include "vtkEMSegment.h"
#include "itkSingleValuedCostFunction.h"
#include "EMLocalRegistrationCostFunction.h"
#include "EMLocalShapeCostFunction.h"
#include "itkPowellOptimizer.h"

namespace itk
{
/** \class EMLocalCostFunctionWrapper
 * \brief Optimizes alignment between atlas and segmentation space 
 *
 * This Class is templated over the type of the fixed and moving
 * images to be compared.
 *
 * This metric measures the KL Divergence between the atlas (moving image) and 
 * the weights (fixed image) computed by the segmentation algorithm. The spatial 
 * correspondance between both images is established through a Transform. Pixel values are
 * taken from the Moving image. Their positions are mapped to the Fixed image
 * and result in general in affine position on it. 
 *
 * \ingroup RegistrationMetrics
 */
class VTK_EMSEGMENT_EXPORT  EMLocalCostFunctionWrapper :  public SingleValuedCostFunction
{
public:

  /** Standard class typedefs. */
  typedef EMLocalCostFunctionWrapper    Self;
  typedef SingleValuedCostFunction  Superclass;

  typedef SmartPointer<Self>         Pointer;
  typedef SmartPointer<const Self>   ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);
 
  /** Run-time type information (and related methods). */
  itkTypeMacro(EMLocalCostFunctionWrapper, SingleValuedCostFunction);

  /**  ParametersType typedef.
   *  It defines a position in the optimization search space. */
  typedef Superclass::ParametersType      ParametersType;
  typedef Superclass::DerivativeType      DerivativeType;
  typedef Superclass::MeasureType         MeasureType ;


  /** Get the derivatives of the match measure. */
  void GetDerivative( const ParametersType & parameters,
                      DerivativeType & derivative ) const
  {
     itkExceptionMacro( "GetDerivative not supported!" );
  }

  /**  Get the value for single valued optimizers. */
  MeasureType GetValue( const ParametersType & parameters ) const
  {
    itkDebugMacro("GetValue( " << parameters << " ) ");
    const double* para_double = parameters.data_block();

    if (this->m_Registration) return this->m_Registration->ComputeCostFunction(para_double);
    if (this->m_Shape) return this->m_Shape->ComputeCostFunction(para_double);
    itkExceptionMacro( "Neither registration nor shape cost function is set!" );
  }

  /**  Get value and derivatives for multiple valued optimizers. */
  void GetValueAndDerivative( const ParametersType & parameters,
                              MeasureType& Value, DerivativeType& Derivative ) const
  {
    itkExceptionMacro( "GetValueAndDerivative not supported!" );
  }

  unsigned int GetNumberOfParameters(void) const {
    if (m_Registration) return  this->m_Registration->GetNumberOfTotalParameters(); 
    if (m_Shape) return  this->m_Shape->GetPCATotalNumOfShapeParameters(); 
    itkExceptionMacro( "Neither registration nor shape cost function is set!" );
  }
  
  void SetRegistrationCostFunction(EMLocalRegistrationCostFunction* init) {assert(!this->m_Shape); this->m_Registration = init;} 
  void SetShapeCostFunction(EMLocalShapeCostFunction* init) {assert(!this->m_Registration); this->m_Shape = init;} 

protected:
  EMLocalCostFunctionWrapper() { m_Registration = NULL; m_Shape = NULL;}; 
  virtual ~EMLocalCostFunctionWrapper() { };
  EMLocalRegistrationCostFunction* m_Registration;
  EMLocalShapeCostFunction* m_Shape;

private:
  EMLocalCostFunctionWrapper(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};

} // end namespace itk

/* Run optimzation */
// Seperate from itk function bc it is only a wrapper around 
void  itkEMLocalOptimization_Registration_Start(EMLocalRegistrationCostFunction* RegCostFunction, double* Parameters, float &Cost) {
  std::cerr << "==================== Start Registration =========================== " << endl;

  RegCostFunction->InitializeCostFunction();
  int NumOfFunctionEvaluations;

  {
     // EMLocalRegistration::Pointer em = EMLocalRegistration::New();
     itk::EMLocalCostFunctionWrapper::Pointer itkRegCostFunction = itk::EMLocalCostFunctionWrapper::New(); 
     itkRegCostFunction->SetRegistrationCostFunction(RegCostFunction);
     int NumPara     =  itkRegCostFunction->GetNumberOfParameters(); 

     itk::PowellOptimizer::Pointer optimizer = itk::PowellOptimizer::New();
     optimizer->SetCostFunction(itkRegCostFunction);
     optimizer->SetMaximize( false );

     // From NR 
     optimizer->SetStepLength( 1.0 );
     optimizer->SetStepTolerance(float(2.0e-4) );
     optimizer->SetValueTolerance( 0.01 );
     optimizer->SetMaximumIteration( 200 );
     optimizer->SetMaximumLineIteration( 100 );

     typedef itk::EMLocalCostFunctionWrapper::ParametersType ParametersType;

     ParametersType InitialPara(NumPara); 
     memcpy(InitialPara.data_block(),Parameters,sizeof(double) *  NumPara);
    
     optimizer->SetInitialPosition(InitialPara);

     try 
     {
       optimizer->StartOptimization();
     }
     catch( itk::ExceptionObject & e )
     {
       std::cerr << "Exception thrown ! " << std::endl;
       std::cerr << "An error ocurred during Optimization" << std::endl;
       std::cerr << "Location    = " << e.GetLocation()    << std::endl;
       std::cerr << "Description = " << e.GetDescription() << std::endl;
       return;
     }

     NumOfFunctionEvaluations = optimizer->GetCurrentIteration();

     ParametersType finalPosition = optimizer->GetCurrentPosition();
     memcpy(Parameters,finalPosition.data_block(), sizeof(double) *  NumPara);
  }
  RegCostFunction->FinalizeCostFunction(Parameters, NumOfFunctionEvaluations);
  std::cerr << "==================== End Registration =========================== " << endl;
}


void  itkEMLocalOptimization_Shape_Start(EMLocalShapeCostFunction* ShapeCostFunction, float **PCAShapeParameters, int PCAMaxX, int PCAMinX, 
                       int PCAMaxY, int PCAMinY, int PCAMaxZ, int PCAMinZ, int BoundaryMinX, int BoundaryMinY, int BoundaryMinZ, 
                       int Boundary_LengthX, int Boundary_LengthY, float** w_m, unsigned char* PCA_ROI, void  **initProbDataPtr, 
                       float** initPCAMeanShapePtr, int* initPCAMeanShapeIncY, int *initPCAMeanShapeIncZ, float*** initPCAEigenVectorsPtr, 
                       int **initPCAEigenVectorsIncY,  int** initPCAEigenVectorsIncZ, float& Cost) {
  std::cerr << "==================== Start Shape Deformation  =========================== " << endl;
  std::cerr << "Implementation: ITK" << endl;

  ShapeCostFunction->InitializeCostFunction(PCAMaxX, PCAMinX, PCAMaxY, PCAMinY, PCAMaxZ, PCAMinZ, BoundaryMinX, BoundaryMinY, BoundaryMinZ,
                      Boundary_LengthX, Boundary_LengthY, w_m, PCA_ROI, initProbDataPtr, initPCAMeanShapePtr, initPCAMeanShapeIncY, 
                      initPCAMeanShapeIncZ, initPCAEigenVectorsPtr, initPCAEigenVectorsIncY, initPCAEigenVectorsIncZ);

  itk::EMLocalCostFunctionWrapper::Pointer itkShapeCostFunction = itk::EMLocalCostFunctionWrapper::New(); 
  itkShapeCostFunction->SetShapeCostFunction(ShapeCostFunction);
  int NumPara     =  itkShapeCostFunction->GetNumberOfParameters(); 
    
  itk::PowellOptimizer::Pointer optimizer = itk::PowellOptimizer::New();
  optimizer->SetCostFunction(itkShapeCostFunction);
  optimizer->SetMaximize( false );

  // From NR 
  optimizer->SetStepLength( 1.0 );
  optimizer->SetStepTolerance(float(2.0e-4) );
  optimizer->SetValueTolerance( 0.01 );
  optimizer->SetMaximumIteration( 200 );
  optimizer->SetMaximumLineIteration( 100 );

  typedef itk::EMLocalCostFunctionWrapper::ParametersType ParametersType;
  
  // Transfere Variables
  float* PCAParameters  = new float[NumPara]; 
  ShapeCostFunction->TransferePCAShapeParametersIntoArray(PCAShapeParameters,PCAParameters); 

  ParametersType InitialPara(NumPara); 
  double *InitialPara_db = InitialPara.data_block();
  for (int i = 0 ; i < NumPara; i++) InitialPara_db[i] = (double) PCAParameters[i]; 
  optimizer->SetInitialPosition(InitialPara);

  // run Algorithm,
  try 
    {
      optimizer->StartOptimization();
    }
  catch( itk::ExceptionObject & e )
    {
      std::cerr << "Exception thrown ! " << std::endl;
      std::cerr << "An error ocurred during Optimization" << std::endl;
      std::cerr << "Location    = " << e.GetLocation()    << std::endl;
      std::cerr << "Description = " << e.GetDescription() << std::endl;
      return;
    }

  Cost = float(optimizer->GetCurrentCost()); 
  std::cerr << "Number of Evaluations :" << optimizer->GetCurrentIteration() << endl;

  
  const double* finalPosition_db = optimizer->GetCurrentPosition().data_block();
  for (int i = 0 ; i < NumPara; i++) PCAParameters[i] = float(finalPosition_db[i]);

  ShapeCostFunction->TransfereArrayIntoPCAShapeParameters(PCAParameters,PCAShapeParameters); 

  delete[] PCAParameters;

  std::cerr << "==================== End Shape Deformation =========================== " << endl;
}


#endif
