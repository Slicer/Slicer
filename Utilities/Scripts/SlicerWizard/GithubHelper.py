"""GitHub REST helpers for SlicerWizard, using personal access tokens.

Responses are ordinary dictionaries, not PyGitHub objects. Authentication uses
GITHUB_TOKEN or Git's credential helper; no JWT signing is needed.
"""

import git
import os
import subprocess
from urllib.parse import quote, urljoin, urlparse

import requests

__all__ = ["logIn", "getRepo", "getFork", "getPullRequest"]


class _CredentialToken:
    def __init__(self, text=None, **kwargs):
        self._keys = set(kwargs)
        for key, value in kwargs.items():
            setattr(self, key, value)
        if text is not None:
            for line in text.split("\n"):
                if "=" in line:
                    key, value = line.split("=", 1)
                    self._keys.add(key)
                    setattr(self, key, value)

    def __str__(self):
        return "\n".join(f"{key}={getattr(self, key)}" for key in self._keys) + "\n\n"


def _credentials(client, request, action="fill"):
    process = client.credential(action, as_process=True, istream=subprocess.PIPE)
    out, err = process.communicate(input=str(request).encode("utf-8"))
    if process.returncode != 0:
        raise git.GitCommandError(["credential", action], process.returncode, err.rstrip())
    return _CredentialToken(out.decode())


class Github:
    """The small synchronous GitHub REST surface required by SlicerWizard."""

    def __init__(self, token):
        token = token.strip()
        self.session = requests.Session()
        # Keep netrc credentials from overriding the explicit bearer token.
        self.session.auth = lambda request: request
        self.session.headers.update({
            "Authorization": f"Bearer {token}",
            "Accept": "application/vnd.github+json",
            "X-GitHub-Api-Version": "2022-11-28",
            "User-Agent": "SlicerWizard",
        })

    @staticmethod
    def _url(path):
        if not path.startswith(("https://", "/")) or path.startswith("//"):
            raise ValueError("Expected a GitHub API path or HTTPS URL")
        url = path if path.startswith("https://") else "https://api.github.com" + path
        parsed = urlparse(url)
        if (parsed.scheme != "https" or parsed.netloc != "api.github.com"
                or parsed.username or parsed.fragment):
            raise ValueError("Refusing to send GitHub credentials to a non-GitHub API URL")
        return url

    def request(self, method, path, **kwargs):
        url = self._url(path)
        # GitHub redirects renamed or transferred repositories; follow those manually
        # so that credentials are only ever sent to the GitHub API host.
        for _ in range(5):
            response = self.session.request(method, url, timeout=30, allow_redirects=False, **kwargs)
            location = response.headers.get("Location")
            if response.status_code in (301, 302, 307, 308) and location:
                url = self._url(urljoin(url, location))
                kwargs.pop("params", None)
                continue
            break
        if not 200 <= response.status_code < 300:
            # Do not retry writes: a network failure can occur after GitHub accepted them.
            raise requests.HTTPError(
                f"GitHub {method} {urlparse(url).path}: HTTP {response.status_code}: {response.text[:1000]}",
                response=response)
        return response


    def pages(self, path, **params):
        """Yield every list item, following GitHub's authenticated pagination."""
        visited = set()
        while path:
            if path in visited:
                raise ValueError("GitHub returned a pagination loop")
            visited.add(path)
            response = self.request("GET", path, params={"per_page": 100, **params})
            data = response.json()
            if not isinstance(data, list):
                raise ValueError("Expected a list from the GitHub API")
            yield from data
            path = response.links.get("next", {}).get("url")
            params = {}

    def get_user(self):
        return self.request("GET", "/user").json()

    def create_repo(self, name, description=None):
        payload = {"name": name}
        if description is not None:
            payload["description"] = description
        return self.request("POST", "/user/repos", json=payload).json()

    def create_pull(self, repository, **payload):
        return self.request("POST", repository["url"] + "/pulls", json=payload).json()

    def edit_pull(self, pull, **payload):
        return self.request("PATCH", pull["url"], json=payload).json()

    def compare(self, repository, base, head):
        return self.request(
            "GET", repository["url"] + f"/compare/{quote(base, safe='')}...{quote(head, safe='')}").json()


def logIn(repo=None):
    """Authenticate a token and approve Git credentials only after success.

    GITHUB_TOKEN takes precedence and is used to authenticate directly, without
    prompting. Otherwise Git obtains the token, including any required prompting.
    In both cases the credentials are approved after successful authentication so
    that a configured credential helper stores them for later ``git push`` calls.
    """
    client = repo.git if repo is not None else git.cmd.Git()
    token = os.environ.get("GITHUB_TOKEN", "").strip()
    if token:
        session = Github(token)
        user = session.get_user()
        cred = _CredentialToken(
            protocol="https", host="github.com", username=user["login"], password=token)
    else:
        cred = _credentials(client, _CredentialToken(protocol="https", host="github.com"))
        session = Github(cred.password)
        session.get_user()
    _credentials(client, cred, action="approve")
    return session


def _isRepoName(name):
    parts = name.split("/")
    return len(parts) == 2 and not any(part in {"", ".", ".."} for part in parts)


def getRepo(session, name=None, url=None):
    """Return repository metadata, or None for a non-GitHub URL or a 404.

    Other API failures propagate rather than masquerading as a missing repo.
    """
    if name is None:
        parsed = urlparse(url or "")
        if parsed.hostname not in {"github.com", "www.github.com"}:
            return None
        name = parsed.path.strip("/").removesuffix(".git")
        if not _isRepoName(name):
            return None
    elif not _isRepoName(name):
        raise ValueError("Expected a GitHub repository name in owner/repository form")
    path = "/repos/" + "/".join(quote(part, safe="") for part in name.split("/"))
    try:
        return session.request("GET", path).json()
    except requests.HTTPError as error:
        if error.response is not None and error.response.status_code == 404:
            return None
        raise


def getFork(session, user, upstream, create=False):
    """Find the user's fork, or create it; reject a conflicting repository."""
    repo = getRepo(session, name=f"{user['login']}/{upstream['name']}")
    if repo is not None:
        if repo.get("fork") and repo.get("parent", {}).get("id") == upstream["id"]:
            return repo
        raise ValueError(f"{repo['full_name']} exists but is not a fork of {upstream['full_name']}")
    if create:
        return session.request("POST", upstream["url"] + "/forks", json={}).json()
    return None


def getPullRequest(session, upstream, ref, user=None, fork=None, target=None):
    """Find an open pull request matching the source branch and optional owner."""
    params = {"state": "open"}
    if target is not None:
        params["base"] = target
    for pull in session.pages(upstream["url"] + "/pulls", **params):
        head = pull["head"]
        if head["ref"] != ref:
            continue
        if user is not None and (head.get("user") or {}).get("login") != user["login"]:
            continue
        if fork is not None and (head.get("repo") or {}).get("id") != fork["id"]:
            continue
        return pull
    return None
