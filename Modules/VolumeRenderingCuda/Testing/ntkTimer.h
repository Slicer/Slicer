#ifndef NTKTIMER_H
#define NTKTIMER_H


class ntkTimer{
 public:

  /**
   * Object constructor.
   */

  ntkTimer();

  /**
   * Object destructor.
   */

  ~ntkTimer();
  
  /**
   * Start timer.
   */

  void Start();

  /**
   * Stop timer.
   */

  void End();
  
  /**
   * Initialize timer.
   */
  
  void Reset();

  /**
   * Get elapsed time (difference between start time and end time) in ms.
   */

  long long getElapsedTime();
  
  /**
   * Get start time in ms.
   */
  
  long long getStartTime();

  /**
   * Get end time in ms.
   */

  long long getEndTime();

  /**
   * Print elapsed time to stdout in the format "text. Elapsed time = ... ms"
   */

  void printElapsedTime(char* text);

  /**
   * Print detailed elapsed time to stdout in the format "text. Elapsed time = ... ms"
   */

  void printDetailedElapsedTime(char* text);

  /**
   * Calculate and print elapsed time with startTime and endTime as input. Format: "text. Elapsed time = ... ms"
   */
  
  void printElapsedTime(long long startTime, long long endTime, char* text);

  /**
   * Calculate and print detailed elapsed time with startTime and endTime as input. Format: "text. Elapsed time = ... ms"
   */
  
  void printDetailedElapsedTime(long long startTime, long long endTime, char* text);
 private:
  long long m_startTime;
  long long m_endTime;
  long long gethrtime();
};

#endif
