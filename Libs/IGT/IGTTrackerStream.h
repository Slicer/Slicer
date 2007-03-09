
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


#ifndef IGTTRACKERSTREAM_H
#define IGTTRACKERSTREAM_H

#include <string>

/**
  * class IGTTrackerStream
  */

class IGTTrackerStream
{
public:

  // Constructors/Destructors
  //  


  /**
   * Empty Constructor
   */
  IGTTrackerStream ( );

  /**
   * Empty Destructor
   */
  virtual ~IGTTrackerStream ( );

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
  void perform_patient_to_image_matrix ( );


  /**
   */
  void register_tracker_to_data_buffer ( );

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

  IGTTrackerAttributes m_TrackerAttributesBuffer;

  // Private attribute accessor methods
  //  


  // Private attribute accessor methods
  //  


  /**
   * Set the value of m_TrackerAttributesBuffer
   * @param new_var the new value of m_TrackerAttributesBuffer
   */
  void setTrackerAttributesBuffer ( IGTTrackerAttributes new_var );

  /**
   * Get the value of m_TrackerAttributesBuffer
   * @return the value of m_TrackerAttributesBuffer
   */
  IGTTrackerAttributes getTrackerAttributesBuffer ( );


  void initAttributes ( ) ;

};

#endif // IGTTRACKERSTREAM_H
