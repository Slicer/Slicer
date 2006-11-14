/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: VotingBinaryHoleFillingImageFilter.cxx,v $
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

// Software Guide : BeginCommandLineArgs
// INPUTS: {BinaryThresholdImageFilterOutput.png}
// OUTPUTS: {VotingBinaryHoleFillingImageFilterOutput1.png}
// 1 1 
// Software Guide : EndCommandLineArgs

// Software Guide : BeginCommandLineArgs
// INPUTS: {BinaryThresholdImageFilterOutput.png}
// OUTPUTS: {VotingBinaryHoleFillingImageFilterOutput2.png}
// 2 2 
// Software Guide : EndCommandLineArgs

// Software Guide : BeginCommandLineArgs
// INPUTS: {BinaryThresholdImageFilterOutput.png}
// OUTPUTS: {VotingBinaryHoleFillingImageFilterOutput3.png}
// 3 3 
// Software Guide : EndCommandLineArgs


//  Software Guide : BeginLatex
//
//  The \doxygen{VotingBinaryHoleFillingImageFilter} applies a voting operation
//  in order to fill-in cavities. This can be used for smoothing contours and
//  for filling holes in binary images. 
//
//  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter}
//
//  Software Guide : EndLatex 


#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"


//  Software Guide : BeginLatex
//
//  The header file corresponding to this filter should be included first.
//
//  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!header}
//
//  Software Guide : EndLatex 


// Software Guide : BeginCodeSnippet
#include "itkVotingBinaryHoleFillingImageFilter.h"
#include "itkPluginFilterWatcher.h"

// Software Guide : EndCodeSnippet
#include "VotingBinaryHoleFillingImageFilterCLP.h"

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  //  Software Guide : BeginLatex
  //
  //  Then the pixel and image types of the input and output must be defined.
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef   short  InputPixelType;
  typedef   short  OutputPixelType;

  typedef itk::Image< InputPixelType,  3 >   InputImageType;
  typedef itk::Image< OutputPixelType, 3 >   OutputImageType;
  // Software Guide : EndCodeSnippet


  typedef itk::ImageFileReader< InputImageType  >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader->SetFileName( inputVolume.c_str() );
  writer->SetFileName( outputVolume.c_str() );

  //  Software Guide : BeginLatex
  //
  //  Using the image types, it is now possible to define the filter type
  //  and create the filter object.
  //
  //  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!instantiation}
  //  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!New()}
  //  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!Pointer}
  // 
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  typedef itk::VotingBinaryHoleFillingImageFilter<
               InputImageType, OutputImageType >  FilterType;

  FilterType::Pointer filter = FilterType::New();
  itk::PluginFilterWatcher watcher(filter, "Voting Binary Hole Filling",
    CLPProcessInformation);

  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  //
  //  The size of the neighborhood is defined along every dimension by
  //  passing a \code{SizeType} object with the corresponding values. The
  //  value on each dimension is used as the semi-size of a rectangular
  //  box. For example, in $2D$ a size of \(1,2\) will result in a $3 \times
  //  5$ neighborhood.
  //
  //  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!Radius}
  //  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!Neighborhood}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  InputImageType::SizeType indexRadius;
  
  indexRadius[0] = radius[0]; // radius along x
  indexRadius[1] = radius[1]; // radius along y
  indexRadius[2] = radius[2]; // radius along z

  filter->SetRadius( indexRadius );
  // Software Guide : EndCodeSnippet



  //  Software Guide : BeginLatex
  //
  //  Since the filter is expecting a binary image as input, we must specify
  //  the levels that are going to be considered background and foreground. This
  //  is done with the \code{SetForegroundValue()} and
  //  \code{SetBackgroundValue()} methods.
  //
  //  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!SetForegroundValue()}
  //  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!SetBackgroundValue()}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  filter->SetBackgroundValue( background );
  filter->SetForegroundValue( foreground );
  // Software Guide : EndCodeSnippet



  //  Software Guide : BeginLatex
  //
  //  We must also specify the majority threshold that is going to be used as
  //  the decision criterion for converting a background pixel into a
  //  foreground pixel. The rule of conversion is that a background pixel will
  //  be converted into a foreground pixel if the number of foreground
  //  neighbors surpass the number of background neighbors by the majority
  //  value. For example, in a 2D image, with neighborhood or radius 1, the
  //  neighborhood will have size $3 \times 3$. If we set the majority value to
  //  2, then we are requiring that the number of foreground neighbors should
  //  be at least (3x3 -1 )/2 + majority. This is done with the
  //  \code{SetMajorityThreshold()} method.
  //
  //  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!SetMajorityThreshold()}
  //
  //  Software Guide : EndLatex 

  // Software Guide : BeginCodeSnippet
  filter->SetMajorityThreshold( majorityThreshold );
  // Software Guide : EndCodeSnippet




  //  Software Guide : BeginLatex
  //
  //  The input to the filter can be taken from any other filter, for example
  //  a reader. The output can be passed down the pipeline to other filters,
  //  for example, a writer. An update call on any downstream filter will
  //  trigger the execution of the median filter.
  //
  //  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!SetInput()}
  //  \index{itk::Voting\-Binary\-Hole\-Filling\-Image\-Filter!GetOutput()}
  //
  //  Software Guide : EndLatex 


  // Software Guide : BeginCodeSnippet
  filter->SetInput( reader->GetOutput() );
  writer->SetInput( filter->GetOutput() );
  writer->Update();
  // Software Guide : EndCodeSnippet


  //  Software Guide : BeginLatex
  // 
  // \begin{figure}
  // \center
  // \includegraphics[width=0.44\textwidth]{BinaryThresholdImageFilterOutput.eps}
  // \includegraphics[width=0.44\textwidth]{VotingBinaryHoleFillingImageFilterOutput1.eps}
  // \includegraphics[width=0.44\textwidth]{VotingBinaryHoleFillingImageFilterOutput2.eps}
  // \includegraphics[width=0.44\textwidth]{VotingBinaryHoleFillingImageFilterOutput3.eps}
  // \itkcaption[Effect of the VotingBinaryHoleFilling filter.]{Effect of the
  // VotingBinaryHoleFillingImageFilter on a slice from a MRI proton density brain image
  // that has been thresholded in order to produce a binary image. The output
  // images have used radius 1,2 and 3 respectively.}
  // \label{fig:VotingBinaryHoleFillingImageFilterOutput}
  // \end{figure}
  //
  //  Figure \ref{fig:VotingBinaryHoleFillingImageFilterOutput} illustrates the effect of
  //  the VotingBinaryHoleFillingImageFilter filter on a thresholded slice of MRI brain
  //  image using neighborhood radii of \(1,1\), \(2,2\) and \(3,3\) that
  //  correspond respectively to neighborhoods of size $ 3 \times 3 $,  $ 5
  //  \times 5 $, $ 7 \times 7 $.  The filtered image demonstrates the
  //  capability of this filter for reducing noise both in the background and
  //  foreground of the image, as well as smoothing the contours of the regions.
  //  
  //  Software Guide : EndLatex 


  return EXIT_SUCCESS;
}

