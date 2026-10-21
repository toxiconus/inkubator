#pragma once
#include <Arduino.h>

struct IncubationDay {
  int day;
  float temp_set;        // nastawa (idealny punkt pracy)
  float temp_min;        // dolna granica tolerancji
  float temp_max;        // górna granica tolerancji
  float humidity_set;    // nastawa wilgotnoci
  float humidity_min;    // dolna granica tolerancji
  float humidity_max;    // górna granica tolerancji
  bool turn;
  bool lockdown;
  const char* notes;
};

// ==================== HARMONOGRAMY ====================

// ------------------------------------------------------------
// KURA (Gallus gallus domesticus)
// róda: Decuypere et al. (2001) – PMC, oraz wytyczne Uniwersytetu Cornell
// Uwaga: w lockdown (dni 19-21) temperatura set obniona do 36,6°C,
// wilgotno podniesiona do 70% (zakres 65-75%).
// ------------------------------------------------------------
static const IncubationDay SCHEDULE_CHICKEN[] = {
  {1,  37.6, 37.5, 37.8, 52, 50, 55, true, false, "Start inkubacji"},
  {2,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {3,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {4,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {5,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {6,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {7,  37.6, 37.5, 37.8, 52, 50, 55, true, false, "Koniec 1. tygodnia - kontrola"},
  {8,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {9,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {10, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {11, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {12, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {13, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {14, 37.6, 37.5, 37.8, 52, 50, 55, true, false, "Koniec 2. tygodnia"},
  {15, 37.6, 37.5, 37.8, 60, 55, 65, true, false, "Wilgotno wzrasta"},
  {16, 37.6, 37.5, 37.8, 60, 55, 65, true, false, ""},
  {17, 37.6, 37.5, 37.8, 60, 55, 65, true, false, ""},
  {18, 37.6, 37.5, 37.8, 60, 55, 65, true, false, "Ostatni dzie obrotu!"},
  {19, 36.6, 36.1, 37.2, 70, 65, 75, false, true, "LOCKDOWN - STOP obrotu"},
  {20, 36.6, 36.1, 37.2, 70, 65, 75, false, true, "Przebicie"},
  {21, 36.6, 36.1, 37.2, 70, 65, 75, false, true, "WYLEG!"}
};

// ------------------------------------------------------------
// KACZKA (Anas platyrhynchos domestica) – równie biegus indyjski
// ródo: Cornell University College of Veterinary Medicine,
// Incubator Warehouse.
// Uwaga: niektóre róda zalecaj chodzenie (cooling) raz dziennie
// przez ok. 15-20 min od dnia 8. – w tym inkubatorze NIE STOSUJEMY.
// Parametry dla biegusa indyjskiego s identyczne jak dla innych ras.
// ------------------------------------------------------------
static const IncubationDay SCHEDULE_DUCK[] = {
  {1,  37.5, 37.3, 37.7, 55, 50, 60, true, false, "Start inkubacji"},
  {2,  37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {3,  37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {4,  37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {5,  37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {6,  37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {7,  37.5, 37.3, 37.7, 55, 50, 60, true, false, "Koniec 1. tygodnia"},
  {8,  37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {9,  37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {10, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {11, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {12, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {13, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {14, 37.5, 37.3, 37.7, 55, 50, 60, true, false, "Koniec 2. tygodnia"},
  {15, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {16, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {17, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {18, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {19, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {20, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {21, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {22, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {23, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {24, 37.5, 37.3, 37.7, 55, 50, 60, true, false, ""},
  {25, 37.5, 37.3, 37.7, 55, 50, 60, true, false, "Ostatni dzie obrotu"},
  {26, 37.2, 37.0, 37.5, 70, 65, 75, false, true, "LOCKDOWN - STOP obrotu"},
  {27, 37.2, 37.0, 37.5, 70, 65, 75, false, true, "Przebicie"},
  {28, 37.2, 37.0, 37.5, 70, 65, 75, false, true, "WYLEG!"}
};

// ------------------------------------------------------------
// G (Anser anser domestica)
// róda: agristuff, Poultry Hatch.
// Uwaga: wilgotno w literaturze waha si od 35 do 60% – przyjto
// warto rodkow 52% jako set.
// ------------------------------------------------------------
static const IncubationDay SCHEDULE_GOOSE[] = {
  {1,  37.5, 37.3, 37.7, 52, 45, 60, true, false, "Start inkubacji"},
  {2,  37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {3,  37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {4,  37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {5,  37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {6,  37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {7,  37.5, 37.3, 37.7, 52, 45, 60, true, false, "Koniec 1. tygodnia"},
  {8,  37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {9,  37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {10, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {11, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {12, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {13, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {14, 37.5, 37.3, 37.7, 52, 45, 60, true, false, "Koniec 2. tygodnia"},
  {15, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {16, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {17, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {18, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {19, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {20, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {21, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {22, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {23, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {24, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {25, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {26, 37.5, 37.3, 37.7, 52, 45, 60, true, false, ""},
  {27, 37.5, 37.3, 37.7, 52, 45, 60, true, false, "Ostatni dzie obrotu"},
  {28, 37.0, 36.8, 37.3, 70, 65, 75, false, true, "LOCKDOWN - STOP obrotu"},
  {29, 37.0, 36.8, 37.3, 70, 65, 75, false, true, "Przebicie"},
  {30, 37.0, 36.8, 37.3, 70, 65, 75, false, true, "WYLEG!"}
};

// ------------------------------------------------------------
// PRZEPIÓRKA (Coturnix coturnix japonica)
// ródo: An Ordinary Existence, USDA.
// ------------------------------------------------------------
static const IncubationDay SCHEDULE_QUAIL[] = {
  {1,  37.6, 37.5, 37.8, 47, 45, 50, true, false, "Start inkubacji"},
  {2,  37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {3,  37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {4,  37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {5,  37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {6,  37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {7,  37.6, 37.5, 37.8, 47, 45, 50, true, false, "Koniec 1. tygodnia"},
  {8,  37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {9,  37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {10, 37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {11, 37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {12, 37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {13, 37.6, 37.5, 37.8, 47, 45, 50, true, false, ""},
  {14, 37.6, 37.5, 37.8, 47, 45, 50, true, false, "Ostatni dzie zwykego obrotu"},
  {15, 37.5, 37.2, 37.7, 68, 65, 70, false, true, "LOCKDOWN - STOP obrotu"},
  {16, 37.5, 37.2, 37.7, 68, 65, 70, false, true, "Przebicie"},
  {17, 37.5, 37.2, 37.7, 68, 65, 70, false, true, "WYLEG!"},
  {18, 37.5, 37.2, 37.7, 68, 65, 70, false, true, "WYLEG!"}
};

// ------------------------------------------------------------
// PERLICZKA (Numida meleagris) – rasa hodowana w Polsce
// UWAGA: w literaturze czsto mylona z innymi ptakami ownymi
// (np. baantem), std rozbienoci. Ponisze wartoci dotycz
// typowej perliczki domowej.
// ------------------------------------------------------------
static const IncubationDay SCHEDULE_GUINEA[] = {
  {1,  37.6, 37.5, 37.8, 50, 45, 55, true, false, "Start inkubacji"},
  {2,  37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {3,  37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {4,  37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {5,  37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {6,  37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {7,  37.6, 37.5, 37.8, 50, 45, 55, true, false, "Koniec 1. tygodnia"},
  {8,  37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {9,  37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {10, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {11, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {12, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {13, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {14, 37.6, 37.5, 37.8, 50, 45, 55, true, false, "Koniec 2. tygodnia"},
  {15, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {16, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {17, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {18, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {19, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {20, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {21, 37.6, 37.5, 37.8, 50, 45, 55, true, false, ""},
  {22, 37.6, 37.5, 37.8, 50, 45, 55, true, false, "Ostatni dzie zwykego obrotu"},
  {23, 37.2, 37.0, 37.5, 70, 65, 75, false, true, "LOCKDOWN - STOP obrotu"},
  {24, 37.2, 37.0, 37.5, 70, 65, 75, false, true, "Przebicie"},
  {25, 37.2, 37.0, 37.5, 70, 65, 75, false, true, ""},
  {26, 37.2, 37.0, 37.5, 70, 65, 75, false, true, ""},
  {27, 37.2, 37.0, 37.5, 70, 65, 75, false, true, "WYLEG!"}
};

// ------------------------------------------------------------
// BAANT (Phasianus colchicus)
// ródo: Farmkeep, wytyczne hodowlane.
// ------------------------------------------------------------
static const IncubationDay SCHEDULE_PHEASANT[] = {
  {1,  37.6, 37.5, 37.8, 52, 50, 55, true, false, "Start inkubacji"},
  {2,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {3,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {4,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {5,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {6,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {7,  37.6, 37.5, 37.8, 52, 50, 55, true, false, "Koniec 1. tygodnia"},
  {8,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {9,  37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {10, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {11, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {12, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {13, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {14, 37.6, 37.5, 37.8, 52, 50, 55, true, false, "Koniec 2. tygodnia"},
  {15, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {16, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {17, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {18, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {19, 37.6, 37.5, 37.8, 52, 50, 55, true, false, ""},
  {20, 37.6, 37.5, 37.8, 52, 50, 55, true, false, "Ostatni dzie zwykego obrotu"},
  {21, 37.1, 36.9, 37.4, 63, 60, 65, false, true, "LOCKDOWN - STOP obrotu"},
  {22, 37.1, 36.9, 37.4, 63, 60, 65, false, true, "Przebicie"},
  {23, 37.1, 36.9, 37.4, 63, 60, 65, false, true, ""},
  {24, 37.1, 36.9, 37.4, 63, 60, 65, false, true, "WYLEG!"}
};

struct SpeciesInfo {
  const char* id;
  const char* name;
  int total_days;
  const IncubationDay* schedule;
};

static const SpeciesInfo SPECIES_LIST[] = {
  {"CHICKEN",  "Kura",        21, SCHEDULE_CHICKEN},
  {"DUCK",     "Kaczka",      28, SCHEDULE_DUCK},
  {"GOOSE",    "Gęś",         30, SCHEDULE_GOOSE},
  {"QUAIL",    "Przepiórka",  17, SCHEDULE_QUAIL},
  {"GUINEA",   "Perliczka",   27, SCHEDULE_GUINEA},
  {"PHEASANT", "Baant",      24, SCHEDULE_PHEASANT}
};

static const int SPECIES_COUNT = 6;

inline bool species_id_equals(const char* left, const char* right) {
  if (!left || !right) return false;
  while (*left && *right) {
    char a = *left;
    char b = *right;
    if (a >= 'a' && a <= 'z') a -= 32;
    if (b >= 'a' && b <= 'z') b -= 32;
    if (a != b) return false;
    left++;
    right++;
  }
  return *left == *right;
}

inline const SpeciesInfo* get_species(const char* id) {
  for (int i = 0; i < SPECIES_COUNT; i++) {
    if (species_id_equals(SPECIES_LIST[i].id, id)) return &SPECIES_LIST[i];
  }
  return &SPECIES_LIST[0]; // fallback na kure
}

inline const IncubationDay* get_day(const SpeciesInfo* sp, int day) {
  if (!sp || day < 1) return nullptr;
  if (day > sp->total_days) day = sp->total_days;
  return &sp->schedule[day - 1];
}
