# Ogg Vorbis Decoder Review – Befunde

Review vom 2026-11-01, Scope: `src/audio/AudioDecoder_Ogg.cpp` (439 Z.), `include/pplib/audio/decoder.h` (`AudioDecoder_Ogg`, Zeilen 576–604).
Öffentliche API: `AudioDecoder_Ogg::open()`, `close()`, `getAudioInfo()`, `seekSample()`, `getPosition()`, `getSamples(size_t, STEREOSAMPLE16*)`, `addSamples(size_t, STEREOSAMPLE32*)`, `getSamples(size_t, STEREOSAMPLE_FLOAT*)`, `addSamples(size_t, STEREOSAMPLE_FLOAT*)`, `IdentOggVorbisAudioFile()`
Review done by: Copilot (Qwen3-8-Flash-Next, Reasoning: xhigh, mit Nachträgen aus dem medium-Lauf)

Methodik: frische Objekte der Datei gegen die vorhandenen `build/asan`-Archive gebaut, Proben außerhalb der Projektquellen, Laufzeit-Checks mit AddressSanitizer + UndefinedBehaviorSanitizer + LeakSanitizer (GCC 14, Linux x86-64, libvorbis/vorbisfile 1.3.7), Referenzvergleiche direkt gegen libvorbisfile. „verifiziert“ bedeutet: Sanitizer- bzw. Programmlauf, wie im Befund angegeben. Befunde ohne Laufzeitbeweis sind mit **[Prüfen]** markiert.
  Testmaterial: `test_44kHz.ogg`/`test_48kHz_mono.dat` aus dem Projekt; dazu mit ffmpeg erzeugt: Mono-/Stereo-Dateien (`-b:a 64k/96k/192k`, `-q 3`), 5.1-Dateien (OGG und FLAC, `-ac 6`), verkettete Dateien (über `concat`-Demuxer bzw. `cat` zweier OGG) und Trunkate (`head -c`). Alle Probeläufe wurden mit jeweils frisch erzeugten Objekten dieser Datei gefahren, damit die Sanitizer-Stacks Zeilen der vorliegenden Quelle zeigen.
  Nachtrag vom selben Tag: Sechs Befunde wurden nach dem Abgleich mit einem früheren Review-Entwurf dieses Moduls eingefügt; jeder ist im Titel mit „(Nachtrag)“ gekennzeichnet und wurde zusätzlich am aktuellen Code bzw. mit der Probe `T_MULTICH` (Multichannel-OGG/-FLAC) geprüft.

## Bugs (kritisch)

- [ ] **`open()` nach `close()` stürzt ab (NULL-Zugriff)** (`AudioDecoder_Ogg.cpp:238`, `close()` in `:269`)
  `close()` gibt `private_data` frei und setzt den Zeiger auf `NULL`, `open()` verwendet ihn danach bedingungslos:
  `OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);` → Zugriff auf `oggp->callbacks`/`&oggp->vf`.
  Verifiziert: UBSan `member access within null pointer of type 'struct OggVorbisPrivateData'` bei `AudioDecoder_Ogg.cpp:238:32`, direkt danach `AddressSanitizer: SEGV on unknown address 0x0000000003b0` (Probe „B_reopen": `open` → `getSamples` → `close` → `open`). Ein Wiederholen von `open()` ist damit nicht möglich, ohne das Objekt zu vernichten; der Aufrufer kann den Zustand auch nicht abfragen.
  Der FLAC-Decoder macht es vor: `AudioDecoder_Flac.cpp:322` beginnt `open()` mit `if (ff != nullptr) close();`.
  Minimaler Fix oben in `open()` (löst gleichzeitig den Leak-Befund „Doppeltes `open()`"):
```c++
    close(); // gibt den vorhandenen Handle frei, macht open() idempotent
    private_data = calloc(sizeof(OggVorbisPrivateData), 1);
    if (!private_data) throw OutOfMemoryException();
    OggVorbisPrivateData* oggp = static_cast<OggVorbisPrivateData*>(private_data);
    init_callback(oggp->callbacks);
```

- [ ] **`getSamples()`/`addSamples()`/`seekSample()` nach `close()`: Absturz statt `DecoderException`** (`AudioDecoder_Ogg.cpp:313`, `:328`, `:374`, `:410`, `:289`)
  Nach `close()` ist `private_data == NULL`; alle Lese- und Seek-Funktionen dereferenzieren ihn sofort (`ov_read(&oggp->vf, …)`, `ov_pcm_seek(&oggp->vf, …)`). Der in `decoder.h` dokumentierte Vertrag der anderen Decoder lautet aber „`@exception DecoderException` Vor `open()` oder nach `close()`" (z. B. `decoder.h:293`, `:301`, `:307`).
  Verifiziert: UBSan „member access within null pointer" in `:313` bzw. `:289`, jeweils gefolgt von `AddressSanitizer: SEGV` (Proben „C_readAfterClose", „C2_seekAfterClose").
  Minimaler Fix, je Funktion als erste Zeile:
```c++
    if (!private_data) throw DecoderException("AudioDecoder_Ogg: Decoder ist nicht geöffnet");
```

- [ ] **16-Bit-Lesepfad: falsche Frame-Zahl bei großem `num` (Ganzzahl-Überlauf)** (`AudioDecoder_Ogg.cpp:308`, Rückgabe in `:322` und `:341`)
  `int rest = num * info.BytesPerSample;` verengt das Produkt von `size_t` in `int`. Bei Stereo (`BytesPerSample == 4`) kippt es bereits ab `num >= 0x20000000`; `rest` wird negativ, die `while (rest > 0)`-Schleife läuft null Mal, und `return num - (rest / 4)` liefert **mehr Frames als angefordert**, ohne etwas geschrieben zu haben. Ein Aufrufer, der sich an die Returnzahl hält (Puffervertrag in `decoder.h:289-291`), schreibt bzw. liest dann über seinen Puffer hinaus.
  Verifiziert (ASan-Build, keine Speicherfehler, weil der Puffer im Probenprogramm bewusst nicht gelesen wurde):
  `getSamples(0x20000000, buf)` auf `test_44kHz.ogg` → **Rückgabe 1073741824 Frames** (angefordert 536870912), `getPosition()` unverändert 0.
  Minimaler Fix: Rechnung durchgehend in `size_t`, und Begrenzung nach oben:
```c++
    size_t rest = num * (size_t)info.BytesPerSample;
    ...
    size_t done = num - (rest / 4);
    return (done <= num) ? done : num; // Analog in der Mono-Variante: rest / 2
```

- [ ] **Float-Pfad: Heap-Überlesen bei verketteten OGG-Dateien mit Kanalwechsel** (`AudioDecoder_Ogg.cpp:379`, analoge Stelle im Float-`addSamples` bei `:414`)
  Die Verzweigung nutzt `info.Channels` — das stammt aus Kette 0 — libvorbisfile liefert aber die PCM-Zeiger der **aktiven** Kette. Enthalten die Ketten unterschiedliche Kanalzahlen, wird `pcm[1]` gelesen, obwohl libvorbis nur ein Feld allokiert hat.
  Verifiziert: `AddressSanitizer: heap-buffer-overflow`, `READ of size 8`, „0 bytes after 8-byte region", Zuteilungsstack `malloc` ← `vorbis_synthesis_init` (libvorbis) ← `ov_read_float` ← `AudioDecoder_Ogg.cpp:379` (Datei `chained.ogg`: Kette 0 stereo, Kette 1 mono, 2 Links).
  Gegenprobe mit minimalem Fix — Kanalzahl der aktiven Kette statt `info.Channels` benutzen (beide Float-Pfade):
```c++
    int chan = (oggp->vf.vi && oggp->vf.current_link >= 0 && oggp->vf.current_link < oggp->vf.links)
                   ? oggp->vf.vi[oggp->vf.current_link].channels : info.Channels;
```
  Damit läuft dieselbe Probe fehlerfrei durch (0 Sanitizer-Meldungen auf `chained.ogg`, `chained2.ogg`, `chained3.ogg`, `mono.ogg`, `test_44kHz.ogg`).
  Randbemerkung zur Gegenrichtung (mono → stereo, `chained3.ogg`): der `info.Channels == 1`-Zweig kopiert nur `pcm[0]`, der zweite Kanal der Stereo-Kette wird lautlos verworfen (verifiziert: unterschiedliche Prüfsummen 474 gegen 18446744073709551524 bei gleicher Framezahl).

- [ ] **`getPosition()` bleibt nach 16-Bit-Lesevorgangen auf 0** (`AudioDecoder_Ogg.cpp:313-341`, `:353`)
  Nur die beiden Float-Pfade ziehen `position` nach (`:395`, `:431`). `getSamples(STEREOSAMPLE16*)` und das darüber laufende `addSamples(STEREOSAMPLE32*)` ändern `position` nicht; `getPosition()` meldet daher nach vollständigem Durchlauf 0. MP3- und FLAC-Decoder führen `position` in allen Pfaden nach (im Review geprüft).
  Verifiziert: Factory-Pfad auf `test_44kHz.ogg`: dekodiert 173695 Frames, `getPosition()=0`; Mono-Datei: `pos after s16 loop=0`, `addSamples32 pos=0`. Nach dem Fix (position in beiden 16-Bit-Zweigen nachziehen) dieselbe Probe: `frames=173695 getPosition=173695`.
  Minimaler Fix: in beiden Zweigen mitzählen, z. B. Stereo
```c++
    size_t done = num - (rest / 4);
    position += done;
    return done;
```
  und Mono akkumuliert über die Schleife (`wr += bytes_read / 2; … position += wr; return wr;`).

- [ ] **Doppeltes `open()` ohne `close()` leakt den kompletten libvorbisfile-Handle** (`AudioDecoder_Ogg.cpp:238`)
  `open()` ruft erneut `ov_open_callbacks()` auf, ohne den vorherigen Handle zu klären. Pro Objekt bleiben dabei Sync-Puffer, Stream-Zustände und Vorbis Decode-Strukturen stehen.
  Verifiziert: LeakSanitizer `69244 byte(s) leaked in 125 allocation(s)`, Direkt-Leak-Stacks in `ov_open_callbacks` → `ogg_sync_buffer` (10412 B), `ogg_stream_init` (16384/8192/4096 B). Kontrollprobe mit nur einem `open()` meldet keine Leaks. Auslöser ist auch der übliche Aufrufpfad: `GetAudioDecoder()` öffnet den Decoder bereits selbst (`AudioDecoder.cpp:51`), ein zusätzliches `open()` des Aufrufers leakt also bereits.
  Minimaler Fix: siehe Snippet beim ersten Befund (`close()` am Anfang von `open()`).

## Bugs (mittel / Warnungen)

- [ ] **`ov_pcm_seek()`-Rückgabewert wird ignoriert** (`AudioDecoder_Ogg.cpp:289`)
  Ein Sprung hinter das Dateiende oder in einen ungültigen Stream wird als Erfolg behandelt, weil anschließend bedingungslos `position = sample;` steht.
  Verifiziert (`chained.ogg`, 173695 Samples): `seekSample(Samples + 100000)` → `getPosition()=273695` (großer als `Samples`), anschließfend liefert `getSamples()` erneut Daten (4 Frames); `seekSample(2000000)` → `pos=2000004`, weiterhin Daten. Ein `seekSample()` vor/ohne `open()` wird ebenfalls kommentarlos akzeptiert.
  Minimaler Fix:
```c++
    if (ov_pcm_seek(&oggp->vf, sample) != 0)
        throw DecoderException("AudioDecoder_Ogg::seekSample: Zielposition nicht erreichbar");
    position = sample;
```

- [ ] **`ov_pcm_total()`-Fehlerwert landet als Mega-Zahl in `AudioInfo`** (`AudioDecoder_Ogg.cpp:174`, `:176`)
  `info.Samples = ov_pcm_total(&vf, -1);`weise t den `ogg_int64_t` ohne Plausibilitätsprüfung einem `uint64_t` zu; libvorbisfile liefert bei unvollständigem/nicht lesbarem Stream einen negativen Fehlerwert. Auch `Length` wird aus dem Unsinn gerechnet.
  Verifiziert (Probe mit werfender `FileObject`-Subklasse, die den Tell-Callback beim vierten Aufruf abbricht): `AudioInfo::Samples = 18446744073709551485`, `open()` läuft durch, `getSamples()` liefert weiter Daten.
  Minimaler Fix:
```c++
    const ogg_int64_t total = ov_pcm_total(&vf, -1);
    info.Samples = (total > 0) ? (uint64_t)total : 0;
```

- [ ] **Verkettete OGG-Dateien: `Samples`/`Length` über alle Ketten, Decode liefert nur die erste** (`AudioDecoder_Ogg.cpp:174`, `:176`, Auswirkung in allen Lesepfaden)
  `ov_pcm_total(&vf, -1)` zählt die Samples aller Links, der sequenzielle Lesebetrieb endet aber am Kettenende.
  Verifiziert: `chained2.ogg` (2 Links mit je 44100 Frames, beide stereo) → `AudioInfo::Samples=88200`, `Length=2000` ms, dekodiert werden 44100 Frames (16-Bit- und Float-Pfad, je 87 Aufrufe). Referenz direkt gegen libvorbisfile: `ov_read()` liefert nach 176400 Bytes (= 44100 Stereo-Frames) 0. Bei `chained.ogg` (stereo → mono) überschneidet sich das mit dem Heap-Befund oben.
  Konsequenz: Fortschrittsanzeige/Ende-Erkennung von Playern und Mixern, die `Samples` vertrauen, falsch um den Faktor (Anzahl Ketten).
  Minimaler Fix: bei `vf.links > 1` `ov_pcm_total(&vf, 0)` verwenden und im Header dokumentieren, dass nur die erste Kette dekodiert wird.

- [ ] **Falscher Exception-Typ bei „noch nicht offen"** (`AudioDecoder_Ogg.cpp:343`, `:387`, `:423`)
  Die Kanalzahlen-Prüfung in den Lesepfaden wirft `UnsupportedAudioFormatException`; nach dem Basisvertrag (`decoder.h:293`, `:301`) wäre dort `DecoderException` erwartet, das `UnsupportedAudioFormatException` gehört in `open()`.
  Verifiziert: frisch konstruierter, nie geöffneter Decoder → `getSamples(512, buf)` wirft `UnsupportedAudioFormatException: channels <1 or >2`; `addSamples(512, STEREOSAMPLE32*)` ebenso; der Float-Pfad wirft zufällig `DecoderException: ...: -131` (libvorbis-Fehlercode auf nullgesetztem Handle, kein Crash).
  Minimaler Fix: in den Lesepfaden `DecoderException` werfen (zusammen mit dem Guard aus dem zweiten Bug-Befund erledigt sich das zumeist).

- [ ] **Echte I/O-Fehler werden als Dateiende interpretiert** (`AudioDecoder_Ogg.cpp:64-70`)
  Der Kommentar am Callback begründet das bewusst (Exception durch die C-Frames von libvorbis ist nicht portabel definiert). Dennoch verschluckt `catch (...)` aber jeden Lesefehler: `ppl_read_func` liefert 0, libvorbisfile hält das Decode für beendet, `getSamples()` liefert 0 wie am Dateiende. Bei einem defekten Bandmedium/Netzwerk-FileObject bricht der Download also lautlos halb ab.
  **[Prüfen]**: ob `FileObject` einen Weg bietet, „echter Fehler" von „Dateiende" zu unterscheiden (in `include/pplib/core/fileobject.h` ist kein separates Fehlerstatus-Flag sichtbar).
  Minimaler Vorschlag, ohne das Werfen-durch-C-Frames-Prinzip aufzubrechen: in `OggVorbisPrivateData` (Zeile 48-51) ein Flag `int io_error;` ergänzen, im `catch (...)` von `ppl_read_func`/`ppl_seek_func` setzen und nach jedem `ov_read()`/`ov_read_float()` in den Lesepfaden prüfen:
```c++
    if (oggp->io_error) { oggp->io_error = 0; throw DecoderException("AudioDecoder_Ogg: Lesefehler in FileObject"); }
```

- [ ] **Ausnahme einer `FileObject`-Subklasse reicht durch die libvorbis-C-Frames hindurch nach außen** (`AudioDecoder_Ogg.cpp:64-70`, `:78-99`) **[Prüfen]**
  Die Callbacks fangen `catch (...)`, aber `ppl_seek_func` ruft `f->tell()` und `f->size()` **außerhalb** des `try`-Blocks (Zeilen 88-90) und `ppl_tell_func` (`:105-108`) hat gar kein `try`. Eine werfende `FileObject`-Implementierung wirft also mitten durch libvorbis/ogg hindurch.
  Verifiziert auf Linux/GCC: funktioniert kontrolliert (Probe wirft in `tell()` beim vierten Aufruf, Exception erreicht den Aufrufer; unter ASan/UBSan kein Leak, kein Crash, weil der RAII-Handle `OggVorbisHandle` (`:118-140`) aufräumt). Auf Windows/MSVC ist Unwinding durch C-Fremd-Frames nicht garantiert → dort **[Prüfen]**.
  Minimaler Fix: `f->tell()`/`f->size()` in den `try`-Block von `ppl_seek_func` holen und `ppl_tell_func` mit `try { … } catch (...) { return -1; }` absichern.

- [ ] **`AudioInfo` wird beim Identifizieren einer trunkierten Datei „gültig"** (`AudioDecoder_Ogg.cpp:174`, Auswirkung in `IdentOggVorbisAudioFile`)
  Verifiziert: `trunc6000.ogg` (6000 Bytes, angeschnittene Kette) → `ident=1`, `ch=2`, `bits=4`, `Samples=0`, `Length=0`, `kbit=192`, `AudioStart=3852`, `AudioEnd=5999`; `getSamples()` liefert 0 Frames ohne Exception. Kurze Trunkate (bis 3000 Bytes) werden korrekt mit `ident=0` abgelehnt.
  Kein Fehler im engeren Sinn, aber die Kombination `ident=1` + `Samples=0` sollte im Header stehen, damit Aufrufer nicht auf „Datei hat 0 Frames" als Formatfehler prüfen. Double-Free durch `OggVorbisHandle` wurde geprüft: Destruktor ruft bedingungslos `ov_clear()`, stderr blieb leer (keine Meldungen).

- [ ] **`ppl_tell_func` liefert `long` (32 Bit auf Windows)** (`AudioDecoder_Ogg.cpp:105-108`)
  Die Signatur ist durch libvorbisfile vorgegeben (`long (*tell_func)(void*)`), eine Änderung ist nicht möglich. Auf Linux (LP64) unkritisch; bei Dateien > 2 GiB unter Windows/LLP64 liefert der Callback abgeschnittene Positionen, was `ov_*_seek()` und die Offset-Berechnung in `GetVorbisAudioInfo` (`:167`) stört. **[Prüfen]** (Windows-Build nicht ausgeführt).

*Nachträge: Die folgenden Befunde standen nicht im ersten Durchlauf; sie wurden beim Abgleich mit einem früheren Review-Entwurf und anschließender Kontrolle am aktuellem Code nachgetragen.*

- [ ] **Multichannel-Ogg passiert die Datei-Erkennung (Nachtrag)** (`AudioDecoder_Ogg.cpp:190-192`)
  `IdentOggVorbisAudioFile()` prüft die Kanalzahl nicht und liefert auch bei 5.1-Material `true`; `AudioInfo` meldet dann `Channels=6` und eine Samplezahl. Begrenzt wird erst in `open()` (`:246`), also mitten in der Decoder-Erstellung (siehe den Drittdatei-Befund zum Decoder-Leak in `AudioDecoder.cpp`). Zum Vergleich: `IdentWave()` (`AudioInfo.cpp:188`) und `IdentAIFF()` (`:281`) lehnen mehr als zwei Kanäle direkt ab; der FLAC-Zweig akzeptiert Multichannel bewusst und begründet das im Code (`AudioInfo.cpp:440-442`) — ein solcher Hinweis fehlt beim Ogg-Zweig.
  Verifiziert (Testprobe `T_MULTICH`, selbst erzeugtes 5.1-Ogg mit ffmpeg `-ac 6 -channel_layout 5.1 -b:a 384k`): `IdentOggVorbisAudioFile()=1` mit `ch=6`, `IdentAudioFile()=1`. `GetAudioDecoder()` wirft `UnsupportedAudioFormatException: channels <1 or >2`; LeakSanitizer meldet zusätzlich den Decoder-Leak (siehe den Drittdatei-Befund zum Decoder-Leak in `AudioDecoder.cpp`).
  Minimaler Fix, falls Ogg weiterhin auf ein/zwei Kanäle beschränkt bleiben soll (der Handle wird per `OggVorbisHandle` abgeräumt, `ov_clear()` darf hier nicht zusätzlich aufgerufen werden):
```c++
    if (vf->vi->channels < 1 || vf->vi->channels > 2) return false; // Handle-Destruktor uebernimmt ov_clear()
```
  Alternativ kann die bewusste Entscheidung bleiben; dann gehört sie in die Klassen-Doku.

- [ ] **`info.Mode` bleibt bei Ogg auf `ChannelMode::UNKNOWN` (Nachtrag)** (`AudioDecoder_Ogg.cpp:142-178`)
  Weder `GetVorbisAudioInfo()` noch `IdentOggVorbisAudioFile()` weisen einen Kanalmodus zu, es bleibt der Default `UNKNOWN` (`audioinfo.h:140`). Die anderen Formate setzen ihn: WAV (`AudioInfo.cpp:219`), AIFF (`:294`, `:324`), FLAC (`:468`), MP3 ebenfalls. Verifiziert in derselben Probe: Stereo-Ogg liefert `Mode=0`. In pplib liest derzeit kein Modul `Mode` (entsprechende Suche in `src/` ohne Treffer), externe Aufrufer können Mono/Stereo aber nicht abfragen. Minimaler Fix in `GetVorbisAudioInfo()`:
```c++
    info.Mode = (vf.vi->channels == 1) ? AudioInfo::ChannelMode::MONO : AudioInfo::ChannelMode::STEREO;
```

- [ ] **Beide Float-Pfade: `num` ohne Obergrenze, Verengung in `long`/`int` (Nachtrag)** (`AudioDecoder_Ogg.cpp:372-374`, `:408-410`)
  `ov_read_float()` erwartet eine `int`-Puffergröße; die Pfade übergeben `(int)(num - total)` und vergleichen mit `(long)num`. Bei `num > INT_MAX` wird die Länge negativ, libvorbisfile schreibt damit unbegrenzt in den Zielbereich, und außerdem ist die Rückgabe-Berechnung unplausibel. Verifiziert: **[Prüfen]** (kein Laufversuch; ASan-Test mit > 2^31 Frames war nicht möglich, weil die Zielallokierung den Speicher überschritten hätte.) Empfehlung wie bei der 16-Bit-Variante: `num` nach oben begrenzen, vor dem Dekodieren.
  Minimaler Fix:
```c++
    if (num > (size_t)INT_MAX / sizeof(float)) num = (size_t)INT_MAX / sizeof(float);
```
  (analog für `ov_read()` mit Faktor 4)

## Wartbarkeit & Aufräumen

- [ ] **`decodebuffer` Größe und Nutzung passen nicht zusammen** (`AudioDecoder_Ogg.cpp:247-250`, `:326-328`)
  `decodebuffer_size = 4096; decodebuffer = (char*)malloc(decodebuffer_size * 4);` legt 16 KiB an, benutzt wird der Puffer aber nur mit maximal `decodebuffer_size` Bytes (`int size = decodebuffer_size; if (rest < size) size = rest;`). Faktor 4 zu groß, und die Variable bedeutet einmal „Bytes", einmal „Bytes/4". Der Stereo- und der Float-Pfad verwenden den Puffer gar nicht. Empfehlung: `malloc(decodebuffer_size)` und `decodebuffer_size` als Byte-Grenze stehen lassen (kein Verhaltensunterschied, 12 KiB weniger pro Decoder).

- [ ] **Toter Code / auskommentierte Reste** (`AudioDecoder_Ogg.cpp:100-105`, `:162-163`, `:350`)
  Auskommentierter `ppl_close_func`-Block, `PrintDebug`-/`ov_bitrate`-Zeilen und ein auskommentierter `OggVorbisPrivateData*` in `addSamples(STEREOSAMPLE32*)`. Empfehlung: entfernen (der Destruktor `close()` ist der aufräumende Pfad, ein `close_func`-Callback wäre bei `FileObject`-Lebenszeit gefährlich).

- [ ] **`pcm` wird nie auf `NULL` geprüft** (`AudioDecoder_Ogg.cpp:374`, `:410`)
  `float** pcm` wird mit `nullptr` initialisiert, bei `got > 0` aber unbedarft indiziert. Eine defensive Zeile (`if (got > 0 && pcm == NULL) got = 0;`) kostet nichts und schützt gegen unerwartete libvorbis-Rückgaben.

- [ ] **Kanalzahl wird doppelt geprüft, mit unterschiedlichen Exceptions** (`AudioDecoder_Ogg.cpp:246` und `:343`, `:387`, `:423`)
  `open()` wirft `UnsupportedAudioFormatException`, die Lesepfade dieselbe Prüfung erneut mit derselben Exception (siehe Exception-Befund oben). Empfehlung: Prüfung nur in `open()`, in den Lesepfaden nichts werfen.

- [ ] **`SampleType` wird in `GetVorbisAudioInfo` nie gesetzt** (`AudioDecoder_Ogg.cpp:142-178`)
  `info.SampleType` bleibt auf dem Default `SampleEncoding::INTEGER` (`audioinfo.h:141`); WAV und FLAC setzen ihn explizit (`AudioInfo.cpp:297`, `:393`). Für OGG ist das inhaltlich vertretbar (Lese-API liefert 16-Bit-Integer), sollte aber im Header stehen, weil die Float-Pfade Fließkomma liefern. Verifiziert: `IdentAudioFile()` auf Ogg-Dateien liefert `type=0` (INTEGER), `bits=16`.

- [ ] **Falscher Funktionsname in der Fehlermeldung des Float-`addSamples` (Nachtrag)** (`AudioDecoder_Ogg.cpp:428`)
  Die Meldung lautet `"AudioDecoder_Ogg::getSamples: %d"`, geworfen wird sie aber in `addSamples(size_t, STEREOSAMPLE_FLOAT*)`. Bei der Fehlersuche führt das in den falschen Pfad. Empfehlung: Text auf `addSamples` korrigieren.

## Dokumentation & Doxygen

- [ ] **`class AudioDecoder_Ogg` ohne jede Doxygen** (`include/pplib/audio/decoder.h:576-604`)
  Alle anderen Decoder-Klassen haben `@class`/`@brief` plus `@exception`-Hinweise (Muster: `decoder.h:183-330` Wave, `:606-724` FLAC, sowie die `@exception DecoderException Vor \`open()\` oder nach \`close()\``-Zeilen `:293`, `:301`, `:307`, `:313`). Für Ogg fehlt alles: keine Aussage zu `position`-Semantik (After welchem Lesevorgang wird sie nachgezogen?), keine zu `seekSample()`-Verhalten außerhalb des Bereichs, keine zu verketteten Dateien, keine zu `AudioInfo::Samples`/`Length`, keine Exception-Liste. Vorschlag: Klassenkommentar im Stil der FLAC-Klasse, mit den Punkten „nur erste Kette bei Verkettung", `BitsPerSample` immer 16, `SampleType` INTEGER, `Samples` aus `ov_pcm_total(-1)`.

- [ ] **Dokumentierter Basisvertrag wird nicht erfüllt** (`include/pplib/audio/decoder.h:293` und `AudioDecoder_Ogg.cpp` Lese-/Seek-Funktionen)
  Die Überschriften der anderen Decoder versprechen `DecoderException` „vor `open()` oder nach `close()`"; Ogg crasht dort (siehe kritische Befunde) bzw. wirft `UnsupportedAudioFormatException`. Nach dem Fix der 16-Bit-Pfade die Ogg-Klasse mit denselben `@exception`-Zeilen versehen.

- [ ] **Keine Aussage zu `IdentOggVorbisAudioFile()` und `GetVorbisAudioInfo()`** (`AudioDecoder_Ogg.cpp:181-193`, `:142-178`)
  Die Callback-Kommentare (`:53-70`, `:73-77`) sind gut und erklären die libvorbis-Verträge; die beiden Info-Funktionen selbst haben keine. Sinnvolle Ergänzung: `AudioStart`/`AudioEnd` sind Seitengrenzen (verifiziert an `test_44kHz.ogg`: `AudioStart=8060`, `AudioEnd=90767` bei 90768 Bytes), `AudioSize`/`AudioEnd` bleiben ungesetzt, wenn `vf.end <= vf.offset` (nicht seekbarer Input), `Bitrate`-Quelle ist nominale/untere/obere Bitrate mit Clamping auf `uint16_t`.

- [ ] **`IsVBR`/`Bitrate`-Ableitung undokumentiert und für VBR-Dateien fraglich** (`AudioDecoder_Ogg.cpp:154-161`) **[Prüfen]**
  `IsVBR` wird auf `false` gesetzt, wenn `bitrate_lower == bitrate_upper`. Bei einer mit `libvorbis -q 3` (VBR-Modus) kodierten Datei meldet pplib `IsVBR=false` und `Bitrate=112`; bei den ABR-Testdateien (`-b:a 64k/192k`) ebenso `IsVBR=false`, `Bitrate` 64/192. Ob libvorbis bei VBR-Kodierung untere und obere Bitrate tatsächlich gleich schreibt (dann ist die Klassifizierung korrekt) oder pplib VBR-Dateien falsch als CBR kennzeichnet, ließ sich mit den hier verfügbaren Werkzeugen nicht eindeutig belegen. Empfehlung: Verhalten im Header festhalten.

- [ ] **Verhalten ohne libvorbis-Bau und Sample-Skalierung undokumentiert (Nachtrag)** (`include/pplib/audio/decoder.h:576-604`)
  Der Klasse fehlen – wie im ersten Doku-Befund genannt – alle Vertragsangaben. Konkret sollten zwei Punkte eingetragen werden: (1) Das Verhalten ohne libvorbis im Build: Der Konstruktor wirft `UnsupportedFeatureException` (`AudioDecoder_Ogg.cpp:200-202`), `open()`/`close()`/`seekSample()` enthalten dann keinen Code, die Lesepfade geben 0 zurück (jeweils `#else`-Zweig, z. B. `:433-436`). (2) Die Skala der Leseformate, analog zur Tabelle bei `AudioDecoder_Flac`: 16-Bit-Pfade über `ov_read()` (native 16-Bit-Werte), Float-Pfade über `ov_read_float()` (normiert, etwa −1,0 bis 1,0). Die `@exception`-Angaben der Basisklasse (`decoder.h:293`, `:301`) fehlen ebenfalls.
  Verifiziert: `open()`-Rumpf komplett im `#ifdef HAVE_LIBVORBIS`; Konstruktor-Wurf im `#ifndef HAVE_LIBVORBIS`.

## Befunde in Drittdateien

- [ ] **`GetAudioDecoder()` leakt den Decoder, wenn `open()` wirft (Nachtrag)** (`src/audio/AudioDecoder.cpp:47`, `:51`)
  Die Funktion erzeugt das Decoder-Objekt (Zeile 47) und ruft `decoder->open(file, &info)` (Zeile 51) ohne Absicherung. Wirft `open()` – etwa bei Multichannel-Ogg (siehe Befund „Multichannel-Ogg passiert die Datei-Erkennung“) –, bleibt das Objekt verloren, zusammen mit den schon allokierten libvorbis-Strukturen. Die Header-Doku (`decoder.h:178-179`) nennt nur „NULL, falls kein passender Decoder gefunden wurde“ und lässt offen, dass `open()`-Ausnahmen durchgereicht werden und das Objekt dann undicht zurücklassen.
  Verifiziert: LeakSanitizer meldet „Direct leak 128 byte(s) in 1 object(s)” im Stack `operator new` ← ... `AudioDecoder.cpp:47`, dazu „Indirect leak 16384 byte(s)“ (`ogg_stream_init`), „12230 byte(s)“ (`ogg_sync_buffer`), „8544 byte(s) in 3 objects“ (`res0_unpack` ← `vorbis_synthesis_headerin`) über `AudioDecoder_Ogg::open` (`AudioDecoder_Ogg.cpp:238`) ← `AudioDecoder.cpp:51`; Auslöser ist eine mit ffmpeg erzeugte 5.1-Ogg-Datei.
  Minimaler Fix in `GetAudioDecoder()`: `open()` in try/catch, im Fehlerfall `delete decoder;` und `decoder = NULL;`, danach `throw;`.

- [ ] **FLAC-Klassen-Doku verspricht eine Kanal-/Bitbreiten-Begrenzung, die die Erkennung nicht umsetzt (Nachtrag)** (`include/pplib/audio/decoder.h:606-614`, `src/audio/AudioInfo.cpp:440-442`, `src/audio/AudioDecoder_Flac.cpp:330`)
  Die Klasse `AudioDecoder_Flac` dokumentiert, `IdentAudioFile()` erkenne mehrkanalige bzw. 8/32-Bit-Dateien „nicht als dekodierbares FLAC“. Der Erkennungszweig akzeptiert sie jedoch bewusst („Erkennung und Unterstuetzung sind getrennt; abgelehnt wird erst in AudioDecoder_Flac::open()“, `AudioInfo.cpp:440-442`); abgelehnt wird erst dort (`AudioDecoder_Flac.cpp:330`). Verifiziert: `IdentAudioFile()` auf einer selbst erzeugten 5.1-FLAC-Datei liefert `true` mit `ch=6` (Testdatei per ffmpeg, `-ac 6`); im verwendeten Build war libFLAC deaktiviert (`build/asan/include/config_pplib.h:98`), die Erkennung ist aber nicht an `HAVE_LIBFLAC` gekoppelt (`AudioInfo.cpp` enthält keine entsprechende Bedingung). Außerdem: der FLAC-Zweig setzt `info.Mode` auf `MONO`/`STEREO` (Zeile 468), also bei sechs Kanälen auf `STEREO`. Empfehlung: Klassen-Doku an die getrennte Erkenntnis-/Unterstützungslogik anpassen und das `Mode`-Verhalten für >2 Kanäle festlegen.

- [ ] **`GetAudioDecoder()` öffnet den Decoder selbst** (`src/audio/AudioDecoder.cpp:39-54`, `:51`)
  Der Aufrufer erhält einen bereits geöffneten Decoder; ein zusätzliches `open()` (nahe liegend, weil `open()` öffentlich ist) leakt pro doppeltem Öffnen ~69 KB (verifiziert, siehe kritischer Leak-Befund). Empfehlung: im Header von `GetAudioDecoder()` dokumentieren, dass der Decoder bereits geöffnet ist.

- [ ] **`IdentOggVorbisAudioFile()` nirgends im Header deklariert** (`src/audio/AudioInfo.cpp:42` lokale Deklaration, `:558` Aufruf; Definition in `AudioDecoder_Ogg.cpp:181-193`)
  Die Funktion ist nur lokal deklariert, die Signatur kann also unbemerkt auseinanderlaufen. Empfehlung: gemeinsame private Deklaration (analog zu anderen `Ident*`-Funktionen) oder `static` plus Registrierung.

- [ ] **Decoder-Interna im öffentlichen Header** (`include/pplib/audio/decoder.h:581-584`)
  `readbuffer`, `buffersize`, `decodebuffer`, `decodebuffer_size` sind Members von `AudioDecoder_Ogg` im Header, benutzt werden sie nur in zwei Funktionen der `.cpp`. Kein Fehler, aber die Größenanpassung (`decodebuffer_size * 4`, s. o.) wirkt sich auf die Objektgröße aller Decoder aus; eine Kapselung wie `private_data` wäre konsistenter. Nur zur Kenntnis.

## Strukturvorschläge (nur zur Diskussion)

- **RAII auch fur `open()`/`close()` verwenden** (`AudioDecoder_Ogg.cpp:118-140`, `:232-277`): Der `OggVorbisHandle` existiert bereits und wird nur im Identify-Pfad genutzt. Wird der Handle (und damit `ov_clear()`) auch in `open()`/`close()`/den Throw-Pfaden zur einzigen Aufräumquelle, erledigen sich doppelten Leak, Handle-Rest nach dem Kanalzahlen-Wurf (`:246`) und Exception-Pfad-Rest automatisch. Aufwand mittel, Risiko gering, Verhalten nach außen unverändert.
- **16-Bit-Lesepfade auf Frame-Zählung umstellen** (`:304-344`): aktuell werden Bytes (`rest`, `info.BytesPerSample`) und Frames (`num`, Rückgabe) gemischt; daraus entstehen der Überlauf-Befund und die falschen Mengenangaben bei Kettenwechsel. Eine Umstellung auf „angeforderte Frames zählen, `ov_read()` mit passender Bytegröße pro Frame" beseitigt beide Klassen. Aufwand mittel, Risiko: Verhaltensänderung bei Restbytes am Kettenende, deshalb nur mit Tests.
- **Kettenbehandlung offen legen** (`:174`, Lesepfade): entweder pro Kette dekodieren (`ov_streams()`, `ov_link()`, `ov_pcm_seek()`/`ov_raw_seek()`) oder bewusst nur Kette 0 plus dokumentierter Hinweis. Aktuell entsteht der Gesamteindruck `Samples = Summe aller Ketten`, während der Decode nach Kette 0 endet.
- **Kopfzeilen- und Kanalannahme aus `AudioInfo` entfernen**: alle vier Lesepfade entscheiden anhand von `info.Channels`, das aus Kette 0 stammt. Einmal pro Lesevorgang die Kanalzahl der aktiven Kette zu bestimmen (Fix im Heap-Befund) ist die kleine Variante; die große Variante wäre, `AudioInfo::Channels` bei verketteten Dateien als „Kette 0" zu kennzeichnen.
