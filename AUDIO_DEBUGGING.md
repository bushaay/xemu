# Audio Debugging Suite

This document describes the comprehensive audio debugging suite added to xemu to help diagnose and fix audio issues, particularly those tracked in [issue #904](https://github.com/xemu-project/xemu/issues/904).

## Overview

The audio debugging suite provides tools to monitor, analyze, and debug audio processing in real-time. It helps identify common audio problems including:

- **Audio Looping**: Detection of buffers that loop excessively
- **Buzzing/Hissing**: Monitoring for abnormal audio patterns
- **Sample Rate Issues**: Tracking sample rate changes
- **Mixing Problems**: Per-voice analysis and isolation
- **Buffer Health**: Monitoring underruns and overruns

## Features

### 1. Audio Statistics & Health

The statistics panel shows real-time information about audio processing:

- **Buffer Status**: Counts of buffer underruns and overruns
- **Voice Activity**: Number of currently active voices
- **Output Levels**: Peak and average levels for left and right channels (in dB)
- **Warnings**: Real-time detection of:
  - Potential audio loops
  - Potential buzzing/hissing
  - Sample rate changes

### 2. Audio Waveform Visualization

Real-time waveform display showing the left and right audio channels. This helps identify:
- Abnormal patterns that may cause buzzing
- Clipping issues
- Stereo balance problems

### 3. Per-Voice Analysis

Detailed table showing all active voices with:
- Voice number
- Active/Paused status
- Sample rate (Hz)
- Loop count (helps identify looping issues)
- Total samples processed
- Peak amplitude (L/R channels)
- Average amplitude (L/R channels)

Voices with high loop counts (>100) are highlighted as they may indicate looping problems.

### 4. Debugging Tools

#### Voice Isolation
- Hover over a voice button to isolate it (hear only that voice)
- Right-click a voice button to toggle mute
- "Unmute All" button to clear all mutes

#### Debug Logging
Toggle logging of audio events to the console for detailed debugging.

#### Configuration Options
- **Log Buffer Underruns**: Record buffer underrun events
- **Log Sample Rate Changes**: Track when sample rates change
- **Detect Audio Loops**: Enable loop detection algorithm
- **Show Mixer Analysis**: Display detailed mixing information

## How to Use

### Opening the Audio Debug Window

1. Launch xemu
2. Click on **Debug** in the menu bar
3. Select **Audio**

The Audio Debug window will open showing all debugging information.

### Diagnosing Common Issues

#### Audio Looping (Issue #904)
1. Open the Audio Debug window
2. Expand "Per-Voice Analysis"
3. Look for voices with high loop counts (highlighted in orange)
4. Use voice isolation to identify which voice is causing the loop
5. Note the voice number and settings for bug reports

#### Buzzing/Hissing (Issue #904)
1. Open "Audio Waveform Visualization"
2. Look for abnormal patterns:
   - High-frequency oscillations
   - Sudden spikes
   - DC offset
3. Check "Audio Statistics" for the buzzing detection warning
4. Use per-voice analysis to identify problematic voices

#### Sample Rate Issues (Issue #904)
1. Enable "Log Sample Rate Changes" in Debugging Tools
2. Watch the "Audio Statistics" panel for sample rate change notifications
3. The detected sample rate will be displayed when changes occur

#### Mixing Problems (Issue #904)
1. Use the voice grid (colored buttons) at the top
2. Hover over voices to isolate them
3. Right-click to mute specific voices
4. Check per-voice amplitude levels to identify balance issues

## Configuration

Audio debugging options can be configured in the config file or through the UI:

```yaml
audio:
  debug:
    enable_logging: bool          # Enable console logging
    log_buffer_underruns: bool    # Log buffer underruns
    log_sample_rate_changes: bool # Log sample rate changes
    detect_audio_loops: bool      # Enable loop detection
    visualize_waveforms: bool     # Enable waveform display
    capture_to_file: bool         # Capture audio to file (future)
    show_mixer_analysis: bool     # Show mixer details (future)
```

## Technical Details

### Debug Data Structures

The debug system tracks the following information per voice:

```c
struct McpxApuDebugVoice {
    // ... standard voice info ...
    
    // Enhanced debugging info
    uint32_t last_ba;          // Last buffer address
    uint32_t loop_count;       // Loop counter
    uint64_t total_samples;    // Total samples processed
    float peak_amplitude_l;    // Peak L channel
    float peak_amplitude_r;    // Peak R channel
    float avg_amplitude_l;     // Average L channel
    float avg_amplitude_r;     // Average R channel
};
```

### Global Statistics

```c
struct McpxApuDebugAudioStats {
    uint32_t buffer_underruns;        // Underrun count
    uint32_t buffer_overruns;         // Overrun count
    float detected_sample_rate;       // Current sample rate
    bool sample_rate_changed;         // Rate change flag
    float output_peak_l/r;            // Output peaks
    float output_avg_l/r;             // Output averages
    bool potential_loop_detected;     // Loop detection flag
    bool potential_buzzing_detected;  // Buzz detection flag
    uint32_t active_voice_count;      // Active voices
    float waveform_l[256];            // L waveform buffer
    float waveform_r[256];            // R waveform buffer
};
```

## API Functions

### Debug Control
- `mcpx_apu_debug_reset_stats()` - Reset all statistics
- `mcpx_apu_debug_set_logging_enabled(bool)` - Enable/disable logging
- `mcpx_apu_debug_get_logging_enabled()` - Get logging state

### Voice Control
- `mcpx_apu_debug_mute_all_except(voice)` - Solo a voice
- `mcpx_apu_debug_unmute_all()` - Clear all mutes
- `mcpx_apu_debug_toggle_mute(voice)` - Toggle voice mute
- `mcpx_apu_debug_is_muted(voice)` - Check mute status

## Reporting Bugs

When reporting audio issues, include:

1. Screenshot of the Audio Debug window
2. Per-voice analysis showing problematic voices
3. Waveform screenshot if buzzing is present
4. Any warnings/alerts shown in Statistics panel
5. Sample rate information
6. Voice isolation results (which voice causes the issue)

## Future Enhancements

Planned features:
- Audio capture to WAV file for offline analysis
- Detailed mixer routing visualization
- Spectrum analyzer
- Voice history timeline
- Automatic issue detection and suggestions

## Credits

This debugging suite was created to help diagnose and fix issues tracked in xemu issue #904, which catalogs various audio problems including looping, buzzing, sample rate issues, and mixing problems.
