Contributing to Slicer
======================

There are many ways to contribute to Slicer, with varying levels of effort.  Do try to
look through the [documentation](https://www.slicer.org/wiki/Documentation/Nightly/Developers) first if something is unclear, and let us know how we can
do better.

  * Ask a question on the [Slicer forum][slicer-forum]
  * Use [Slicer issues][slicer-issues] to submit a feature request or bug, or add to the discussion on an existing issue
  * Submit a [Pull Request](https://github.com/Slicer/Slicer/pulls) to improve Slicer or its documentation

We encourage a range of Pull Requests, from patches that include passing tests and
documentation, all the way down to half-baked ideas that launch discussions.

The PR Process, Circle CI, and Related Gotchas
----------------------------------------------

#### How to submit a PR ?

If you are new to Slicer development and you don't have push access to the Slicer
repository, here are the steps:

1. [Fork and clone](https://help.github.com/articles/fork-a-repo/) the repository.
2. Run the developer setup script [`Utilities/SetupForDevelopment.sh`](https://github.com/Slicer/Slicer/blob/master/Utilities/SetupForDevelopment.sh).
3. Create a branch.
4. [Push](https://help.github.com/articles/pushing-to-a-remote/) the branch to your GitHub fork.
5. Create a [Pull Request](https://github.com/Slicer/Slicer/pulls).

This corresponds to the `Fork & Pull Model` mentioned in the [GitHub flow](https://guides.github.com/introduction/flow/index.html)
guides.

When submitting a PR, the developers following the project will be notified. That
said, to engage specific developers, you can add `Cc: @<username>` comment to notify
them of your awesome contributions.
Based on the comments posted by the reviewers, you may have to revisit your patches.


#### How to efficiently contribute ?

We encourage all developers to:

* add or update tests. There are plenty of existing tests to inspire from. The
  testing [how-tos](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Tutorials/Testing) are
  also resourceful.

* consider potential backward compatibility breakage and discuss these on the
  [Slicer forum][slicer-forum]. For example, update of ITK, Python, Qt or VTK version, change to
  core functionality, should be carefully reviewed and integrated. Ideally, several
  developers would test that the changes don't break extensions.

#### How to write commit messages ?

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
  keywords](https://help.github.com/en/articles/closing-issues-using-keywords).

Keep in mind that the significant time is invested in reviewing commits and
*pull requests*, so following these guidelines will greatly help the people
doing reviews.

These guidelines are largely inspired by Chris Beam's
[How to Write a Commit Message](https://chris.beams.io/posts/git-commit/)
post.

Examples:
  - Bad: `BUG: Check pointer validity before dereferencing` -> implementation detail, self-explanatory (by looking at the code)
  - Good: `BUG: Fix crash in Module X when clicking Apply button`
  - Bad: `ENH: More work in qSlicerXModuleWidget` -> more work is too vague, qSlicerXModuleWidget is too low level
  - Good: `ENH: Add float image outputs in module X`
  - Bad: `COMP: Typo in cmake variable` -> implementation detail, self-explanatory
  - Good: `COMP: Fix compilation error with Numpy on Visual Studio`


#### How to integrate a PR ?

Getting your contributions integrated is relatively straightforward, here
is the checklist:

* All tests pass
* Consensus is reached. This usually means that at least two reviewers approved
  the changes (or added a `LGTM` comment) and at least one business day passed
  without anyone objecting. `LGTM` is an acronym for _Looks Good to Me_.
* To accommodate developers explicitly asking for more time to test the
  proposed changes, integration time can be delayed by few more days.

* If you do NOT have push access, a Slicer core developer will integrate your PR. If
  you would like to speed up the integration, do not hesitate to send a note on
  the [Slicer forum][slicer-forum].


#### Automatic testing of pull requests

Every pull request is tested automatically using CircleCI each time you push a
commit to it. The Github UI will restrict users from merging pull requests until
the CI build has returned with a successful result indicating that all tests have
passed.

The testing infrastructure is described in details in the
[3D Slicer Improves Testing for Pull Requests Using Docker and CircleCI](https://blog.kitware.com/3d-slicer-improves-testing-for-pull-requests-using-docker-and-circleci/)
blog post.


#### Nightly tests

After changes are integrated, every evening at 10pm EST (3am UTC), Slicer build bots (aka factories)
will build, test and package the Slicer application and all its extensions on Linux, MacOSX
and Windows. Results are published daily on CDash ([Stable](http://slicer.cdash.org/index.php?project=Slicer4) & [Preview](http://slicer.cdash.org/index.php?project=SlicerPreview))
and developers that introduced changes resulting in build or test failures are notified by
email.


#### Decision-making process

1. Given the topic of interest, initiate discussion on the [Slicer forum][slicer-forum].

2. Identify a small circle of community members that are interested to study the
   topic in more depth.

3. Take the discussion off the general list, work on the analysis of options and
   alternatives, summarize findings on the wiki or similar. [Labs](https://www.slicer.org/wiki/Documentation/Labs)
   page are usually a good ground for such summary.

4. Announce on the [Slicer forum][slicer-forum] the in-depth discussion of the topic for the
   [Slicer Community hangout](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Meetings),
   encourage anyone that is interested in weighing in on the topic to join the
   discussion. If there is someone who is interested to participate in the discussion,
   but cannot join the meeting due to conflict, they should notify the leaders of
   the given project and identify the time suitable for everyone.

5. Hopefully, reach consensus at the hangout and proceed with the agreed plan.


*The initial version of these guidelines was established during the [winter
 project week 2017](http://www.na-mic.org/Wiki/index.php/2017_Winter_Project_Week/UpdatingCommunityForums).*

#### Benevolent dictators for life

The [benevolent dictators](https://en.wikipedia.org/wiki/Benevolent_dictator_for_life) can
integrate changes to keep the platform healthy and help interpret
or address conflict related to the contribution guidelines.


These currently include:

* Jean-Christophe Fillion-Robin
* Andras Lasso
* Steve Pieper

*Alphabetically ordered by last name.*

The Slicer community is inclusive and welcomes anyone to work to become a core
developer and then a BDFL. This happens with hard work and approval of the existing
BDFL.

[slicer-forum]: https://discourse.slicer.org
[slicer-issues]: https://issues.slicer.org