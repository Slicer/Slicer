Contributing to Slicer
======================

There are many ways to contribute to Slicer, with varying levels of effort.  Do try to
look through the [documentation](https://www.slicer.org/wiki/Documentation/Nightly/Developers) first if something is unclear, and let us know how we can
do better.

  * Ask a question on the [slicer-devel email list](http://massmail.spl.harvard.edu/mailman/listinfo/slicer-devel)
  * Submit a feature request or bug, or add to the discussion on the [Slicer issue tracker](http://na-mic.org/Mantis)
  * Submit a [Pull Request](https://github.com/Slicer/Slicer/pulls) to improve Slicer or its documentation

We encourage a range of Pull Requests, from patches that include passing tests and
documentation, all the way down to half-baked ideas that launch discussions.

The PR Process, Circle CI, and Related Gotchas
----------------------------------------------

#### How to submit a PR ?

If you are new to Slicer development and you don't have push access to the Slicer
repository, here are the steps:

1. [Fork and clone](https://help.github.com/articles/fork-a-repo/) the repository.
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

* review and follow the style guidelines described
[on the wiki](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Style_Guide#Commits).

* add or update tests. There are plenty of existing tests to inspire from. The
  testing [how-tos](https://www.slicer.org/wiki/Documentation/Nightly/Developers/Tutorials/Testing) are
  also resourceful.

* consider potential backward compatibility breakage and discuss these on the
  mailing list. For example, update of ITK, Python, Qt or VTK version, change to
  core functionality, should be carefully reviewed and integrated. Ideally, several
  developers would test that the changes don't break extensions.

#### How to integrate a PR ?

Getting your contributions integrated is relatively straightforward, here
is the checklist:

* All tests pass
* Consensus is reached. This usually means that at least two reviewers approved
  the changes (or added a `LGTM` comment) and at least one business day passed
  without anyone objecting. `LGTM` is an acronym for _Looks Good to Me_.
* To accommodate developers explicitly asking for more time to test the
  proposed changes, integration time can be delayed by few more days.

Next, there are two scenarios:
* You do NOT have push access: A Slicer core developer will integrate your PR. If
  you would like to speed up the integration, do not hesitate to send a note on
  the mailing list.
* You have push access: Follow [Integrating topic from external contributor](https://www.slicer.org/wiki/Slicer:git-svn#Integrating_topic_from_external_contributor)
  instructions on the wiki.


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
will build, test and package Slicer application and all its extensions on Linux, MacOSX
and Windows. Results are published daily on [CDash](http://slicer.cdash.org/index.php?project=Slicer4)
and developers introducing changes introducing build or test failures are notified by
email.


#### Decision-making process

1. Given the topic of interest, initiate discussion on the [mailing list](http://massmail.spl.harvard.edu/mailman/listinfo/slicer-devel).

2. Identify a small circle of community members that are interested to study the
   topic in more depth.

3. Take the discussion off the general list, work on the analysis of options and
   alternatives, summarize findings on the wiki or similar. [Labs](https://www.slicer.org/wiki/Documentation/Labs)
   page are usually a good ground for such summary.

4. Announce on the mailing list the in-depth discussion of the topic for the
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

The Slicer community is inclusive and welcome anyone to work to become a core
developer and then a BDFL. This happens with hard work and approval of the existing
BDFL.
