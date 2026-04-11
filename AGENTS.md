# AGENTS.md

Guidance for agents working in this repository.

## Scope

- This workspace is a legacy Win32/MFC C++ codebase centered on `NetTarget.sln`.
- If code and this document disagree, follow the code.
- Keep this file focused on durable repo guidance. Do update it when you learn stable workflow or architecture facts that will help later chats.
- Do not update this file for one-off task notes, temporary branch state, or short-lived experiments.

## Repository Map

- `NetTarget.sln`: main Visual Studio solution.
- `NetTarget/Game2`: main game executable, menus, dialogs, track selection, multiplayer UI, and network/session flow.
- `NetTarget/MainCharacter`: player craft state and gameplay-affecting toggles.
- `NetTarget/VideoServices`: renderer and texture/wall/floor drawing code.
- `NetTarget/Util`: shared utilities and config.
- `include/`: shared headers used across projects.
- `lib/`: native libraries and DLLs copied into build output.
- `NetTarget/share/`: runtime assets/data copied alongside the game.
- `HoverCad/`, `InternetRoom/`: legacy side tools/projects; do not touch unless the task clearly requires them.

## Build Environment

- Primary build is Visual Studio 2022 using `Debug|Win32` or `Release|Win32`.
- On this machine, Visual Studio 2022 Community MSBuild is at `F:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe`.
- Useful command:

```powershell
msbuild NetTarget.sln /p:Configuration=Debug /p:Platform=Win32
```

- For changes centered on the main game executable, prefer validating `Game2` in `Release|Win32`. Kill any running `HoverRace.exe` before building to avoid link or manifest embedding locks:

```powershell
Get-Process HoverRace -ErrorAction SilentlyContinue | Stop-Process
msbuild NetTarget\Game2\Game2.vcxproj /p:Configuration=Release /p:Platform=Win32
if ($LASTEXITCODE -eq 0) { Start-Process D:\code\HoverRace\NetTarget\Release\HoverRace.exe }
```

- The checked-in project files reference local SDK installs. Expect build failures unless these exist or are remapped:
  - Steamworks SDK under `D:\steamworks_sdk_153a\...`
  - Boost 1.56 under `F:\local\boost_1_56_0`
  - DirectX SDK (June 2010) x86 libraries
- `Game2.vcxproj` uses MFC, MultiByte character set, and Win32 output paths.

## Working Style

- Make narrow, surgical changes. This code relies on legacy MFC/Win32 patterns, resource IDs, and precompiled headers.
- Preserve existing character-set assumptions; avoid introducing wide-string-only flows unless the surrounding code already does so.
- Keep header/source pairs in sync. Many features span `.h`, `.cpp`, `.rc`, and `resource.h` together.
- Do not casually modernize old code style in unrelated areas.
- Avoid editing generated or machine-local artifacts unless the task is explicitly about build setup.

## Main Code Paths

These areas are the most likely places to be involved in gameplay, UI, and networking changes:

- `NetTarget/Game2/NetInterface.*`: IMR window layout, TCP connection refactor, minimap/player-list/game-details flow, and race option propagation.
- `NetTarget/Game2/InternetRoom.*`: multiplayer room UI and host/client settings wiring.
- `NetTarget/Game2/NetworkSession.*`: pre-race session synchronization, including sending option changes before race start.
- `NetTarget/Game2/TrackSelect.*`: track search, keyboard navigation, and map preview behavior.
- `NetTarget/Game2/Game2.rc` and `NetTarget/Game2/resource.h`: dialogs and control IDs for new UI.
- `NetTarget/Game2/GameApp.*` and `NetTarget/Game2/ClientSession.*`: local game state and session-facing option handling.
- `NetTarget/MainCharacter/MainCharacter.*`: gameplay limits for mines, cans, and allowed crafts.
- `NetTarget/VideoServices/3DViewportRendering.cpp`: texture/rendering correctness fixes.
- `NetTarget/Util/Config.*`: Steam username/config behavior.

## When To Update This File

- Update it after discovering stable project rules, recurring pitfalls, required validation steps, or cross-module dependencies.
- Update it after confirming a build command, SDK prerequisite, directory purpose, or file ownership pattern that future agents will likely need.
- Do not add commit-by-commit history, temporary debugging notes, or details that are likely to go stale quickly.

## Change Coordination Rules

- For dialog or menu changes, update all of:
  - the relevant `.cpp`
  - the matching `.h`
  - `Game2.rc`
  - `resource.h`
- For multiplayer option changes, check all affected layers:
  - UI state in `InternetRoom` or `TrackSelect`
  - propagation in `NetInterface`
  - timing/sync in `NetworkSession`
  - gameplay enforcement in `ClientSession` and `MainCharacter`
  - persistence/defaults in `GameApp` or `Util/Config` if applicable
- For track-selection changes, preserve:
  - type-to-select behavior
  - search/filter behavior
  - map preview rendering
  - current selection after filtering
- For renderer changes, prefer minimal math fixes and visually regression-test walls, floors, distant surfaces, and texture fitting.

## Validation

After changes, validate the smallest relevant surface area you touched:

- Build the affected Win32 projects.
- If multiplayer/UI code changed, smoke-test host/client flow and pre-race option syncing.
- If track selection changed, test search, keyboard navigation, and preview map updates.
- If gameplay restrictions changed, verify mines/cans/craft restrictions both in setup UI and in-race behavior.
- If renderer code changed, compare visually against known-problem scenes before and after.

## Things To Leave Alone

- Do not delete or reset local untracked workspace content such as `migrations/`, `NetTarget/share/`, or a local `.gitignore` unless the user asks.
- Avoid churn in `Debug/`, `Release/`, `.vs/`, `.sdf`, `.suo`, or `.vcxproj.user` files.
- Avoid broad project-file edits unless the task is specifically about build configuration.
