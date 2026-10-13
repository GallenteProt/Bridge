# Bridge

Server-side DayZ script mod for **DayZ End**. It sends server data (FPS, players, positions, deaths) to the manager
over HTTP (`RestApi`) and runs the commands the manager returns (kick, notify, teleport, heal, kill, shutdown).
Players do not need it: it is loaded with `-serverMod`.

The mod is shipped **unpacked** (no PBO): the server must start with `-filePatching`.

## Install or update

Linux / macOS:

    curl -fsSL https://raw.githubusercontent.com/GallenteProt/Bridge/main/install.sh | bash -s -- /path/to/server

Windows (PowerShell):

    & ([scriptblock]::Create((irm https://raw.githubusercontent.com/GallenteProt/Bridge/main/install.ps1))) -Server C:\DayZServer

The mod is copied to `<server>/Bridge`, the installed commit is stored in `Bridge/.version`.
Run the same command again to update: it downloads only when a newer commit exists. DayZ End does the same by itself
(Bridge card of the server settings and the check before every start).

Launch parameters: `-serverMod=Bridge -filePatching`.

## Config

The manager writes `<server>/profiles/Bridge/bridge.json` (`url`, `key`, `instance`, `posInterval`, `enabled`).
Without that file the mod does nothing.
