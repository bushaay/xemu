# Audio Debug Window UI Layout

This document shows the structure and layout of the enhanced Audio Debug window.

```
┌─────────────────────────────────────────────────────────────────────────┐
│ Audio Debug                                                          [X] │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│ ┌───────────────────────────┬───────────────────────────────────────┐ │
│ │ VOICE GRID (16x16)        │ CONTROLS                              │ │
│ │                           │                                       │ │
│ │ [00][01][02]...[0F]       │ Monitor: [AC97 ▼]                     │ │
│ │ [10][11][12]...[1F]       │                                       │ │
│ │ [20][21][22]...[2F]       │ ☑ GP Realtime                         │ │
│ │  ...                      │ ☑ EP Realtime                         │ │
│ │ [F0][F1][F2]...[FF]       │ ☑ HRTF Filtering                      │ │
│ │                           │                                       │ │
│ │ Green = Active voice      │ Utilization: 45.2%                    │ │
│ │ Gray = Inactive           │ Frames:      1234                     │ │
│ │ Red = Muted               │ VP:          142 us                   │ │
│ │                           │ ▼ VP Workers                          │ │
│ │ Hover = Isolate (solo)    │ GP Cycles:   0234                     │ │
│ │ Right-click = Toggle mute │ EP Cycles:   0056                     │ │
│ └───────────────────────────┴───────────────────────────────────────┘ │
│                                                                         │
│ ═══════════════════════════════════════════════════════════════════   │
│ Enhanced Audio Debugging Suite                                         │
│ ─────────────────────────────────────────────────────────────────────  │
│                                                                         │
│ ▼ Audio Statistics & Health                                            │
│ ├─ Buffer Status:                                                      │
│ │   • Underruns: 0                                                     │
│ │   • Overruns:  0                                                     │
│ │                                                                       │
│ ├─ Voice Activity:                                                     │
│ │   • Active Voices: 12 / 256                                          │
│ │                                                                       │
│ ├─ Output Levels:                                                      │
│ │   • Peak L: -12.3 dB, R: -11.8 dB                                   │
│ │   • Avg  L: -24.5 dB, R: -23.9 dB                                   │
│ │                                                                       │
│ ├─ Warnings:                                                           │
│ │   ⚠ WARNING: Potential audio loop detected!                         │
│ │   ⚠ WARNING: Potential buzzing/hissing detected!                    │
│ │   ℹ Sample rate changed to: 48000 Hz                                │
│ │                                                                       │
│ └─ [Reset Statistics]                                                  │
│                                                                         │
│ ▼ Audio Waveform Visualization                                         │
│ ┌───────────────────────────────────────────────────────────────────┐ │
│ │ 1.2 ┤                                                             │ │
│ │     │     ╱╲      ╱╲                                              │ │
│ │ 0.6 ┤    ╱  ╲    ╱  ╲                                             │ │
│ │     │   ╱    ╲  ╱    ╲                                            │ │
│ │ 0.0 ┼──╯──────╲╱──────╲──────────────────────────────────────    │ │
│ │     │                  ╲      ╱╲                                  │ │
│ │-0.6 ┤                   ╲    ╱  ╲                                 │ │
│ │     │                    ╲  ╱    ╲                                │ │
│ │-1.2 ┤                     ╲╱      ╲                               │ │
│ │     └────────────────────────────────────────────────────────────│ │
│ │     Left Channel (Green)  Right Channel (Red)                    │ │
│ └───────────────────────────────────────────────────────────────────┘ │
│                                                                         │
│ ▼ Per-Voice Analysis                                                   │
│ ┌───────────────────────────────────────────────────────────────────┐ │
│ │ V# │ Active │ Rate(Hz) │ Loop │ Samples  │ Peak(L/R) │ Avg(L/R)  │ │
│ ├────┼────────┼──────────┼──────┼──────────┼───────────┼───────────┤ │
│ │ 03 │ Active │   48000  │    5 │   123456 │ 0.85/0.82 │ 0.42/0.39 │ │
│ │ 07 │ Active │   44100  │  142 │   987654 │ 0.95/0.91 │ 0.55/0.52 │ │
│ │ 12 │ Paused │   24000  │    0 │    45678 │ 0.12/0.15 │ 0.05/0.06 │ │
│ │ 1A │ Active │   48000  │    2 │   234567 │ 0.78/0.76 │ 0.38/0.35 │ │
│ │ ... (scrollable)                                                  │ │
│ └───────────────────────────────────────────────────────────────────┘ │
│                                                                         │
│ ▼ Debugging Tools                                                      │
│ ├─ Voice Isolation: [Unmute All]                                      │
│ │                                                                       │
│ ├─ Debug Logging:                                                      │
│ │   ☑ Enable Audio Logging                                            │
│ │   (ℹ Log audio events to console for debugging)                     │
│ │                                                                       │
│ └─ Configuration:                                                      │
│     ☑ Log Buffer Underruns                                             │
│     ☑ Log Sample Rate Changes                                          │
│     ☑ Detect Audio Loops                                               │
│     ☐ Show Mixer Analysis                                              │
│                                                                         │
│ ▼ Issue #904 Notes                                                     │
│ ├─ This debugging suite helps diagnose common audio issues:           │
│ │   • Audio Looping - Check loop counts in Per-Voice Analysis         │
│ │   • Buzzing/Hissing - Monitor waveform for abnormal patterns        │
│ │   • Sample Rate Issues - Watch for sample rate change notifications │
│ │   • Mixing Issues - Use voice isolation to identify problem voices  │
│ │   • Buffer Issues - Monitor underruns/overruns in statistics        │
│ └─────────────────────────────────────────────────────────────────── │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

## Main Menu Integration

```
┌─────────────────────────────────────────────────────────────┐
│ Machine  System  Input  Display  Audio  Network  Debug  Help│
└─────────────────────────────────────────────────────────────┘
                                      │
                                      ▼
                            ┌─────────────────────┐
                            │ Audio Settings      │
                            ├─────────────────────┤
                            │ Volume              │
                            │ ━━━━━━━━━━━ 100%   │
                            │                     │
                            │ Quality             │
                            │ ☑ Real-time DSP     │
                            │                     │
                            │ Audio Debugging     │ ◄── NEW
                            │ ☑ Enable logging    │ ◄── NEW
                            │ ☑ Detect loops      │ ◄── NEW
                            │ ☑ Log underruns     │ ◄── NEW
                            │ ☑ Log rate changes  │ ◄── NEW
                            └─────────────────────┘
```

## Debug Menu

```
┌─────────────────────────────────────────────────────────────┐
│ Machine  System  Input  Display  Audio  Network  Debug  Help│
└─────────────────────────────────────────────────────────────┘
                                                    │
                                                    ▼
                                          ┌───────────────────┐
                                          │ Monitor       ~   │
                                          │ Audio             │ ◄── Opens window
                                          │ Video             │
                                          │ RenderDoc: Capture│
                                          └───────────────────┘
```

## Color Coding

- **Green buttons** = Active voices (pulsing brightness)
- **Gray buttons** = Inactive voices
- **Red hue** = Muted voices
- **Orange text** = Warnings (loops, buzzing)
- **Blue text** = Info (sample rate changes)
- **Green waveform** = Left channel
- **Red waveform** = Right channel

## Interactive Elements

### Voice Grid Buttons
- **Hover** → Isolates (solos) that voice
- **Right-click** → Toggles mute for that voice
- **Tooltip** → Shows detailed voice information

### Collapsible Headers
- **Click arrow** → Expand/collapse section
- **Sections remember state** across sessions

### Scrollable Areas
- **Per-Voice Analysis** → Scrolls if many voices active
- **Mouse wheel** → Scroll through voice list

## Key Features Visible in UI

1. **Voice Grid** - Visual overview of all 256 voices at a glance
2. **Statistics Panel** - Real-time health monitoring
3. **Waveform Plot** - Actual audio visualization
4. **Voice Table** - Detailed per-voice metrics
5. **Warning System** - Automatic problem detection
6. **Configuration** - User-controllable debug options

## Usage Flow

```
1. User notices audio problem in game
   ↓
2. Opens Debug → Audio window
   ↓
3. Checks Statistics for warnings
   ↓
4. If loop warning → Check Per-Voice Analysis for high loop counts
   ↓
5. If buzzing warning → Check Waveform Visualization
   ↓
6. Hovers over suspected voice to isolate it
   ↓
7. Confirms which voice is problematic
   ↓
8. Reports issue with voice number and settings
```

## Screen Space

- **Main area**: Voice grid + controls (existing layout preserved)
- **Enhanced area**: New sections below, collapsible to save space
- **Total height**: Approximately 800-1000 pixels when fully expanded
- **Width**: Same as existing window (~600 pixels scaled)

All sections are collapsible to minimize screen space when not needed!
