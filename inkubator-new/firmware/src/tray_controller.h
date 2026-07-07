// tray_controller.h
#pragma once

#include "config_manager.h"
#include "incubation_profile.h"

#define MAX_TRAYS 2

struct TrayState {
    int index;
    bool active;
    int pin;
    int pin_pwm_channel;
    bool pin_active;
    int current_pwm;
    
    const SpeciesProfile* incubation_profile; // nowy
    IncubationDay current_day_data;
    
    unsigned long long start_millis;
    unsigned long long last_day_check_ms;
    int current_day;
    int days_remaining;
    const IncubationDay* today; // stary
    
    bool in_lockdown;
    bool should_rotate;
    float target_temp;
    float target_humidity_min;
    float target_humidity_max;
    float target_humidity;
    
    unsigned long long next_rotation_ms;
    unsigned long long rotation_start_ms;
    bool currently_rotating;
    unsigned long long manual_move_until_ms;
    bool has_last_rotation;
    int last_rotation_day;
    int last_rotation_hour;
    int last_rotation_minute;
    unsigned long long last_rotation_elapsed_ms;
    bool rotation_event_pending;
};

extern TrayState tray_states[MAX_TRAYS];

void tray_init(unsigned long long base_start_millis);
void tray_update_all(unsigned long long now_millis);
void tray_update_days(unsigned long long now_millis);
void tray_update_state(TrayState& st, unsigned long long now_millis);
void tray_increment_day(int tray_idx, unsigned long long now_millis);
void tray_manual_move(int tray_idx, unsigned long duration_ms, unsigned long long now_millis);
void tray_stop(int tray_idx, unsigned long long now_millis);
void tray_set_active(int tray_idx, bool active);
void tray_set_species(int tray_idx, const char* species_id);
bool tray_set_day(int tray_idx, int day, unsigned long long now_millis);
void tray_update_days_from_rtc();
void tray_set_start_unix(int tray_idx, unsigned long long unix);
void tray_reset_day(int tray_idx);
unsigned long long tray_get_start_unix(int tray_idx);
void tray_get_chamber_humidity(float& out_min, float& out_max);
void tray_get_chamber_humidity_target(float& out_target_humidity);
void tray_get_chamber_temp(float& out_target_temp);
String tray_status_json(unsigned long long now_millis);