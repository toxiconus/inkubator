#include "species_profiles.h"
#include <LittleFS.h>
#include "tray_controller.h"
#include "config_manager.h"

static JsonDocument species_profiles_doc;
static bool species_profiles_loaded = false;

static bool validate_phase(JsonVariant phase, JsonVariant critical) {
  if (!phase.is<JsonObject>()) return false;
  float temp_ideal = phase["temp_ideal"] | NAN;
  float temp_min = phase["temp_min"] | NAN;
  float temp_max = phase["temp_max"] | NAN;
  if (!isfinite(temp_ideal) || !isfinite(temp_min) || !isfinite(temp_max)) return false;
  if (temp_min > temp_ideal || temp_ideal > temp_max) return false;
  if (critical.is<JsonObject>()) {
    float crit_min = critical["temp_min"] | -100.0f;
    float crit_max = critical["temp_max"] | 200.0f;
    if (temp_min < crit_min || temp_max > crit_max) return false;
  }
  int hour_start = phase["hour_start"] | -1;
  int hour_end = phase["hour_end"] | -1;
  if (hour_start < 0 || hour_end <= hour_start) return false;
  return true;
}

static bool validate_profile(JsonVariant profile) {
  if (!profile.is<JsonObject>()) return false;
  if (!profile["id"].is<const char*>()) return false;
  if (!profile["category"].is<const char*>()) return false;
  if (!profile["name"].is<const char*>()) return false;
  if (!profile["total_hours"].is<int>()) return false;
  JsonVariant critical = profile["critical"];
  JsonArray phases = profile["phases"].as<JsonArray>();
  if (!phases) return false;
  int expected_start = 0;
  for (JsonVariant phase : phases) {
    if (!validate_phase(phase, critical)) return false;
    if (!phase["name"].is<const char*>()) return false;
    int hour_start = phase["hour_start"] | -1;
    if (hour_start != expected_start) return false;
    expected_start = phase["hour_end"] | -1;
  }
  int total_hours = profile["total_hours"] | 0;
  if (expected_start != total_hours) return false;
  return true;
}

bool load_species_profiles() {
  species_profiles_loaded = false;
  if (!LittleFS.exists("/species_profiles.json")) {
    Serial.println("[SPECIES] Brak /species_profiles.json w LittleFS");
    return false;
  }

  File f = LittleFS.open("/species_profiles.json", "r");
  if (!f) {
    Serial.println("[SPECIES] Nie mozna otworzyc /species_profiles.json");
    return false;
  }

  species_profiles_doc.clear();
  DeserializationError err = deserializeJson(species_profiles_doc, f);
  f.close();
  if (err) {
    Serial.print("[SPECIES] Blad JSON: ");
    Serial.println(err.c_str());
    return false;
  }

  JsonArray species = species_profiles_doc["species"].as<JsonArray>();
  if (!species) {
    Serial.println("[SPECIES] Brak tablicy species w JSON");
    return false;
  }

  for (JsonVariant item : species) {
    if (!validate_profile(item)) {
      Serial.println("[SPECIES] Niepoprawny profil species w JSON");
      return false;
    }
  }

  species_profiles_loaded = true;
  Serial.printf("[SPECIES] Zaladowano %u profili species\n", species.size());
  return true;
}

JsonVariant find_species_profile(const String& species_id) {
  if (!species_profiles_loaded) return JsonVariant();
  String normalized = species_id;
  normalized.trim();
  normalized.toLowerCase();

  JsonArray species = species_profiles_doc["species"].as<JsonArray>();
  if (!species) return JsonVariant();

  for (JsonVariant item : species) {
    const char* id = item["id"];
    if (!id) continue;
    String candidate = id;
    candidate.toLowerCase();
    if (candidate == normalized) {
      return item;
    }
  }
  return JsonVariant();
}

JsonVariant get_active_phase(JsonVariant profile, int current_day) {
  if (!profile.is<JsonObject>()) return JsonVariant();
  JsonArray phases = profile["phases"].as<JsonArray>();
  if (!phases) return JsonVariant();

  int current_hour = (current_day - 1) * 24;
  for (JsonVariant phase : phases) {
    int start = phase["hour_start"] | -1;
    int end = phase["hour_end"] | -1;
    if (current_hour >= start && current_hour < end) {
      return phase;
    }
  }
  return JsonVariant();
}

static bool invalid_range(float minv, float maxv) {
  return !isfinite(minv) || !isfinite(maxv) || minv > maxv;
}

static ChamberAggregation intersect_ranges(const ChamberAggregation& a, const ChamberAggregation& b) {
  ChamberAggregation result;
  result.temp_min = max(a.temp_min, b.temp_min);
  result.temp_max = min(a.temp_max, b.temp_max);
  result.humidity_min = max(a.humidity_min, b.humidity_min);
  result.humidity_max = min(a.humidity_max, b.humidity_max);
  result.target_temp = NAN;
  result.valid = !(invalid_range(result.temp_min, result.temp_max) || invalid_range(result.humidity_min, result.humidity_max));
  return result;
}

static ChamberAggregation make_aggregation(JsonVariant phase) {
  ChamberAggregation agg;
  agg.target_temp = phase["temp_ideal"] | NAN;
  agg.temp_min = phase["temp_min"] | NAN;
  agg.temp_max = phase["temp_max"] | NAN;
  agg.humidity_min = phase["humidity_min"] | NAN;
  agg.humidity_max = phase["humidity_max"] | NAN;
  agg.valid = !invalid_range(agg.temp_min, agg.temp_max) && !invalid_range(agg.humidity_min, agg.humidity_max) && isfinite(agg.target_temp);
  return agg;
}

String serialize_species_profiles() {
  if (!species_profiles_loaded) return String("[]");
  String output;
  serializeJson(species_profiles_doc["species"], output);
  return output;
}

String api_species_profiles_active() {
  JsonDocument out;
  JsonArray activeArr = out["active"].to<JsonArray>();

  for (int i = 0; i < MAX_TRAYS; i++) {
    extern TrayState tray_states[MAX_TRAYS];
    TrayState& st = tray_states[i];
    if (!st.active || !st.species_info || st.current_day <= 0) continue;
    JsonVariant profile = find_species_profile(st.species_info->id);
    JsonVariant phase = get_active_phase(profile, st.current_day);
    if (profile.isNull() || phase.isNull()) continue;

    JsonObject entry = activeArr.add<JsonObject>();
    entry["tray"] = i;
    entry["species_id"] = profile["id"].is<const char*>() ? profile["id"].as<const char*>() : "";
    entry["species_name"] = profile["name"].is<const char*>() ? profile["name"].as<const char*>() : "";
    entry["current_day"] = st.current_day;
    {
      int total_hours = profile["total_hours"] | 0;
      entry["total_days"] = (total_hours + 23) / 24;
    }
    entry["phase_name"] = phase["name"].is<const char*>() ? phase["name"].as<const char*>() : "";
    entry["target_temp"] = phase["temp_ideal"] | 0.0f;
    entry["temp_min"] = phase["temp_min"] | 0.0f;
    entry["temp_max"] = phase["temp_max"] | 0.0f;
    entry["humidity_min"] = phase["humidity_min"] | 0.0f;
    entry["humidity_max"] = phase["humidity_max"] | 0.0f;
    entry["notes"] = phase["notes"].is<const char*>() ? phase["notes"].as<const char*>() : "";
  }

  ChamberAggregation agg;
  bool has_aggregate = false;
  float target_temp_sum = 0.0f;
  int target_temp_count = 0;

  for (JsonVariant item : activeArr) {
    if (!item.is<JsonObject>()) continue;
    float tmin = item["temp_min"] | NAN;
    float tmax = item["temp_max"] | NAN;
    float th = item["humidity_min"] | NAN;
    float hh = item["humidity_max"] | NAN;
    float ttarget = item["target_temp"] | NAN;
    if (!isfinite(tmin) || !isfinite(tmax) || !isfinite(th) || !isfinite(hh) || !isfinite(ttarget)) continue;

    ChamberAggregation itemAgg;
    itemAgg.temp_min = tmin;
    itemAgg.temp_max = tmax;
    itemAgg.humidity_min = th;
    itemAgg.humidity_max = hh;
    itemAgg.target_temp = NAN;
    itemAgg.valid = true;

    if (!has_aggregate) {
      agg = itemAgg;
      has_aggregate = true;
    } else {
      agg = intersect_ranges(agg, itemAgg);
    }

    target_temp_sum += ttarget;
    target_temp_count += 1;
  }

  JsonObject aggJson = out["aggregate"].to<JsonObject>();
  if (has_aggregate && agg.valid && target_temp_count > 0) {
    aggJson["target_temp"] = target_temp_sum / target_temp_count;
    aggJson["temp_min"] = agg.temp_min;
    aggJson["temp_max"] = agg.temp_max;
    aggJson["humidity_min"] = agg.humidity_min;
    aggJson["humidity_max"] = agg.humidity_max;
    aggJson["valid"] = true;
  } else {
    aggJson["target_temp"] = configMgr.cfg.system.target_temp;
    aggJson["temp_min"] = configMgr.cfg.system.target_temp - 0.5f;
    aggJson["temp_max"] = configMgr.cfg.system.target_temp + 0.5f;
    aggJson["humidity_min"] = 45.0f;
    aggJson["humidity_max"] = 65.0f;
    aggJson["valid"] = false;
  }

  String output;
  serializeJson(out, output);
  return output;
}
