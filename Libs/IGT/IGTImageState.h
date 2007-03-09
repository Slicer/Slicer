
#ifndef IGTAURORATRACKER_H
#define IGTAURORATRACKER_H

#include <string>

/**
  * class IGTAuroraTracker
  */

class IGTAuroraTracker
{
public:

  // Constructors/Destructors
  //  


  /**
   * Empty Constructor
   */
  IGTAuroraTracker ( );

  /**
   * Empty Destructor
   */
  virtual ~IGTAuroraTracker ( );

  // Static Public attributes
  //  

  // Public attributes
  //  


  // Public attribute accessor methods
  //  


  // Public attribute accessor methods
  //  


protected:

  // Static Protected attributes
  //  

  // Protected attributes
  //  


  // Protected attribute accessor methods
  //  


  // Protected attribute accessor methods
  //  


private:

  // Static Private attributes
  //  

  // Private attributes
  //  


  // Private attribute accessor methods
  //  


  // Private attribute accessor methods
  //  



};

#endif // IGTAURORATRACKER_H


#ifndef IGTIMAGESTATE_H
#define IGTIMAGESTATE_H

#include <string>
#include <vector>



/**
  * class IGTImageState
  */

class IGTImageState
{
public:

  // Constructors/Destructors
  //  


  /**
   * Empty Constructor
   */
  IGTImageState ( );

  /**
   * Empty Destructor
   */
  virtual ~IGTImageState ( );

  // Static Public attributes
  //  

  // Public attributes
  //  


  // Public attribute accessor methods
  //  


  // Public attribute accessor methods
  //  


protected:

  // Static Protected attributes
  //  

  // Protected attributes
  //  


  // Protected attribute accessor methods
  //  


  // Protected attribute accessor methods
  //  


private:

  // Static Private attributes
  //  

  // Private attributes
  //  

  int m_ImageSizeX;
  int m_ImageSizeY;
  vtkImageData m_image;

  // Private attribute accessor methods
  //  


  // Private attribute accessor methods
  //  


  /**
   * Set the value of m_ImageSizeX
   * @param new_var the new value of m_ImageSizeX
   */
  void setImageSizeX ( int new_var );

  /**
   * Get the value of m_ImageSizeX
   * @return the value of m_ImageSizeX
   */
  int getImageSizeX ( );


  /**
   * Set the value of m_ImageSizeY
   * @param new_var the new value of m_ImageSizeY
   */
  void setImageSizeY ( int new_var );

  /**
   * Get the value of m_ImageSizeY
   * @return the value of m_ImageSizeY
   */
  int getImageSizeY ( );


  /**
   * Set the value of m_image
   * @param new_var the new value of m_image
   */
  void setImage ( vtkImageData new_var );

  /**
   * Get the value of m_image
   * @return the value of m_image
   */
  vtkImageData getImage ( );


  void initAttributes ( ) ;

};

#endif // IGTIMAGESTATE_H
