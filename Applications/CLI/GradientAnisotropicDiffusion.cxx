/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

//  Software Guide : BeginCommandLineArgs
//    INPUTS: {BrainProtonDensitySlice.png}
//    OUTPUTS: {GradientAnisotropicDiffusionImageFilterOutput.png}
//    5 0.25 3
//  Software Guide : EndCommandLineArgs
//  Software Guide : BeginLatex
//  The \doxygen{GradientAnisotropicDiffusionImageFilter}  implements an
//  $N$-dimensional version of the classic Perona-Malik anisotropic diffusion
//  equation for scalar-valued images \cite{Perona1990}.  
//
//  The conductance term for this implementation is chosen as a function of the
//  gradient magnitude of the image at each point, reducing the strength of
//  diffusion at edge pixels.
// 
//  \begin{equation}
//  C(\mathbf{x}) = e^{-(\frac{\parallel \nabla U(\mathbf{x}) \parallel}{K})^2}
//  \end{equation}
//
//  The numerical implementation of this equation is similar to that described
//  in the Perona-Malik paper \cite{Perona1990}, but uses a more robust technique
//  for gradient magnitude estimation and has been generalized to $N$-dimensions.
//
//  \index{itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter}
//
//  Software Guide : EndLatex 


#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkXMLFilterWatcher.h"

//  Software Guide : BeginLatex
//
//  The first step required to use this filter is to include its header file.
//
//  \index{itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter!header}
//
//  Software Guide : EndLatex 

// Software Guide : BeginCodeSnippet
#include "itkGradientAnisotropicDiffusionImageFilter.h"
// Software Guide : EndCodeSnippet

#include "GradientAnisotropicDiffusionCLP.h"

int main( int argc, char * argv[] )
{

  PARSE_ARGS;

  //  Software Guide : BeginLatex
  //
  //  Types should be selected based on the pixel types required for the
  //  input and output images.  The image types are defined using the pixel
  //  type and the dimension.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef    float   InputPixelType;
  typedef    float   OutputPixelType;

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;
  // Software Guide : EndCodeSnippet


  typedef itk::ImageFileReader< InputImageType >  ReaderType;


  //  Software Guide : BeginLatex
  //
  //  The filter type is now instantiated using both the input image and the
  //  output image types. The filter object is created by the \code{New()}
  //  method.
  //
  //  \index{itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter!instantiation}
  //  \index{itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter!New()}
  //  \index{itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter!Pointer}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef itk::GradientAnisotropicDiffusionImageFilter<
               InputImageType, OutputImageType >  FilterType;
  FilterType::Pointer filter = FilterType::New();
  // Software Guide : EndCodeSnippet


  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputVolume.c_str() );


  //  Software Guide : BeginLatex
  //
  //  The input image can be obtained from the output of another filter. Here,
  //  an image reader is used as source.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  itk::XMLFilterWatcher watcher(filter, "Gradient Anisotropic Diffusion");
  filter->SetInput( reader->GetOutput() );
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  This filter requires three parameters, the number of iterations to be
  //  performed, the time step and the conductance parameter used in the
  //  computation of the level set evolution. These parameters are set using
  //  the methods \code{SetNumberOfIterations()}, \code{SetTimeStep()} and
  //  \code{SetConductanceParameter()} respectively.  The filter can be
  //  executed by invoking Update().
  //
  //  \index{itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter!Update()}
  //  \index{itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter!SetTimeStep()}
  //  \index{itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter!SetConductanceParameter()}
  //  \index{itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter!SetNumberOfIterations()}
  //  \index{SetTimeStep()!itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter}
  //  \index{SetNumberOfIterations()!itk::Gradient\-Anisotropic\-Diffusion\-Image\-Filter}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  filter->SetNumberOfIterations( numberOfIterations );
  filter->SetTimeStep( timeStep );
  filter->SetConductanceParameter( conductance );
  
  filter->Update();
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  Typical values for the time step are $0.25$ in $2D$ images and $0.125$
  //  in $3D$ images. The number of iterations is typically set to $5$; more
  //  iterations result in further smoothing and will increase the computing
  //  time linearly.
  //
  //  Software Guide : EndLatex 


  //
  //  The output of the filter is rescaled here and then sent to a writer.
  //
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( outputVolume.c_str() );
 
  writer->SetInput( filter->GetOutput() );
  writer->Update();


  //  Software Guide : BeginLatex
  //  
  // \begin{figure} \center
  // \includegraphics[width=0.44\textwidth]{BrainProtonDensitySlice.eps}
  // \includegraphics[width=0.44\textwidth]{GradientAnisotropicDiffusionImageFilterOutput.eps}
  // \itkcaption[GradientAnisotropicDiffusionImageFilter output]{Effect of the
  // GradientAnisotropicDiffusionImageFilter on a slice from a MRI Proton
  // Density image  of the brain.}
  // \label{fig:GradientAnisotropicDiffusionImageFilterInputOutput}
  // \end{figure}
  //
  //  Figure \ref{fig:GradientAnisotropicDiffusionImageFilterInputOutput}
  //  illustrates the effect of this filter on a MRI proton density image of
  //  the brain. In this example the filter was run with a time step of $0.25$,
  //  and $5$ iterations.  The figure shows how homogeneous regions are 
  // smoothed and edges are preserved.
  //
  //  \relatedClasses
  //  \begin{itemize}
  //  \item \doxygen{BilateralImageFilter}
  //  \item \doxygen{CurvatureAnisotropicDiffusionImageFilter}
  //  \item \doxygen{CurvatureFlowImageFilter}
  //  \end{itemize}
  //
  //  Software Guide : EndLatex 

  return EXIT_SUCCESS;
}

