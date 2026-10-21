#pragma once
#include <Arduino.h>
#include "incubation_schedule.h"

// ============================================================================
// TRAY CONTROLLER – poprawiona wersja
// ============================================================================

#define MAX_TRAYS 3

// ── Stan jednej tacy ───────────────────────────────────────────────────────
struct TrayState {
  int index;
  bool active;
  const SpeciesInfo* species_info;
  unsigned long long start_millis;     // 64-bit czas startu
  int current_day;                     // 0 = jeszcze nie wystartowaa
  int days_remaining;
  bool in_lockdown;
  bool should_rotate;
  float target_humidity_min;
  float target_humidity_max;
  float target_humidity;
  float target_temp;
  int pin;
  int pin_pwm_channel;                 // Kanal LEDC PWM dla tego pinu (-1 = nie skonfigurowany)
  bool pin_active;                     // Dla kompatybilnosci wstecz
  const IncubationDay* today;

  // --- PWM Rotation State ---
  unsigned long long next_rotation_ms; // Kiedy nastepny ruch
  unsigned long long rotation_start_ms;// Kiedy zaczal sie obecny ruch
  bool currently_rotating;             // Czy wlasnie sie obraca
  int current_pwm;                     // Aktualna wartosc PWM (0-255)
  unsigned long long manual_move_until_ms; // rczny ruch – aktywny do tego czasu
  bool has_last_rotation;
  int last_rotation_day;
  int last_rotation_hour;
  int last_rotation_minute;
  unsigned long long last_rotation_elapsed_ms;
  bool rotation_event_pending;
};

// ── Zmienne globalne ───────────────────────────────────────────────────────
extern TrayState tray_states[MAX_TRAYS];
extern unsigned long long global_start_millis;

void tray_init(unsigned long long base_start_millis);
void tray_update_state(TrayState& st, unsigned long long now_millis);
void tray_update_all(unsigned long long now_millis);
void tray_set_species(int tray_idx, const char* species_id);
bool tray_set_day(int tray_idx, int day, unsigned long long now_millis);
void tray_manual_move(int tray_idx, unsigned long duration_ms, unsigned long long now_millis);
void tray_stop(int tray_idx, unsigned long long now_millis);
void tray_set_active(int tray_idx, bool active);
void tray_get_chamber_humidity(float& out_min, float& out_max);
void tray_get_chamber_humidity_target(float& out_target_humidity);
void tray_get_chamber_temp(float& out_target_temp);
String tray_status_json(unsigned long long now_millis);
