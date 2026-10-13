# Installs or updates the Bridge mod in a DayZ server folder (Windows PowerShell 5.1+ / PowerShell 7).
#   & ([scriptblock]::Create((irm https://raw.githubusercontent.com/GallenteProt/Bridge/main/install.ps1))) -Server C:\DayZServer
# Run the same command again to update: nothing is downloaded when the installed commit is the newest one.
param(
  [string]$Server = (Get-Location).Path,
  [string]$Repo = $(if ($env:BRIDGE_REPO) { $env:BRIDGE_REPO } else { 'GallenteProt/Bridge' }),
  [string]$Branch = $(if ($env:BRIDGE_BRANCH) { $env:BRIDGE_BRANCH } else { 'main' })
)
$ErrorActionPreference = 'Stop'
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

if (-not (Test-Path -LiteralPath $Server -PathType Container)) { throw "server folder not found: $Server" }

$remote = (Invoke-RestMethod -Headers @{ Accept = 'application/vnd.github.sha' } -Uri "https://api.github.com/repos/$Repo/commits/$Branch").ToString().Trim()
if ($remote -notmatch '^[0-9a-f]{40}$') { throw "cannot read the latest commit of $Repo@$Branch" }
$modDir = Join-Path $Server 'Bridge'
$verFile = Join-Path $modDir '.version'
$installed = if (Test-Path -LiteralPath $verFile) { (Get-Content -LiteralPath $verFile -Raw).Trim() } else { '' }

if ($remote -eq $installed -and (Test-Path -LiteralPath (Join-Path $modDir 'config.cpp'))) {
  Write-Host "Bridge is up to date ($($remote.Substring(0, 7)))"
  return
}

$tmp = Join-Path ([IO.Path]::GetTempPath()) ('bridge-' + [Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $tmp | Out-Null
try {
  $zip = Join-Path $tmp 'bridge.zip'
  Invoke-WebRequest -UseBasicParsing -Uri "https://codeload.github.com/$Repo/zip/$remote" -OutFile $zip
  Expand-Archive -LiteralPath $zip -DestinationPath $tmp -Force
  $src = Get-ChildItem -LiteralPath $tmp -Directory | ForEach-Object { Join-Path $_.FullName 'Bridge' } | Where-Object { Test-Path -LiteralPath (Join-Path $_ 'config.cpp') } | Select-Object -First 1
  if (-not $src) { throw 'the downloaded archive has no Bridge\config.cpp' }

  $new = Join-Path $Server 'Bridge.new'
  if (Test-Path -LiteralPath $new) { Remove-Item -LiteralPath $new -Recurse -Force }
  Copy-Item -LiteralPath $src -Destination $new -Recurse
  Set-Content -LiteralPath (Join-Path $new '.version') -Value $remote -NoNewline
  if (Test-Path -LiteralPath $modDir) { Remove-Item -LiteralPath $modDir -Recurse -Force }
  Rename-Item -LiteralPath $new -NewName 'Bridge'
}
finally { Remove-Item -LiteralPath $tmp -Recurse -Force -ErrorAction SilentlyContinue }

if ($installed) { Write-Host "Bridge updated $($installed.Substring(0, 7)) -> $($remote.Substring(0, 7))" } else { Write-Host "Bridge installed ($($remote.Substring(0, 7)))" }
Write-Host 'Server launch parameters: -serverMod=Bridge -filePatching'
Write-Host 'Restart the server to load the new scripts.'
