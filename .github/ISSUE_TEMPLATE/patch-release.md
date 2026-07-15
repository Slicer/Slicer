---
name: Patch Release
about: Track required steps for creating a Slicer patch release
title: Patch Release Slicer vX.Y.Z
labels: type:enhancement
assignees: ''

---

<!-- Before submitting this issue, replace X.Y.Z and YYYY-MM-DD with the corresponding values in both the title and content -->

Checklist **Installers Release**:

* [ ] [Pin](https://docs.github.com/en/enterprise-cloud@latest/issues/tracking-your-work-with-issues/pinning-an-issue-to-your-repository) this issue.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-release-in-progress-post) the `Release in progress` post.
* [ ] [Disable](https://github.com/Slicer/Slicer/wiki/Release-Process#enable-or-disable-regular-nightly-builds) regular nightly builds.
  * [ ] Linux
  * [ ] macOS
  * [ ] Windows
* [ ] `CMakeLists.txt`: [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#cmakeliststxt-update-the-slicer-version-information-for-the-release) the Slicer version information for the release.
* [ ] [Tag](https://github.com/Slicer/Slicer/wiki/Release-Process#tag-the-repository) the repository with `vX.Y.Z`.
* [ ] [Create](https://github.com/Slicer/Slicer/wiki/Release-Process#create-release-or-patch-release-on-slicer-packageskitwarecom) release `X.Y.Z` on [slicer-packages.kitware.com](https://slicer-packages.kitware.com/#folder/5f4474d0e1d8c75dfc705482).
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-release-scripts) release scripts.
* [ ] [Generate](https://github.com/Slicer/Slicer/wiki/Release-Process#generate-application-and-extension-packages) application and extension packages. See https://slicer.cdash.org/index.php?project=SlicerStable&date=YYYY-MM-DD.
  * [ ] Linux
  * [ ] macOS
  * [ ] Windows
* [ ] Update [Release Details](https://github.com/Slicer/Slicer/wiki/Release-Details) on the Slicer GitHub wiki.

--------------------------------------

Checklist **Post Release**:

_These steps will take place in the week following **Installers Release** steps_

* [ ] [Re-enable](https://github.com/Slicer/Slicer/wiki/Release-Process#enable-or-disable-regular-nightly-builds) regular nightly builds.
  * [ ] Linux
  * [ ] macOS
  * [ ] Windows
* [ ] Manually [sign the packages](https://github.com/Slicer/Slicer/wiki/Signing-Application-Packages).
  * [ ] macOS (sign and notarize)
  * [ ] Windows
* [ ] [Upload](https://github.com/Slicer/Slicer/wiki/Release-Process#upload-signed-release-packages) signed release packages in the release folder.
  * [ ] macOS
  * [ ] Windows
* [ ] In the `X.Y.Z` release folder, update each `Slicer_<os>_<arch>_<revision>` items changing the `pre_release` metadata from `true` to `false`.
  * [ ] Linux
  * [ ] macOS
  * [ ] Windows
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-extensionstats-module) `ExtensionStats` module.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-external-websites) external websites.
* [ ] [Generate release notes](https://github.com/Slicer/Slicer/wiki/Generating-Release-Notes) and prepare Discourse content.
* [ ] Finalize Slicer Announcement and release notes in HackMD (or equivalent shared document).
* [ ] Publish Slicer Announcement & release notes on Discourse.
* [ ] Unpin this issue.
