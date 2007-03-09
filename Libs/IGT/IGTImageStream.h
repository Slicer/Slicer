
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


#ifndef IGTIMAGESTREAM_H
#define IGTIMAGESTREAM_H

#include <string>
#include <vector>



/**
  * class IGTImageStream
  */

class IGTImageStream
{
public:

  // Constructors/Destructors
  //  


  /**
   * Empty Constructor
   */
  IGTImageStream ( );

  /**
   * Empty Destructor
   */
  virtual ~IGTImageStream ( );

  // Static Public attributes
  //  

  // Public attributes
  //  


  // Public attribute accessor methods
  //  


  // Public attribute accessor methods
  //  



  /**
   */
  void obtain_patient_to_image_matrix ( );

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

  IGTImageState m_ImageBuffer;

  // Private attribute accessor methods
  //  


  // Private attribute accessor methods
  //  


  /**
   * Set the value of m_ImageBuffer
   * @param new_var the new value of m_ImageBuffer
   */
  void setImageBuffer ( IGTImageState new_var );

  /**
   * Get the value of m_ImageBuffer
   * @return the value of m_ImageBuffer
   */
  IGTImageState getImageBuffer ( );


  void initAttributes ( ) ;

};

#endif // IGTIMAGESTREAM_H
