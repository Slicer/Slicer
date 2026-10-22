"""Offline tests for SlicerWizard's token-authenticated GitHub REST operations."""

import os
from contextlib import ExitStack
import importlib
from types import SimpleNamespace
import unittest
from unittest.mock import Mock, patch

import requests

from SlicerWizard import GithubHelper as helper

wizard_module = importlib.import_module("SlicerWizard.ExtensionWizard")


def response(data, status=200, links=None, headers=None):
    result = Mock(status_code=status, links=links or {}, headers=headers or {}, text="API error")
    result.json.return_value = data
    return result


class GithubTest(unittest.TestCase):
    def setUp(self):
        self.http = Mock()
        session = patch.object(helper.requests, "Session", return_value=self.http)
        session.start()
        self.addCleanup(session.stop)
        self.github = helper.Github("test-token")
        self.upstream = {
            "id": 1, "name": "ExtensionsIndex", "full_name": "Slicer/ExtensionsIndex",
            "url": "https://api.github.com/repos/Slicer/ExtensionsIndex",
        }
        self.fork = {
            "id": 2, "name": "ExtensionsIndex", "full_name": "alice/ExtensionsIndex",
            "url": "https://api.github.com/repos/alice/ExtensionsIndex",
            "owner": {"login": "alice"}, "fork": True, "parent": {"id": 1},
        }

    def test_token_headers_timeout_and_no_redirects(self):
        self.http.request.return_value = response({"login": "alice"})
        self.assertEqual(self.github.get_user(), {"login": "alice"})
        self.assertEqual(self.http.headers.update.call_args.args[0]["Authorization"], "Bearer test-token")
        self.http.request.assert_called_once_with(
            "GET", "https://api.github.com/user", timeout=30, allow_redirects=False)

    def test_environment_token_approves_git_credentials_after_authentication(self):
        self.http.request.return_value = response({"login": "alice"})
        with patch.dict(os.environ, {"GITHUB_TOKEN": "environment-token"}), patch.object(helper, "_credentials") as creds:
            helper.logIn()
        self.assertEqual(creds.call_count, 1)
        self.assertEqual(creds.call_args.kwargs, {"action": "approve"})
        request = creds.call_args.args[1]
        self.assertEqual((request.username, request.password), ("alice", "environment-token"))
        self.assertEqual(self.http.headers.update.call_args.args[0]["Authorization"], "Bearer environment-token")

    def test_netrc_cannot_override_bearer_authentication(self):
        auth = self.http.auth
        session = requests.sessions.Session()
        session.auth = auth
        session.headers["Authorization"] = "Bearer test-token"
        with patch("requests.sessions.get_netrc_auth", return_value=("netrc-user", "netrc-password")) as netrc:
            prepared = session.prepare_request(requests.Request("GET", "https://api.github.com/user"))
        netrc.assert_not_called()
        self.assertEqual(prepared.headers["Authorization"], "Bearer test-token")
        session.close()

    def test_credentials_approved_only_after_authentication(self):
        credential = helper._CredentialToken(username="alice", password="test-token")
        with patch.dict(os.environ, {}, clear=True), patch.object(
                helper, "_credentials", return_value=credential) as creds:
            self.http.request.return_value = response({"login": "alice"})
            helper.logIn(Mock())
            self.assertEqual(creds.call_args.kwargs, {"action": "approve"})
            creds.reset_mock()
            self.http.request.return_value = response({}, status=401)
            with self.assertRaises(requests.HTTPError):
                helper.logIn(Mock())
            self.assertEqual(creds.call_count, 1)

    def test_pagination(self):
        url = "https://api.github.com/user/repos?page=2"
        self.http.request.side_effect = [
            response([{"name": "one"}], links={"next": {"url": url}}),
            response([{"name": "two"}]),
        ]
        self.assertEqual(list(self.github.pages("/user/repos")), [{"name": "one"}, {"name": "two"}])
        self.assertEqual(self.http.request.call_args.kwargs["params"], {"per_page": 100})

    def test_rejects_external_pagination_before_sending_credentials(self):
        self.http.request.return_value = response([], links={"next": {"url": "https://evil.example/steal"}})
        with self.assertRaises(ValueError):
            list(self.github.pages("/user/repos"))
        self.assertEqual(self.http.request.call_count, 1)

    def test_rejects_untrusted_api_urls(self):
        for url in ["https://api.github.com.evil.example/user", "https://user@api.github.com/user",
                    "http://api.github.com/user", "//evil.example/user"]:
            with self.subTest(url=url), self.assertRaises((ValueError, requests.HTTPError)):
                self.github.request("GET", url)
        self.http.request.assert_not_called()

    def test_rejects_pagination_loops_and_nonlists(self):
        self.http.request.return_value = response([], links={"next": {"url": "/user/repos"}})
        with self.assertRaisesRegex(ValueError, "pagination loop"):
            list(self.github.pages("/user/repos"))
        self.http.request.return_value = response({})
        with self.assertRaisesRegex(ValueError, "Expected a list"):
            list(self.github.pages("/user/repos"))

    def test_api_errors_are_explicit_and_writes_not_retried(self):
        for status in [301, 401, 403, 429, 500]:
            with self.subTest(status=status):
                self.http.request.reset_mock()
                self.http.request.return_value = response({}, status=status)
                with self.assertRaisesRegex(requests.HTTPError, f"HTTP {status}"):
                    self.github.create_repo("example")
                self.http.request.assert_called_once()
        self.http.request.side_effect = requests.Timeout("timeout")
        with self.assertRaises(requests.Timeout):
            self.github.create_repo("example")

    def test_repository_url_and_not_found(self):
        self.http.request.return_value = response(self.upstream)
        self.assertEqual(helper.getRepo(self.github, url="https://github.com/Slicer/ExtensionsIndex.git"),
                         self.upstream)
        self.http.request.reset_mock()
        self.assertIsNone(helper.getRepo(self.github, url="https://notgithub.com/owner/repo"))
        self.http.request.assert_not_called()
        self.http.request.return_value = response({}, status=404)
        self.assertIsNone(helper.getRepo(self.github, name="owner/missing"))
        self.http.request.return_value = response({}, status=403)
        with self.assertRaises(requests.HTTPError):
            helper.getRepo(self.github, name="owner/forbidden")

    def test_repository_redirects_stay_on_github_api(self):
        moved = "https://api.github.com/repositories/1"
        self.http.request.side_effect = [
            response({}, status=301, headers={"Location": moved}), response(self.upstream)]
        self.assertEqual(helper.getRepo(self.github, name="Slicer/OldName"), self.upstream)
        self.assertEqual(self.http.request.call_args.args[1], moved)
        self.assertNotIn("params", self.http.request.call_args.kwargs)
        self.http.request.reset_mock()
        self.http.request.side_effect = [
            response({}, status=307, headers={"Location": "https://evil.example/steal"})]
        with self.assertRaises(ValueError):
            self.github.request("GET", "/repos/Slicer/OldName")
        self.assertEqual(self.http.request.call_count, 1)

    def test_malformed_repository_url_is_not_found(self):
        for url in ["https://github.com/Slicer", "https://github.com/Slicer/Slicer/tree/main", "git@github.com:a/b.git"]:
            with self.subTest(url=url):
                self.assertIsNone(helper.getRepo(self.github, url=url))
        self.http.request.assert_not_called()

    def test_create_repository_omits_unspecified_description(self):
        self.http.request.return_value = response({"id": 3})
        self.github.create_repo("new")
        self.assertEqual(self.http.request.call_args.kwargs["json"], {"name": "new"})
        self.github.create_repo("new", description="")
        self.assertEqual(self.http.request.call_args.kwargs["json"], {"name": "new", "description": ""})

    def test_find_and_create_fork(self):
        self.http.request.return_value = response(self.fork)
        self.assertEqual(helper.getFork(self.github, {"login": "alice"}, self.upstream), self.fork)
        self.http.request.side_effect = [response({}, status=404), response(self.fork, status=202)]
        self.assertEqual(helper.getFork(self.github, {"login": "alice"}, self.upstream, create=True), self.fork)
        self.assertEqual(self.http.request.call_args.args[:2], ("POST", self.upstream["url"] + "/forks"))

    def test_conflicting_repository_is_not_treated_as_fork(self):
        self.http.request.return_value = response({**self.fork, "parent": {"id": 99}})
        with self.assertRaisesRegex(ValueError, "not a fork"):
            helper.getFork(self.github, {"login": "alice"}, self.upstream, create=True)

    def test_pull_lookup_handles_deleted_forks_and_filters(self):
        pull = {
            "url": self.upstream["url"] + "/pulls/3",
            "head": {"ref": "update", "user": {"login": "alice"}, "repo": {"id": 2}},
        }
        self.http.request.return_value = response([
            {"head": {"ref": "update", "user": None, "repo": None}}, pull,
        ])
        self.assertEqual(helper.getPullRequest(self.github, self.upstream, "update",
                                              user={"login": "alice"}, fork=self.fork, target="main"), pull)
        self.assertEqual(self.http.request.call_args.kwargs["params"]["base"], "main")

    def test_create_and_edit_pull_and_compare(self):
        pull = {"url": self.upstream["url"] + "/pulls/3"}
        self.http.request.return_value = response(pull)
        self.github.create_pull(self.upstream, title="Title", body="Body", head="alice:update", base="main")
        self.assertEqual(self.http.request.call_args.args[:2], ("POST", self.upstream["url"] + "/pulls"))
        self.github.edit_pull(pull, title="Updated", state="open")
        self.assertEqual(self.http.request.call_args.args[:2], ("PATCH", pull["url"]))
        self.assertEqual(self.http.request.call_args.kwargs["json"], {"title": "Updated", "state": "open"})
        self.github.compare(self.upstream, "feature/old", "feature/new")
        self.assertTrue(self.http.request.call_args.args[1].endswith("/compare/feature%2Fold...feature%2Fnew"))

    def test_publish_workflow_uses_rest_repository_fields(self):
        repo = Mock(remotes=[], active_branch=SimpleNamespace(name="main"))
        repo.is_dirty.return_value = False
        project = Mock(project="Demo")
        def getValue(name, default=None, **kwargs):
            if default is None:
                raise KeyError(name)
            return default
        project.getValue.side_effect = getValue
        remote = {
            "name": "Demo", "url": "https://api.github.com/repos/alice/Demo",
            "html_url": "https://github.com/alice/Demo",
            "clone_url": "https://github.com/alice/Demo.git",
        }
        self.http.request.side_effect = [
            response({"login": "alice"}), response([]), response(remote, status=201),
        ]
        wizard = wizard_module.ExtensionWizard()
        with patch.object(wizard_module, "GithubHelper", helper, create=True), \
                patch.object(helper, "logIn", return_value=self.github), \
                patch.object(wizard_module, "getRepo", return_value=repo), \
                patch.object(wizard_module, "ExtensionProject", return_value=project), \
                patch.object(wizard, "_setExtensionUrl") as set_url:
            wizard.publish(SimpleNamespace(destination="example", cmakefile="CMakeLists.txt", name=None))
        self.assertEqual(self.http.request.call_args.kwargs["json"], {"name": "Demo"})
        repo.create_remote.assert_called_once_with("origin", remote["clone_url"])
        repo.create_remote.return_value.push.assert_called_once()
        self.assertEqual(set_url.call_args_list[0].args[1:], ("HOMEPAGE", remote["html_url"]))

    def test_register_workflow_creates_edits_and_dry_runs_pull_requests(self):
        for update, dry_run in [(False, False), (True, False), (True, True)]:
            with self.subTest(update=update, dry_run=dry_run), ExitStack() as stack:
                upstream = {**self.upstream, "clone_url": "https://github.com/Slicer/ExtensionsIndex.git",
                            "git_url": "git://github.com/Slicer/ExtensionsIndex.git"}
                fork = {**self.fork, "clone_url": "https://github.com/alice/ExtensionsIndex.git",
                        "git_url": "git://github.com/alice/ExtensionsIndex.git"}
                branch = "update-Demo-main" if update else "add-Demo-main"
                pull = {
                    "url": upstream["url"] + "/pulls/3", "html_url": "https://github.com/Slicer/ExtensionsIndex/pull/3",
                    "head": {"ref": branch, "user": {"login": "alice"}, "repo": {"id": 2}},
                }
                local = Mock()
                index = Mock(working_tree_dir="index")
                index.is_dirty.return_value = False
                base = Mock()
                base.commit.tree = {"Demo.json": object()} if update else {}
                downstream_remote = Mock()
                upstream_remote = Mock(refs={"main": base})
                description = Mock(scm="git", scmurl="https://github.com/alice/Demo.git", scmrevision="new")
                replies = [response(upstream), response({"login": "alice"}), response(fork),
                           response([pull] if update else [])]
                if update:
                    replies.extend([response({"url": "https://api.github.com/repos/alice/Demo"}),
                                    response({"html_url": "https://github.com/alice/Demo/compare/old...new"})])
                if not dry_run:
                    replies.append(response(pull, status=200 if update else 201))
                self.http.request.reset_mock()
                self.http.request.side_effect = replies
                stack.enter_context(patch.object(wizard_module, "GithubHelper", helper, create=True))
                stack.enter_context(patch.object(helper, "logIn", return_value=self.github))
                stack.enter_context(patch.object(wizard_module, "getRepo", side_effect=[local, index]))
                stack.enter_context(patch.object(wizard_module, "getRemote",
                                                side_effect=[downstream_remote, upstream_remote]))
                stack.enter_context(patch.object(wizard_module, "ExtensionDescription",
                                                side_effect=[description, SimpleNamespace(scmrevision="old")]))
                wizard = wizard_module.ExtensionWizard()
                stack.enter_context(patch.object(wizard, "_extensionIndexCommitMessage",
                                                return_value="ENH: Demo\n\nDescription"))
                wizard.contribute(SimpleNamespace(
                    destination="example", cmakefile="CMakeLists.txt", name="Demo",
                    index="index", target="main", test=False, dryRun=dry_run))
                writes = [call for call in self.http.request.call_args_list if call.args[0] in {"POST", "PATCH"}]
                self.assertEqual(len(writes), 0 if dry_run else 1)
                if writes:
                    self.assertEqual(writes[0].args[0], "PATCH" if update else "POST")
                    self.assertEqual(writes[0].kwargs["json"]["title"], "ENH: Demo [main]")
                    if update:
                        self.assertIn("compare/old...new", writes[0].kwargs["json"]["body"])
                    else:
                        self.assertEqual(writes[0].kwargs["json"]["head"], "alice:add-Demo-main")


if __name__ == "__main__":
    unittest.main()
