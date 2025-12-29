//
// xemu User Interface
//
// Copyright (C) 2020-2022 Matt Borgerson
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "debug.hh"
#include "common.hh"
#include "misc.hh"
#include "font-manager.hh"
#include "viewport-manager.hh"
#include "widgets.hh"
#include "../xemu-settings.h"
#include <cmath>

#define MAX_VOICES 256

DebugApuWindow::DebugApuWindow() : m_is_open(false), m_show_advanced(false),
    m_show_waveform(false), m_show_voice_details(false), m_show_mixer_analysis(false)
{
}

void DebugApuWindow::Draw()
{
    if (!m_is_open)
        return;

    ImGui::SetNextWindowContentSize(ImVec2(600.0f*g_viewport_mgr.m_scale, 0.0f));
    if (!ImGui::Begin("Audio Debug", &m_is_open,
                      ImGuiWindowFlags_NoCollapse |
                          ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    const struct McpxApuDebug *dbg = mcpx_apu_get_debug_info();


    ImGui::Columns(2, "", false);
    ImGui::SetColumnWidth(0, 360*g_viewport_mgr.m_scale);

    int now = SDL_GetTicks() % 1000;
    float t = now/1000.0f;
    float freq = 1;
    float v = fabs(sin(M_PI*t*freq));
    float c_active = mix(0.4, 0.97, v);
    float c_inactive = 0.2f;

    int voice_monitor = -1;
    int voice_info = -1;
    int voice_mute = -1;

    ImGui::PushFont(g_font_mgr.m_fixed_width_font);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2*g_viewport_mgr.m_scale, 2*g_viewport_mgr.m_scale));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4*g_viewport_mgr.m_scale, 4*g_viewport_mgr.m_scale));
    for (int i = 0; i < MAX_VOICES; i++) {
        if (i % 16) {
            ImGui::SameLine();
        }

        float c, s, h;
        h = 0.6;
        if (dbg->vp.v[i].active) {
            if (dbg->vp.v[i].paused) {
                c = c_inactive;
                s = 0.4;
            } else {
                c = c_active;
                s = 0.7;
            }
            if (mcpx_apu_debug_is_muted(i)) {
                h = 1.0;
            }
        } else {
            c = c_inactive;
            s = 0;
        }

        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(h, s, c));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(h, s, 0.8));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(h, 0.8f, 1.0));
        char buf[12];
        snprintf(buf, sizeof(buf), "%02x", i);
        ImGui::Button(buf);
        if (/*dbg->vp.v[i].active &&*/ ImGui::IsItemHovered()) {
            voice_monitor = i;
            voice_info = i;
        }
        if (ImGui::IsItemClicked(1)) {
            voice_mute = i;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopID();
    }
    ImGui::PopStyleVar(3);
    ImGui::PopFont();

    if (voice_info >= 0) {
        const struct McpxApuDebugVoice *voice = &dbg->vp.v[voice_info];
        ImGui::BeginTooltip();
        bool is_paused = voice->paused;
        ImGui::Text("Voice 0x%x/%d %s", voice_info, voice_info, is_paused ? "(Paused)" : "");
        ImGui::SameLine();
        ImGui::Text(voice->stereo ? "Stereo" : "Mono");

        ImGui::Separator();
        ImGui::PushFont(g_font_mgr.m_fixed_width_font);

        const char *noyes[2] = { "NO", "YES" };
        ImGui::Text("Stream: %-3s Loop: %-3s Persist: %-3s Multipass: %-3s "
                    "Linked: %-3s",
                    noyes[voice->stream], noyes[voice->loop],
                    noyes[voice->persist], noyes[voice->multipass],
                    noyes[voice->linked]);

        const char *cs[4] = { "1 byte", "2 bytes", "ADPCM", "4 bytes" };
        const char *ss[4] = {
            "Unsigned 8b PCM",
            "Signed 16b PCM",
            "Signed 24b PCM",
            "Signed 32b PCM"
        };

        assert(voice->container_size < 4);
        assert(voice->sample_size < 4);
        const char *spb_or_bin_label = "Samples per Block";
        unsigned int spb_or_bin = voice->samples_per_block;
        if (voice->multipass) {
            spb_or_bin_label = "Multipass Bin";
            spb_or_bin = voice->multipass_bin;
        }
        ImGui::Text("Container Size: %s, Sample Size: %s, %s: %d",
                    cs[voice->container_size], ss[voice->sample_size],
                    spb_or_bin_label, spb_or_bin);
        ImGui::Text("Rate: %f (%d Hz)", voice->rate, (int)(48000.0/voice->rate));
        ImGui::Text("EBO=%d CBO=%d LBO=%d BA=%x",
            voice->ebo, voice->cbo, voice->lbo, voice->ba);
        ImGui::Text("Mix: ");
        for (int i = 0; i < 8; i++) {
            if (i == 4) ImGui::Text("     ");
            ImGui::SameLine();
            char buf[64];
            if (voice->vol[i] == 0xFFF) {
                snprintf(buf, sizeof(buf),
                    "Bin %2d (MUTE) ", voice->bin[i]);
            } else {
                snprintf(buf, sizeof(buf),
                    "Bin %2d (-%.3f) ", voice->bin[i],
                    (float)((voice->vol[i] >> 6) & 0x3f) +
                    (float)((voice->vol[i] >> 0) & 0x3f) / 64.0);
            }
            ImGui::Text("%-17s", buf);
        }

        int mon = mcpx_apu_debug_get_monitor();
        if (mon == MCPX_APU_DEBUG_MON_VP) {
            if (voice->multipass_dst_voice != 0xFFFF) {
                ImGui::Text("Multipass Dest Voice: 0x%02x",
                            voice->multipass_dst_voice);
            }
            if (voice->multipass) {
                ImGui::Text("Multipass Src Voices:");
                int n = 0;

                for (int i = 0; i < MAX_VOICES; i++) {
                    if (dbg->vp.v[i].multipass_dst_voice == voice_info) {
                        if (n > 0 && ((n & 7) == 0)) {
                            ImGui::Text("                     ");
                        }
                        ImGui::SameLine();
                        ImGui::Text("0x%02x", i);
                        n++;
                    }
                }
            }
        }

        ImGui::PopFont();
        ImGui::EndTooltip();
    }

    if (voice_monitor >= 0) {
        mcpx_apu_debug_isolate_voice(voice_monitor);
    } else {
        mcpx_apu_debug_clear_isolations();
    }
    if (voice_mute >= 0) {
        mcpx_apu_debug_toggle_mute(voice_mute);
    }

    ImGui::NextColumn();

    static int mon = 0;
    mon = (int)mcpx_apu_debug_get_monitor();
    if (ImGui::Combo("Monitor", &mon, "AC97\0VP Only\0GP Only\0EP Only\0GP/EP if enabled\0")) {
        mcpx_apu_debug_set_monitor((McpxApuDebugMonitorPoint)mon);
    }

    static bool gp_realtime;
    gp_realtime = dbg->gp_realtime;
    if (ImGui::Checkbox("GP Realtime\n", &gp_realtime)) {
        mcpx_apu_debug_set_gp_realtime_enabled(gp_realtime);
    }

    static bool ep_realtime;
    ep_realtime = dbg->ep_realtime;
    if (ImGui::Checkbox("EP Realtime\n", &ep_realtime)) {
        mcpx_apu_debug_set_ep_realtime_enabled(ep_realtime);
    }

    ImGui::Checkbox("HRTF Filtering\n", &g_config.audio.hrtf);

    ImGui::PushFont(g_font_mgr.m_fixed_width_font);

    bool color = (dbg->utilization > 0.9);
    if (color) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,0,0,1));
    ImGui::Text("Utilization: %.2f%%", (dbg->utilization*100));
    if (color) ImGui::PopStyleColor();

    ImGui::Text("Frames:      %04d", dbg->frames_processed);
    ImGui::Text("VP:          %4d us", dbg->vp.total_worker_time_us);
    if (ImGui::TreeNode("VP Workers")) {
        ImGui::Text(" W: #  us");
        ImGui::SameLine();
        ImGui::Text(" W: #  us");
        for (int i = 0; i < dbg->vp.num_workers; i++) {
            if (i % 2) ImGui::SameLine();
            ImGui::Text("%2d:%2d %3d", i, dbg->vp.workers[i].num_voices,
                        dbg->vp.workers[i].time_us);
        }
        ImGui::TreePop();
    }
    ImGui::Text("GP Cycles:   %04d", dbg->gp.cycles);
    ImGui::Text("EP Cycles:   %04d", dbg->ep.cycles);

    ImGui::PopFont();
    ImGui::Columns(1);
    
    // Enhanced Audio Debugging Suite for Issue #904
    ImGui::Separator();
    ImGui::Text("Enhanced Audio Debugging Suite");
    
    if (ImGui::CollapsingHeader("Audio Statistics & Health", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        // Buffer health monitoring
        ImGui::Text("Buffer Status:");
        ImGui::BulletText("Underruns: %u", dbg->stats.buffer_underruns);
        ImGui::BulletText("Overruns:  %u", dbg->stats.buffer_overruns);
        
        // Active voice monitoring
        ImGui::Text("Voice Activity:");
        ImGui::BulletText("Active Voices: %u / 256", dbg->stats.active_voice_count);
        
        // Output level monitoring
        ImGui::Text("Output Levels:");
        ImGui::BulletText("Peak L: %.2f dB, R: %.2f dB", 
            20.0f * log10f(dbg->stats.output_peak_l + 0.0001f),
            20.0f * log10f(dbg->stats.output_peak_r + 0.0001f));
        ImGui::BulletText("Avg  L: %.2f dB, R: %.2f dB", 
            20.0f * log10f(dbg->stats.output_avg_l + 0.0001f),
            20.0f * log10f(dbg->stats.output_avg_r + 0.0001f));
        
        // Issue #904 specific warnings
        if (dbg->stats.potential_loop_detected) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.0f, 1.0f));
            ImGui::BulletText("WARNING: Potential audio loop detected!");
            ImGui::PopStyleColor();
        }
        
        if (dbg->stats.potential_buzzing_detected) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.0f, 1.0f));
            ImGui::BulletText("WARNING: Potential buzzing/hissing detected!");
            ImGui::PopStyleColor();
        }
        
        if (dbg->stats.sample_rate_changed) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.8f, 1.0f, 1.0f));
            ImGui::BulletText("Sample rate changed to: %.0f Hz", dbg->stats.detected_sample_rate);
            ImGui::PopStyleColor();
        }
        
        if (ImGui::Button("Reset Statistics")) {
            mcpx_apu_debug_reset_stats();
        }
        
        ImGui::Unindent();
    }
    
    if (ImGui::CollapsingHeader("Audio Waveform Visualization")) {
        ImGui::Indent();
        
        m_show_waveform = true;
        
        // Plot waveform data
        static ImPlotAxisFlags wf_axis = ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_NoGridLines;
        
        if (ImPlot::BeginPlot("##AudioWaveform", ImVec2(-1, 150*g_viewport_mgr.m_scale))) {
            ImPlot::SetupAxes("Time", "Amplitude", wf_axis, ImPlotAxisFlags_Lock);
            ImPlot::SetupAxesLimits(0, AUDIO_DEBUG_WAVEFORM_SIZE, -1.2, 1.2, ImPlotCond_Always);
            
            // Plot left and right channels
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
            ImPlot::PlotLine("Left", dbg->stats.waveform_l, AUDIO_DEBUG_WAVEFORM_SIZE);
            ImPlot::PopStyleColor();
            
            ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            ImPlot::PlotLine("Right", dbg->stats.waveform_r, AUDIO_DEBUG_WAVEFORM_SIZE);
            ImPlot::PopStyleColor();
            
            ImPlot::EndPlot();
        }
        
        ImGui::Unindent();
    }
    
    if (ImGui::CollapsingHeader("Per-Voice Analysis")) {
        ImGui::Indent();
        
        m_show_voice_details = true;
        
        ImGui::BeginChild("VoiceScroll", ImVec2(0, 200*g_viewport_mgr.m_scale), true);
        
        ImGui::PushFont(g_font_mgr.m_fixed_width_font);
        ImGui::Text("V#  | Active | Rate(Hz) | Loop | Samples  | Peak(L/R) | Avg(L/R)");
        ImGui::Separator();
        
        for (int i = 0; i < MAX_VOICES; i++) {
            if (!dbg->vp.v[i].active) continue;
            
            const struct McpxApuDebugVoice *v = &dbg->vp.v[i];
            
            ImGui::Text("%3d | %-6s | %7d  | %4u | %8llu | %.2f/%.2f | %.2f/%.2f",
                i,
                v->paused ? "Paused" : "Active",
                (int)(48000.0/v->rate),
                v->loop_count,
                (unsigned long long)v->total_samples,
                v->peak_amplitude_l, v->peak_amplitude_r,
                v->avg_amplitude_l, v->avg_amplitude_r);
            
            // Highlight voices with potential issues
            if (v->loop_count > 100) {
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.0f, 1.0f));
                ImGui::Text(" <-- High loop count!");
                ImGui::PopStyleColor();
            }
        }
        
        ImGui::PopFont();
        ImGui::EndChild();
        
        ImGui::Unindent();
    }
    
    if (ImGui::CollapsingHeader("Debugging Tools")) {
        ImGui::Indent();
        
        ImGui::Text("Voice Isolation:");
        ImGui::SameLine();
        if (ImGui::Button("Unmute All")) {
            mcpx_apu_debug_unmute_all();
        }
        
        ImGui::Text("Debug Logging:");
        bool logging = mcpx_apu_debug_get_logging_enabled();
        if (ImGui::Checkbox("Enable Audio Logging", &logging)) {
            mcpx_apu_debug_set_logging_enabled(logging);
        }
        ImGui::SameLine();
        HelpMarker("Log audio events to console for debugging");
        
        ImGui::Text("Configuration:");
        ImGui::Checkbox("Log Buffer Underruns", &g_config.audio.debug.log_buffer_underruns);
        ImGui::Checkbox("Log Sample Rate Changes", &g_config.audio.debug.log_sample_rate_changes);
        ImGui::Checkbox("Detect Audio Loops", &g_config.audio.debug.detect_audio_loops);
        ImGui::Checkbox("Show Mixer Analysis", &g_config.audio.debug.show_mixer_analysis);
        
        ImGui::Unindent();
    }
    
    if (ImGui::CollapsingHeader("Issue #904 Notes")) {
        ImGui::Indent();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.2f, 1.0f));
        ImGui::TextWrapped("This debugging suite helps diagnose common audio issues:");
        ImGui::PopStyleColor();
        ImGui::BulletText("Audio Looping - Check loop counts in Per-Voice Analysis");
        ImGui::BulletText("Buzzing/Hissing - Monitor waveform for abnormal patterns");
        ImGui::BulletText("Sample Rate Issues - Watch for sample rate change notifications");
        ImGui::BulletText("Mixing Issues - Use voice isolation to identify problem voices");
        ImGui::BulletText("Buffer Issues - Monitor underruns/overruns in statistics");
        ImGui::Unindent();
    }
    
    ImGui::End();
}

// Utility structure for realtime plot
struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<ImVec2> Data;
    ScrollingBuffer() {
        MaxSize = 2000;
        Offset  = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(float x, float y) {
        if (Data.size() < MaxSize)
            Data.push_back(ImVec2(x,y));
        else {
            Data[Offset] = ImVec2(x,y);
            Offset =  (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset  = 0;
        }
    }
};

DebugVideoWindow::DebugVideoWindow()
{
    m_is_open = false;
    m_transparent = false;
    m_position_restored = false;
    m_resize_init_complete = false;
    m_prev_scale = g_viewport_mgr.m_scale;
}

void DebugVideoWindow::Draw()
{
    if (!m_is_open)
        return;

    if (!m_position_restored) {
        ImGui::SetNextWindowPos(ImVec2(g_config.display.debug.video.x_pos,
                                       g_config.display.debug.video.y_pos),
                                ImGuiCond_Once, ImVec2(0, 0));
        m_transparent = g_config.display.debug.video.transparency;
        m_position_restored = true;
    }

    float alpha = m_transparent ? 0.2 : 1.0;
    PushWindowTransparencySettings(m_transparent, 0.2);

    if (!m_resize_init_complete || (g_viewport_mgr.m_scale != m_prev_scale)) {
        ImGui::SetNextWindowSize(ImVec2(
            g_config.display.debug.video.x_winsize * g_viewport_mgr.m_scale,
            g_config.display.debug.video.y_winsize * g_viewport_mgr.m_scale));
        m_resize_init_complete = true;
    }
    m_prev_scale = g_viewport_mgr.m_scale;

    if (ImGui::Begin("Video Debug", &m_is_open)) {
        double x_start, x_end;
        static ImPlotAxisFlags rt_axis = ImPlotAxisFlags_NoTickLabels;
        ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(5,5));
        ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.25f);
        static ScrollingBuffer fps;
        static float t = 0;
        if (runstate_is_running()) {
            t += ImGui::GetIO().DeltaTime;
            fps.AddPoint(t, g_nv2a_stats.increment_fps);
        }
        x_start = t - 10.0;
        x_end = t;

        float plot_width = 0.5 * (ImGui::GetWindowSize().x -
                                  2 * ImGui::GetStyle().WindowPadding.x -
                                  ImGui::GetStyle().ItemSpacing.x);

        ImGui::SetNextWindowBgAlpha(alpha);
        if (ImPlot::BeginPlot("##ScrollingFPS", ImVec2(plot_width,75*g_viewport_mgr.m_scale))) {
            ImPlot::SetupAxes(NULL, NULL, rt_axis, rt_axis | ImPlotAxisFlags_Lock);
            ImPlot::SetupAxesLimits(x_start, x_end, 0, 65, ImPlotCond_Always);
            if (fps.Data.size() > 0) {
                ImPlot::PlotShaded("##fps", &fps.Data[0].x, &fps.Data[0].y, fps.Data.size(), 0, 0, fps.Offset, 2 * sizeof(float));
                ImPlot::PlotLine("##fps", &fps.Data[0].x, &fps.Data[0].y, fps.Data.size(), 0, fps.Offset, 2 * sizeof(float));
            }
            ImPlot::Annotation(x_start, 65, ImPlot::GetLastItemColor(), ImVec2(0,0), true, "FPS: %d", g_nv2a_stats.increment_fps);
            ImPlot::EndPlot();
        }

        ImGui::SameLine();

        x_end = g_nv2a_stats.frame_count;
        x_start = x_end - NV2A_PROF_NUM_FRAMES;

        ImPlot::PushStyleColor(ImPlotCol_Line, ImPlot::GetColormapColor(1));
        ImGui::SetNextWindowBgAlpha(alpha);
        if (ImPlot::BeginPlot("##ScrollingMSPF", ImVec2(plot_width,75*g_viewport_mgr.m_scale))) {
            ImPlot::SetupAxes(NULL, NULL, rt_axis, rt_axis | ImPlotAxisFlags_Lock);
            ImPlot::SetupAxesLimits(x_start, x_end, 0, 100, ImPlotCond_Always);
            ImPlot::PlotShaded("##mspf", &g_nv2a_stats.frame_history[0].mspf, NV2A_PROF_NUM_FRAMES, 0, 1, x_start, 0, g_nv2a_stats.frame_ptr, sizeof(g_nv2a_stats.frame_working));
            ImPlot::PlotLine("##mspf", &g_nv2a_stats.frame_history[0].mspf, NV2A_PROF_NUM_FRAMES, 1, x_start, 0, g_nv2a_stats.frame_ptr, sizeof(g_nv2a_stats.frame_working));
            ImPlot::Annotation(x_start, 100, ImPlot::GetLastItemColor(), ImVec2(0,0), true, "MSPF: %d", g_nv2a_stats.frame_history[(g_nv2a_stats.frame_ptr - 1) % NV2A_PROF_NUM_FRAMES].mspf);
            ImPlot::EndPlot();
        }
        ImPlot::PopStyleColor();

        ImGui::SetNextItemOpen(g_config.display.debug.video.advanced_tree_state,
                               ImGuiCond_Once);
        g_config.display.debug.video.advanced_tree_state =
            ImGui::TreeNode("Advanced");

        if (g_config.display.debug.video.advanced_tree_state) {
            ImGui::SetNextWindowBgAlpha(alpha);
            if (ImPlot::BeginPlot("##ScrollingDraws", ImVec2(-1,-1))) {
                ImPlot::SetupAxes(NULL, NULL, ImPlotAxisFlags_None, ImPlotAxisFlags_AutoFit);
                ImPlot::SetupAxisScale(ImAxis_Y1, ImPlotScale_Log10);

                ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1500);
                ImPlot::SetupAxisLimits(ImAxis_X1, 0, NV2A_PROF_NUM_FRAMES);

                ImGui::PushID(0);
                ImPlot::PushStyleColor(ImPlotCol_Line, ImPlot::GetColormapColor(0));
                ImPlot::PushStyleColor(ImPlotCol_Fill, ImPlot::GetColormapColor(0));
                ImPlot::PlotLine("MSPF", &g_nv2a_stats.frame_history[0].mspf, NV2A_PROF_NUM_FRAMES, 1, 0, 0, g_nv2a_stats.frame_ptr, sizeof(g_nv2a_stats.frame_working));
                ImPlot::PopStyleColor(2);
                ImGui::PopID();

                for (int i = 0; i < NV2A_PROF__COUNT; i++) {
                    ImGui::PushID(i+1);
                    char title[64];
                    snprintf(title, sizeof(title), "%s: %d",
                        nv2a_profile_get_counter_name(i),
                        nv2a_profile_get_counter_value(i));
                    ImPlot::PushStyleColor(ImPlotCol_Line, ImPlot::GetColormapColor(i+1));
                    ImPlot::PushStyleColor(ImPlotCol_Fill, ImPlot::GetColormapColor(i+1));
                    ImPlot::PlotLine(title, &g_nv2a_stats.frame_history[0].counters[i], NV2A_PROF_NUM_FRAMES, 1, 0, 0, g_nv2a_stats.frame_ptr, sizeof(g_nv2a_stats.frame_working));
                    ImPlot::PopStyleColor(2);
                    ImGui::PopID();
                }

                ImPlot::EndPlot();
            }
            ImGui::TreePop();
        }

        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(2)) {
            m_transparent = !m_transparent;
        }

        ImPlot::PopStyleVar(2);

        ImVec2 debug_window_pos = ImGui::GetWindowPos();
        g_config.display.debug.video.x_pos = debug_window_pos.x;
        g_config.display.debug.video.y_pos = debug_window_pos.y;

        ImVec2 debug_window_size = ImGui::GetWindowSize();
        g_config.display.debug.video.x_winsize =
            debug_window_size.x / g_viewport_mgr.m_scale;
        g_config.display.debug.video.y_winsize =
            debug_window_size.y / g_viewport_mgr.m_scale;
        g_config.display.debug.video.transparency = m_transparent;
    }
    ImGui::End();
    ImGui::PopStyleColor(5);
}

DebugApuWindow apu_window;
DebugVideoWindow video_window;
