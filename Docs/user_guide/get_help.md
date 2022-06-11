# Get Help

[Contact the Slicer community or commercial partners](about.md#contact-us) if you have any questions, bug reports, or enhancement requests - following the guidelines described below.

## I need help in using Slicer

- You can start with typing your question into Google web search. There is a good chance that your question has been asked and answered before and all questions ever asked about Slicer are publicly available and indexed by Google. Most up-to-date information sources are the [Slicer forum](https://discourse.slicer.org) and [Slicer documentation on read-the-docs](https://slicer.readthedocs.io/). Google may find older discussions on former Slicer mailing lists and wiki pages, which may or may not be exactly accurate for the current version of Slicer, but may still provide useful hints.
- Try your best to sort out the issue by reading [documentation](https://slicer.readthedocs.io), [portfolio of training materials](https://www.slicer.org/wiki/Documentation/Nightly/Training), and checking error logs (in application menu bar: View->Error log).
- If you are still unclear about what to do: [ask a question on the Slicer Forum](https://discourse.slicer.org). In addition to describing the specific question, it helps if you describe the context of your question (who you are, what you are working on, why it is important, what is the overall goal of your project). Knowing more about you and your project increases the chance that somebody volunteers to answer the question and you may get a more relevant answer.

## I want to report a problem

If you are not sure if Slicer behaves incorrectly or you are not using it properly then [ask about it on the Slicer Forum](https://discourse.slicer.org/c/support) (in the `Support` category). If you are _sure_ that Slicer is not working as intended then [submit a bug report in the Slicer issue tracker](https://github.com/Slicer/Slicer/issues/new?assignees=&labels=type%3Abug&template=bug_report.md).

In your question/report provide all the information that is described in the [bug reporting template](https://github.com/Slicer/Slicer/blob/master/.github/ISSUE_TEMPLATE/bug_report.md#summary).

:::{tip}

Don't be anonymous: real people trying hard to solve real problems are more likely to get valuable help. If you tell about yourself and your project then it may get more attention and the problem may be resolved sooner.

::: 

## I would like to request enhancement or new feature

First search on the [Slicer forum](https://discourse.slicer.org) and in the [Slicer issue tracker](https://github.com/Slicer/Slicer/issues?q=is%3Aissue+is%3Aopen+label%3Atype%3Aenhancement) to see if someone asked for this feature already. If you find a very similar request, tell us that you are interested in it too by adding a comment and/or adding a "thumbs-up" to the top post.

If you cannot find a similar feature request, then write a post in the [Feature request category](https://discourse.slicer.org/c/support/feature-requests) to discuss it with Slicer developers and community members.

:::{tip}

If you write about yourself and your project then there is a higher chance that your request will be worked on. Describe what assistance you can offer for the implementation (your own time, funding, etc.).

:::

## I would like to let the Slicer community know, how Slicer helped me in my research

Please send us the citation for your paper posting in [Community category in Slicer forum](https://discourse.slicer.org/c/community/).

Background: Funding for Slicer is provided through competitive mechanisms primarily by the United States government and to a lesser extent through funding from other governments. The justification for those resources is that Slicer enables scientific work. Knowing about scientific publications enabled by Slicer is a critical step in this process. Given the international nature of the Slicer community, the nationality of the scientists is not important. Every good paper counts.

## Troubleshooting

### Slicer application does not start

- Your computer CPU or graphics capabilities may not meet [minimum system requirements](getting_started.md#system-requirements).
  - Updating your graphics driver may fix some problems, but if that does not help and you have an old computer then you may need to upgrade to a more recently manufactured computer or switch to a software renderer.
    A software renderer is particularly useful for running Slicer on a headless machine, such as a virtual machine at a cloud computing provider with strong CPU but no GPU, using Remote Desktop Protocol.

    ````{note}

    **Setting up software renderer on Windows:**

    - Download Mesa OpenGL driver from <https://github.com/pal1000/mesa-dist-win/releases> (MSVC version - mesa3d-X.Y.Z-release-msvc.7z).
    - Extract the archive and copy files from the x64 folder into the bin subfolder in the Slicer install tree.
    - Configure the rendere by setting environment variables then launch Slicer:

        ```txt
        set GALLIUM_DRIVER=swr
        set MESA_GL_VERSION_OVERRIDE=3.3COMPAT
        Slicer.exe
        ```

    This software renderer has been tested to work well on Windows virtual machines on Microsoft Azure.

    ````

- Slicer may not work if it is installed in a folder that has special characters in their name. Try installing Slicer in a path that only contains latin letters and numbers (a-z, 0-9).
- Your Slicer settings might have become corrupted
  - Try launching Slicer using `Slicer.exe --disable-settings` (if it fixes the problem, delete Slicer.ini and Slicer-<yourslicerversion>.ini files from your Slicer settings directory.
  - Rename or remove your Slicer settings directory (for example, `c:\Users\<yourusername>\AppData\Roaming\NA-MIC`). See instructions for getting the settings directory [here](settings.md#settings-file-location). Try to launch Slicer.
- There may be conflicting/incompatible libraries in your system path (most likely caused by installing applications that place libraries in incorrect location on your system). Check your system logs for details and report the problem.
  - On Windows:
    - Start Event Viewer (eventvwr.exe), select Windows Logs / Application, and find the application error. If there is a DLL loading problem a line similar to this will appear: `Faulting module path: <something>.dll`. If you found a line similar to this, then try the following workaround: Start a command window. Enter `set path=` to clear the path variable. Enter Slicer.exe to start Slicer. If Slicer starts successfully then you need to remove remove unnecessary items from the system path (or delete the libraries installed at incorrect locations).
    - If Slicer still does not work then collect some more information and report the problem:
      - Get DLL dependency information using Dependency Walker tool:
        - Download depends.exe from [here](https://www.dependencywalker.com/)
        - Run depends.exe using the Slicer launcher: `Slicer.exe --launch path\to\depends.exe "bin\SlicerApp-real.exe"`
        - In dependency walker: Make sure the full path of DLLs are shown (click View / Full paths if you only see the DLL names). Use File / Save as... => Comma Separated Values (*.csv) to save logs to a file.
      - Enable process loading logging using the sxstrace tool, start Slicer, and save the log file (see instructions [here](https://technet.microsoft.com/en-ca/library/hh875651.aspx))
  - On Linux:
    - Some linux versions require building your own kerberos and openssl as [described and tracked in this issue](https://github.com/Slicer/Slicer/issues/5663).
