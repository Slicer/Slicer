(I) Overview:

This code implements a parallelization scheme to achieve stochastic tractography. It does not require the Slicer platform to run. However, the stochastic tractography generator from Slicer3 must be used to generate configuration set containing all Numpy format required files as inputs.

(II) Description:

(2.a) structure of folders:



(2.b) scripts you can launch:

(2.b.1) autoTracto2Regions.py: used when a 2 roi based tractography is to be done, takes no argument, must be launchned from the folder containing your data cases you created with the generator 

use: shell>> python autoTracto2Regions.py
 
(2.b.2) autoTracto2RegionsWithMask.py: idem as (2.b.1) but your configuration contains an external brain mask (more specifically a white matter mask)

use:  shell>> python autoTracto2RegionsWithMask.py 

(2.b.3) autoSlicerTracto2Regions.py: idem as (2.b.1) but communicates the results to a unique Slicer3 platform on a specific machine, takes 2 arguments  

care must be taken
  - the slicer dameon must be activated, if not it must be and the Slicer3 restarted
  - a unique instance of Slicer3 must be launchned on the target machine

use:  python autoSlicerTracto2Regions.py <IP address of the machine running the Slicer3> <port of slicer daemon=18943>
 e.g. shell>> python autoSlicerTracto2Regions.py 127.0.0.1 18943

(2.b.4) autoSlicerTracto2RegionsWithMask.py: idem as (2.b.3) but your configuration contains an external brain mask (more specifically a white matter mask)  


(2.b.5) ModifyHeader.py: used to modify on the fly the parameters of the parameters *.in file of your configuration set. Any parameter generated through the Slicer3 interface could be modified

use: (1) shell>> python ModifyHeader.py : display the whole list of parameters
     (2) python ModifyHeader.py <tag of parameter to modify> <value of parameter you want to set>
    e.g. shell>> python ModifyHeader.py totalTracts 300

  
(2.b.6) AnalyzeData.py: 


