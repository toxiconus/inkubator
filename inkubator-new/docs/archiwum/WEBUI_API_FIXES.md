# Web UI API Endpoint Fixes

## Issues Identified

### 1. **API Endpoint Mismatches** ❌ → ✅

The HTML web interface was calling endpoints that don't match the firmware implementation:

#### Set Day Functionality
- **Before**: `GET /api/set-day?day=10`
- **After**: `POST /api/tray/set-day` with body `{"tray": 0, "day": 10}`
- **Status**: ✅ Fixed

#### Reset Day Functionality  
- **Before**: `GET /api/reset-day`
- **After**: `POST /api/reset`
- **Status**: ✅ Fixed

#### Reboot Functionality
- **Before**: `GET /api/reboot`
- **After**: `POST /api/reboot`
- **Status**: ✅ Fixed

### 2. **Chart.js CDN Tracking Prevention** 🚫 → 📊

**Error**: `Tracking Prevention blocked access to storage for https://cdn.jsdelivr.net/...`

**Root Cause**: Browser's Tracking Prevention feature blocks cross-site resource loading.

**Solution**:
- Modified HTML to first try loading from local `/chart.min.js`
- Falls back to CDN if local version unavailable
- Gracefully hides chart if both sources fail

**To Enable Chart Locally**:
1. Download Chart.js: `https://cdn.jsdelivr.net/npm/chart.js@4.4.1/dist/chart.umd.min.js`
2. Save as `firmware/data/chart.min.js`
3. Rebuild and upload to ESP32 SPIFFS
4. Chart will load from local storage (no CDN needed)

## Files Modified

- **firmware/data/index.html**
  - Line ~7: Removed CDN script tag
  - Line ~641: Changed `resetDay()` to use `POST /api/reset`
  - Line ~649: Changed `setDay()` to use `POST /api/tray/set-day` with JSON body
  - Line ~666: Changed `rebootDevice()` to use `POST /api/reboot`
  - Added `loadChartJS()` function with fallback mechanism

## API Changes Summary

| Function | Old Endpoint | New Endpoint | Method | Change |
|----------|-------------|-------------|--------|--------|
| Set Day | `/api/set-day?day=X` | `/api/tray/set-day` | GET → POST | ✅ Fixed |
| Reset | `/api/reset-day` | `/api/reset` | GET → POST | ✅ Fixed |
| Reboot | `/api/reboot` | `/api/reboot` | GET → POST | ✅ Fixed |
| Chart.js | CDN only | Local + CDN fallback | - | ✅ Fixed |

## Testing

To verify the fixes:

1. **Set Day**: Click "📍 Ustaw Dzień" button and enter a day number (should work now)
2. **Reset Day**: Click "⏱️ Reset Dnia" button (should work now)
3. **Reboot**: Click "🔄 Restart ESP32" button (should work now)
4. **Chart**: Monitor Dashboard tab - chart should display without tracking prevention errors

## Notes

- The `setDay()` function applies to tray 0 by default. To support multiple trays, extend the UI to include tray selection.
- All changes are backward compatible with the firmware API endpoints.
- Chart.js is optional; if unavailable, the chart gracefully hides and other UI elements remain functional.
