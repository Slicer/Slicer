#!/usr/bin/env python-real

import os
import sys

def main(input, sigma, output):

    import SimpleITK as sitk

    reader = sitk.ImageFileReader()
    reader.SetFileName(input)
    image = reader.Execute()

    pixelID = image.GetPixelID()

    gaussian = sitk.SmoothingRecursiveGaussianImageFilter()
    gaussian.SetSigma(sigma)
    image = gaussian.Execute(image)

    caster = sitk.CastImageFilter()
    caster.SetOutputPixelType(pixelID)
    image = caster.Execute(image)

    writer = sitk.ImageFileWriter()
    writer.SetFileName (output)
    writer.Execute (image)


if __name__ == "__main__":
    if len (sys.argv) < 4:
        print("Usage: TemplateKey <input> <sigma> <output>")
        sys.exit (1)
    main(sys.argv[1], float(sys.argv[2]), sys.argv[3])
