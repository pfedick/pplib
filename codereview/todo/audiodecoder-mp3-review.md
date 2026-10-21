# AudioDecoder_MP3 Review – Befunde

Review vom 2026-10-21, Scope: `src/audio/AudioDecoder_MP3.cpp` (399 Zeilen).
Öffentliche API: `pplib::AudioDecoder_MP3` (`open`, `close`, `getAudioInfo`, `seekSample`, `getPosition`, `getSamples(size_t, STEREOSAMPLE16*)`, `addSamples(size_t, STEREOSAMPLE32*)`, `getSamples(size_t, STEREOSAMPLE_FLOAT*)`, `addSamples(size_t, STEREOSAMPLE_FLOAT*)`).
Review done by: Copilot (Copilot SDK in VS Code)

Alle kritischen Befunde wurden mit Probeprogrammen gegen `build/debug/libpplib_audio.a` + `libpplib_core.a` (GCC 14, Linux/x86-64, `-g -O0`, `HAVE_MPG123 1`, libmpg123 1.32.x) verifiziert; valgrind 3.x für Invalid Access/Leaks.

Referenz auf Basisreview: [audiodecoder-review.md](./audiodecoder-review.md).

## Bugs (kritisch)

- [ ] **Use-After-Free: `close()` gibt `readbuffer`/`outbuffer` frei, ohne sie auf `NULL` zu setzen** (`src/audio/AudioDecoder_MP3.cpp:140-141`)
  ```cpp
  void AudioDecoder_MP3::close()
  {
      free(readbuffer);      // Zeile 140 – Zeiger bleibt != NULL
      free(outbuffer);       // Zeile 141
      ...
  }
  ```
  Ein anschließendes `open()` allokiert wegen der Guard-Bedingungen in Zeile 126-133 (`if (!readbuffer)` / `if (!outbuffer)`) **nicht** neu, und `fillDecodeBuffer()` schreibt in den längst freigegebenen 1 MiB-Puffer (Zeile 193).
  Verifiziert: `open()` → `close()` → `open()` → `getSamples(…, STEREOSAMPLE16*)`. valgrind:
  `Syscall param read(buf) points to unaddressable byte(s)` und `Invalid write of size 8` in `AudioDecoder_MP3.cpp:193` (aufrufend aus `getSamples(..., STEREOSAMPLE16*)`). Laufzeitverhalten auf dieser Maschine: `BadAddressException`, Exit-Code 139.
  Behebung: nach jedem `free()` den Zeiger auf `nullptr` setzen (und `outbuffer`-Größe analog behandeln), oder besser: `std::vector<uint8_t>`/`std::unique_ptr<uint8_t[]>` als Member, dann entfällt die Klasse von Fehlern vollständig.

- [ ] **Double-Free: jedes explizite `close()` crasht im Destruktor** (`src/audio/AudioDecoder_MP3.cpp:140-141`, Destruktor `82`)
  Da `close()` die Zeiger nicht nullt und der Destruktor bedingungslos `close()` aufruft (Zeile 82), wird derselbe Block zweimal freigegeben. Ein zweimaliges `close()` crasht ebenso.
  Verifiziert: `open(); close(); …` (Destruktor am Scope-Ende) → SIGSEGV, Exit 139; `close(); close();` → identisch.
  Behebung: wie oben (`free` + `= nullptr`), zusätzlich `close()` idempotent machen.

- [ ] **Format-Hardcoding in `open()` → stillschweigendes Resampling und Upmixing** (`src/audio/AudioDecoder_MP3.cpp:118-120`)
  ```cpp
  mpg123_format_none((mpg123_handle*)decoder);
  mpg123_format((mpg123_handle*)decoder, 44100, MPG123_STEREO, MPG123_ENC_SIGNED_16);
  lastDecodeFormat = MPG123_ENC_SIGNED_16;
  ```
  Die Format-Tabelle wird auf 44,1 kHz/Stereo/16 bit festgenagelt, und `lastDecodeFormat` wird so gesetzt, als wäre genau dieses Format bereits aktiv. Der **erste** Lesezugriff über `getSamples(..., STEREOSAMPLE16*)` erkennt deshalb keinen needed-Formatwechsel (Bedingung Zeile 220 ist falsch) und wendet das eigentliche Container-Format nie an. libmpg123 resampelt/upmixt stattdessen kommentarlos (Doku von libmpg123: „it will duplicate mono channels and even do resampling if needed … the resampling of libmpg123 is very crude and doesn't even contain any kind of proper interpolation").
  Verifiziert an `mp3_22k.mp3` (22050 Hz, mono) und `mp3_48k.mp3` (48000 Hz, mono): aktiv gemeldetes Format nach dem ersten `mpg123_getformat()` ist jeweils `rate=44100 channels=2 enc=208`. Bei der Mono-Datei sind `int16[5000] == int16[5001] == -1356` (Kanal dupliziert), und die Frame-Zahl entspricht der hochgerechneten 44,1-kHz-Länge statt der tatsächlichen 22,05-kHz-Länge. Ein Aufrufer, der `getAudioInfo().Frequency`/`.Samples` zugrunde legt, bekommt also zeitlich gestreckte, interpolierte Samples.
  Behebung: Format-Tabelle aus `info` aufbauen und `lastDecodeFormat` auf „unbekannt" setzen:
  ```cpp
  mpg123_format_none((mpg123_handle*)decoder);
  int chmask = (this->info.Channels == 1) ? MPG123_MONO : MPG123_STEREO;   // nicht info.Channels direkt übergeben!
  mpg123_format((mpg123_handle*)decoder, this->info.Frequency, chmask, MPG123_ENC_SIGNED_16 | MPG123_ENC_FLOAT_32);
  lastDecodeFormat = 0;   // „noch kein Format aktiv"
  ```
  Zusätzlich in `fillDecodeBuffer()` bei `MPG123_NEW_FORMAT` das Ergebnis von `mpg123_getformat()` auswerten und `lastDecodeFormat` nachziehen (siehe Warnungsblock).

- [ ] **Formatwechsel mitten im Stream verwirft Samples, ohne `position` zurückzusetzen** (`src/audio/AudioDecoder_MP3.cpp:220-228`, `265-273`, `313-321`, `357-365`)
  Beim Wechsel der Sample-Breite (z. B. erst `getSamples(…, STEREOSAMPLE16*)`, dann `getSamples(…, STEREOSAMPLE_FLOAT*)`) wird der Decoder zurückgespult (`seekSample(0)`, Zeile 227/272/320/364), `position` bleibt aber auf dem alten Wert stehen. Die danach gelieferten Samples stammen wieder vom Anfang, `getPosition()` (Zeile 254/302/347/394) zählt aber weiter.
  Verifiziert an `mp3_22k.mp3`: `getSamples(1000, s16)` → `got=1000 pos=1000`, `[0]=414/395`; danach `getSamples(1000, f32)` → `got=1000 pos=2000`, `[0]=0/0`, `[999]=1,57234e-37` (Denormale = Datenmüll); danach erneut `getSamples(1000, s16)` → `got=1000 pos=3000`, `[0]=0/0`. Es gingen also die Samples 1000-1999 verloren, und die gelieferten Werte sind teilweise kein gültiges Audiomaterial.
  Behebung: Der Formatwechsel ist nur an Position 0 verlustfrei. Empfehlung:
  1. Bei Formatwechsel `position = 0;` setzen und dem Aufrufer die neue Position mitteilen, oder
  2. (besser) beide Encodings in der Format-Tabelle erlauben und den aktiven Zustand aus `mpg123_getformat()` ableiten, sodass kein Reset nötig ist; oder
  3. Formatwechsel nach `position > 0` mit `UnsupportedFeatureException` ablehnen, statt still Daten zu überspringen.

- [ ] **Division durch Null bei `info.Channels == 0` (SIGFPE)** (`src/audio/AudioDecoder_MP3.cpp:229`, `234`; ebenso `274`/`279`, `322`/`327`, `366`/`371`)
  `open()` prüft nur `info.Format != AudioInfo::MP3` (Zeile 108) — Kanalanzahl, Abtastrate und `AudioStart` aus einem vom Aufrufer übergebenen `AudioInfo` werden ungeprüft übernommen. Bei `Channels == 0` wird `bytes_per_frame = 0` und die Division `av / bytes_per_frame` ausgeführt, **bevor** die Kanal-Prüfung in Zeile 245/293/338/385 greift.
  Verifiziert: `open(file, &info)` mit selbstgebautem `AudioInfo{Format=MP3, Channels=0}` → `getSamples(1000, s16)` → Gleitkomma-Ausnahme/SIGFPE, Exit 136.
  Behebung: In `open()` validieren (`Channels == 1 || Channels == 2`, `Frequency > 0`, `AudioStart < file.size()`) und in den Lesemethoden `bytes_per_frame == 0` vor der Division abfangen.

- [ ] **Lesevorgang endet nicht an `info.Samples`/`info.AudioEnd`, Daten hinter dem Trackende werden gefüttert** (`src/audio/AudioDecoder_MP3.cpp:189`, `230-253`)
  `fillDecodeBuffer()` berechnet `restbytes = ff->size() - ff->tell()` und ignoriert `info.AudioEnd`. ID3v1-Tags, Padding und angehängte Daten landen im Decoder; zusätzlich geben die Lesemethoden mehr Frames zurück, als `getAudioInfo().Samples` hergibt.
  Verifiziert: `mp3_22k.mp3` → insgesamt 81920 Frames gelesen, `info.Samples = 24188`; `mono.mp3` → 172424 vs. 47187. libmpg123 meldet dazu: `Encountered more data after announced end of track … Frankenstein!`.
  Behebung: `restbytes = ff->size() - ff->tell()`, aber auf `info.AudioEnd + 1 - ff->tell()` begrenzen; in den Lesemethoden `num = min(num, info.Samples - position)` (nach Fix des `position`-Befunds oben).

## Bugs (mittel / Warnungen)

- [ ] **`mpg123_feedseek()`-Rückgabe ignoriert, `input_offset` ungenutzt** (`src/audio/AudioDecoder_MP3.cpp:166-167`)
  ```cpp
  off_t input_offset = 0;
  mpg123_feedseek((mpg123_handle*)decoder, 0, SEEK_SET, &input_offset);
  ```
  Für Feed-Handles liefert `mpg123_feedseek()` den Byte-Offset, ab dem gefüttert werden muss, und einen Fehlercode. Beides wird verworfen; die Datei wird stattdessen auf `info.AudioStart` (Zeile 162) gesetzt. Damit stimmt die interne Byte-Offset-Buchhaltung von libmpg123 nicht mit dem tatsächlich gefütterten Bereich überein — genau das, was libmpg123 für Seek-Genauigkeit und Gapless-Auswertung nutzt.
  Behebung: Rückgabewert prüfen und `ff->seek(info.AudioStart + input_offset)` verwenden (bzw. `mpg123_feedseek`-Ergebnis direkt als Feed-Position nutzen).

- [ ] **`seekSample(sample != 0)` wirft, toter Code dahinter** (`src/audio/AudioDecoder_MP3.cpp:171-173`)
  Alle Seek-Ziele außer 0 werden mit `UnsupportedFeatureException` abgelehnt; die zwei auskommentierten Zeilen 172-173 (Clamping) sind tot. Damit ist Zufallszugriff auf MP3 nicht möglich, obwohl libmpg123 das über `mpg123_feedseek()` (Frame-basiert) unterstützt.
  Behebung: Implementierung nachrüsten:
  ```cpp
  void AudioDecoder_MP3::seekSample(size_t sample)
  {
      if (!decoder) throw DecoderException("AudioDecoder_MP3::seekSample: not open");
      off_t input_offset = 0;
      int ret = mpg123_feedseek((mpg123_handle*)decoder, static_cast<off_t>(sample), SEEK_SET, &input_offset);
      if (ret != MPG123_OK) throw DecoderException("mpg123_feedseek: %s", mpg123_strerror((mpg123_handle*)decoder));
      ff->seek(info.AudioStart + input_offset);
      out_offset = out_size = 0;
      position = sample;
  }
  ```
  Bis dahin in `decoder.h` dokumentieren, dass nur `seekSample(0)` unterstützt wird.

- [ ] **Ergebnis von `mpg123_getformat()` bei `MPG123_NEW_FORMAT` wird verworfen** (`src/audio/AudioDecoder_MP3.cpp:199-202`)
  `rate`, `channels` und `enc` werden gelesen, aber nirgends gespeichert. `lastDecodeFormat` bleibt damit falsch, und `info.Channels`/`info.Frequency` können von den tatsächlich gelieferten Daten abweichen (bei VBR/Multi-Rate-Streams oder nach dem Format-Hardcoding-Bug). Die von libmpg123 ausdrücklich empfohlene Reaktion auf `MPG123_NEW_FORMAT` („call mpg123_getformat() to get the currently active output format") unterbleibt.
  Behebung: `lastDecodeFormat = enc;` setzen und `rate`/`channels` gegen `info` validieren (Notfall: `DecoderException` oder Doku-Hinweis).

- [ ] **`close()` setzt `info` nicht zurück** (`src/audio/AudioDecoder_MP3.cpp:138-152`)
  Nach `close()` liefert `getAudioInfo()` (Zeile 154-157) weiterhin die Daten der zuvor geöffneten Datei. Die Basisklassen-Doku (`decoder.h:97-98`) erklärt den Zustand für „undefiniert" — praktisch ist das eine Stolperfalle, wenn dasselbe Objekt für eine neue Datei genutzt wird, ohne dass `open()` aufgerufen wurde.
  Behebung: `info = AudioInfo{};` in `close()`, oder Doku auf „gültig" ändern (dann aber konsistent über alle vier Decoder).

- [ ] **`while (size == 0)`-Schleife ohne harte Iterationsgrenze** (`src/audio/AudioDecoder_MP3.cpp:186-206`)
  Bei `ret == MPG123_OK` und `size == 0` (theoretisch möglich, z. B. nach `MPG123_NEW_FORMAT` ohne Nutzdaten) oder bei ständig wiederkehrendem `NEED_MORE` mit `len > 0` aber unvollständigem Frame läuft die Schleife sehr lange bzw. unendlich. Ein Abbruchkriterium (EOF-Flag, maximale Iterationen) fehlt.
  Behebung: Zustand explizit machen (`bool eof = false;` setzen, wenn `maxbytes == 0` oder `len == 0`) und bei „kein Fortschritt trotz EOF" mit `DecoderException` aussteigen.

- [ ] **`info.Channels` wird als mpg123-Kanalarmas übergeben** (`src/audio/AudioDecoder_MP3.cpp:223`, `268`, `316`, `360`)
  `mpg123_format()` erwartet die Konstanten `MPG123_MONO (1)` / `MPG123_STEREO (2)`. Dass `info.Channels` (1 bzw. 2) zufällig dieselben Werte hat, ist eine stille Kopplung an eine Fremd-Bibliothek — ändert libmpg123 die Werte, schweigt der Compiler. Ebenso ist `info.Frequency` (ein `uint32_t`-Feld) für den `long`-Parameter nicht typsicher.
  Behebung: explizite Abbildung mit Kommentar (siehe Behebungsvorschlag im Format-Hardcoding-Befund).

## Code Smells & Modernisierung (C++20, REFACTORING.md)

- [ ] **1 MiB Eingangs- und 32 KiB Ausgangspuffer pro Objekt** (`src/audio/AudioDecoder_MP3.cpp:48-49`, `127`, `131`)
  `#define READBUFFER 1024 * 1024` wird in Zeile 190-191 auch als Obergrenze pro `ff->read()` benutzt. Für die anvisierten Mikrocontroller-Ziele (RP2040: 264 KiB RAM) ist das unmöglich. Empfehlung: Puffergrößen als Konstanten mit klarer Semantik (`constexpr size_t kFeedChunkBytes = 8192;`) und idealerweise konfigurierbar; Makros durch `constexpr` ersetzen (Typ- und Scope-Sicherheit).
- [ ] **Globaler Zustand über statische Variablen + Mutex** (`src/audio/AudioDecoder_MP3.cpp:46-47`, `67-76`, `84-91`)
  `MPG123_GlobalMutex`/`mpg123_instances` implementieren ein Referenzkonto für `mpg123_init()`/`mpg123_exit()`. Das ist statischer, nicht zerstörbarer Zustand (Static-Init-/Shutdown-Order bei globalen Decodern) und wirft bei jeder Konstruktion/Destruktion einen Lock. Empfehlung: function-local static mit `std::once_flag` für `mpg123_init()`, `mpg123_exit()` über einen RAII-Helper; `mpg123_instances` entfällt dann.
- [ ] **Totes Member-Ensemble `isRunning`, `needInput`, `samplesize`, `out_offset`/`out_size`-Halbierung** (`include/pplib/audio/decoder.h:238-241`)
  `isRunning` und `needInput` werden in `open()`/`close()` gesetzt (Zeile 123-124, 149-150), aber von keiner Methode gelesen; `samplesize` (Zeile 55) wird nie ausgewertet. Empfehlung: entfernen — jeder tote Zustand ist eine Quelle für Desync-Bugs (siehe `position`-/`lastDecodeFormat`-Befunde).
- [ ] **Typlose Handle-Speicherung `void* decoder`** (`include/pplib/audio/decoder.h:231`)
  Führt zu 20 manuellen `(mpg123_handle*)`-Casts in dieser Datei. Empfehlung: `mpg123_handle* decoder;` mit Forward-Deklaration/`struct mpg123_handle;` im Header, oder PImpl mit `std::unique_ptr` und eigenem Deleter (`mpg123_delete`).
- [ ] **Vier nahezu identische Lesemethoden** (`src/audio/AudioDecoder_MP3.cpp:215-397`)
  Die Blöcke unterscheiden sich nur in Zieltyp, `=`/`+=` und `bytes_per_frame` (2 vs. 4). Genau hier sind dieselben Fehler viermal kopiert (Formatwechsel, `bytes_per_frame`, `position`). Empfehlung: eine private Kernmethode `template <class T> size_t readFrames(size_t num, T* dst, bool additive)` plus Concept für die Sample-Typen; `bytes_per_frame` aus `sizeof(typename T::value_type)` ableiten statt hart zu rechnen.
- [ ] **C-Style-Casts im Datenpfad** (`src/audio/AudioDecoder_MP3.cpp:237`, `240`, `282`, `288`, `330`, `333`, `374`, `380`)
  `(STEREOSAMPLE16*)(outbuffer + out_offset)` usw. — Aliasing auf `uint8_t*`-Puffer. Empfehlung: `reinterpret_cast` mit Begründung, oder getrennte, typgerechte Puffer (`std::vector<STEREOSAMPLE16>`/`std::vector<float>`), was auch die `bytes_per_frame`-Rechnung überflüssig macht.
- [ ] **`#ifndef HAVE_MPG123`-Muster in jeder Methode** (`src/audio/AudioDecoder_MP3.cpp:97`, `217`, `262`, `310`, `354`)
  Fünf Kopien; im `#else`-Zweig von `fillDecodeBuffer()` (Zeile 210-212) wird still `0` zurückgegeben, statt eine Exception zu werfen. Empfehlung: wie in `AudioDecoder_Ogg` angedeutet — eigene Stub-Implementierung, damit „nicht gebaut" ≠ „Dateiende".

## Dokumentation & Doxygen

- [ ] **Klasse `AudioDecoder_MP3` ohne Doxygen** (`include/pplib/audio/decoder.h:228`)
  Es fehlt: unterstützte MPEG-Layer/Versionen (libmpg123), dass VBR unterstützt wird (`AudioInfo::IsVBR`), dass `seekSample()` nur `0` kann, und dass Abtastrate/Kanalzahl des Outputs an die Datei gebunden sind (kein Resampling gewünscht). Für IntelliSense wäre außerdem der Hinweis auf die benötigte libmpg123 (`HAVE_MPG123`) und das Verhalten ohne Build-Support (`UnsupportedFeatureException`) wichtig.
- [ ] **`seekSample()`-Doku fehlt / irreführend** (`include/pplib/audio/decoder.h:249` Kontext, Basis `104-107`)
  Die Basis-Doku verspricht Seek auf Sample-Nummern; die MP3-Implementierung wirft für alles außer 0. Zumindest überschreibend in `decoder.h:249` dokumentieren (`@exception UnsupportedFeatureException bei sample != 0`).
- [ ] **Rückgabewerte der Lesemethoden undokumentiert** (`include/pplib/audio/decoder.h:251-257`)
  Nicht dokumentiert: Rückgabe = tatsächlich gelesene Frames; Verhalten am Dateiende (0, keine Exception); dass `num` nicht an `info.Samples` begrenzt wird (siehe kritischer Befund).
- [ ] **`fillDecodeBuffer()` ohne Kommentar zu Rückgabe und Seiteneffekten** (`include/pplib/audio/decoder.h:244`)
  Die Methode füllt `outbuffer` und setzt `out_offset`/`out_size`; die Rückgabe 0 bedeutet „Ende". Das ist nur aus dem Code ersichtlich und gehört in den Header (bzw. als private Methode in die Quelle mit Doku).
- [ ] **`out_offset`/`out_size`/`lastDecodeFormat` ohne Einheitenkommentar** (`include/pplib/audio/decoder.h:239-242`)
  `out_size`/`out_offset` sind Bytes, `position` ist Frames, `lastDecodeFormat` sind mpg123-Encoding-Bits. Ohne Einheitenangabe sind die `bytes_per_frame`-Rechnungen kaum prüfbar.

## Befunde in Drittdateien

- [ ] **`AudioInfo::Samples` für MP3 stammt aus Xing/Frame-Zählung und kann deutlich unter der realen Decodiermenge liegen** (`src/audio/AudioInfo.cpp:184`)
  Beobachtet: `info.Samples = 24188` (22 kHz) bzw. `47187` (mono), libmpg123 liefert aber 81920 bzw. 172424 Frames. Solange der Decoder nicht an `info.Samples` begrenzt (kritischer Befund oben), ist die Diskrepanz für Aufrufer nicht erkennbar.
- [ ] **`FileObject::read()` wirft bei Dateiende statt Short-Count** (`src/core/File.cpp:489-491`)
  In `fillDecodeBuffer()` (Zeile 193) wird das aktuell durch die vorangehende `maxbytes`-Prüfung (Zeile 191-192) abgefedert; verlässt man sich auf `read()`, droht eine Exception mitten im Decode-Pfad.
- [ ] **`decoder.h` deklariert MP3-Member in falscher Kohärenz** (`include/pplib/audio/decoder.h:231-242`)
  `void* decoder` verhindert, dass der Compiler die mpg123-spezifischen Zustandsfelder prüft; `bool isRunning`/`bool needInput` (240-241) und `size_t samplesize` (238) werden nie ausgewertet. Siehe Code-Smell-Block — Änderung ist Header-seitig und damit API-relevant.
