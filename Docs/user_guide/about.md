# About 3D Slicer

## What is 3D Slicer?

- A software application for visualization and analysis of medical image computing data sets. All commonly used data sets are supported, such as images, segmentations, surfaces, annotations, transformations, etc., in 2D, 3D, and 4D. Visualization is available on desktop and in virtual reality. Analysis includes segmentation, registration, and various quantifications.
- A research software platform, which allows researchers to quickly develop and evaluate new methods and distribute them to clinical users. All features are available and extensible in Python and C++. A full Python environment is provided where any Python packages can be installed and combined with built-in features. Slicer has a built-in Python console and can act as a Jupyter notebook kernel with remote 3D rendering capabilities.
- Product development platform, which allows companies to quickly prototype and release products to users. Developers can focus on developing new methods and do not need to spend time with redeveloping basic data import/export, visualization, interaction features. The application is designed to be highly customizable (with custom branding, simplified user interface, etc.). 3D Slicer is completely free and there are no restrictions on how it is used - it is up to the software distributor to ensure that the developed application is suitable for the intended use.

**Note:** There is no restriction on use, but Slicer is **NOT** approved for clinical use and the distributed application is intended for research use. Permissions and compliance with applicable rules are the responsibility of the user. For details on the license see [here](https://www.slicer.org/wiki/License).

Highlights:
- Free, [open-source](http://en.wikipedia.org/wiki/Open_source>) software available on multiple operating systems: Linux, macOS and Windows.
- Multi organ: from head to toe.
- Support for multi-modality imaging including, MRI, CT, US, nuclear medicine, and microscopy.
- Real-time interface for medical devices, such as surgical navigation systems, imaging systems, robotic devices, and sensors.
- Highly extensible: users can easily add more capabilities by installing additional modules from the Extensions manager, running custom Python scripts in the built-in Python console, run any executables from the application's user interface, or implement custom modules in Python or C++.
- Large and active user community.

## License

The 3D Slicer software is distributed under a BSD-style open source license that is broadly compatible with the Open Source Definition by [The Open Source Initiative](http://opensource.org/) and contains no restrictions on legal uses of the software.

To use Slicer, please read the [3D Slicer Software License Agreement](https://github.com/Slicer/Slicer/blob/master/License.txt) before downloading any binary releases of the Slicer.

### Historical notes about the license

The Slicer License was drafted in 2005 by lawyers working for Brigham and Women's Hospital (BWH), a teaching affiliate of Harvard Medical School, to be BSD-like but with a few extra provisions related to medical software.  It is specific to BWH so it's not directly reusable, but it could serve as a template for projects with similar goals.

It was written in part because BWH was the prime contractor on an NIH-funded development consortium ([NA-MIC](https://www.na-mic.org/)) and wanted all code contributions to be compatible with ultimate use in real-world medical products (that is, commercial FDA-approved medical devices, which are almost universally closed source even if they build on open software).  Compliance with the Slicer License was required for subcontractors, a group that included GE Research, Kitware and several universities (MIT, UNC...) who all reviewed and accepted this license.

The license has been in continuous use since 2005 for the 3D Slicer software package (slicer.org) that as of 2021 has been downloaded more than a million times and has been referenced in about 12,000 academic publications (https://www.slicer.org/wiki/Main_Page/SlicerCommunity). Some of the code is also now being used in several medical products for which this license has been reviewed and accepted by the companies involved.

### License terms and reasons

Here are some of the key points that BWH included in addition to BSD terms to make the license suit the case of a large hospital distributing open source medical software.

For using and redistributing 3D Slicer:
- The license states that the code is "designed for research" and "CLINICAL APPLICATIONS ARE NEITHER RECOMMENDED NOR ADVISED" to make it extra clear that any commercial clinical uses of the code are solely the responsibility of the user and not BWH or the other developers.  This is a disclaimer rather than a legal restriction.

For making changes or adding any source code or data to 3D Slicer:
- Contributors explicitly grant royalty free rights if they contribute code covered by a patent they control (i.e. to avoid submarine patents).
- No GPL or other copyleft code is allowed because that could make it complicated and risky to mix Slicer code with private intellectual property, which is often present in regulated medical products.
- Contributors affirm that they have de-identified any patient data they contribute to avoid issues with HIPAA or related regulations.

### Status compared to other open source licenses

As of June 2021, the Slicer License has been used for over 15 years without incident. In May of 2021, a discourse user [suggested](https://discourse.slicer.org/t/apply-for-osi-open-source-license-status/17791) submitting the license to [the OSI license review process](https://opensource.org/approval). After some discussion and hearing no objections, the community leadership decided to [submit the license for review](http://lists.opensource.org/pipermail/license-review_lists.opensource.org/2021-May/thread.html). Although the OSI process is not legally binding, the discussion could give potential Slicer users perspective on how provisions of the license compare with other commonly used licenses.
The discussion concluded that bundling the contribution agreement in the license makes it non-approvable by OSI and the requirement to use the software for legal purposes may not be consistent with the [Open Source Definition](https://opensource.org/osd). Otherwise the license terms appear not to be controversial. Interested parties should review the [full discussion](http://lists.opensource.org/pipermail/license-review_lists.opensource.org/2021-June/thread.html#5166) for details.

## How to cite

### 3D Slicer as a platform

To acknowledge 3D Slicer as a platform, please cite the [Slicer web site](http://www.slicer.org/) and the following publications when publishing work that uses or incorporates 3D Slicer:

**Fedorov A., Beichel R., Kalpathy-Cramer J., Finet J., Fillion-Robin J-C., Pujol S., Bauer C., Jennings D., Fennessy F.M., Sonka M., Buatti J., Aylward S.R., Miller J.V., Pieper S., Kikinis R. [3D Slicer as an Image Computing Platform for the Quantitative Imaging Network](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3466397/pdf/nihms383480.pdf). Magnetic Resonance Imaging. 2012 Nov;30(9):1323-41. PMID: 22770690. PMCID: PMC3466397.**

### Individual modules

To acknowledge individual modules: each module has an acknowledgment tab in the top section. Information about contributors and funding source can be found there:

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/acknowledgement_ack_tab.png)

Additional information (including information about the underlying publications) can be typically found on the manual pages accessible through the help tab in the top section:

![](https://github.com/Slicer/Slicer/releases/download/docs-resources/acknowledgement_help_tab.png)

## Acknowledgments

Slicer is made possible through contributions from an international community of scientists from a multitude of fields, including engineering and biomedicine. The following sections give credit to some of the major contributors to the 3D Slicer core effort. Each 3D Slicer extension has a separate acknowledgements page with information specific to that extension.

Ongoing Slicer support depends on YOU

Please give the Slicer repository a star on github. This is an easy way to show thanks and it can help us qualify for useful services that are only open to widely recognized open projects.
Don't forget to cite our publications because that helps us get new grant funding.
If you find Slicer is helpful like the community please get involved. You don't need to be a programmer to help!

### Major Contributors

- Ron Kikinis: Principal Investigator
- Steve Pieper: Chief Architect
- Jean-Christophe Fillion-Robin: Lead Developer
- Nicole Aucoin
- Stephen Aylward
- Andrey Fedorov
- Noby Hata
- Hans Johnson
- Tina Kapur
- Gabor Fichtinger
- Andras Lasso
- Csaba Pinter
- Jim Miller
- Sonia Pujol: Director of Training
- Junichi Tokuda
- Lauren O'Donnell
- Andinet Enquobahrie
- Beatriz Paniagua

*Contributors are not only developers, but also individual helping to secure funding and move the platform forward.*

### Groups Contributing to the Core Engineering of Slicer in a Major Way

- SPL: Ron Kikinis, Nicole Aucoin, Lauren O'Donnell, Andrey Fedorov, Isaiah Norton, Sonia Pujol, Noby Hata, Junichi Tokuda
- Isomics: Steve Pieper, Alex Yarmarkovich
- Kitware: Jean-Christophe Fillion-Robin, Julien Finet, Will Schroeder, Stephen Aylward, Andinet Enquobahrie, Beatriz Paniagua, Matt McCormick, Johan Andruejol, Max Smolens, Alexis Girault, Sam Horvath
- University of Iowa: Hans Johnson
- GE: Jim Miller
- Perk Lab, Queen's University: Andras Lasso, Tamas Ungi, Csaba Pinter, Gabor Fichtinger
- Kapteyn Astronomical Institute, University of Groningen: Davide Punzo

### Funding Sources

Many of the activities around the Slicer effort are made possible through funding from public and private sources. The National Institutes of Health of the USA is a major contributor through a variety of competitive grants and contracts.

See a selection of funding grants [here](https://www.slicer.org/wiki/Documentation/4.x/Acknowledgments#Grants).

## Commercial Use

We invite commercial entities to use 3D Slicer.

### Slicer's License makes Commercial Use Available

- 3D Slicer is a free open source software distributed under a BSD style license.
- The license does not impose restrictions on the use of the software.
- 3D Slicer is NOT FDA approved. It is the users responsibility to ensure compliance with applicable rules and regulations.
- For details, please see the 3D Slicer Software License Agreement.

### Commercial Partners

- [Ebatinca SL](https://ebatinca.com/) is an international technology company in Las Palmas, Spain focused on technology for sustainable development.
- [Isomics](http://www.isomics.com/) uses 3D Slicer in a variety of academic and commercial research partnerships in fields such as planning and guidance for neurosurgery, quantitative imaging for clinical trials, clinical image informatics.
- [Kitware](http://www.kitware.com/opensource/slicer.html) focuses on solving the world’s most complex scientific challenges through customized software solutions. The company has a long history of contributing to open source platforms that serve as the foundation of many medical visualization and data processing applications. Kitware helps customers develop commercial products based on 3D Slicer and has used the platform to rapidly prototype solutions in nearly every aspect of medical imaging.
- [Pixel Medical](http://pixelmedical.ca) builds on and contributes to 3D Slicer to develop innovative medical software from idea to clinical prototype to finished product, and to support academic research projects. Areas of expertise include radiation therapy, image guided therapy, virtual & augmented reality, hardware & device support, and machine learning & artificial intelligence.

_Listed in alphabetical order._

### 3D Slicer based products

Many companies prefer not to disclose what software components they use in their products, therefore here we can only list a few commercial products that are based on 3D Slicer:

- Allen Institute for Brain Science: [Cell Locator](https://github.com/BICCN/cell-locator/#readme), Desktop application for manually aligning specimens to annotated 3D spaces.
- Radiopharmaceutical Imaging and Dosimetry: RPTDose, a 3D Slicer-based application that streamlines and integrates quantitative imaging analysis and dose estimation techniques to guide and optimize the use of radiopharmaceutical therapy agents in clinical trials. See more information on this [Kitware blog](https://blog.kitware.com/kitware-customer-highlight-radiopharmaceutical-imaging-and-dosimetry-llc-rapid/).
- [SonoVol](http://sonovol.com/) is developing a whole-body ultrasound imaging system for small animals. This start-up company arose from research in the Department of Biomedical Engineering at the University of North Carolina at Chapel Hill. See more information on this [Kitware blog](https://blog.kitware.com/kitware-customer-highlight-sonovol/).
- Xoran Technologies: Image-guided Platform for Deep Brain Stimulation Surgery 1. See more information on this [Kitware blog](https://blog.kitware.com/xoran-technologies-and-kitware-collaborate-on-image-guided-platform-for-deep-brain-stimulation-surgery/).
- [Xstrahl](http://www.xstrahl.com/) is developing a Small Animal Radiation Research Platform (SARRP) that uses 3D Slicer as its front-end application for radiation therapy beam placement and system control. See more information on this [Kitware blog](https://blog.kitware.com/kitware-customer-highlight-muriplan-from-xstrahl-a-3d-slicer-based-radiotherapy-treatment-planning-system/).

_Listed in alphabetical order._

## Contact us

It is recommended to post any questions, bug reports, or enhancement requests to the [Slicer forum](https://discourse.slicer.org).

Our online issue tracker is available [here](https://issues.slicer.org).

For commercial/confidential consulting, contact one of the [commercial partners](#commercial-partners).
