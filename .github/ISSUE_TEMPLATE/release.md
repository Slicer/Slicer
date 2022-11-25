---
name: Release
about: Track required steps for creating a Slicer release
title: Release Slicer vX.Y
labels: type:enhancement
assignees: ''

---

<!-- Before submitting this issue, replace X.Y and YYYY-MM-DD with the corresponding values in both the title and content -->

_Based on the state of the relevant sub-tasks, consider updating the issue description changing `:question:` (:question:)  into either `:hourglass:` (:hourglass:) or `:heavy_check_mark:` (:heavy_check_mark:)._

--------------------------------------

Checklist **Installers Release**:

* [ ] [Pin](https://docs.github.com/en/enterprise-cloud@latest/issues/tracking-your-work-with-issues/pinning-an-issue-to-your-repository) this issue.
* [ ] Finalize fixes related to failed extensions. See dashboard [here](https://slicer.cdash.org/index.php?project=SlicerPreview&filtercount=1&showfilters=1&field1=builderrors&compare1=43&value1=0).
* [ ] [Announce](https://github.com/Slicer/Slicer/wiki/Release-Process#announce-release-process-in-progress) release is in progress.
* [ ] [Disable](https://github.com/Slicer/Slicer/wiki/Release-Process#enable-or-disable-regular-nightly-builds) regular nightly builds.
* [ ] `CMakeLists.txt`: [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#cmakeliststxt-update-the-slicer-version-information-for-the-release) the Slicer version information for the release.
* [ ] [Tag](https://github.com/Slicer/Slicer/wiki/Release-Process#tag-the-repository) the repository with `vX.Y.0`.
* [ ] `CMakeLists.txt`: [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#cmakeliststxt-update-the-slicer-version-information-for-the-development) the Slicer version information for the development.
* [ ] [Create](https://github.com/Slicer/Slicer/wiki/Release-Process#create-release-or-patch-release-on-slicer-packageskitwarecom) release `X.Y.0` on [slicer-packages.kitware.com](https://slicer-packages.kitware.com/#folder/5f4474d0e1d8c75dfc705482).
* [ ] [Tag and publish](https://github.com/Slicer/Slicer/wiki/Release-Process#tag-and-publish-slicerbuildenvironment-docker-image) SlicerBuildEnvironment docker image.
* [ ] [Create](https://github.com/Slicer/Slicer/wiki/Release-Process#create-maintenance-branch) maintenance branch called `X.Y` based on the first tag of the `X.Y` series.
* [ ] Update readthedocs.
  * [Activate](https://readthedocs.org/projects/slicer/versions/) the build of the `X.Y` maintenance branch in readthedocs.
  * Add a [Redirect](https://readthedocs.org/dashboard/slicer/redirects/) of type _Exact Redirects_ from `/en/vX.Y$rest` to `/en/X.Y`.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-release-scripts) release scripts.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-extensionsindex) ExtensionsIndex by creating the `X.Y` branch.
* [ ] Update CDash: Add `Extensions-X.Y-Nightly` group to https://slicer.cdash.org/index.php?project=SlicerStable.
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
* [ ] [Upload](https://github.com/Slicer/Slicer/wiki/Release-Process#upload-signed-release-packages) signed release packages in the release folder.
  * Linux: NA
  * macOS: :question:
  * Windows: :question:
* [ ] In the `X.Y.0` release folder, update each `Slicer_<os>_<arch>_<revision>` items changing the `pre_release` metadata from `true` to `false`.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-external-websites) external websites.
* [ ] [Update](https://github.com/Slicer/Slicer/wiki/Release-Process#update-extensionstats-module) `ExtensionStats` module.
* [ ] Finalize Slicer Announcement.
* [ ] Publish Slicer Announcement & update discourse (draft [here](https://drive.google.com/drive/u/0/folders/1ZN1iFUHrGvqWHtoH_qlNUsZWbxXwsAs-)).
* [ ] Re-target unresolved GitHub issues.
* [ ] Unpin this issue.
