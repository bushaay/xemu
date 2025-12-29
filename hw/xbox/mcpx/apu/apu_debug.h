/*
 * QEMU MCPX Audio Processing Unit implementation
 *
 * Copyright (c) 2020-2021 Matt Borgerson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MCPX_APU_DEBUG_H
#define MCPX_APU_DEBUG_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_VOICE_WORKERS 16
#define AUDIO_DEBUG_HISTORY_SIZE 512
#define AUDIO_DEBUG_WAVEFORM_SIZE 256

typedef enum McpxApuDebugMonitorPoint {
    MCPX_APU_DEBUG_MON_AC97,
    MCPX_APU_DEBUG_MON_VP,
    MCPX_APU_DEBUG_MON_GP,
    MCPX_APU_DEBUG_MON_EP,
    MCPX_APU_DEBUG_MON_GP_OR_EP
} McpxApuDebugMonitorPoint;

struct McpxApuDebugVoice
{
    bool active;
    bool paused;
    bool stereo;
    uint8_t bin[8];
    uint16_t vol[8];

    bool stream;
    bool loop;
    bool persist;
    bool multipass;
    bool linked;
    uint8_t multipass_bin;
    uint16_t multipass_dst_voice;
    int container_size, sample_size;
    unsigned int samples_per_block;
    uint32_t ebo, cbo, lbo, ba;
    float rate;
    
    // Enhanced debugging info for issue #904
    uint32_t last_ba;          // Last buffer address for loop detection
    uint32_t loop_count;       // Number of times buffer looped
    uint64_t total_samples;    // Total samples processed
    float peak_amplitude_l;    // Peak amplitude left channel
    float peak_amplitude_r;    // Peak amplitude right channel
    float avg_amplitude_l;     // Average amplitude left channel
    float avg_amplitude_r;     // Average amplitude right channel
};

struct McpxApuDebugVp
{
    struct McpxApuDebugVoice v[256];
    int num_workers;
    struct {
        int num_voices;
        int time_us;
    } workers[MAX_VOICE_WORKERS];
    int total_worker_time_us;
};

struct McpxApuDebugDsp
{
    int cycles;
};

struct McpxApuDebugAudioStats
{
    // Buffer health
    uint32_t buffer_underruns;
    uint32_t buffer_overruns;
    
    // Sample rate tracking
    float detected_sample_rate;
    bool sample_rate_changed;
    
    // Output monitoring
    float output_peak_l;
    float output_peak_r;
    float output_avg_l;
    float output_avg_r;
    
    // Issue #904 specific tracking
    bool potential_loop_detected;    // Audio looping detection
    bool potential_buzzing_detected; // Buzzing/hissing detection
    uint32_t active_voice_count;     // Number of active voices
    
    // Waveform visualization data (ring buffer)
    float waveform_l[AUDIO_DEBUG_WAVEFORM_SIZE];
    float waveform_r[AUDIO_DEBUG_WAVEFORM_SIZE];
    int waveform_idx;
};

struct McpxApuDebug
{
    struct McpxApuDebugVp vp;
    struct McpxApuDebugDsp gp, ep;
    int frames_processed;
    float utilization;
    bool gp_realtime, ep_realtime;
    
    // Enhanced debugging statistics
    struct McpxApuDebugAudioStats stats;
};

#ifdef __cplusplus
extern "C" {
#endif

const struct McpxApuDebug *mcpx_apu_get_debug_info(void);
McpxApuDebugMonitorPoint mcpx_apu_debug_get_monitor(void);
void mcpx_apu_debug_set_monitor(McpxApuDebugMonitorPoint monitor);
void mcpx_apu_debug_isolate_voice(uint16_t v);
void mcpx_apu_debug_clear_isolations(void);
void mcpx_apu_debug_toggle_mute(uint16_t v);
bool mcpx_apu_debug_is_muted(uint16_t v);
void mcpx_apu_debug_set_gp_realtime_enabled(bool enable);
void mcpx_apu_debug_set_ep_realtime_enabled(bool enable);

// Enhanced debugging functions for issue #904
void mcpx_apu_debug_reset_stats(void);
void mcpx_apu_debug_set_logging_enabled(bool enable);
bool mcpx_apu_debug_get_logging_enabled(void);
void mcpx_apu_debug_mute_all_except(uint16_t v);
void mcpx_apu_debug_unmute_all(void);

#ifdef __cplusplus
}
#endif

#endif
