# Coordinate systems

## Introduction

One of the issues while dealing with medical images and applications are the
differences between the coordinate systems. There are three coordinate systems
commonly used in imaging applications: a difference can be made between the
**world**, **anatomical** and the **image coordinate system**.

The following figure illustrates the three spaces and their corresponding
axes:

![coordinate_systems](https://github.com/Slicer/Slicer/releases/download/docs-resources/coordinate_systems.png){w=600px align=center}

Each coordinate system serves one purpose and represents its data in
a particular way.

Anatomy image based on an [image shared by the My MS organization](https://my-ms.org/mri_planes.htm).

Note that Chand John of Stanford created a [detailed presentation about the way coordinates are handled in Slicer](https://www.na-mic.org/w/img_auth.php/3/3f/Coordinate_Systems_Demystified.ppt).

### World coordinate system

The world coordinate system is typically a Cartesian coordinate system in
which a model (e.g. a MRI scanner or a patient) is positioned. Every model has
its own coordinate system but there is only one world coordinate system to
define the position and orientation of each model.

### Anatomical coordinate system

The most important model coordinate system for medical imaging techniques is
the anatomical space (also called patient coordinate system). This space
consists of three planes to describe the standard anatomical position of a
human:

- the *axial plane* is parallel to the ground and separates the head
  (Superior) from the feet (Inferior).
- the *coronal plane* is perpendicular to the ground and separates the front
  (Anterior) from the back (Posterior).
- the *sagittal plane* is perpendicular to the ground and separates the Left from the Right.

From these planes it follows that all axes have their notation in a positive
direction (e.g. the negative Superior axis is represented by the Inferior axis).

The anatomical coordinate system is a continuous three-dimensional space in
which an image has been sampled. In neuroimaging, it is common to define this
space with respect to the human whose brain is being scanned. Hence, the 3D
basis is defined along the anatomical axes of anterior-posterior,
inferior-superior, and left-right.

However different medical applications use different definitions of this 3D
basis. Most common are the following bases:

- LPS (Left, Posterior, Superior) is used in DICOM images

$$
LPS = \begin{Bmatrix}
\text{from right towards left} \\
\text{from anterior towards posterior} \\
\text{from inferior towards superior}
\end{Bmatrix}
$$

- RAS (Right, Anterior, Superior) is similar to LPS with the first two axes
  flipped

$$
RAS = \begin{Bmatrix}
\text{from left towards right} \\
\text{from posterior towards anterior} \\
\text{from inferior towards superior}
\end{Bmatrix}
$$

Thus, the only difference between the two conventions is that the sign of the
first two coordinates is inverted.

Both bases are equally useful and logical. It is just necessary to know to
which basis an image is referenced.

### Image coordinate system

The image coordinate system describes how an image was acquired with respect
to the anatomy. Medical scanners create regular, rectangular arrays of points
and cells which start at the upper left corner. The $i$ axis increases to the
right, the $j$ axis to the bottom and the $k$ axis backwards.

In addition to the intensity value of each voxel $(i j k)$ the origin and
spacing of the anatomical coordinates are stored too.

- The origin represents the position of the first voxel (0, 0, 0) in the
  anatomical coordinate system, e.g. (100, 50, -25) mm.
- The spacing specifies the distance between voxels along each axis, e.g.
  (1.5, 0.5, 0.5) mm.

The following 2D example shows the meaning of origin and spacing:

![image_coordinates](https://github.com/Slicer/Slicer/releases/download/docs-resources/coordinate_systems_image_coordinates.png){w=300px align=center}

Using the origin and spacing, the corresponding position of each (image
coordinate) voxel in anatomical coordinates can be calculated.

## Image transformation

The transformation from an image space vector $(i j k)'$ to an anatomical
space vector $\vec{x}$ is an affine transformation, consists of a linear
transformation $\mathbf{A}$ followed by a translation $\vec{t}$.

$$
\vec{x} = A \begin{pmatrix} i & j & k \end{pmatrix}' + \vec{t}
$$

The transformation matrix $\mathbf{A}$ is a $3 \times 3$ matrix and carries
all information about space directions and axis scaling.

$\vec{t}$ is a $3 \times 1$ vector and contains information about the
geometric position of the first voxel.

$$
\begin{pmatrix} x_1 \\ x_2 \\ x_3 \end{pmatrix} =
\begin{pmatrix} A_{11} & A_{12} & A_{13} \\ A_{21} & A_{22} & A_{23} \\ A_{31} & A_{32} & A_{33} \end{pmatrix}
\begin{pmatrix} i \\ j \\ k \end{pmatrix} + \begin{pmatrix} t_1 \\ t_2 \\ t_3 \end{pmatrix}
$$

The last equation shows that the linear transformation is performed by a
matrix multiplication and the translation by a vector addition. To represent
both, the transformation and the translation, by a matrix multiplication an
augmented matrix must be used. This technique requires that the matrix
$\mathbf{A}$ is augmented with an extra row of zeros at the bottom, an extra
column-the translation vector-to the right, and a $1$ in the lower right
corner. Additionally, all vectors have to be written as homogeneous
coordinates, which means that a $1$ is augmented at the end.

$$
\begin{pmatrix} x_1 \\ x_2 \\ x_3 \\ 1 \end{pmatrix} =
\begin{pmatrix} A_{11} & A_{12} & A_{13} & t_1 \\ A_{21} & A_{22} & A_{23} & t_2 \\
                A_{31} & A_{32} & A_{33} & t_3 \\ 0 & 0 & 0 & 1 \end{pmatrix}
\begin{pmatrix} i \\ j \\ k \\ 1 \end{pmatrix}
$$

Depending on the used anatomical space (LPS or RAS) the $4 \times 4$ matrix is
called **IJKtoLPS**- or **IJKtoRAS**-matrix, because it represents the
transformation from IJK to LPS or RAS.

## 2D example or calculating an *IJtoLS*-matrix

The following figure shows the anatomical space with an L(P)S basis on the
left and the corresponding image coordinates on the right.

![IJtoLS](https://github.com/Slicer/Slicer/releases/download/docs-resources/coordinate_systems_IJtoLS.png){w=550px align=center}

The origin (the coordinates of the first pixel in anatomical space) is
(50, 300) mm and the spacing (the distance between two pixels) is
(50, 50) mm.

As this is a 2D example $\mathbf{A}$ is a $2 \times 2$ matrix and $\vec{t}$ a
$2 \times 1$ vector. Therefore, the equation of the affine transformation is:

$$
\begin{pmatrix} L \\ S \\ 1 \end{pmatrix} =
\begin{pmatrix} A_{11} & A_{12} & t_1 \\ A_{21} & A_{22} & t_2 \\ 0 & 0 & 1 \end{pmatrix}
\begin{pmatrix} i \\ j \\ 1 \end{pmatrix}
$$

By multiplying the **IJtoLS**-matrix and the vector of the right side, the
following product will be obtained:

![matrix_multiplication](https://github.com/Slicer/Slicer/releases/download/docs-resources/coordinate_systems_matrix_multiplication.png){align=center}

The last equation and the matrix product show that a total of 6 unknown
variables $(A_{11}, A_{12}, A_{21}, A_{22}, t_1, t_2)$ have to be determined.
The knowledge of origin and spacing however allows the following relations
between image and anatomical space:

$$
\begin{pmatrix} L \\ S \end{pmatrix} \equiv \begin{pmatrix} i \\ j \end{pmatrix} \qquad
\begin{pmatrix} 50 \\ 300 \end{pmatrix} \equiv \begin{pmatrix} 0 \\ 0 \end{pmatrix} \qquad
\begin{pmatrix} 100 \\ 300 \end{pmatrix} \equiv \begin{pmatrix} 1 \\ 0 \end{pmatrix} \qquad
\begin{pmatrix} 50 \\ 250 \end{pmatrix} \equiv \begin{pmatrix} 0 \\ 1 \end{pmatrix} \qquad \dots
$$

Thus, at least six equations can be derived:

$$
\begin{align*}
50 &= A_{11} \cdot 0 + A_{12} \cdot 0 + t_1 \cdot 1 \\
300 &= A_{21} \cdot 0 + A_{22} \cdot 0 + t_2 \cdot 1 \\
100 &= A_{11} \cdot 1 + A_{12} \cdot 0 + t_1 \cdot 1 \\
300 &= A_{21} \cdot 1 + A_{22} \cdot 0 + t_2 \cdot 1 \\
50 &= A_{11} \cdot 0 + A_{12} \cdot 1 + t_1 \cdot 1 \\
250 &= A_{21} \cdot 0 + A_{22} \cdot 1 + t_2 \cdot 1
\end{align*}
$$

As mentioned above, the translation $\vec{t}$ contains the information about
the geometric position of the first pixel and is therefore equivalent to the
origin. This result is also confirmed by the first equations.

The solution of the other equations leads to the following **IJtoLS**-matrix:

$$
IJtoLS = \begin{pmatrix} 50 & 0 & 50 \\ 0 & -50 & 300 \\ 0 & 0 & 1 \end{pmatrix}
$$

In the event that a R(A)S basis was used, just the left and anterior axis of
the anatomical space are flipped, and the image coordinate system appears in
the same way as in the L\(P\)S case.

![IJtoRS](https://github.com/Slicer/Slicer/releases/download/docs-resources/coordinate_systems_IJtoRS.png){w=550px align=center}

For this 2D example the **IJtoRS**-matrix would be:

$$
IJtoRS = \begin{pmatrix} -50 & 0 & 250 \\ 0 & -50 & 300 \\ 0 & 0 & 1 \end{pmatrix}
$$

This matrix looks very similar to the **IJtoLS**-matrix with 2 differences:

- The translation $\vec{t}$ has changed because of another origin.
- The right axis is flipped, so the first column of the **IJtoRS**-matrix
  has just an inverted sign.

## Coordinate system convention in Slicer

DICOM and most medical imaging software use the **LPS coordinate system** for
storing all data. The choice of origin is arbitrary because only relative
differences have meaning, so there is no universal standard, but it is often
set to some geometric center of the imaging system, or it is chosen to be
near the center of an object of interest.

Both LPS and RAS were in wide use in the early 2000s when development of
Slicer was started and Slicer developers chose to use the RAS coordinate
system. Historically scans by GE equipment used RAS while Siemens and others
used LPS. Since several GE researchers were early contributors to Slicer, RAS
was adopted for the internal representation.

Slicer still **uses RAS coordinate system for storing coordinate values
internally** for all data types, but for compatibility with other software, it
**assumes that all data in files are stored in LPS coordinate system** (unless
the coordinate system in the file is explicitly stated to be RAS). To achieve
this, whenever Slicer reads or writes a file, it may need to flip the sign of
the first two coordinate axes to convert the data to RAS coordinate system.

### Relations to other software/conventions

#### ITK

[ITK](https://itk.org/) uses the LPS convention.

#### Using MATLAB to map Slicer RAS coordinates (e.g. fiducials) to voxel space of a NIfTI Image

To extract the "voxel to world" transformation matrix from a NIFTI file's
header (entry: `qto_xyz:1-4`) in MATLAB:

```Matlab
d = inv(M) * [ R A S 1 ]'
```

where `M` is the matrix and `R A S` are coordinates in Slicer, then `d` gives
a vector of voxel coordinates.

(Solution courtesy of András Jakab, University of Debrecen)

## References

- https://people.cs.uchicago.edu/~glk/unlinked/nrrd-iomf.pdf
- http://www.grahamwideman.com/gw/brain/orientation/orientterms.htm
- https://nifti.nimh.nih.gov/nifti-1/documentation/faq
- https://teem.sourceforge.net/nrrd/format.html
- [DICOM 2013 PS3.3 Image Position and Image Orientation](https://dicom.nema.org/dicom/2013/output/chtml/part03/sect_C.7.html#sect_C.7.6.2.1.1)
