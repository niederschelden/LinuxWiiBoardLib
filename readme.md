# README
## LinuxWiiBoardLib

LinuxWiiBoardLib baut auf [YAWiiBBD](https://github.com/niederschelden/YAWiiBBD) auf und nutzt die Datenverarbeitung von [WiiWeight](https://github.com/keldu/WiiWeight). Ziel ist es, ein gepairtes Wii Balance Board über `event`- und `hidraw`-Dateien direkt anzusprechen, ohne selbst Hand an die L2CAP-Kommunikation zu legen.

## Implementierungsstatus

### Gerätesuche
- Suche nach `event`-Dateien über das `input`-Subsystem mittels `udev`.
- Danach werden `hidraw`-Dateien im `/dev/`-Verzeichnis ermittelt und per `ioctl` weitere Infos geholt.
- Momentan müssen `event`- und `hidraw`-Dateien manuell zugeordnet werden.


## Nächste Schritte
- Zusammenführen der beiden Quellcodes main.c und main2.c
- Automatische Zuordnung von `hidraw`- und `event`-Dateien für das gleiche Gerät.

## Module
main.c: Auswahl von event- und hidraw-Devices aus /dev/input/event[x] und /dev/hidraw[x].
main2.c: Kontinuierliches Auslesen und Verarbeitung der Datenströme.

## Verwendung

Zum Kompilieren:
```bash
gcc -o teil1 main.c -ludev

gcc -o teil2 main2.c kalman.c 
```

>**Hinweis:** Zugriff auf `hidraw`-Dateien erfordert Root-Rechte. Geplante Anpassungen über `udev`-Regeln, um dies zu umgehen. Will sagen: 

```bash
sudo ./teil1

./teil2 
```