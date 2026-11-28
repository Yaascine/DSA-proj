# Portability note (macOS core ↔ Windows MFC GUI)

## Verified here (macOS, Apple clang 21, CMake 4.5.1)
- `make test` and the CMake/CTest build are both green.
- `core` is pure C++20 and contains **no** `<windows.h>`, MFC/`afx*`, `<unistd.h>`,
  `__attribute__`, or `M_PI` (checked by search). It uses only standard headers.
- Determinism holds across `-O2` and `-O3 -DNDEBUG`.

## For the Windows/MSVC build (P4)
The core is written to compile on MSVC (Visual Studio 2022, `/std:c++20`):
- Standard C++20 only; no compiler-specific extensions.
- `<inttypes.h>` `PRIu64`/`PRIx64`, `std::filesystem`, and aggregate init are MSVC-supported.
- CMake sets `CMAKE_CXX_STANDARD 20`; the MFC GUI is added only under `if(WIN32)`.

**Not yet done (cannot be done on macOS):** an actual MSVC compile and the MFC GUI build.
P4 should do an early Windows build (Day 8 at the latest) to catch any MSVC quirks before
integration. If MSVC flags anything in `core`, fix it there (keep it portable) rather than
adding Windows-only code to `core`.

## Suggested Windows build (P4)
```
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```
