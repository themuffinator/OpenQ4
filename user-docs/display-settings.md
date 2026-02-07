# Display Settings and Multi-Screen Guide

This guide covers OpenQ4 display/window settings for end users, including multi-monitor behavior and aspect controls.

## Quick Start

- Press `Alt+Enter` to toggle fullscreen/windowed mode.
- Run `listDisplays` in the console to list monitor indices for `r_screen`.
- After changing most video cvars, run `vid_restart`.

## Core Display Settings

| Setting | Default | What it does |
|---|---:|---|
| `r_fullscreen` | `1` | `1` = fullscreen, `0` = windowed. |
| `r_borderless` | `0` | Borderless window mode when `r_fullscreen 0`. |
| `r_windowWidth` | `1280` | Windowed width. |
| `r_windowHeight` | `720` | Windowed height. |
| `r_mode` | `3` | Preset mode index. Use `-1` for custom width/height. |
| `r_customWidth` | `720` | Custom width used when `r_mode -1`. |
| `r_customHeight` | `486` | Custom height used when `r_mode -1`. |
| `r_displayRefresh` | `0` | Requested fullscreen refresh rate (0 = default/driver choice). |
| `r_screen` | `-1` | SDL3 monitor target (`-1` auto/current, `0..N` explicit index). |

## Aspect Ratio and FOV

| Setting | Default | Values |
|---|---:|---|
| `r_aspectRatio` | `-1` | `-1` auto, `0` 4:3, `1` 16:9, `2` 16:10 |

- `r_aspectRatio -1` uses the current render size automatically, so FOV follows your actual output aspect.

## UI Aspect Correction (New)

This controls 2D UI layout behavior (menu, HUD, console, loading/initializing screens):

| Setting | Default | What it does |
|---|---:|---|
| `ui_aspectCorrection` | `1` | `1` keeps classic 4:3-style correction for all 2D UI. `0` stretches 2D UI to the full 2D draw region. |

## Multi-Monitor Behavior (New)

When the render surface spans multiple monitors:

- 2D elements (console, HUD, menus, loading/initializing UI) are constrained to the **primary display region**.
- 2D aspect behavior inside that region is controlled by `ui_aspectCorrection`.
- Menu cursor mapping follows the same 2D region so mouse interaction stays aligned.

3D world rendering is unchanged by these UI cvars.

## Useful Console Examples

### Recommended Modern Defaults

```cfg
seta r_screen -1
seta r_aspectRatio -1
seta ui_aspectCorrection 1
vid_restart
```

### Borderless Window on a Specific Monitor

```cfg
seta r_fullscreen 0
seta r_borderless 1
seta r_screen 1
vid_restart
```

### Custom Fullscreen Resolution

```cfg
seta r_fullscreen 1
seta r_mode -1
seta r_customWidth 2560
seta r_customHeight 1440
vid_restart
```

### Stretch Menu + HUD (No 4:3 Correction)

```cfg
seta ui_aspectCorrection 0
```

## Troubleshooting

- If a display change does not apply, run `vid_restart`.
- If monitor targeting looks wrong, run `listDisplays`, then set `r_screen` to the correct index and restart video.
- If UI appears too centered/boxed on wide displays, set `ui_aspectCorrection 0`.
