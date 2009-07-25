Module Name: OpenCV 

This module is written by Atsushi Yamada (Fujimoto Lab., Nagoya Institute of Technology, Japan)
 CMakeLists.txt, FindOpenCV.cmake, and FindOpenIGTLink.cmake are contributions of K. Chinzei(AIST) and T. Takeuchi (SFC Corp.).

Acknowledgement: K. Chinzei (AIST), Y. Hayashi(Nagoya Univ.), T. Takeuchi (SFC Corp.), J. Tokuda(BWH), N. Hata(BWH), and H. Fujimoto(NIT) 

Date: 07/17/2009 on 2009 SummerSecondDisplayProject

Documentation of OpenCV module
1. Install
This module requires OpenIGTLink and OpenCV. You need to install these software in advance.
(a) OpenIGTLink: http://www.na-mic.org/Wiki/index.php/OpenIGTLink/Library
You can use SVN repository.
(b) OpenCV: http://sourceforge.net/projects/opencvlibrary/files/
OpenCV-1.0.0 is required. If you use higher version of OpenCV, you can see much error messages about CXCORE. We recommend 'make install' after './configure'.

We prepare the special CMakeLists.txt, FindOpenCV.cmake, and FindOpenIGTLink.cmake (written by K. Chinzei (AIST) and T. Takeuchi (SFC Corp.)). These files can configure necessary pathes automatically.

2. Operation
This module has some push buttons on the left pain of Slicer3. You can display the secondary window which has the captured video image and overlayed text data by clicking `secondary window on` button.If you push the `secondary window off` or `secondary window on` again, the Slicer3 is crushed.

3. System requirement
This module uses USB camera with OpenCV. It was tested on MacBook(white, Core Duo, OS X 10.5.7) and iMac(Core2 Duo, 2009, OS X 10.5.7 with built-in iSight camera.

4. Module Local Test Operation:
1. Select 'OpenCV' module.
2. Click 'secondary window on' button. Then, you can see the secondary window.
3. Select 'Transforms' module, and create transform node 'LinearTransform'.
4. Select 'Data' module, and check 'Display MRML ID's'. You can see that the MRML ID of 'Linear Transform' is 'vtkMRMLLinearTransformNode4'.
5. Select 'videoOpenIGTLink' module again, and click 'MRML observer on' button.
6. Select 'Transform' module, and select transform node 'LinearTransForm'.
7. If you move Translation progress bars and Rotation progress bars, you can see that status values overlaid the camera view changes smoothy.



