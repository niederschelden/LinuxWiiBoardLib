# README
## Projektübersicht für LinuxWiiBoardLib

LinuxWiiBoardLib ging aus (YAWiiBBD)[https://github.com/niederschelden/YAWiiBBD] hervor, integriert aber die Funktionalität zur Datenempfangsverarbeitung aus (WiiWeight)[https://github.com/keldu/WiiWeight]. Ziel ist es, die Kommunikation mit einem bereits gepairten Wii Balance Board über event- und hidraw-Dateien zu ermöglichen, um eine direkte Interaktion zu schaffen und die umständlichere L2CAP-Kommunikation aus YAWiiBBD zu vermeiden.

## Aktueller Status der Implementierung
### Gerätesuche:
Initiale Suche nach event-Dateien über das input-Subsystem mittels udev.
Im zweiten Schritt werden hidraw-Dateien durch Scannen des /dev/-Verzeichnisses ermittelt und über ioctl-Aufrufe zusätzliche Informationen abgerufen.
Die aktuelle Implementierung setzt eine manuelle Zuordnung durch den Benutzer voraus, bei der event- und hidraw-Dateien entsprechend ihrer Funktionalität gekoppelt werden.

>** Root-Rechte für hidraw: **
Der Zugriff auf hidraw-Dateien erfordert derzeit Root-Rechte. Dieses Sicherheitsproblem ist bekannt und wird in späteren Iterationen angepasst (z.B. durch Anpassungen der Berechtigungen oder den Einsatz von udev-Regeln).

## Nächste Schritte

### Multithreading zur kontinuierlichen Ereignisverarbeitung:
Ein separater Thread wird implementiert, der fortlaufend event-Nachrichten verarbeitet und ausgibt.
Der Hauptthread bleibt blockierend auf Benutzereingaben fokussiert, um diese an die hidraw-Datei weiterzugeben.

## Verwendung
```bash
gcc -o testprogramm main.c -ludev

```
