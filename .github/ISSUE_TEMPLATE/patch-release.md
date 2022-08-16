---
name: Patch Release
about: Track required steps for creating a Slicer patch release
title: Patch Release Slicer vX.Y.Z
labels: type:enhancement
assignees: ''

---

<!-- Before submitting this issue, replace X.Y.Z with the corresponding value in both the title and content -->

_Based on the state of the relevant sub-tasks, consider updating the issue description changing `:question:` (:question:)  into either `:hourglass:` (:hourglass:) or `:heavy_check_mark:` (:heavy_check_mark:)._

Checklist **Patch Release**:

* [ ] [Pin](https://docs.github.com/en/enterprise-cloud@latest/issues/tracking-your-work-with-issues/pinning-an-issue-to-your-repository) this issue
* [ ] `CMakeLists.txt`: [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#cmakeliststxt-update-the-slicer-version-information-for-the-release) the Slicer version information for the release.
* [ ] [Tag](https://github.com/Slicer/Slicer/wiki/Release-Process#tag-the-repository) the repository with `vX.Y.Z`.
* [ ] `CMakeLists.txt`: [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#cmakeliststxt-update-the-slicer-version-information-for-the-development) the Slicer version information for the development.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-release-scripts) release scripts.
* [ ] [Generate](https://github.com/Slicer/Slicer/wiki/Release-Process#generate-application-and-extension-packages) application and extension packages. See https://slicer.cdash.org/index.php?project=SlicerStable&date=YYYY-MM-DD
  * Linux: :question:
  * macOS: :question:
  * Windows: :question:
* [ ] Manually [sign the packages](https://github.com/Slicer/Slicer/wiki/Signing-Application-Packages)
  * Linux: NA
  * macOS: signed :question:, notarized :question:
  * Windows: :question:
* [ ] [Create](https://github.com/Slicer/Slicer/wiki/Release-Process#create-release-or-patch-release-on-slicer-packageskitwarecom) release `X.Y.Z` on [slicer-packages.kitware.com](https://slicer-packages.kitware.com/#folder/5f4474d0e1d8c75dfc705482).
* [ ] [Copy](https://slicer-package-manager.readthedocs.io/en/latest/faq.html#how-to-create-a-new-release-with-existing-uploaded-packages) application & extension packages from `draft` into the new release created on `slicer-packages.kitware.com`.
* [ ] Upload signed release packages in the release folder: For the relevant `Slicer_<os>_<arch>_<revision>` items, replace the content by uploading the signed release packages. The _Replace file content_ action is available in the `Files & links`.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-extensionstats-module) `ExtensionStats` module from
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-external-websites) external websites
* [ ] Update [Release Details](https://github.com/Slicer/Slicer/wiki/Release-Details) on the Slicer GitHub wiki.
* [ ] Unpin this issue
