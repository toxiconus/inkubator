# Upload do ESP po WiFi

## Endpointy obsługiwane przez firmware

1. `/upload` (formularz webowy)
   - Metoda: `HTTP GET` wyświetla formularz uploadu.
   - Metoda: `HTTP POST` wysyła plik do LittleFS przez multipart/form-data.
   - Wymaga autoryzacji Basic: `admin` / `5465INKUBATOR5465`
   - Pole formularza: `path` (ścieżka w LittleFS)
   - Pole pliku: `file`

2. `/api/lfs/upload`
   - Metoda: `HTTP POST`
   - Wymaga autoryzacji Basic: `admin` / `5465INKUBATOR5465`
   - Parametr URL lub query string: `path`
   - Body: zawartość pliku jako plain text
   - Zapisuje plik w LittleFS pod ścieżką `/path`
   - Przykład:
     ```powershell
     $ip = '192.168.5.1'
     $user = 'admin'
     $pass = '5465INKUBATOR5465'
     $path = 'index.html'
     $content = Get-Content -Raw -Path 'j:\inkubator\inkubator-mini\data\index.html'
     curl.exe -u "$user:$pass" -X POST "http://$ip/api/lfs/upload?path=$path" -H "Content-Type: text/plain" --data-binary "$content"
     ```

3. `/update` (OTA firmware)
   - Metoda: `HTTP POST`
   - Wymaga autoryzacji Basic: `admin` / `5465INKUBATOR5465`
   - Służy do wgrywania firmware bin, nie do zwykłych plików LittleFS.

## Jak działa upload w firmware

- `main.cpp` definiuje:
  - `OTA_USER` = `admin`
  - `WIFI_PASSWORD` = `5465INKUBATOR5465`
- Funkcje zapisu LittleFS:
  - `api_upload_page()` – formularz HTML do wysłania pliku.
  - `handleFileUpload()` – obsługuje multipart/form-data i zapisuje plik do LittleFS.
  - `api_lfs_upload()` – zapisuje plain text z body do pliku.

## Najszybszy sposób dla `index.html`

Użyj `/api/lfs/upload` z `curl.exe` lub narzędziem HTTP, jeśli znasz adres IP urządzenia.

Przykład:
```powershell
$ip = '192.168.5.1'
$user = 'admin'
$pass = '5465INKUBATOR5465'
$localPath = 'j:\inkubator\inkubator-mini\data\index.html'
$remotePath = 'index.html'
$auth = "$($user):$($pass)"
curl.exe -u $auth -X POST "http://$ip/api/lfs/upload?path=$remotePath" -H "Content-Type: text/plain" --data-binary "@$localPath"
```

## Przykładowe komendy ręcznego uploadu plików HTML i innych

### 1. Upload dowolnego pliku tekstowego (HTML, JSON, JS, CSS)

```powershell
$ip = '192.168.5.1'
$user = 'admin'
$pass = '5465INKUBATOR5465'
$localPath = 'j:\inkubator\inkubator-mini\data\species_profiles.json'
$remotePath = 'data/species_profiles.json'
$content = Get-Content -Raw -Path $localPath
curl.exe -u "$user:$pass" -X POST "http://$ip/api/lfs/upload?path=$remotePath" -H "Content-Type: text/plain" --data-binary "$content"
```

### 2. Upload pliku HTML bezpośrednio do katalogu root LittleFS

```powershell
$ip = '192.168.5.1'
$user = 'admin'
$pass = '5465INKUBATOR5465'
$localPath = 'j:\inkubator\inkubator-mini\data\index.html'
$remotePath = 'index.html'
$content = Get-Content -Raw -Path $localPath
curl.exe -u "$user:$pass" -X POST "http://$ip/api/lfs/upload?path=$remotePath" -H "Content-Type: text/plain" --data-binary "$content"
```

### 3. Upload pliku przy użyciu formularza multipart/form-data (`/upload`)

Jeśli `/upload` jest dostępny, możesz wysłać plik jako multipart/form-data z pola `file` i parametrem `path`.

```powershell
$ip = '192.168.5.1'
$user = 'admin'
$pass = '5465INKUBATOR5465'
$localPath = 'j:\inkubator\inkubator-mini\data\index.html'
$remotePath = 'index.html'
curl.exe -u "$user:$pass" -X POST "http://$ip/upload" -F "path=$remotePath" -F "file=@$localPath"
```

### 4. Upload plików z Linux/macOS (curl)

```bash
IP=192.168.5.1
USER=admin
PASS=5465INKUBATOR5465
LOCAL='data/index.html'
REMOTE='index.html'

curl -u "$USER:$PASS" -X POST "http://$IP/api/lfs/upload?path=$REMOTE" \
  -H 'Content-Type: text/plain' --data-binary @"$LOCAL"
```

### 5. Co możesz uploadować
- pliki HTML: `index.html`, `data/index v1 bad.html`
- pliki JSON: `data/config.json`, `data/species_profiles.json`
- pliki JS/CSS/tekstowe: `data/steps.md`, `scripts.js`
- inne pliki jako plain text, ale nie binarne dane

> Jeśli plik ma zawartość binarną, użyj formularza `/upload` w multipart/form-data zamiast `/api/lfs/upload`.

## Testy na urządzeniu 192.168.5.1
- `POST /upload` z użyciem `admin:5465INKUBATOR5465` -> działa, odpowiedź `Upload zakończony`, plik zapisano jako `/index.html`
- `POST /api/lfs/upload` z użyciem `admin:5465INKUBATOR5465` -> połączenie zresetowane
- `GET /upload` -> powinno być dostępne jako formularz uploadu, jeśli endpoint jest aktywny

Na tym urządzeniu najlepszą metodą wysyłki jest aktualnie `/upload` z multipart/form-data.

## Uwaga
- Plik zostanie zapisany w LittleFS pod `/index.html`.
- Po zapisie możesz odświeżyć stronę w przeglądarce, aby sprawdzić efekt.
- Jeśli chcesz zapisać katalog `data/`, użyj odpowiedniej ścieżki w parametrach `path`.
