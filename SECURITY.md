# Security

## Guidelines

Developers of 3D Slicer assume that the application is used in a trusted environment, by trusted people, on trusted data.

- **No reporting:** 3D Slicer does not send data to external servers without explicit user request. User requests include downloading sample data sets, extension packages, and retrieving information on the latest application and extensions updates.
- **No listening:** 3D Slicer does not open any ports listening on the network without explicit user request. User requests include starting a DICOM receiver or enabling the Slicer web API.
- **No code in data files:** 3D Slicer does not store code in data files that is executed automatically when the file is opened. This makes it harder to run arbitrary code when data is read. Note that it does not offer protection against more advanced exploits such as hand-crafting a file that uses buffer overflows to run malicious code.

Slicer core developers do their best following these policies and request all extension developers to do the same. Please report if any of these guidelines are not followed by either Slicer core or extensions.

## Pre-built Binaries

Binary packages for 3D Slicer application and extensions available at https://download.slicer.org and https://extensions.slicer.org/ are built by [Kitware](https://www.kitware.com/) from publicly available source code and data. Efforts are made to ensure the integrity of the packages during building and distribution from these servers. If any user does not trust this process, then [instructions are provided for building from source code](https://slicer.readthedocs.io/en/latest/developer_guide/build_instructions).

## Reporting Security Issues

If you believe you have found a potential security issue in Slicer, please report it to us as described below.

**Please do not report security vulnerabilities through public GitHub issues.**

Instead, send email to [slicer+security@discoursemail.com](mailto:slicer+security@discoursemail.com).

You should receive a response within 24 hours.

### Preferred Languages

We prefer all communications to be in English.

### Supported Versions

You can access the most up-to-date Slicer packages through the official [Slicer download](https://download.slicer.org/) website.

This table summarizes our general policy for updates to our binary distributions.

| Version | Support Status     | Update frequency                          |
| ------- | ------------------ | ----------------------------------------- |
| Preview | :white_check_mark: | Continual integration of features & fixes |
| Stable  | :white_check_mark: | Essential security fixes                  |

In general older releases are not updated.

> [!NOTE]
> There is no restriction on use, but Slicer is NOT approved for clinical use and the distributed application is intended for research use. Permissions and compliance with applicable rules are the responsibility of the user. For details on the license see [here](https://slicer.readthedocs.io/en/latest/user_guide/about.html#license).

### Scope

Reports may pertain to various aspects of the Slicer ecosystem, including:

- Slicer applications, modules and extensions
- Websites associated with Slicer

> [!NOTE]
> It's important to acknowledge that our impact on extension developers may be limited, and consequently, we disclaim responsibility for their actions. However, we are committed to forwarding reports to the best of our abilities.

> [!IMPORTANT]
> While we may not be able to offer legally binding commitments, we will do our best in addressing any reported security concerns.

> [!WARNING]
> While we greatly appreciate contributions to make Slicer more secure, please be aware that the Slicer community does not run a bug bounty program, and no financial compensation is offered for such contributions.
