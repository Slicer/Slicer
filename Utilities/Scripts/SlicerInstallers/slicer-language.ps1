<#
    List or install 3D Slicer's interface languages (Windows).

    Usage:
        slicer-language.ps1 list          print every language Slicer has
                                          translations for, with how complete
                                          each translation is
        slicer-language.ps1 fr-FR         install that language and switch
                                          Slicer to it

    Parameters:
        -Language <code>  the language to install, e.g. fr-FR, es-419, pt-BR,
                          zh-Hans; also accepted positionally. The literal
                          'list' is the same as -List.
        -List             print the available language codes and exit
        -SlicerExe <path> the Slicer.exe to use, when this script cannot find
                          the one it ships next to

    Environment:
        SLICER_QUIET      silence progress messages; warnings and errors still show

    Installing a language installs the SlicerLanguagePacks extension
    (https://github.com/Slicer/SlicerLanguagePacks) along with that language's
    translation files. Slicer itself has to run for this: only the application
    can talk to the extensions server, compile translations and write its own
    settings. It is launched twice, without splash or main window: once to
    install the extension (skipped when a previous run already did, so repeated
    runs converge), and once more -- so the freshly installed extension's
    LanguageTools module is loaded -- to run that module's own logic: download
    the .ts translation files, compile them to .qm with Slicer's bundled
    lrelease and install them into the application's translation folder. The
    second run then verifies the .qm file for the requested locale exists and
    actually loads (by installing it as a QTranslator into the running Slicer,
    exactly what the application does at startup), and only after that writes
    the same 'language' and 'Internationalization/Enabled' settings the
    extension's own LanguageTools module writes.

    Needs network access.
#>
[CmdletBinding()]
param(
    [Parameter(Position = 0)] [string]$Language = '',
    [switch]$List,
    [string]$SlicerExe = ''
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

# Informational output, suppressed when SLICER_QUIET is set (any non-empty
# value). Warnings and errors go to their own streams and are never silenced.
function Write-Step($msg) { if (-not $env:SLICER_QUIET) { Write-Host "==> $msg" -ForegroundColor Cyan } }
function Write-Note($msg) { if (-not $env:SLICER_QUIET) { Write-Host "    $msg" -ForegroundColor DarkGray } }
function Write-Done($msg) { if (-not $env:SLICER_QUIET) { Write-Host $msg -ForegroundColor Green } }

function Show-Usage {
    Write-Host @"
Usage: slicer-language.ps1 list
       slicer-language.ps1 <language-code> [-SlicerExe <path>]

List or install 3D Slicer's interface languages.

  list             print the available language codes and how complete each
                   translation is
  <language-code>  install that language's translation files and switch
                   Slicer's interface to it, e.g. fr-FR, es-419, pt-BR
"@
}

# The Slicer.exe belonging to the installation this script sits in. It is
# installed into Slicer's own bin directory, so the launcher is one level up and
# nothing has to be passed in.
function Find-Slicer {
    if (-not $PSScriptRoot) { return '' }
    $root = Split-Path -Parent $PSScriptRoot
    foreach ($candidate in @((Join-Path $root 'Slicer.exe'), (Join-Path $PSScriptRoot 'Slicer.exe'))) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) { return $candidate }
    }
    return ''
}

# Print every language Slicer has translations for, with how complete each
# translation is. The SlicerLanguagePacks extension queries this very endpoint
# for its own language list, so what it reports is exactly what can be
# installed. Weblate spells locales with underscores (pt_BR) but the extension
# and Qt use dashes (pt-BR), so codes are printed in the form this script
# accepts.
function Show-Languages {
    Write-Step 'Querying the list of 3D Slicer translations...'
    try {
        $stats = Invoke-RestMethod -UseBasicParsing `
            -Uri 'https://hosted.weblate.org/api/components/3d-slicer/3d-slicer/statistics/?format=json&page_size=1000'
    } catch {
        Write-Error 'Could not query the translation server; browse https://hosted.weblate.org/projects/3d-slicer/ instead.'
        exit 1
    }
    Write-Host ''
    Write-Host ('  {0,-12} {1,-40} {2}' -f 'CODE', 'LANGUAGE', 'TRANSLATED')
    # English is the source language, not a translation -- skip it.
    $stats.results |
        Where-Object { $_.code -ne 'en' } |
        Sort-Object -Property translated_percent -Descending |
        ForEach-Object {
            Write-Host ('  {0,-12} {1,-40} {2,9}%' -f $_.code.Replace('_', '-'), $_.name, $_.translated_percent)
        }
    Write-Host ''
    Write-Step "Run 'slicer-language.ps1 <code>' with one of the codes above to install it."
}

function Install-Language {
    param([string]$Code, [string]$Exe)

    $extPyCode = @'
import sys
import slicer

# The repository is SlicerLanguagePacks, but the extensions catalog knows the
# extension by its base name, LanguagePacks.
name = 'LanguagePacks'
ok = False
try:
    em = slicer.app.extensionsManagerModel()
    em.interactive = False  # never pop up dialogs
    if em.isExtensionInstalled(name):
        ok = True
    else:
        try:
            ok = em.downloadAndInstallExtensionByName(name, True, True)
        except TypeError:
            # Older Slicer without the (installDependencies, waitForInstallation) overload.
            ok = em.downloadAndInstallExtensionByName(name)
except Exception as exc:
    print('Failed to install %s: %s' % (name, exc), file=sys.stderr)
slicer.util.exit(0 if ok else 1)
'@

    # $Code was validated by the caller, so substituting it into the script
    # cannot break out of the quoted string.
    $langPyCode = @'
import glob
import os
import re
import sys

import qt
import slicer

lang = '@LANG@'


def series():
    # Translations are maintained per Slicer series ("5.8", ...), so ask the
    # running application which one this is rather than guessing from a path.
    try:
        return '%s.%s' % (slicer.app.majorVersion, slicer.app.minorVersion)
    except Exception:
        pass
    try:
        match = re.match(r'(\d+\.\d+)', slicer.app.applicationVersion)
        if match:
            return match.group(1)
    except Exception:
        pass
    return ''


def main():
    # The previous launch installed the extension; this fresh launch loaded its
    # LanguageTools module, whose logic does the actual work.
    try:
        import LanguageTools
    except ImportError:
        print('SlicerLanguagePacks is installed but its LanguageTools module did not load.', file=sys.stderr)
        return 1

    locale = qt.QLocale(lang)
    if locale.name() == 'C':
        print("Qt does not recognize '%s' as a language code; run 'slicer-language.ps1 list' to see the available codes." % lang, file=sys.stderr)
        return 1

    try:
        logic = LanguageTools.LanguageToolsLogic()
        if not logic.lreleasePath:
            print('This Slicer does not bundle the lrelease tool needed to compile translations; install a more recent Slicer.', file=sys.stderr)
            return 1
        # The main branch tracks the preview release, so it is the fallback when
        # this series has no translations of its own yet.
        for branch in [b for b in (series(), 'main') if b]:
            logic.slicerVersion = branch
            try:
                logic.downloadTsFilesFromGithub('https://github.com/Slicer/SlicerLanguageTranslations')
                break
            except Exception:
                logic.removeTemporaryFolder()
        else:
            print('Could not download the translation files from https://github.com/Slicer/SlicerLanguageTranslations.', file=sys.stderr)
            return 1
        logic.normalizeTsFiles()
        logic.convertTsFilesToQmFiles()
        logic.installQmFiles()
        try:
            logic.installFontFiles()  # fonts for Chinese and other non-Latin scripts
        except Exception:
            pass
        logic.removeTemporaryFolder()
    except Exception as exc:
        print('Installing the translation files failed: %s' % exc, file=sys.stderr)
        return 1

    # installQmFiles() put one file per component here, named after the locale
    # Qt normalizes the code to. Slicer_<locale>.qm is the one that has to be
    # there: it holds the application's own strings, and is the file Slicer
    # loads at startup.
    folder = slicer.app.translationFolders()[0]
    if not os.path.isfile(os.path.join(folder, 'Slicer_%s.qm' % locale.name())):
        # A .ts file whose language Qt cannot place is normalized to the 'C'
        # locale, so those files are not a language anyone can ask for.
        codes = sorted({os.path.basename(f)[len('Slicer_'):-len('.qm')].replace('_', '-')
                        for f in glob.glob(os.path.join(folder, 'Slicer_*.qm'))} - {'C'})
        print("No translation exists for '%s'. Available: %s" % (lang, ', '.join(codes)), file=sys.stderr)
        return 1

    # Prove Slicer can load it, by installing it into this very process the
    # same way the application does at startup.
    translator = qt.QTranslator()
    if not translator.load(locale, 'Slicer', '_', folder) or not slicer.app.installTranslator(translator):
        print("The '%s' translation files in %s exist but Qt could not load them." % (lang, folder), file=sys.stderr)
        return 1

    # Only now that the translation is present and loadable, switch Slicer to it.
    settings = slicer.app.userSettings()
    settings.setValue('Internationalization/Enabled', True)
    settings.setValue('language', lang)
    count = len(glob.glob(os.path.join(folder, '*_%s.qm' % locale.name())))
    print('Installed and verified %d translation files for %s in %s.' % (count, lang, folder))
    return 0


slicer.util.exit(main())
'@ -replace '@LANG@', $Code

    # Assembled in pieces to keep the source lines short.
    $extFail = 'Could not install SlicerLanguagePacks (Slicer exited with code {0}). ' +
               'Launch Slicer and use its Extensions Manager instead.'
    $langFail = "Could not install or verify the '$Code' translation (details above; " +
                "Slicer exited with code {0}). Launch Slicer and use the extension's " +
                'LanguageTools module instead.'

    $steps = @(
        @{ Code = $extPyCode;  Fail = $extFail;
           Message = 'Installing the SlicerLanguagePacks extension...' },
        @{ Code = $langPyCode; Fail = $langFail;
           Message = "Downloading the '$Code' translation and switching Slicer to it..." }
    )
    foreach ($step in $steps) {
        $pyFile = Join-Path $env:TEMP ("slicer-set-language-" + [guid]::NewGuid().ToString('N') + ".py")
        Set-Content -Path $pyFile -Value $step.Code -Encoding ASCII

        Write-Step $step.Message
        try {
            $langProc = Start-Process -FilePath $Exe `
                -ArgumentList "--no-splash --no-main-window --python-script `"$pyFile`"" -Wait -PassThru
        } finally {
            Remove-Item $pyFile -Force -ErrorAction SilentlyContinue
        }
        if ($langProc.ExitCode -ne 0) {
            throw ($step.Fail -f $langProc.ExitCode)
        }
    }
    Write-Done "Slicer will start in '$Code' (translation installed and verified)."
}

# ---------------------------------------------------------------------------- #
# main
# ---------------------------------------------------------------------------- #
if ($Language -eq 'list') { $List = $true; $Language = '' }

if ($List) {
    Show-Languages
    exit 0
}

if (-not $Language) {
    Show-Usage
    exit 2
}

# The value lands inside a quoted Python string, so only these characters may pass.
if ($Language -notmatch '^[A-Za-z0-9_-]+$') {
    Write-Error "Expected a language code like fr-FR, es-419 or pt-BR (got '$Language'), or 'list' to see the available codes."
    exit 1
}

$exe = if ($SlicerExe) { $SlicerExe } else { Find-Slicer }
if ((-not $exe) -or (-not (Test-Path -LiteralPath $exe -PathType Leaf))) {
    Write-Error 'Could not find Slicer.exe next to this script; pass -SlicerExe <path> to point at it.'
    exit 1
}

Install-Language -Code $Language -Exe $exe
