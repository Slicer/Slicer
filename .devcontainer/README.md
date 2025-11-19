# Dev Containers
Dev Containers leverage Docker to build container images for development and production. Multi-stage builds are used segment the build and runtime environments, while optimizing for container image size and build speed.

## Quick Start

To get started, follow the instructions below.

### Prerequisites

First, ensure your using a recent enough version of Docker Engine that supports [BuildKit](https://docs.docker.com/build/buildkit/). If you plan on running heavy graphical tasks locally, Hardware Acceleration for algorithms and 3D rendering is also recommended. While other compatible devcontainer tools may be used, Visual Studio Code is recommended for simplicity.

#### System Software
- [Docker Engine](https://docs.docker.com/engine/install/)
  - https://get.docker.com - simple universal install script
  - [Linux post-installation](https://docs.docker.com/engine/install/linux-postinstall/) - manage Docker as a non-root user
- [Git LFS](https://git-lfs.github.com/) - for managing large assets
  - Use for version controlling media such as figures 
  - Necessary for cloning example simulation files
- [NVIDIA Container Toolkit](https://github.com/NVIDIA/nvidia-container-toolkit) - optional for enabling Hardware Acceleration
  - [Installing the Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) - only necessary host running Docker Engine

#### Development Tools
- [Visual Studio Code](https://code.visualstudio.com/) - alternative to Dev Containers CLI
  - [Remote Development](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.vscode-remote-extensionpack) - via SSH, Tunnels, Containers, WSL
    - [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) - specific to just Containers
  - [Docker extension](https://marketplace.visualstudio.com/items?itemName=ms-azuretools.vscode-docker) - for introspecting Docker daemon
  - [Using SSH keys](https://code.visualstudio.com/remote/advancedcontainers/sharing-git-credentials#_using-ssh-keys) - sharing Git credentials with container
- [Dev Container CLI](https://github.com/devcontainers/cli) - alternative to VSCode
  - [Installation via NPM](https://github.com/devcontainers/cli?tab=readme-ov-file#npm-install) - for custom install setup
  - [Installation via VSCode](https://code.visualstudio.com/docs/devcontainers/devcontainer-cli) - for simple install setup
    - Note: CLI installed via VSCode is warped but bugged, install via NPM is recommended for now
    - https://github.com/devcontainers/cli/issues/799
- [GitHub CLI](https://cli.github.com/) - for interacting with PR Workflows
    - [Installation](https://github.com/cli/cli#installation) - specifically [for Linux](https://github.com/cli/cli/blob/trunk/docs/install_linux.md)
    - [Configuration](https://cli.github.com/manual/) - login authentication and setup

### Environment Setup

#### Configure GitHub

Configure credentials for the GitHub CLI using GitHub login with your account and browser like so:

```shell
$ gh auth login
? Where do you use GitHub? GitHub.com
? What is your preferred protocol for Git operations on this host? HTTPS
? Authenticate Git with your GitHub credentials? Yes
? How would you like to authenticate GitHub CLI? Login with a web browser
```

Then follow the prompts to authenticate and select the appropriate permissions:

<details><summary>Example auth login</summary>

```shell
! First copy your one-time code: ABCD-EFGH
Press Enter to open https://github.com/login/device in your browser... 
✓ Authentication complete.
- gh config set -h github.com git_protocol https
✓ Configured git protocol
✓ Logged in as <username>
```

- [GitHub CLI quickstart](https://docs.github.com/en/github-cli/github-cli/quickstart)

</details>

### Cloning, Building and Running

Next, recursively clone this repository, bake default image tags using buildx, and then simply run containers using the build docker image.

```shell
# Clone the git repository
git clone https://github.com/Slicer/Slicer.git

# Move into the repository
cd Slicer

# Bake the dever stage as a test
docker buildx bake dever

# Run container from image as a test
docker run -it --rm slicer:dever bash
```

### Launching Development Containers

Finally, use the CLI to bring up and exec into the Dev Container:

```shell
# To bring up a dev container
devcontainer up --workspace-folder .
# Or to bring up without using previous dev container
devcontainer up --workspace-folder . --remove-existing-container
# Or to bring up without using previous build cache
devcontainer up --workspace-folder . --remove-existing-container --build-no-cache
# To exec into existing dev container
devcontainer exec --workspace-folder . bash
```

Alternatively, open VSCode and use the Remote Containers extension:

```shell
code .
# Press Ctrl+Shift+P to open the Command Palette
#  Type and select `Dev Containers: Reopen in Container`
# Or to bring up without using previous dev container
#  Type `Dev Containers: Rebuild and Reopen in Container`
# Or to bring up without using previous build cache
#  Type `Dev Containers: Rebuild Without Cache and Reopen in Container`
```

Note: using Dev Containers from a remote host is also possible:

-  [Open a folder on a remote SSH host in a container](https://code.visualstudio.com/docs/devcontainers/containers#_open-a-folder-on-a-remote-ssh-host-in-a-container)
-  [Open a folder on a remote Tunnel host in a container](https://code.visualstudio.com/docs/devcontainers/containers#_open-a-folder-on-a-remote-tunnel-host-in-a-container)

### Further Reading and Concepts

Afterwards, you may want to further familiarize yourself more with the following topics:

- Docker
  - Multi-stage
    - https://docs.docker.com/build/building/multi-stage/
  - BuildKit
    - https://docs.docker.com/build/buildkit/
  - Bake
    - https://docs.docker.com/build/bake/
- Development Containers
  - https://docs.nav2.org/development_guides/devcontainer_docs/index.html
  - https://containers.dev/
  - https://code.visualstudio.com/docs/devcontainers/containers

## Buildkit Extras

The same stages used for Dev Containers can also be reused to build, test, and package Slicer for one-off tasks.

To package Slicer using BuildKit, use the following commands and find the tarballs in the result folder:

```shell
docker buildx bake package
ls ./result/*.tar.gz
```
<!-- 
If you'd instead like to run tests and inspect the test results:

```shell
docker buildx bake test
ls ./result/build/Slicer-SuperBuild-Debug/Slicer-build/Testing/Temporary
```

Or to just build Slicer and inspect the build output:

```shell
docker buildx bake build
ls ./result/build/Slicer-SuperBuild
```
 -->
