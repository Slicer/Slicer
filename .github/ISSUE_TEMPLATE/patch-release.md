---
name: Patch Release
about: Track required steps for creating a Slicer patch release
title: Patch Release Slicer vX.Y.Z
labels: type:enhancement
assignees: ''

---

<!-- Before submitting this issue, replace X.Y.Z and YYYY-MM-DD with the corresponding values in both the title and content -->

_Based on the state of the relevant sub-tasks, consider updating the issue description changing `:question:` (:question:)  into either `:hourglass:` (:hourglass:) or `:heavy_check_mark:` (:heavy_check_mark:)._

--------------------------------------

Checklist **Installers Release**:

* [ ] [Pin](https://docs.github.com/en/enterprise-cloud@latest/issues/tracking-your-work-with-issues/pinning-an-issue-to-your-repository) this issue.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-release-in-progress-post) the `Release in progress` post.
* [ ] [Disable](https://github.com/Slicer/Slicer/wiki/Release-Process#enable-or-disable-regular-nightly-builds) regular nightly builds.
* [ ] `CMakeLists.txt`: [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#cmakeliststxt-update-the-slicer-version-information-for-the-release) the Slicer version information for the release.
* [ ] [Tag](https://github.com/Slicer/Slicer/wiki/Release-Process#tag-the-repository) the repository with `vX.Y.Z`.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-release-scripts) release scripts.
* [ ] [Generate](https://github.com/Slicer/Slicer/wiki/Release-Process#generate-application-and-extension-packages) application and extension packages. See https://slicer.cdash.org/index.php?project=SlicerStable&date=YYYY-MM-DD.
  * Linux: :question:
  * macOS: :question:
  * Windows: :question:
* [ ] Update [Release Details](https://github.com/Slicer/Slicer/wiki/Release-Details) on the Slicer GitHub wiki.

--------------------------------------

Checklist **Post Release**:

_These steps will take place in the week following **Installers Release** steps_

* [ ] [Re-enable](https://github.com/Slicer/Slicer/wiki/Release-Process#enable-or-disable-regular-nightly-builds) regular nightly builds.
* [ ] Manually [sign the packages](https://github.com/Slicer/Slicer/wiki/Signing-Application-Packages).
  * Linux: NA
  * macOS: signed :question:, notarized :question:
  * Windows: :question:
* [ ] [Create](https://github.com/Slicer/Slicer/wiki/Release-Process#create-release-or-patch-release-on-slicer-packageskitwarecom) release `X.Y.Z` on [slicer-packages.kitware.com](https://slicer-packages.kitware.com/#folder/5f4474d0e1d8c75dfc705482).
* [ ] [Copy](https://github.com/Slicer/Slicer/wiki/Release-Process#copy-draft-application--extension-packages-into-the-new-release-folder) application & extension packages from `draft` into the new release created on `slicer-packages.kitware.com`.
* [ ] [Upload](https://github.com/Slicer/Slicer/wiki/Release-Process#upload-signed-release-packages) signed release packages in the release folder.
  * Linux: NA
  * macOS: :question:
  * Windows: :question:
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-extensionstats-module) `ExtensionStats` module.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-external-websites) external websites.
* [ ] Finalize Slicer Announcement.
* [ ] Unpin this issue.
