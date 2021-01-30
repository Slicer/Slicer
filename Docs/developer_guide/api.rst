==========
Slicer API
==========

Tutorials
---------

Check out these `developer tutorials <https://www.slicer.org/wiki/Documentation/Nightly/Training#PerkLab.27s_Slicer_bootcamp_training_materials>`_ to get started with customizing and extending 3D Slicer using Python scripting or C++.

C++
---

Majority of Slicer core modules and all basic infrastructure are implemented in C++.
Documentation of these classes are available at: http://apidocs.slicer.org/master/

Python
------

Native Python documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Python-style documentation is available for the following packages:

.. toctree::
   :maxdepth: 4

   mrml
   saferef
   slicer
   teem
   vtkAddon
   vtkITK

Doxygen-style documentation
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Slicer core infrastructure is mostly implemented in C++ and it is made available in Python in the ``slicer`` namespace.
Documentation of these classes is available at: http://apidocs.slicer.org/master/

This documentation is generated using the Doxygen tool, which uses C++ syntax. The following rules can help in interpreting this documentation for Python:

-  Qt classes (class name starts with ``q``): for example, `qSlicerMarkupsPlaceWidget <http://apidocs.slicer.org/master/classqSlicerMarkupsPlaceWidget.html>`_
-  VTK classes VTK classes (class name starts with ``vtk``): for example, `vtkMRMLModelDisplayNode <http://apidocs.slicer.org/master/classvtkMRMLModelDisplayNode.html>`_
-  Public Types: most commonly used for specifying enumerated values (indicated by ``enum`` type).
   These values can be accessed as ``slicer.className.typeName``, for example ``slicer.qSlicerMarkupsPlaceWidget.HidePlaceMultipleMarkupsOption``
-  Properties: these are values that are accessible as object attributes in Python and can be read and written as ``objectName.propertyName``. For example:
   ::

         >>> w = slicer.qSlicerMarkupsPlaceWidget()
         >>> w.deleteAllMarkupsOptionVisible
         True
         >>> w.deleteAllMarkupsOptionVisible=False
         >>> w.deleteAllMarkupsOptionVisible
         False

-  Public slots: publicly available methods. Note that ``setSomeProperty`` methods show up in the documentation but in Python these methods
   are not available and instead property values can be set using ``someProperty = ...``.

-  Signals: signals that can be connected to Python methods
   ::

         def someFunction():
            print("clicked!")

         b = qt.QPushButton("MyButton")
         b.connect("clicked()", someFunction)  # someFunction will be called when the button is clicked
         b.show()

-  Public member functions: methods that have ``Q_INVOKABLE`` keyword next to them are available from Python. ``virtual`` and ``override`` specifiers can be ignored.

   - ``className`` (for Qt classes): constructor, shows the arguments that can be passed when an object is created
   - ``New`` (for VTK classes): constructor, never needs an argument
   - ``~className``: destructor, can be ignored, Python calls it automatically when needed
   - ``SafeDownCast`` (for VTK classes): not needed for Python, as type conversions are automatic

-  Static Public Member Functions: can be accessed as ``slicer.className.memberFunctionName(arguments)`` for example: ``slicer.vtkMRMLModelDisplayNode.GetSliceDisplayModeAsString(0)``
-  Protected Slots, Member Functions, Attributes: for internal use only, not accessible in Python
-  Mapping commonly used data types from C++ documentation to Python:

   -  ``void`` -> Python: if the return value of a method is this type then it means that no value is returned
   -  ``someClass*`` (object pointer) -> Python: since Python takes care of reference counting, it can be simply interpreted in Python as ``someClass``.
      The called method can modify the object.
   -  ``int``, ``char``, ``short`` (with optional ``signed`` or ``unsigned`` prefix) -> Python: ``int``
   -  ``float``, ``double`` -> Python: ``float``
   -  ``double[3]`` -> Python: initialize a variable before the method call as ``point = np.zeros(3)`` (or ``point = [0.0, 0.0, 0.0]``) and use it as argument in the function
-  ``const char *``, ``std::string``, ``QString``, ``const QString&``  -> Python: ``str``
   -  ``bool`` -> Python: ``bool``
   
