# OpenQ4 SDL3 Input Key Matrix Audit (2026-02-06)

This checklist audits SDL3 input parity against the legacy Win32 path for:

- console
- GUI edit fields
- chat
- binds
- numpad
- modifiers

## Method

- Reviewed event generation in `src/sys/win32/win_sdl3.cpp`.
- Compared behavior to legacy `WM_KEY*` + `WM_CHAR`/DirectInput paths in:
  - `src/sys/win32/win_wndproc.cpp`
  - `src/sys/win32/win_input.cpp`
- Reviewed consumer paths:
  - `src/framework/Console.cpp`
  - `src/framework/EditField.cpp`
  - `src/ui/EditWindow.cpp`
  - `src/ui/Window.cpp`
  - `src/framework/Session.cpp`

## Checklist

| Area | Coverage | Result | Notes |
|---|---|---|---|
| Console input | Toggle key, enter/submit, tab-complete, history, paging, backspace, ctrl shortcuts | Pass | `SE_KEY` + `SE_CHAR` paths align for core behavior. |
| GUI edit fields | Backspace/delete, cursor movement, enter handling, ctrl-h/ctrl-a/ctrl-e style chars | Pass | Backspace regression fixed via control-char synthesis on keydown. |
| Chat input | Text entry/editing in GUI-driven chat fields | Pass (ASCII/control) | Same event flow as edit controls; control chars restored. |
| Binds | Key-down bind execution in session loop (including function keys and mouse/wheel) | Pass (core) | `SE_KEY` behavior is aligned for common bindable keys. |
| Numpad | KP enter/arrows/home/end/ins/del, KP operators, KP equals | Pass (core) | SDL3 mapping includes KP key families used by id key enums. |
| Modifiers | Ctrl/Shift/Alt/RightAlt behavior | Pass (core) | RightAlt locale behavior now matches legacy intent for english vs selected non-english languages. |

## Remaining Parity Gaps

1. Non-English key layout parity is still incomplete.
   - Legacy Win32 selected per-language scan tables (`english/spanish/french/german/italian`) for bind/key semantics.
   - SDL3 path currently uses a fixed scancode mapping for many printable keys.
   - Impact: certain non-English layout key identities (for binds/console key expectations) can still differ from legacy behavior.

2. Console key localization parity is incomplete outside the default mapping.
   - Legacy `Sys_GetConsoleKey` depended on language-specific scan table selection.
   - SDL3 currently returns the static grave/tilde slot from the fixed scan table.
   - Impact: console-toggle key expectations on non-English layouts may differ.

3. Text input is byte-oriented for `SDL_EVENT_TEXT_INPUT`.
   - SDL text payload is UTF-8, but current queueing emits one `SE_CHAR` per byte.
   - Impact: multi-byte characters/IME paths are not fully represented at the event layer.

## Recommended Next Steps

1. Port legacy language scan-table behavior into SDL3 key translation, or replace with an SDL layout-aware translation strategy that preserves legacy bind semantics.
2. Make `Sys_GetConsoleKey` layout-aware in SDL3 path so non-English console toggle parity matches legacy expectations.
3. Add UTF-8 decoding before emitting `SE_CHAR` events to avoid byte-splitting non-ASCII input.
