# CZVR Request Queue - EuroScope Plugin (v0.1 scaffold)

## What this does right now

- Watches every flight plan once per second (`OnTimer`).
- When a flight plan's ground state becomes `TAXI` (EuroScope's own "taxiing out"
  state), it's automatically appended to an internal priority queue.
- A TAG item ("REQ Queue Status") shows `#<position> <mm:ss waited>` on any
  aircraft currently in the queue, e.g. `#2 04:12`.
- A TAG function ("REQ Queue Menu") opens a right-click-style popup with
  Move to Top / Move Up / Move Down / Remove from Queue, so a controller can
  override the FIFO order manually.
- Aircraft are dropped from the queue automatically once they leave TAXI
  (progress to DEPA, get pushed back to PUSH, disconnect, etc).

This matches what you asked for: automatic FIFO queueing off ground state,
with manual reorder as the override - nothing more, nothing less. It does
**not** attempt to replicate vStrips' PDC/Hoppie integration, stand
management, or anything airport-specific.

## Known issue #1 - you have to source the SDK yourself

Since EuroScope 3.2, the plugin dev environment (`EuroScopePlugIn.h` +
`EuroScope.lib`) stopped shipping with the normal installer. There's an open
VATSIM forum thread confirming this (forum.vatsim.net, "Plugin Environment
Euroscope v3.2+"). You have three real options, in order of how much I'd
trust them:

1. **Ask in the VATCAN engineering channels.** CZUL (Montreal) and at least
   one other Canadian FIR already have custom EuroScope plugins on GitHub
   (`rt-2/CZUL-Euroscope-Plugin`, `ronyan/VATCANSitu`). Whoever built those
   has a working SDK + lib and has already solved this exact bootstrapping
   problem. This is almost certainly faster than digging through old
   installer archives, and it's the kind of thing your Facility Engineer
   role gives you a legitimate reason to ask about.
2. Pull an archived copy of the SDK folder referenced in that forum thread.
3. Extract the header from any open-source plugin repo (I already verified
   the `EuroScopePlugIn.h` API calls used in this code against a real copy
   pulled from GitHub - see "What I verified" below) - but you still need
   `EuroScope.lib` specifically, which isn't usually committed to those repos
   since it's a binary import library, not source.

## Building - via GitHub Actions (recommended, no local Windows needed)

This repo is private. Drop your two SDK files into `sdk/` (see
`sdk/README.txt`), commit, and push. `.github/workflows/build-plugin.yml`
builds on a real `windows-2022` MSVC runner and uploads `CZVR_ReqQueue.dll`
as a workflow artifact - download it from the Actions run summary.

Pinned to `windows-2022` deliberately, not `windows-latest`: that label
switched to Visual Studio 2026 between June 8-15, 2026, which changes the
CMake generator string and drops the bundled Windows 10 SDK. Don't move to
`windows-latest`/VS2026 until this CMake config has been verified against it.

## Building - locally (only if you have Visual Studio on Windows)

```
cmake -B build -A Win32 -DEUROSCOPE_SDK_DIR="C:/path/to/sdk"
cmake --build build --config Release
```

**x86 matters**: EuroScope itself is a 32-bit process. A 64-bit DLL will
silently fail to load. Make sure your Visual Studio config platform is
Win32, not x64.

## Testing without Windows

EuroScope reportedly runs under Wine on Mac (see `jonaseberle/euroscope-afv-wine`
on GitHub) - a real option for loading the built DLL and click-testing the
tag item/popup menu without a VM. Caveat: that same project has documented
Wine-specific crashes with at least one other plugin (TopSky, as of an
August 2025 note), so "runs under Wine" isn't a guarantee for this plugin
specifically - you'll only know once you try loading it.

## Known issue #2 - untested against a live EuroScope session

I don't have Windows or EuroScope available to actually compile and load
this. Everything here is written against the real SDK header (I pulled a
copy and grepped it rather than going from memory), so the function
signatures, `GetGroundState()` return values (`"PUSH"`, `"TAXI"`, `"DEPA"`,
or empty), and popup list API are confirmed accurate as of that header.
What I *can't* verify without a live session:

- Whether `OnFunctionCall`'s reliance on `FlightPlanSelectASEL()` correctly
  identifies the clicked aircraft in all cases (this is the standard pattern
  used in other plugins, but tag-click semantics can be fiddly - test this
  first).
- Actual visual behavior of the tag item in a real DEP/GND list layout.
- Performance with a busy airport's worth of flight plans (unlikely to be an
  issue with a `std::vector` scan once a second, but worth confirming on a
  loaded sweatbox session, not just solo).

Treat the first real build-and-load as a debugging session, not a victory
lap.

## What I verified vs. assumed

Verified directly against a real `EuroScopePlugIn.h`:
- `CPlugIn` constructor signature
- `GetGroundState()` return values
- `RegisterTagItemType` / `RegisterTagItemFunction` signatures
- `OnGetTagItem` / `OnFunctionCall` / `OnTimer` signatures
- `OpenPopupList` / `AddPopupListElement` signatures
- `FlightPlanSelectFirst` / `FlightPlanSelectNext` / `FlightPlanSelect` /
  `FlightPlanSelectASEL` exist with the expected purpose

Assumed / not yet verified: exact SDK version bundled with whatever
EuroScope build CZVR actually runs. There's a documented history of the
header drifting between versions (see the VATSIM forum thread linked above),
so if something doesn't compile, that mismatch is the first thing to check.

## Setting it up in EuroScope once built

1. Settings menu -> Plug-ins -> Load
2. Select `CZVR_ReqQueue.dll`
3. In your DEP/GND tag layout editor, add a new column, set its type to
   "REQ Queue Status"
4. Bind a mouse button on that same column to the "REQ Queue Menu" function

## Next steps once this compiles and loads

- Confirm the ASEL-based click detection actually works before building
  anything else on top of it
- Decide whether "TAXI" is really the right trigger state for your ops
  (e.g. do you want PUSH included too, for pushback requests specifically?)
- Only after both of those: consider persistence across EuroScope reconnects,
  or wiring this into your existing sector file / GitHub workflow.
