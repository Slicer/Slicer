# Contributing to Slicer

There are many ways to contribute to Slicer, with varying levels of effort.  Do try to
look through the [documentation](https://slicer.readthedocs.io/en/latest/index.html) first if something is unclear, and let us know how we can
do better.

  * Ask a question on the [Slicer forum][slicer-forum]
  * Use [Slicer issues][slicer-issues] to submit a feature request or bug, or add to the discussion on an existing issue
  * Submit a [Pull Request](https://github.com/Slicer/Slicer/pulls) to improve Slicer or its documentation

We encourage a range of Pull Requests, from patches that include passing tests and
documentation, all the way down to half-baked ideas that launch discussions.

## Submitting a Pull Request

### How to submit a PR ?

If you are new to Slicer development and you don't have push access to the Slicer
repository, here are the steps:

1. [Fork and clone](https://docs.github.com/get-started/quickstart/fork-a-repo) the repository.
2. Run the developer setup script [`Utilities/SetupForDevelopment.sh`](https://github.com/Slicer/Slicer/blob/main/Utilities/SetupForDevelopment.sh).
3. Create a branch.
4. [Push](https://docs.github.com/get-started/using-git/pushing-commits-to-a-remote-repository) the branch to your GitHub fork.
5. Create a [Pull Request](https://github.com/Slicer/Slicer/pulls).

This corresponds to the `Fork & Pull Model` described in the [GitHub collaborative development](https://docs.github.com/pull-requests/collaborating-with-pull-requests/getting-started/about-collaborative-development-models)
documentation.

> [!TIP]
>
> When submitting a PR, the developers following the project will be notified. That
> said, to engage specific developers, you can add `Cc: @<username>` comment to notify
> them of your awesome contributions.
> Based on the comments posted by the reviewers, you may have to revisit your patches.


### How to efficiently contribute ?

We encourage all developers to:

* add or update tests. There are plenty of existing tests to inspire from. The
  testing [how-tos](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Tutorials/Testing) are
  also resourceful.

* consider potential backward compatibility breakage and discuss these on the
  [Slicer forum][slicer-forum]. For example, update of ITK, Python, Qt or VTK version, change to
  core functionality, should be carefully reviewed and integrated. Ideally, several
  developers would test that the changes don't break extensions.

### How to write commit messages ?

Write your commit messages using the standard prefixes for Slicer commit
messages:

  * `BUG:` Fix for runtime crash or incorrect result
  * `COMP:` Compiler error or warning fix
  * `DOC:` Documentation change
  * `ENH:` New functionality
  * `PERF:` Performance improvement
  * `STYLE:` No logic impact (indentation, comments)
  * `WIP:` Work In Progress not ready for merge

The body of the message should clearly describe the motivation of the commit
(**what**, **why**, and **how**). In order to ease the task of reviewing
commits, the message body should follow the following guidelines:

  1. Leave a blank line between the subject and the body.
  This helps `git log` and `git rebase` work nicely, and allows to smooth
  generation of release notes.
  2. Try to keep the subject line below 72 characters, ideally 50.
  3. Capitalize the subject line.
  4. Do not end the subject line with a period.
  5. Use the imperative mood in the subject line (e.g. `BUG: Fix spacing
  not being considered.`).
  6. Wrap the body at 80 characters.
  7. Use semantic line feeds to separate different ideas, which improves the
  readability.
  8. Be concise, but honor the change: if significant alternative solutions
  were available, explain why they were discarded.
  9. If the commit refers to a topic discussed on the [Slicer forum][slicer-forum], or fixes
  a regression test, provide the link. If it fixes a compiler error, provide a
  minimal verbatim message of the compiler error. If the commit closes an
  issue, use the [GitHub issue closing
  keywords](https://docs.github.com/issues/tracking-your-work-with-issues/linking-a-pull-request-to-an-issue).

Keep in mind that the significant time is invested in reviewing commits and
*pull requests*, so following these guidelines will greatly help the people
doing reviews.

Examples:

- ❌ `BUG: Check pointer validity before dereferencing` -> implementation detail, self-explanatory (by looking at the code)
- ✅ `BUG: Fix crash in Module X when clicking Apply button`
- ❌ `ENH: More work in qSlicerXModuleWidget` -> more work is too vague, qSlicerXModuleWidget is too low level
- ✅ `ENH: Add float image outputs in module X`
- ❌ `COMP: Typo in cmake variable` -> implementation detail, self-explanatory
- ✅ `COMP: Fix compilation error with Numpy on Visual Studio`

_These guidelines are largely inspired by Chris Beam's
[How to Write a Commit Message](https://chris.beams.io/posts/git-commit/)
post._

### How to integrate a PR ?

Getting your contributions integrated is relatively straightforward, here
is the checklist:

* All tests pass
* Consensus is reached. This usually means that at least one reviewer approved
  the changes and at least one business day passed
  without anyone objecting.
* To accommodate developers explicitly asking for more time to test the
  proposed changes, integration time can be delayed by few more days.

> [!TIP]
>
> If you do NOT have push access, a Slicer core developer will integrate your PR. If
> you would like to speed up the integration, do not hesitate to send a note on
> the [Slicer forum][slicer-forum].

### Automatic testing of pull requests

Every pull request (PR) is automatically tested using GitHub Actions each time a
commit is pushed. The GitHub interface prevents merging until all required CI workflows
have completed successfully, ensuring that changes meet the project’s testing and
quality standards.

The testing infrastructure was migrated from CircleCI to GitHub Actions, but the
principles described in the blog post [3D Slicer Improves Testing for Pull Requests Using Docker and CircleCI](https://blog.kitware.com/3d-slicer-improves-testing-for-pull-requests-using-docker-and-circleci/)
remain relevant for understanding the overall approach and concepts.

### Nightly tests

After changes are integrated, every evening at 10pm EST (3am UTC), Slicer build bots (aka factories)
will build, test and package the Slicer application and all its extensions on Linux, macOS
and Windows. Results are published daily on CDash:
- [Stable](https://slicer.cdash.org/index.php?project=SlicerStable)
- [Preview](https://slicer.cdash.org/index.php?project=SlicerPreview))

Developers that introduced changes resulting in build or test failures are notified by
email.


[slicer-forum]: https://discourse.slicer.org
[slicer-issues]: https://github.com/Slicer/Slicer/issues
