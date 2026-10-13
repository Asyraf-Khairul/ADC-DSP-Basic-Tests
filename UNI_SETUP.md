# Continue the DSP tests on a university computer

The step-by-step hardware tests are in [TEST_GUIDE.md](TEST_GUIDE.md).
This guide gets the same experimental source onto another computer and builds it.

- Repository: https://github.com/Asyraf-Khairul/ADC-DSP-Basic-Tests
- Testing branch: `testing/dsp-suite`
- Original DSP-suite implementation commit: `e0200de`
- Protected by our workflow: leave `main` unchanged; no merge, release or force-push.

The branch can gain newer commits. Use its latest tip for continuing work and
record `git rev-parse HEAD` with each set of measurements.
The separate `SDR-RX` repository is NOT required: the DSP source is included.

## 1. Copy this prompt into Codex on the university computer

Use a local Codex session with terminal/file access on that computer, opened
in a writable project parent folder. A remote-only session cannot install
tools on the university PC without a connection to it.
If Codex is not already available, sign in through your approved local client
or ask university IT; this project does not install Codex itself.

```text
Set up my experimental Raspberry Pi Pico ADC/DSP project on this computer.

Repository: https://github.com/Asyraf-Khairul/ADC-DSP-Basic-Tests.git
Branch: testing/dsp-suite
Setup guide: UNI_SETUP.md on that branch
Hardware guide: TEST_GUIDE.md on that branch

Please perform the setup, not just describe it:
1. Inspect this computer's OS, available tools and writable workspace. Use a
   new ADC-DSP-Basic-Tests-testing folder here unless an existing matching
   clone is available. Do not assume my home PC's username or folder paths.
2. Clone testing/dsp-suite. If reusing a clone, check its remote, branch and
   working tree first. Preserve any local changes and stop for direction if
   updating would overwrite or conflict with them. Use fast-forward-only
   updates. Never reset, delete or overwrite an existing project.
3. Read UNI_SETUP.md, README.md, TEST_GUIDE.md and any applicable repository
   instructions. Verify the checked-out branch and report its commit hash.
4. Inspect and, where permitted, install the missing Git, VS Code/Pico
   extension and Pico C/C++ build dependencies from official sources.
   Prefer existing or per-user tools. Request required approvals and stop
   for university IT if policy blocks installation; do not bypass policy,
   disable security controls, or store credentials in this repository.
5. Match the project's Pico SDK 2.1.0, Arm toolchain 13_3_Rel1 and picotool
   2.1.0 where available, using this machine's actual paths. Target RP2040
   board pico, not Pico 2 and not MicroPython. Do not regenerate or replace
   the application's source with a new-project template.
6. Build Release normal firmware with BENCH_SELF_TEST=OFF in build, then
   the optional DSP software-test firmware with BENCH_SELF_TEST=ON in
   build-selftest. Diagnose setup errors without changing DSP behaviour.
   Ask before changing SDK versions or the firmware implementation.
7. Verify both UF2 outputs. Copy them into the ignored firmware folder as
   adc_dsp_suite.uf2 and dsp_software_selftest.uf2, respectively, only after
   successful builds. Report tool versions, output paths and build status.
8. Stop before flashing any device. Ask me to identify the Pico and confirm
   flashing; explain BOOTSEL and which UF2 is the normal firmware.
9. Keep me on testing/dsp-suite. Do not merge into main, create a release,
   change the default branch, commit, or push anything during this setup.
10. Once setup succeeds, guide me through TEST_GUIDE.md one checkpoint at a
    time, beginning with safe wiring and measured ADC input voltage.
    Wait for my measurements before claiming a hardware test passes.
```

Codex access is governed by filesystem/network permissions and administrator
policy; this prompt does not grant unrestricted access. You may need to approve
downloads, complete a GUI/sign-in step, or ask IT for installation permission.
See [OpenAI's permissions documentation](https://learn.chatgpt.com/docs/permissions).

## 2. Retrieve the branch manually if needed

In a writable parent folder, with Git installed, run:

```sh
git clone --branch testing/dsp-suite https://github.com/Asyraf-Khairul/ADC-DSP-Basic-Tests.git ADC-DSP-Basic-Tests-testing
cd ADC-DSP-Basic-Tests-testing
git branch --show-current
git log -1 --oneline
git status --short
```

Expected branch: `testing/dsp-suite`. Expected status: no local changes.
Do not run the clone command into an existing folder.
If GitHub requests authentication, use your approved sign-in flow; do not paste
passwords or tokens into chat or commit them. Write access is needed to push.

For an existing clone, first check `git remote -v` and `git status --short`.
Only with a clean working tree and the correct repository:

```sh
git fetch origin
git switch testing/dsp-suite
git pull --ff-only origin testing/dsp-suite
```

If no local testing branch exists and automatic tracking does not select it,
use `git switch --track origin/testing/dsp-suite` instead of the switch line.
If switching or fast-forwarding fails, stop and inspect; do not use force/reset.

## 3. Install or select build tools

The easiest Windows route is VS Code plus the official Raspberry Pi Pico
extension. Its extension ID is `raspberry-pi.raspberry-pi-pico`.
If VS Code's command-line launcher is available and installation is allowed:

```sh
code --install-extension raspberry-pi.raspberry-pi-pico
code .
```

Use the extension's existing-project/import workflow for this folder, select
`pico`, and match the versions in `CMakeLists.txt`. Review any import-generated
changes; do not overwrite the source or commit machine-specific paths.
The extension can manage SDK/build-tool downloads. Refer to the
[official Pico extension instructions](https://github.com/raspberrypi/pico-vscode)
and [Raspberry Pi's setup walkthrough](https://www.raspberrypi.com/news/get-started-with-raspberry-pi-pico-series-and-vs-code/).

Required for a command-line build: Git, CMake, Ninja, working Python 3,
Arm GNU embedded compiler (`arm-none-eabi-gcc/g++`), Pico SDK and picotool.
Prefer SDK 2.1.0 / toolchain 13_3_Rel1 / picotool 2.1.0 to reproduce the
existing build. Reuse installed compatible tools rather than installing duplicates.
Do not copy build caches from another PC: they contain absolute local paths.

In the Pico-configured terminal, configure and build:

```sh
cmake -S . -B build -G Ninja -DPICO_BOARD=pico -DCMAKE_BUILD_TYPE=Release -DBENCH_SELF_TEST=OFF
cmake --build build
cmake -S . -B build-selftest -G Ninja -DPICO_BOARD=pico -DCMAKE_BUILD_TYPE=Release -DBENCH_SELF_TEST=ON
cmake --build build-selftest
```

Check that each command succeeds before continuing. Outside a configured
terminal, locate the installed tools and supply `PICO_SDK_PATH`,
`PICO_TOOLCHAIN_PATH`, `CMAKE_MAKE_PROGRAM` and `Python3_EXECUTABLE` as needed.
Use paths discovered on THIS computer, not `C:/Users/asyra/...`.
On Windows, `python --version` must launch real Python, not the Store alias.

Expected outputs:

| Build | Output | Purpose |
| --- | --- | --- |
| Normal | `build/adc_dsp_standalone.uf2` | Live ADC, AFG, OLED and GP16 audio tests |
| Self-test | `build-selftest/adc_dsp_standalone.uf2` | Synthetic DSP tests, no live ADC/PWM |

Both files have the same base name, so keep their directories distinct.
After successful builds, these PowerShell commands create the aliases used
in the hardware guide (replace existing aliases only if they are your own
previous generated builds):

```powershell
New-Item -ItemType Directory -Path firmware -Force
Copy-Item -LiteralPath build/adc_dsp_standalone.uf2 -Destination firmware/adc_dsp_suite.uf2
Copy-Item -LiteralPath build-selftest/adc_dsp_standalone.uf2 -Destination firmware/dsp_software_selftest.uf2
Get-FileHash firmware/adc_dsp_suite.uf2 -Algorithm SHA256
Get-FileHash firmware/dsp_software_selftest.uf2 -Algorithm SHA256
```

`build/`, `build-selftest/` and `firmware/` are intentionally ignored by Git.
They are not missing from your clone by accident. Rebuild them on the new PC.
Building successfully is not proof of physical ADC/DSP operation.

## 4. Flash only when ready

Disconnect generator signals first. Confirm the target is your RP2040 Pico.
Hold BOOTSEL while connecting USB; after its `RPI-RP2` drive appears, copy
the chosen UF2 onto that drive. This replaces the Pico's running firmware,
not your GitHub source. Keep a known-good UF2 if you want to restore it.

Start with the normal firmware. It uses GP27=I, GP26=Q, GP16=audio and
GP18/GP19=SH1106 OLED. The encoder is GP20/GP21; Button 1 is GP17
(page/back/hold for coarse tuning), and Button 2 is GP22 (menu/select/apply).
Normal firmware can be configured and observed entirely on the OLED;
serial is optional for detailed logs. Read the voltage limits and wiring checkpoints in
[TEST_GUIDE.md](TEST_GUIDE.md) BEFORE connecting generator outputs.
The software-test firmware instead waits for a USB serial connection and
runs synthetic checks. Reflash normal firmware before AFG testing.

## 5. Continue between home and university

Before work, check you are on `testing/dsp-suite`, inspect local changes, then
use `git pull --ff-only` when clean. Before leaving, review your changes and
explicitly ask Codex to commit and push selected files to this testing branch.
Only committed AND pushed changes are available on the other computer.
If the university clears local files at logout, push approved work beforehand.

Keep measured results separate from expected results. Record commit, UF2 hash,
settings and observations for each test. Do not merge into `main` or create an
official release until you explicitly decide the testing is complete.
Sign out of GitHub/Codex on shared PCs and follow university credential policy.
