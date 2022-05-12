# About 3D Slicer

## What is 3D Slicer?

- A software application for visualization and analysis of medical image computing data sets. All commonly used data sets are supported, such as images, segmentations, surfaces, annotations, transformations, etc., in 2D, 3D, and 4D. Visualization is available on desktop and in virtual reality. Analysis includes segmentation, registration, and various quantifications.
- A research software platform, which allows researchers to quickly develop and evaluate new methods and distribute them to clinical users. All features are available and extensible in Python and C++. A full Python environment is provided where any Python packages can be installed and combined with built-in features. Slicer has a built-in Python console and can act as a Jupyter notebook kernel with remote 3D rendering capabilities.
- Product development platform, which allows companies to quickly prototype and release products to users. Developers can focus on developing new methods and do not need to spend time with redeveloping basic data import/export, visualization, interaction features. The application is designed to be highly customizable (with custom branding, simplified user interface, etc.). 3D Slicer is completely free and there are no restrictions on how it is used - it is up to the software distributor to ensure that the developed application is suitable for the intended use.

**Note:** There is no restriction on use, but Slicer is **NOT** approved for clinical use and the distributed application is intended for research use. Permissions and compliance with applicable rules are the responsibility of the user. For details on the license see [here](https://www.slicer.org/wiki/License).

Highlights:
- Free, [open-source](https://en.wikipedia.org/wiki/Open_source>) software available on multiple operating systems: Linux, macOS and Windows.
- Multi organ: from head to toe.
- Support for multi-modality imaging including, MRI, CT, US, nuclear medicine, and microscopy.
- Real-time interface for medical devices, such as surgical navigation systems, imaging systems, robotic devices, and sensors.
- Highly extensible: users can easily add more capabilities by installing additional modules from the Extensions manager, running custom Python scripts in the built-in Python console, run any executables from the application's user interface, or implement custom modules in Python or C++.
- Large and active user community.

## License

The 3D Slicer software is distributed under a BSD-style open source license that is broadly compatible with the Open Source Definition by [The Open Source Initiative](https://opensource.org/) and contains no restrictions on legal uses of the software.

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

As of June 2021, the Slicer License has been used for over 15 years without incident. In May of 2021, a discourse user [suggested](https://discourse.slicer.org/t/apply-for-osi-open-source-license-status/17791) submitting the license to [the OSI license review process](https://opensource.org/approval). After some discussion and hearing no objections, the community leadership decided to [submit the license for review](https://lists.opensource.org/pipermail/license-review_lists.opensource.org/2021-May/thread.html). Although the OSI process is not legally binding, the discussion could give potential Slicer users perspective on how provisions of the license compare with other commonly used licenses.
The discussion concluded that bundling the contribution agreement in the license makes it non-approvable by OSI and the requirement to use the software for legal purposes may not be consistent with the [Open Source Definition](https://opensource.org/osd). Otherwise the license terms appear not to be controversial. Interested parties should review the [full discussion](https://lists.opensource.org/pipermail/license-review_lists.opensource.org/2021-June/thread.html#5166) for details.

## How to cite

### 3D Slicer as a platform

To acknowledge 3D Slicer as a platform, please cite the [Slicer web site](https://www.slicer.org/) and the following publications when publishing work that uses or incorporates 3D Slicer:

**Fedorov A., Beichel R., Kalpathy-Cramer J., Finet J., Fillion-Robin J-C., Pujol S., Bauer C., Jennings D., Fennessy F.M., Sonka M., Buatti J., Aylward S.R., Miller J.V., Pieper S., Kikinis R. [3D Slicer as an Image Computing Platform for the Quantitative Imaging Network](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC3466397/pdf/nihms383480.pdf). Magnetic Resonance Imaging. 2012 Nov;30(9):1323-41. PMID: 22770690. PMCID: PMC3466397.**

## The 3D Slicer name and logo

3D Slicer and the logo are trademarks of Brigham and Women's Hospital (BWH) and may not be used without permission.  Such permission is broadly granted for academic or commercial uses, such as documenting the use of Slicer in your project or promoting the use of Slicer by others.  Please use the original Slicer logo colors and do not alter the shape or text.  Using Slicer to imply that BWH or the Slicer community endorses your product or project is not permitted without permission.  For other uses please contact Ron Kikinis (kikinis@bwh.harvard.edu) and Steve Pieper (pieper@bwh.harvard.edu).

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

Many of the activities around the Slicer effort are made possible through funding from public and private sources. The National Institutes of Health of the USA is a major contributor through a variety of competitive grants and contracts. Funding sources that contribute to development of Slicer core or extensions include:

|Grant Number|Description|Grant PIs|Start Date|End Date|Title|
|------------|-----------|---------|----------|--------|-----|
|[NIH 1R01HL153166-01](https://projectreporter.nih.gov/project_info_description.cfm?aid=10029738&icde=51006191)|Computer Modeling of the Tricuspid Valve in HLHS|Matthew Jolley|2021-06-30|2025-06-30|Computer Modeling of the Tricuspid Valve in Hypoplastic Left Heart Syndrome|
|[CZI EOSS 4](https://chanzuckerberg.com/eoss/proposals/3d-slicer-in-my-language-internationalization-and-usability-improvements/)   |Slicer internationalization and usability|Pujol, Sonia|2021-09-01|2023-09-01   |3D Slicer in My Language: Internationalization and Usability Improvements|
|[NIH 4P41EB015902](https://projectreporter.nih.gov/project_info_description.cfm?aid=9115586&icde=31485398&ddparam=&ddvalue=&ddsub=&cr=3&csb=default&cs=ASC)|Neuroimage Analysis Center|Ron Kikinis|2013-08-01|2023-08-31|Application of Slicer to image-guided neurosurgery and other applications through steered computatiton and image navigation databases|
|CANARIE RS3-036|SlicerAIGT|Fichtinger, Gabor|2020-10-01|2023-03-31|Develop free open source research software for AI-powered image guided therapy on Slicer platform|
|[NIH R01MH112748](https://reporter.nih.gov/search/-oPV-FDakki3bPslkjMUwg/project-details/10053340)|High-accuracy brain segmentation tools|Bouix, Sylvain|2017-12-01|2022-10-31|High Resolution, Comprehensive Atlases of the Human Brain Morphology|
|[NIH R44DK115332](https://www.sbir.gov/sbirsearch/detail/1683213)|Renal Biopsy            |Enquobahrie, Andinet A.|2019-08-13|2021-07-31|Advanced virtual simulator for real-time ultrasound-guided renal biopsy training|
|ICEX 0202101723|SlicerVR Collaboration|Juan Ruiz Alzola|2021-01-16|2021-12-31|Sistema Virtual Colaborativa Aplicación Médicas|
|[NIH R01EB025212](https://govtribe.com/award/federal-grant-award/project-grant-r01eb025212)|Software annotation     |Enquobahrie, Andinet A.|2019-07-02|2021-03-31|Software for Practical Annotation and Exchange of Virtual Anatomy|
|CANARIE RS319|SlicerIGT|Fichtinger, Gabor|2019-10-01|2020-09-30|Canadian Research Software Infrastructure|
|CANARIE RS214|SlicerRT|Fichtinger, Gabor|2017-07-01|2020-09-30|Canadian Research Software Infrastructure|
|CHOP                      |SlicerHeart|Matthew Jolley|2015-08-15|2020-08-15|Pediatric cardiac valve modeling, Children Hospital of Philadelphia|
|[MAC/1.1b/098](https://mt4sd.ulpgc.es/en/macbioidi-project/)|MACBIOLDI|Juan Ruiz Alzola|2017-01-01|2020-09-30|Medical Technology for Sustainable Development (MedTec4SusDev)|
|[NIH 2P41EB015898](https://projectreporter.nih.gov/project_info_details.cfm?aid=8855115&icde=27026518)|DiffusionMRI            |Tempany, Clare M|2004-04-01|2020-06-30|Image Guided Therapy Center|
|[NIH 5P41EB015898](https://projectreporter.nih.gov/project_info_description.cfm?aid=9125821&icde=31485478&ddparam=&ddvalue=&ddsub=&cr=2&csb=default&cs=ASC)|National Center for Image Guided Therapy|Clare Tempany|2004-04-01|2020-06-30|Use of Slicer in a wide array of image-guided therapy research for prostate cancer, neurosurgery, and image naviation|
|[NIH 1R01EB021391](https://projectreporter.nih.gov/project_info_description.cfm?aid=9123966&icde=31459429&ddparam=&ddvalue=&ddsub=&cr=2&csb=default&cs=ASC)|Shape                   |Paniagua, Beatriz|2016-09-19|2020-06-30|SHAPE ANALYSIS TOOLBOX FOR MEDICAL IMAGE COMPUTING PROJECTS|
|[NIH U24CA194354](https://projectreporter.nih.gov/project_info_description.cfm?aid=8875289&icde=27050248)|Slicer-Radiomics-U24    |Aerts, Hugo|2015-04-01|2020-03-31|Quantitative Radiomics System Decoding the Tumor Phenotype|
|[NIH 1U01CA190234](https://projectreporter.nih.gov/project_info_description.cfm?aid=8799943&icde=27026470)|Slicer-Radiomics-U01    |Aerts, Hugo|2015-01-01|2019-12-01|Genotype And Imaging Phenotype Biomarkers In Lung Cancer|
|[NIH 5U24CA180924](https://projectreporter.nih.gov/project_info_details.cfm?aid=9127923&icde=31460433&ddparam=&ddvalue=&ddsub=&cr=2&csb=default&cs=ASC)|Spatially mapped molecular data analysis|Joel Saltz|2014-09-01|2019-08-31|Tools to Analyze Morphology and Spatially Mapped Molecular Data|
|[NIH 5R01CA184354](https://projectreporter.nih.gov/project_info_description.cfm?aid=8828624&icde=27036841&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|NIRFAST (Dartmouth)     |Davis, Scott C.|2014-04-01|2019-02-28|Mri Fluorescence Tomography For Quantifying Tumor Receptor Concentration In Vivo|
|[NIH R43DE027595](https://projectreporter.nih.gov/project_info_description.cfm?aid=9465772&icde=36620728&ddparam=&ddvalue=&ddsub=&cr=2&csb=default&cs=ASC&pball=)|VROrthognathic          |Paniagua, Beatriz|2017-09-07|2018-09-06|High-Fidelity Virtual Reality Trainer for Orthognathic Surgery|
|[NIH 1R21DE025306](https://projectreporter.nih.gov/project_info_description.cfm?aid=9111256&icde=31459429&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|CMF                     |Paniagua, Beatriz|2016-09-01|2018-08-31|TEXTURAL BIOMARKERS OF ARTHRITIS FOR THE SUBCHONDRAL BONE IN THE TEMPOROMANDIBULAR JOINT|
|[NIH 1U01NS082086](https://projectreporter.nih.gov/project_info_description.cfm?aid=8462842&icde=27164806&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|HD_SHAPEANALSS          |Gerig, Guido|2012-09-28|2018-08-31|4D Shape Analysis for Modeling Spatiotemporal Change Trajectories in Huntington’s|
|[NIH U24 CA180918](https://projectreporter.nih.gov/project_info_description.cfm?aid=8911287&icde=27026906)|[QIICR]((https://qiicr.org))|Ron Kikinis, Andrey Fedorov|2013-09-04|2018-08-31|Quantitative Image Informatics for Cancer Research (QIICR)|
|[NIH 5R01NS055903](https://projectreporter.nih.gov/project_info_description.cfm?aid=8900362&icde=27164764&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|HD_KIDS                 |Nopoulos, Peggy|2009-03-01|2018-07-31|Growth and Development of the Striatum in Huntington's Disease|
|[NIH 1U01CA199459](https://projectreporter.nih.gov/project_info_description.cfm?aid=8971083&icde=27026834)|SlicerDMRI Diffusion MRI|O'Donnell, Lauren Jean|2015-09-22|2018-07-31|Open Source Diffusion MRI Technology For Brain Cancer Research|
|[NIH 5R01EB020667-02](https://projectreporter.nih.gov/project_info_description.cfm?aid=9100712&icde=34329960)|OpenIGTLink             |Tokuda, Junichi|2015-07-01|2018-06-30|OpenIGTLink: A Network Communication Interface for Closed-Loop Image-Guided Interventions|
|[NIH 5P41EB015902](https://projectreporter.nih.gov/project_info_description.cfm?aid=8890837&icde=27036647&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|DiffusionMRI            |Kikinis, Ron|2013-08-01|2018-05-31|Neuroimaging Analysis Center (Nac)|
|[NIH 2R42HD081712](https://projectreporter.nih.gov/project_info_description.cfm?aid=9141675&icde=31459353&ddparam=&ddvalue=&ddsub=&cr=5&csb=default&cs=ASC)|Craniosynostosis        |Linguraru, Marius George|2016-05-01|2018-04-30|IMAGE-GUIDED PLANNING SYSTEM FOR SKULL CORRECTION IN CHILDREN WITH CRANIOSYNOSTOSIS: PHASE II|
|[NIH R01CA160902](https://projectreporter.nih.gov/project_info_description.cfm?aid=8817256&icde=27036729&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|DWI                     |Maier, Stephan E|2012-04-01|2018-02-28|Advancement And Validation Of Prostate Diffusion And Spectroscopic Mri|
|[NIH 1R01DE024450](https://projectreporter.nih.gov/project_info_description.cfm?aid=8576556&icde=18353487)|CMF                     |Cevidanes, Lucia|2013-09-10|2017-08-31|Quantification Of 3D Bony Changes In Temporomandibular Joint Osteoarthritis|
|[NIH 2R42CA167907](https://projectreporter.nih.gov/project_info_description.cfm?aid=8979242&icde=27036988&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|PET/CT Calibration Phantom|Kinahan, Paul E|2012-05-01|2017-07-31|Calibrated Methods For Quantitative Pet/Ct Imaging Phase Ii|
|[NIH R42CA167907](https://projectreporter.nih.gov/project_info_description.cfm?aid=8979242&icde=27036988&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|PET/CT Calibration Phantom|Kinahan, Paul E.|2012-05-01|2017-07-01|Calibrated Methods For Quantitative Pet/Ct Imaging Phase Ii|
|NA                       |HD_TRACKON              |Tabrizi, Sarah|2012-01-01|2016-12-31|TRACK-ON HD|
|CCO ACRU                 |SlicerRT                |Fichtinger, Gabor|2011-01-01|2016-12-31|Cancer Care Ontario Applied Cancer Research Unit, Canada|
|CCO OCAIRO               |SlicerRT                |Jaffray, David|2011-01-01|2016-12-31|Ontario Consortium for Adaptive Interventions in Radiation Oncology, Canada|
|NA                       |HD_TRAJECTORY           |Kim, Eun Young|2014-11-01|2016-10-31|Developing a Robust Segmentation Pipeline That Allows for Consistent Trajectory Estimation of HD Gene Positive Individuals Across Multiple Longitudinal MRI Sites|
|[NIH 1R41HD081712](https://projectreporter.nih.gov/project_info_description.cfm?aid=8778815&icde=27036063&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASCIMAGE-GUIDED)|Craniosynostosis        |Linguraru, Marius George|2014-09-26|2016-08-31|Image-Guided Planning System For Skull Correction In Children With Craniosynostos|
|[NIH 5R01NS040068](https://projectreporter.nih.gov/project_info_description.cfm?aid=8338456&icde=27164778&ddparam=&ddvalue=&ddsub=&cr=4&csb=default&cs=ASC)|HD_PREDICT              |Paulsen, Jane|2000-08-01|2016-08-31|Neurobiological Predictors of Huntington's Disease (PREDICT-HD)|
|[NIH 3R42CA153488](https://projectreporter.nih.gov/project_info_description.cfm?aid=8863934&icde=27037113&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|PET-CT guided needle biopsy|Cleary, Kevin R.|2012-09-01|2016-08-01|Improving Liver Lesion Biopsy In The Ct Suite Through Fusion With Pet Images|
|[NIH 1R43DE024334](https://projectreporter.nih.gov/project_info_description.cfm?aid=8710950&icde=27036891)|OrthognathicTrac        |Enquobahrie, Andinet A.|2014-08-05|2016-07-31|Real-Time Image Guidance For Improved Orthognathic Surgery|
|[NIH 1R01EB014947](https://projectreporter.nih.gov/project_info_description.cfm?aid=8272742&icde=13552329)|PediatricRadiologicDecisionSupport|Murphy, Shawn N|2012-08-01|2016-07-31|Mi2B2 Enabled Pediatric Radiological Decision Support|
|[NIH 5R01CA111288 ](https://projectreporter.nih.gov/project_info_description.cfm?aid=8906771&icde=27026518)|ProstateBRP             |Tempany, Clare M.|2004-12-01|2016-07-01|Enabling Technologies For Mri-Guided Prostate Interventions|
|[NIH 5U01CA151261](https://projectreporter.nih.gov/project_info_description.cfm?aid=8707214&icde=27026645)|ProstateQIN             |Fennessy, Fiona|2010-09-01|2016-07-01|Quantitative Mri Of Prostate Cancer As A Biomarker And Guide For Treatment|
|[NIH 1U01NS082074](https://projectreporter.nih.gov/project_info_description.cfm?aid=8596213&icde=27164895&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|HD_GENETICS             |Calhoun/Turner|2013-07-01|2016-06-30|Imaging and Genetics in Huntington's Disease|
|[NIH 1U01NS083173](https://projectreporter.nih.gov/project_info_description.cfm?aid=8529927&icde=27164835&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|HD_PET                  |Feigin, Andrew|2013-07-01|2016-06-30|Brain Network Imaging: A Novel Biomarker for Preclinical Huntington’s Disease|
|[NIH 1R01CA170665](https://projectreporter.nih.gov/project_info_description.cfm?aid=8384153&icde=27037397&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|[TubeTK](https://tubetk.org)|Dayton, Paul A.|2012-09-01|2016-06-01|Micro-Tumor Detection By Quantifying Tumor-Induced Vascular Abnormalities|
|[NIH 1U01NS083223](https://projectreporter.nih.gov/project_info_description.cfm?aid=8652000&icde=27164795&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|HD_WHITEMATTER          |Westin, Carl-Fredrik|2014-01-01|2015-12-31|Characterization of White Matter in Huntington’s Disease Using Diffusion MRI|
|CCO RC                    |SlicerRT               |Fichtinger, Gabor|2010-01-01|2015-12-31|Cancer Care Ontario Research Chair, Canada|
|[NIH 1U01NS082083](https://projectreporter.nih.gov/project_info_description.cfm?aid=8462829&icde=27164863&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|HD_FMRI_DWI             |Rao, Stephen Mark|2012-09-26|2015-08-31|Functional Connectivity in Premanifest Huntington’s Disease|
|[NIH R41CA196565](https://projectreporter.nih.gov/project_info_description.cfm?aid=8905274&icde=27036277&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|Duke Prostate Registration|Palmeri, Mark L. & McCormick, Matthew M.|2015-04-01|2015-04-01|Prostate Cancer Assessment Via Integrated 3D Arfi Elasticity Imaging And Multi-Parametric Mri|
|[NIH 1R43EB016621](https://projectreporter.nih.gov/project_info_description.cfm?aid=8472102&icde=27037328&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|[TubeTK](https://tubetk.org)|Aylward, Stephen R.|2013-05-01|2015-04-01|In-Field Fast Procedure Support And Automation|
|[NIH 1R41NS081792](https://projectreporter.nih.gov/project_info_description.cfm?aid=8453963&icde=27037364&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|[TubeTK](https://tubetk.org)|Aylward, Stephen R.|2013-01-01|2014-12-01|Multimodality Image-Based Assessment System For Traumatic Brain Injury|
|[NIH 2R42CA153488](https://projectreporter.nih.gov/project_info_description.cfm?aid=8390856&icde=27037039&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|PET-CT guided needle biopsy|Cleary, Kevin R.|2012-09-01|2014-08-01|Improving Liver Lesion Biopsy In The Ct Suite Through Fusion With Pet Images|
|[NIH 1R43CA165621](https://projectreporter.nih.gov/project_info_description.cfm?aid=8252988&icde=27037450&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|[TubeTK](https://tubetk.org)|Aylward, Stephen R|2012-12-01|2014-08-01|Quantitative Ultrasound Analysis Of Vascular Morphology For Cancer Assessment|
|[NIH 5U01NS082085](https://projectreporter.nih.gov/project_info_description.cfm?aid=8462830&icde=27164813&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|HD_SUBCORTICAL_SHAPE    |Miller, Michael & Ross, Christopher|2012-09-26|2014-07-31|Basal Ganglia Shape Analysis and Circuitry in Huntington's Disease|
|[NIH 5U54EB005149](https://projectreporter.nih.gov/project_info_description.cfm?aid=8501010&icde=27164945&ddparam=&ddvalue=&ddsub=&cr=5&csb=default&cs=ASC)|HD_DWI                  |Kikinis, Ron|2010-09-30|2014-06-30|National Alliance for Medical Image Computing|
|[NIH 5R01NS054893](https://projectreporter.nih.gov/project_info_description.cfm?aid=8077226&icde=27164732&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|HD_FMRI                 |Paulsen, Jane|2007-05-15|2013-04-30|Cognitive and Functional Brain Changes in Preclinical Huntington's Disease (HD)|
|[NIH R41CA153488](https://projectreporter.nih.gov/project_info_description.cfm?aid=7999618&icde=27037084&ddparam=&ddvalue=&ddsub=&cr=1&csb=default&cs=ASC)|PET-CT guided needle biopsy|Cleary, Kevin R.|2010-07-01|2012-06-01|Improving Liver Lesion Biopsy In The Ct Suite Through Fusion With Pet Images|

## Commercial Use

We invite commercial entities to use 3D Slicer.

### Slicer's License makes Commercial Use Available

- 3D Slicer is a free open source software distributed under a BSD style license.
- The license does not impose restrictions on the use of the software.
- 3D Slicer is NOT FDA approved. It is the users responsibility to ensure compliance with applicable rules and regulations.
- For details, please see the 3D Slicer Software License Agreement.

### Commercial Partners

- [Ebatinca SL](https://ebatinca.com/) is an international technology company in Las Palmas, Spain focused on technology for sustainable development.
- [Isomics](https://www.isomics.com/) uses 3D Slicer in a variety of academic and commercial research partnerships in fields such as planning and guidance for neurosurgery, quantitative imaging for clinical trials, clinical image informatics.
- [Kitware](https://www.kitware.com/opensource/slicer.html) focuses on solving the world’s most complex scientific challenges through customized software solutions. The company has a long history of contributing to open source platforms that serve as the foundation of many medical visualization and data processing applications. Kitware helps customers develop commercial products based on 3D Slicer and has used the platform to rapidly prototype solutions in nearly every aspect of medical imaging.
- [Pixel Medical](https://pixelmedical.ca) builds on and contributes to 3D Slicer to develop innovative medical software from idea to clinical prototype to finished product, and to support academic research projects. Areas of expertise include radiation therapy, image guided therapy, virtual & augmented reality, hardware & device support, and machine learning & artificial intelligence.

_Listed in alphabetical order._

### 3D Slicer based products

Many companies prefer not to disclose what software components they use in their products, therefore here we can only list a few commercial products that are based on 3D Slicer:

- Allen Institute for Brain Science: Allen Institute for Brain Science is developing Cell Locator, a Desktop application for manually aligning specimens to annotated 3D spaces. See more information on this [Kitware blog](https://blog.kitware.com/cell-locator-a-3d-slicer-based-desktop-application-that-manually-aligns-specimens-to-annotated-3d-spaces-developed-for-the-allen-institute-for-brain-science/).
- Radiopharmaceutical Imaging and Dosimetry: RPTDose, a 3D Slicer-based application that streamlines and integrates quantitative imaging analysis and dose estimation techniques to guide and optimize the use of radiopharmaceutical therapy agents in clinical trials. See more information on this [Kitware blog](https://blog.kitware.com/kitware-customer-highlight-radiopharmaceutical-imaging-and-dosimetry-llc-rapid/).
- [SonoVol](https://sonovol.com/) is developing a whole-body ultrasound imaging system for small animals. This start-up company arose from research in the Department of Biomedical Engineering at the University of North Carolina at Chapel Hill. See more information on this [Kitware blog](https://blog.kitware.com/kitware-customer-highlight-sonovol/).
- Xoran Technologies: Image-guided Platform for Deep Brain Stimulation Surgery. See more information on this [Kitware blog](https://blog.kitware.com/xoran-technologies-and-kitware-collaborate-on-image-guided-platform-for-deep-brain-stimulation-surgery/).
- [Xstrahl](https://www.xstrahl.com/) is developing a Small Animal Radiation Research Platform (SARRP) that uses 3D Slicer as its front-end application for radiation therapy beam placement and system control. See more information on this [Kitware blog](https://blog.kitware.com/kitware-customer-highlight-muriplan-from-xstrahl-a-3d-slicer-based-radiotherapy-treatment-planning-system/).

_Listed in alphabetical order._

## Contact us

It is recommended to post any questions, bug reports, or enhancement requests to the [Slicer forum](https://discourse.slicer.org).

Our online issue tracker is available [here](https://issues.slicer.org).

For commercial/confidential consulting, contact one of the [commercial partners](#commercial-partners).
