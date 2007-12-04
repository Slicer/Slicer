#include "itkStochasticTractographyFilter.h"
#include "vnl/vnl_math.h"
#include "vnl/vnl_matrix_fixed.h"
#include "vnl/vnl_vector_fixed.h"
#include "vnl/vnl_matrix.h"
#include "vnl/vnl_sym_matrix.h"
#include "vnl/vnl_vector.h"
#include "vnl/vnl_diag_matrix.h"
#include "vnl/algo/vnl_qr.h"
//#include "vnl/algo/vnl_svd.h"
#include "vnl/algo/vnl_matrix_inverse.h"
//#include "vnl/algo/vnl_symmetric_eigensystem.h"
#include "itkSymmetricEigenAnalysis.h"
#include "vnl/vnl_transpose.h"
#include "itkVariableSizeMatrix.h"
#include "itkPathIterator.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"

namespace itk{

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::StochasticTractographyFilter():
  m_TotalTracts(0),m_MaxTractLength(0),m_Gradients(NULL), m_TransformedGradients(NULL),m_bValues(NULL),
  m_SampleDirections(NULL), m_A(NULL), m_Aqr(NULL), m_LikelihoodCachePtr(NULL),
  m_MaxLikelihoodCacheSize(0), m_CurrentLikelihoodCacheElements(0),
  m_ClockPtr(NULL), m_TotalDelegatedTracts(0), m_OutputTractContainer(NULL){
  this->m_SeedIndex[0]=0;
  this->m_SeedIndex[1]=0;
  this->m_SeedIndex[2]=0;
  this->m_MeasurementFrame.set_identity();
  this->SetNumberOfRequiredInputs(2); //Filter needs a DWI image and a Mask Image
  
  
  m_ClockPtr = RealTimeClock::New();
  this->m_RandomGenerator.reseed( ((unsigned long) this->m_ClockPtr->GetTimeStamp()) );
  //load in default sample directions
  this->LoadDefaultSampleDirections();
} 

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::~StochasticTractographyFilter(){
  delete this->m_A;
  delete this->m_Aqr;
} 

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::ProbabilisticallyInterpolate( vnl_random& randomgenerator, 
  const TractType::ContinuousIndexType& cindex,
  typename InputDWIImageType::IndexType& index){
                      
  for(int i=0; i<3; i++){
    if ((vcl_ceil(cindex[i]+vnl_math::eps)-cindex[i]) < randomgenerator.drand64())
       index[i]=(int)vcl_ceil(cindex[i]);
     else index[i]=(int)vcl_floor(cindex[i]);
  }
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::UpdateGradientDirections(void){
  //the gradient direction is transformed into IJK space
  //by moving into the image space and then to IJK space
  
  this->m_TransformedGradients = GradientDirectionContainerType::New();
  unsigned int N = this->m_Gradients->Size();
  for(unsigned int i=0; i<N; i++){
    GradientDirectionContainerType::Element g_i = 
      this->m_MeasurementFrame *
      this->m_Gradients->GetElement(i);
    
    /** The correction to LPS space is not neccessary as of itk 3.2 **/
    //g_i[0] = -g_i[0];
    //g_i[1] = -g_i[1];
    g_i = this->GetInput()->GetDirection().GetInverse() * g_i;  
    this->m_TransformedGradients->InsertElement(i, g_i);
  }
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::UpdateTensorModelFittingMatrices( void ){
  //std::cout<<"UpdateTensorFittingMatrix\n";
  //estimate the parameters using linear LS estimation
  //using convention specified by Salvador
  //solve for Beta in: logPhi=X*Beta
  //number of rows of the matrix depends on the number of inputs,
  //i.e. the number of measurements of the voxel (n)
  unsigned int N = this->m_TransformedGradients->Size();
  
  if(this->m_A!=NULL)
    delete this->m_A;
  this->m_A = new vnl_matrix< double >(N, 7); //potential memory leak here
  vnl_matrix< double >& A = *(this->m_A);
  
  for(unsigned int j=0; j< N ; j++){
    GradientDirectionContainerType::Element g = m_TransformedGradients->GetElement(j);
    const bValueType&  b_i = m_bValues->GetElement(j);
    
    A(j,0)=1.0;
    A(j,1)=-1*b_i*(g[0]*g[0]);
    A(j,2)=-1*b_i*(g[1]*g[1]);
    A(j,3)=-1*b_i*(g[2]*g[2]);
    A(j,4)=-1*b_i*(2*g[0]*g[1]);
    A(j,5)=-1*b_i*(2*g[0]*g[2]);
    A(j,6)=-1*b_i*(2*g[1]*g[2]);
  }
  
  //Store a QR decomposition to quickly estimate
  //the weighing matrix for each voxel
  if(this->m_Aqr!=NULL)
    delete this->m_Aqr;
  this->m_Aqr = new vnl_qr< double >(A);  //potential memory leak here
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::CalculateTensorModelParameters( const DWIVectorImageType::PixelType& dwivalues,
  vnl_diag_matrix<double>& W,
  TensorModelParamType& tensormodelparams){
  
  unsigned  int N = this->m_TransformedGradients->Size();
  
  //setup const references for code clarity
  const vnl_matrix< double >& A = *(this->m_A);
  const vnl_qr< double >& Aqr = *(this->m_Aqr);
  
  //vnl_vector is used because the itk vector is limited in its methods and does not
  //contain an internal vnl class like VariableSizematrix
  //also itk_matrix has methods which are compatible with vnl_vectors
  vnl_vector< double > logPhi( N );
  
  for(unsigned int j=0; j< N ; j++){
    //fill up the logPhi vector using log(dwi) values
    logPhi.put(j, vcl_log(static_cast<double>(dwivalues[j]) + vnl_math::eps));
  }
  
  /** Find WLS estimate of the parameters of the Tensor model **/
  
  // First estimate W by LS estimation of the intensities
  //vnl_matrix< double > Q = Aqr.Q();
  //vnl_vector< double > QtB = Aqr.Q().transpose()*logPhi;
  //vnl_vector< double > QTB = Aqr.QtB(logPhi);
  //vnl_matrix< double > R = Aqr.R(); 
  W = A* vnl_qr< double >(Aqr.R()).solve(Aqr.QtB(logPhi));
  //W = A * Aqr.solve(logPhi);  
  for(vnl_diag_matrix< double >::iterator i = W.begin();i!=W.end(); i++){
    *i = vcl_exp( *i );
  }
  
  // Now solve for parameters using the estimated weighing matrix
  tensormodelparams = vnl_qr< double >((W*A).transpose()*W*A).solve(
    (W*A).transpose()*W*logPhi);
  int a;
  //tensormodelparams = vnl_qr< double >((W*A)).solve(W*logPhi);
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::CalculateConstrainedModelParameters( const TensorModelParamType& tensormodelparams,
    ConstrainedModelParamType& constrainedmodelparams){          
              
  vnl_sym_matrix< double > D( 3, 0 );
  double alpha =0;
  double beta=0;
  //set the tensor model parameters into a Diffusion tensor
  D(0,0) = tensormodelparams[1];
  D(0,1) = tensormodelparams[4];
  D(0,2) = tensormodelparams[5];
  D(1,0) = tensormodelparams[4];
  D(1,1) = tensormodelparams[2];
  D(1,2) = tensormodelparams[6];
  D(2,0) = tensormodelparams[5];
  D(2,1) = tensormodelparams[6];
  D(2,2) = tensormodelparams[3];
  
  //pass through the no gradient intensity Z_0 and
  //calculate alpha, beta and v hat (the eigenvector 
  //associated with the largest eigenvalue)
  vnl_matrix_fixed< double, 3, 3 > S(0.0);
  vnl_vector_fixed< double, 3 > Lambda(0.0);
  SymmetricEigenAnalysis< vnl_sym_matrix< double >,
    vnl_vector_fixed< double, 3 >, vnl_matrix_fixed< double, 3, 3 > >
    eigensystem( 3 );
  eigensystem.ComputeEigenValuesAndVectors( D, Lambda, S );
  
  //need to take abs to get rid of negative eigenvalues
  alpha = (vcl_abs(Lambda[0]) + vcl_abs(Lambda[1])) / 2;
  beta = vcl_abs(Lambda[2]) - alpha;
  
  constrainedmodelparams[0] = tensormodelparams[0];
  constrainedmodelparams[1] = alpha;
  constrainedmodelparams[2] = beta;
  constrainedmodelparams[3] = S[2][0];
  constrainedmodelparams[4] = S[2][1];
  constrainedmodelparams[5] = S[2][2];                  
}
              
template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::CalculateNoiseFreeDWIFromConstrainedModel( const ConstrainedModelParamType& constrainedmodelparams,
    DWIVectorImageType::PixelType& noisefreedwi){
    
  unsigned int N = this->m_TransformedGradients->Size();
  const double& z_0 = constrainedmodelparams[0];
  const double& alpha = constrainedmodelparams[1];
  const double& beta = constrainedmodelparams[2];
  TractOrientationContainerType::Element v_hat( constrainedmodelparams[3],
    constrainedmodelparams[4],
    constrainedmodelparams[5]);
  
  for(unsigned int i=0; i < N ; i++ ){
    const double& b_i = this->m_bValues->GetElement(i);
    const GradientDirectionContainerType::Element& g_i = 
      this->m_TransformedGradients->GetElement(i);
    
    noisefreedwi.SetElement(i,
      vcl_exp(z_0-(alpha*b_i+beta*b_i*vnl_math_sqr(dot_product(g_i, v_hat)))));
  }     
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::CalculateResidualVariance( const DWIVectorImageType::PixelType& noisydwi,
    const DWIVectorImageType::PixelType& noisefreedwi,
    const vnl_diag_matrix< double >& W,
    const unsigned int numberofparameters,
    double& residualvariance){
    
  unsigned int N = this->m_TransformedGradients->Size();
  
  residualvariance=0;
  
  /** Not sure if we should be taking difference of log or nonlog intensities **/
  /** residual variance is too low if we take the difference of log intensities **/
  /** perhaps using WLS will correct this problem **/
  for(unsigned int i=0; i<N; i++)
    residualvariance+=vnl_math_sqr(W(i,i) * (vcl_log(noisydwi[i]/noisefreedwi[i])));
  residualvariance/=(N-numberofparameters);
}
                                 
template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::CalculateLikelihood( const DWIVectorImageType::PixelType &dwipixel, 
    TractOrientationContainerType::ConstPointer orientations,
    ProbabilityDistributionImageType::PixelType& likelihood){
    
  unsigned int N = this->m_TransformedGradients->Size();
  TensorModelParamType tensorparams( 0.0 );
  vnl_diag_matrix< double > W(N,0);
  ConstrainedModelParamType constrainedparams( 0.0 );
  DWIVectorImageType::PixelType noisefreedwi(N);
  double residualvariance=0;
  double jointlikelihood=1;
  
  CalculateTensorModelParameters( dwipixel, W, tensorparams );
  CalculateConstrainedModelParameters( tensorparams, constrainedparams );
  CalculateNoiseFreeDWIFromConstrainedModel( constrainedparams, noisefreedwi );
  CalculateResidualVariance( dwipixel, noisefreedwi, W, 6, residualvariance );
  
  for(unsigned int i=0; i < orientations->Size(); i++){
    /** Vary the entry corresponding to the estimated
      Tract orientation over the selected sample directions,
      while preserving the best estimate for the other parameters **/
    TractOrientationContainerType::Element currentdir = orientations->GetElement(i);
    
    /** Incorporate the current sample direction into the secondary parameters **/
    constrainedparams[3]=currentdir[0];
    constrainedparams[4]=currentdir[1];
    constrainedparams[5]=currentdir[2];
    
    /** Obtain the estimated
      intensity for this choice of Tract direction **/
    CalculateNoiseFreeDWIFromConstrainedModel(constrainedparams, noisefreedwi);
    
    jointlikelihood = 1.0;
    for(unsigned int j=0; j<N; j++){
      /** Calculate the likelihood given the residualvariance,
        estimated intensity and the actual intensity (refer to Friman) **/
      jointlikelihood *= 
        (noisefreedwi[j]/vcl_sqrt(2*vnl_math::pi*residualvariance))*
          vcl_exp(-vnl_math_sqr(noisefreedwi[j]*vcl_log(dwipixel[j]/noisefreedwi[j]))/
                (2*residualvariance));
    }
    likelihood[i]=jointlikelihood;  
  }
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::CalculatePrior( TractOrientationContainerType::Element v_prev, 
    TractOrientationContainerType::ConstPointer orientations,
    ProbabilityDistributionImageType::PixelType& prior ){
    
  const double gamma = 1;
          
  for(unsigned int i=0; i < orientations->Size(); i++){
    if(v_prev.squared_magnitude()==0){
      prior[i]=1.0;
    }
    else{
      prior[i] = dot_product(orientations->GetElement(i),v_prev);;
      if(prior[i]<0){
        prior[i]=0;
      }
      else{
        prior[i]=vcl_pow(prior[i],gamma);
      }
    }
  }
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::CalculatePosterior( const ProbabilityDistributionImageType::PixelType& likelihood,
    const ProbabilityDistributionImageType::PixelType& prior,
    ProbabilityDistributionImageType::PixelType& posterior){
    
    double sum=0;
    for(unsigned int i=0; i<likelihood.Size(); i++){
      sum+=likelihood[i]*prior[i];
    }
    for(unsigned int i=0; i<likelihood.Size(); i++){
      posterior[i] = (likelihood[i]*prior[i])/sum;
    }
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::SampleTractOrientation( vnl_random& randomgenerator, 
    const ProbabilityDistributionImageType::PixelType& posterior,
    TractOrientationContainerType::ConstPointer orientations,
    TractOrientationContainerType::Element& choosendirection ){
    
      double randomnum = randomgenerator.drand64();
      int i=0;
      double cumsum=0;
      
      //will crash in the unlikely case that 0 was choosen as the randomnum
      while(cumsum < randomnum){
        cumsum+=posterior[i];
        i++;
      }
      choosendirection = orientations->GetElement(i-1);
      
      //std::cout<< "cumsum: " << cumsum<<std::endl;
      //std::cout<<"selected orientation:( " << (i-1) 
      //  <<") "<<choosendirection<< std::endl;
}
template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
bool
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::FiberExistenceTest( vnl_random& randomgenerator,
  typename InputWhiteMatterProbabilityImageType::ConstPointer wmpimage,
  typename InputWhiteMatterProbabilityImageType::IndexType index ){
  double randomnum = randomgenerator.drand64();
  if( randomnum < wmpimage->GetPixel( index ) )
    return true;
  else
    return false;
}
//the seedindex is in continuous IJK coordinates
template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::StochasticTractGeneration( typename InputDWIImageType::ConstPointer dwiimagePtr,
  typename InputWhiteMatterProbabilityImageType::ConstPointer wmpimagePtr,
  typename InputDWIImageType::IndexType seedindex,
  unsigned long randomseed,
  TractType::Pointer tract){
  
  TractType::ContinuousIndexType cindex_curr = seedindex;
  typename InputDWIImageType::IndexType index_curr = {{0,0,0}};
  ProbabilityDistributionImageType::PixelType 
      prior_curr(this->m_SampleDirections->Size()); 
  ProbabilityDistributionImageType::PixelType 
      posterior_curr(this->m_SampleDirections->Size());
  TractOrientationContainerType::Element v_curr(0,0,0);
  TractOrientationContainerType::Element v_prev(0,0,0);
  
  tract->Initialize();
  vnl_random randomgenerator(randomseed);
  //std::cout<<randomseed<<std::endl;
  
  for(unsigned int j=0; j<this->m_MaxTractLength; j++){
    this->ProbabilisticallyInterpolate( randomgenerator, cindex_curr, index_curr );
    
    if(!dwiimagePtr->GetLargestPossibleRegion().IsInside(index_curr)){
      break;
    }
    
    if( FiberExistenceTest( randomgenerator, wmpimagePtr, index_curr ) ){
      tract->AddVertex(cindex_curr);
      
      this->CalculatePrior( v_prev, this->m_SampleDirections, prior_curr);
      
      const ProbabilityDistributionImageType::PixelType&
        cachelikelihood_curr = this->AccessLikelihoodCache(index_curr);
                              
      if( cachelikelihood_curr.GetSize() != 0){
        //use the cached direction
        this->CalculatePosterior( cachelikelihood_curr, prior_curr, posterior_curr);
      }
      else{
        //do the likelihood calculation and discard
        //std::cout<<"Cache Miss!\n";
        ProbabilityDistributionImageType::PixelType 
          likelihood_curr_temp(this->m_SampleDirections->Size());

        this->CalculateLikelihood(static_cast< DWIVectorImageType::PixelType >(
          dwiimagePtr->GetPixel(index_curr)),
          this->m_SampleDirections,
          likelihood_curr_temp);
        this->CalculatePosterior( likelihood_curr_temp, prior_curr, posterior_curr);
      }
      this->SampleTractOrientation(randomgenerator, posterior_curr,
        this->m_SampleDirections, v_curr);
      
      //takes into account voxels of different sizes
      //converts from a step length of 1 mm to the corresponding length in IJK space
      const typename InputDWIImageType::SpacingType& spacing = dwiimagePtr->GetSpacing();
      cindex_curr[0]+=v_curr[0]/spacing[0];
      cindex_curr[1]+=v_curr[1]/spacing[1];
      cindex_curr[2]+=v_curr[2]/spacing[2];
      v_prev=v_curr;
    }
    else{
      //fiber doesn't exist in this voxel
      //std::cout<<"Stopped Tracking: No Fiber in this Voxel\n";
      break;
    }
  }
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::GenerateTractContainerOutput(){
  //allocate tractcontainer
  this->m_OutputTractContainer = TractContainerType::New();
  
  this->UpdateGradientDirections();
  this->UpdateTensorModelFittingMatrices();
  this->m_TotalDelegatedTracts = 0;
  
  //calculate the number of voxels to cache from Megabyte memory size limit
  ProbabilityDistributionImageType::PixelType 
    element(this->GetSampleDirections()->Size());
  unsigned long elementsize = sizeof(ProbabilityDistributionImageType::PixelType) + 
    sizeof(double)*element.Size();
  this->m_MaxLikelihoodCacheElements = 
    (this->m_MaxLikelihoodCacheSize*1048576)/elementsize;
  std::cout << "MaxLikelhoodCacheElements: "
    << this->m_MaxLikelihoodCacheElements
    << std::endl;

  //setup the multithreader
  StochasticTractGenerationCallbackStruct data;
  data.Filter = this;
  this->GetMultiThreader()->SetSingleMethod( StochasticTractGenerationCallback,
    &data );
  this->GetMultiThreader()->SetNumberOfThreads(this->GetNumberOfThreads());
  std::cout<<"Number of Threads: " << this->GetMultiThreader()->GetNumberOfThreads() << std::endl; 
  //start the multithreaded execution
  this->GetMultiThreader()->SingleMethodExecute();
  std::cout<< "CurrentLikelihoodCacheElements: " << 
    this->m_CurrentLikelihoodCacheElements << std::endl; 
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::GenerateData(){
  //Generate the tracts
  this->GenerateTractContainerOutput();
  
  //allocate outputs
  this->AllocateOutputs();
  
  //write tracts to output image
  this->TractContainerToConnectivityMap(this->m_OutputTractContainer);

}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
ITK_THREAD_RETURN_TYPE 
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::StochasticTractGenerationCallback( void *arg )
{
  StochasticTractGenerationCallbackStruct* str=
    (StochasticTractGenerationCallbackStruct *)
      (((MultiThreader::ThreadInfoStruct *)(arg))->UserData);

  typename InputDWIImageType::ConstPointer inputDWIImagePtr = str->Filter->GetInput();
  typename InputWhiteMatterProbabilityImageType::ConstPointer inputWMPImage = 
    str->Filter->GetWhiteMatterProbabilityImageInput();

  unsigned long randomseed=0;
  
  while(str->Filter->DelegateTract(randomseed)){
    //std::cout<<randomseed<<std::endl;
    //generate the tract
    TractType::Pointer tract = TractType::New();
    
    str->Filter->StochasticTractGeneration( inputDWIImagePtr,
      inputWMPImage,
      str->Filter->GetSeedIndex(),
      randomseed,
      tract);
    
    //only store tract if it is of nonzero length
    if( tract->GetVertexList()->Size() > 4 ){
      //std::cout<<"Storing tract\n";
      str->Filter->StoreTract(tract);
    }
    else{
      //std::cout<<"Not Storing Tract\n";
    }
  }
  return ITK_THREAD_RETURN_VALUE;
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
ProbabilityDistributionImageType::PixelType&
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::AccessLikelihoodCache( typename InputDWIImageType::IndexType index ){
  this->m_LikelihoodCacheMutexImagePtr->GetPixel(index).Lock();
  
  ProbabilityDistributionImageType::PixelType& likelihood = 
    m_LikelihoodCachePtr->GetPixel( index );
  typename InputDWIImageType::ConstPointer inputDWIImagePtr = this->GetInput();
  
  if( likelihood.GetSize() !=0){
    //entry found in cache
    this->m_LikelihoodCacheMutexImagePtr->GetPixel(index).Unlock();
    return likelihood;
  }
  //we need to lock m_CurrentLikelihoodCacheElements as well but not crucial right now
  else if( this->m_CurrentLikelihoodCacheElements < this->m_MaxLikelihoodCacheElements ){
    //entry not found in cache but we have space to store it
    likelihood.SetSize(this->m_SampleDirections->Size());

    this->CalculateLikelihood(static_cast< DWIVectorImageType::PixelType >(
      inputDWIImagePtr->GetPixel(index)),
      this->m_SampleDirections,
      likelihood);
    this->m_CurrentLikelihoodCacheElements++;
    
    this->m_LikelihoodCacheMutexImagePtr->GetPixel(index).Unlock();
    return likelihood;
  }
  else{
    //entry not found in cache and no space to store it
    this->m_LikelihoodCacheMutexImagePtr->GetPixel(index).Unlock();
    return likelihood;
  }
  this->m_LikelihoodCacheMutexImagePtr->GetPixel(index).Unlock();
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
bool
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::DelegateTract(unsigned long& randomseed){
  bool success = false;
  this->m_TotalDelegatedTractsMutex.Lock();
  if(this->m_TotalDelegatedTracts < this->m_TotalTracts){
    randomseed = this->m_RandomGenerator.lrand32();
    this->m_TotalDelegatedTracts++;
    success = true;
    //a tract was successfully delegated
  }
  else success = false; //all tracts have been delegated
  this->m_TotalDelegatedTractsMutex.Unlock();
  
  return success;
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::TractContainerToConnectivityMap(TractContainerType::Pointer tractcontainer){
  //zero the output image
  typename OutputConnectivityImageType::Pointer outputPtr = this->GetOutput();
  outputPtr->FillBuffer(0);

  typedef PathIterator< OutputConnectivityImageType, TractType > OutputTractIteratorType;
  
  for(int i=0; i<tractcontainer->Size(); i++ ){
    TractType::Pointer tract = tractcontainer->GetElement(i);
    //std::cout<< tract->EndOfInput() <<std::endl;
    OutputTractIteratorType outputtractIt( outputPtr,
      tract );
      
    for(outputtractIt.GoToBegin(); !outputtractIt.IsAtEnd(); ++outputtractIt){
    /* there is an issue using outputtractIt.Value() */
      outputtractIt.Set(outputtractIt.Get()+1);
    }
  }
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::StoreTract(TractType::Pointer tract){
    this->m_OutputTractContainerMutex.Lock();
    this->m_OutputTractContainer->InsertElement( 
      this->m_OutputTractContainer->Size(),
      tract);
    this->m_OutputTractContainerMutex.Unlock();
}

template< class TInputDWIImage, class TInputWhiteMatterProbabilityImage, class TOutputConnectivityImage >
void
StochasticTractographyFilter< TInputDWIImage, TInputWhiteMatterProbabilityImage, TOutputConnectivityImage >
::GenerateTensorImageOutput(void){
  this->UpdateGradientDirections();
  this->UpdateTensorModelFittingMatrices();
  
  //allocate the tensor image
  this->m_OutputTensorImage = OutputTensorImageType::New();
  m_OutputTensorImage->CopyInformation( this->GetInput() );
  m_OutputTensorImage->SetBufferedRegion( this->GetInput()->GetBufferedRegion() );
  m_OutputTensorImage->SetRequestedRegion( this->GetInput()->GetRequestedRegion() );
  m_OutputTensorImage->Allocate();
  
  //define an iterator for the input and output images
  typedef itk::ImageRegionConstIterator< InputDWIImageType > DWIImageIteratorType;
  typedef itk::ImageRegionIterator< OutputTensorImageType > TensorImageIteratorType;
  
  DWIImageIteratorType 
  inputDWIit( this->GetInput(), m_OutputTensorImage->GetRequestedRegion() );
  
  TensorImageIteratorType outputtensorit
    ( m_OutputTensorImage, m_OutputTensorImage->GetRequestedRegion() );
  
  unsigned int N = this->m_TransformedGradients->Size();
  TensorModelParamType tensormodelparams( 0.0 );
  vnl_diag_matrix< double > W(N,0);
  
  for(inputDWIit.GoToBegin(), outputtensorit.GoToBegin();
    !outputtensorit.IsAtEnd(); ++inputDWIit, ++outputtensorit){
    CalculateTensorModelParameters( inputDWIit.Get(),
      W, tensormodelparams);
      
    OutputTensorImageType::PixelType& D = outputtensorit.Value();
    //set the tensor model parameters into a Diffusion tensor
    D(0,0) = tensormodelparams[1];
    D(0,1) = tensormodelparams[4];
    D(0,2) = tensormodelparams[5];
    D(1,0) = tensormodelparams[4];
    D(1,1) = tensormodelparams[2];
    D(1,2) = tensormodelparams[6];
    D(2,0) = tensormodelparams[5];
    D(2,1) = tensormodelparams[6];
    D(2,2) = tensormodelparams[3];
    
    //std::cout<<D;
  }
}

}
