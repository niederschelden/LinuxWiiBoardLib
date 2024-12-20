# YAWiiBB - Yet Another Wii Balance Board Driver

YAWiiBB ist eine einfache C-Bibliothek, um das Wii Balance Board ohne Wii-Konsole zu verwenden, was ich speziell für physiotherapeutische Übungen tun möchte. Anfangs war ich ziemlich begeistert von Projekten wie [namacha's wiiboard](https://github.com/namacha/wiiboard) und [Pierrick Koch's wiiboard](https://github.com/PierrickKoch/wiiboard). Doch da `pyBluez` soweit ich es sehe nicht mehr weiterentwickelt wird, konnte ich keine Python-Lösung mit Python 3 betreiben.

Statt mich mit schweren C/C++-Treibern herumzuschlagen, die mir einfach zu komplex waren, habe ich meine eigene, leichte Bibliothek in C geschrieben, die genau meine Anforderungen erfüllt. YAWiiBB lässt sich als Python-Subprozess starten und liefert zuverlässig die Sensordaten des Balance Boards ohne überflüssige Komplexität.

Die Bibliothek ist unter der GNU General Public License lizenziert und steht in keinerlei Verbindung zu Nintendo. Ich entwickle das Projekt unabhängig und nutze lediglich die Hardware.

> **Wichtiger Hinweis:** 
> ## Zuerst lesen
> Bevor du dich in dieses Projekt stürzt, nimm dir bitte einen Moment Zeit, um [keldu/WiiWeight](https://github.com/keldu/WiiWeight) zu überprüfen. Der Autor hat eine deutlich effizientere Methode entwickelt, um Daten von der Wii Balance Board zu extrahieren.
>
> ### Warum du dies in Betracht ziehen solltest
>
> Die von **keldu** bereitgestellte Lösung nutzt den internen Bluetooth-Stack von Linux, um das notwendige Pairing und die Datenerfassung von der Wii Balance Board zu handhaben. Das bedeutet, dass du dich nicht mit der Low-Level-Bluetooth-Kommunikation oder den Komplexitäten beim Parsen von Rohdaten von der Plattform auseinandersetzen musst.
>
> ### Technische Details
>
> - **Bluetooth-Management**: Das Linux-Bluetooth-Subsystem bietet ein robustes Framework zur Verwaltung von Bluetooth-Geräten. Es kümmert sich nahtlos um die Geräteerkennung, das Pairing und die Verbindungsverwaltung, sodass du dich auf höherwertige Funktionalitäten konzentrieren kannst.
>
> - **Datenextraktion**: Anstatt die Rohdaten manuell vom Eingabegerät zu lesen, verwendet keldu's Implementierung die integrierte Bluetooth-Unterstützung in Linux. Dadurch wird sichergestellt, dass alle relevanten Daten von der Wii Balance Board effektiv erfasst werden, ohne dass umfangreicher zusätzlicher Code erforderlich ist.
>
> - **Benutzerfreundlichkeit**: Durch die Nutzung einer etablierten Bibliothek kannst du erheblich Entwicklungszeit sparen und die Komplexität deines Projekts reduzieren. Das bedeutet, dass du schnell mit der Implementierung von Funktionen fortfahren kannst, anstatt dich mit den Feinheiten der Gerätekommunikation zu beschäftigen.
>
> ### Fazit
>
> 	Wenn du nur Messwerte bei Zustandsänderungen benötigst, verwende am besten Keldus Ansatz, um das Rad nicht neu zu erfinden. Eine Implementierung von Keldus Methode findest du im Verzeichnis „testing“.
>   Falls du jedoch Zugriff auf Kalibrierungsdaten brauchst oder einen kontinuierlichen Messdatenstrom benötigst, wirf einen Blick auf meine Lösung, die speziell für diese Anwendungsfälle entwickelt wurde.


# Du wurdest gewarnt: Wenn du jetzt weiterliest, betrittst du die Welt des Kopfschmerzes.

## Funktionsübersicht

YAWiiBB kann:
- Das Balance Board über Bluetooth finden und verbinden
- Kommunikation und Datenübertragung via L2CAP
- Rohdaten der Sensoren lesen und Gewicht aus diesen Daten berechnen
- Kalibrierungsdaten verarbeiten
- Erweiterte Ausgabe und Debugging über das `YAWIIBB_EXTENDED`-Flag konfigurieren

Siehe auch [Doxygen Doku](https://niederschelden.github.io/YAWiiBBD/files.html)

### Hauptmerkmale

- **Flexibilität**: Erweiterter Debug-Modus lässt sich per `YAWIIBB_EXTENDED` aktivieren
- **Kalibrierung**: Nutzt und verarbeitet Kalibrierungsdaten für genauere Messungen
- **Mehrstufiges Logging**: Zusätzliche Log-Level (RAW, DECODE, DEBUG, VERBOSE) bei aktivem `YAWIIBB_EXTENDED`

## Installation und Voraussetzungen

### Voraussetzungen

- **Betriebssystem**: Linux mit Bluetooth-Unterstützung
- **Bibliotheken**: `bluetooth` und `bluetooth-dev` (z.B. für HCI- und L2CAP-Kommunikation)
- **Compiler**: Unterstützt `gcc` und `clang`



### Installieren Sie die erforderlichen Pakete unter Ubuntu/Debian:

sudo apt-get update
sudo apt-get install bluez libbluetooth-dev build-essential


## Installation

### Klonen Sie das Repository:

```bash

git clone https://github.com/niederschelden/YAWiiBBD

cd YAWiiBB
```
### Kompilieren Sie den Code:

```bash

gcc -Wall -o YAWiiBBD YAWiiBBD.c YAWiiBBessentials.c -lbluetooth
```
oder alternativ mit Erweiterungen:

```bash

gcc -Wall -o YAWiiBBD YAWiiBBD.c YAWiiBBessentials.c -lbluetooth -DYAWIIBB_EXTENDED
```
## Ausführen
Balance Board in pairing Modus setzen, noch aber nicht pairen.
```bash
./YAWiiBBD
```
Beenden mit Enter oder Druck auf den Hauptknopf.
Am Ende wird ein Hinweis ausgegeben, wie man die Boardsuche durch Eingabe der korrekten MAC adresse überspringt

## Byte-Zuordnungen im Datenstrom

### Byte-Positionen der Sensordaten (wenn byte[1] == 0x32)

| Sensorposition    | Bytes | Beschreibung                     |
|-------------------|-------|----------------------------------|
| Button            | 3     | 0x08 == gedrückt                 |
| Top-Right (TR)    | 4-5   | Rohdaten des Sensors oben rechts |
| Bottom-Right (BR) | 6-7   | Rohdaten des Sensors unten rechts|
| Top-Left (TL)     | 8-9   | Rohdaten des Sensors oben links  |
| Bottom-Left (BL)  | 10-11 | Rohdaten des Sensors unten links  |

### Byte-Positionen für Kalibrierungsdaten (wenn byte[1] == 0x21)

| Kalibrierungsstufe | Top-Right (TR) | Bottom-Right (BR) | Top-Left (TL) | Bottom-Left (BL) |
|------------------------------|----------------|--------------------|---------------|-------------------|
| 0 kg (erstes Datenpaket)     | 7-8            | 9-10              | 11-12         | 13-14            |
| 17 kg (erstes Datenpaket)    | 15-16          | 17-18             | 19-20         | 21-22            |
| 34 kg (zweites Datenpaket)   | 7-8            | 9-10              | 11-12         | 13-14            |

### Byte-Positionen für Kalibrierungsdaten (wenn byte[1] == 0x20)

Batteriestatus: Die Batteriestatuswerte sind in den Bytes 7(-8?) codiert und zur basis 200 (also wäre 100 = 50%).
LED-Zustand: Ist glaube ich, im Byte 4 und möglicherweise auch in Byte 5 codiert.

## Wii Balance Board Befehle

Die folgenden Kommandos werden zur Kommunikation mit dem Wii Balance Board verwendet. Diese Befehle wurden von der [WiiBrew-Website](https://wiibrew.org/wiki/Wii_Balance_Board#Wii_Initialisation_Sequence) sowie aus den oben genannten python scripten übernommen. Ich habe festgestellt, dass nicht alle Kommandos tatsächlich benötigt werden, um Daten vom Board zu erhalten. Einige Kommandos können in der `main`-Funktion deaktiviert werden, indem du ihre entsprechenden Flags auf `false` setzt (ausser LED, da ist die Logik umgekehrt: false bedeutet, dass es per led_on_command eingeschaltet wird):

```Markdown
WiiBalanceBoard board = {
    .needStatus = true,
    .needCalibration = true,
    .needActivation = true,
    .led = false,
    .needDumpStart = true,
    .is_running = true
};
```
### Befehle

Status-Befehl:
```Markdown
status_command[] = { 0x52, 0x12, 0x00, 0x32 };
```
Aktivierungsbefehl:
```Markdown
activate_command[] = { 0x52, 0x13, 0x04 };
```
Kalibrierungsbefehl:
```Markdown
calibration_command[] = { 0x52, 0x17, 0x04, 0xa4, 0x00, 0x24, 0x00, 0x18 };
```
LED ein-/aus-schalten:
```Markdown
led_on_command[] = { 0x52, 0x11, 0x10 };
```
Daten-Dump-Befehl:
```Markdown
data_dump_command[] = { 0x52, 0x15, 0x00, 0x32 };
```
### Hinweise

Die meisten dieser Kommandos werden nicht zwingend benötigt, um Daten zu empfangen. Sie wurden jedoch in den Code integriert, da der genaue Zweck einiger Befehle möglicherweise nicht vollständig bekannt ist. Es wird empfohlen, nur die Kommandos zu aktivieren, die für deine spezifischen Anforderungen notwendig sind.


### Anpassung des Log-Levels

Das Log-Level wird über LogLevel gesteuert. Standardmäßig ist RAW aktiviert; mit YAWIIBB_EXTENDED gibt es zusätzliche Level (DECODE, DEBUG, VERBOSE).

Um das Ausgabelevel anzupassen, setzen Sie die gewünschte Option in YAWiiBBD.c vor dem Kompilieren:

```c

#ifdef YAWIIBB_EXTENDED
const LogLevel debug_level = DEBUG; // Optionen: RAW, DECODE, DEBUG, VERBOSE
#else...
```
Durch das Kompilieren mit dem -DYAWIIBB_EXTENDED-Flag werden zusätzliche Log-Level aktiviert

## Lizenzen und Haftungsausschluss

Dieses Projekt steht unter der GNU General Public License v3.0. Weitere Details finden Sie in der LICENSE-Datei.

Haftungsausschluss: Diese Software wurde unabhängig entwickelt und steht in keinerlei Verbindung zu Nintendo oder dessen Tochtergesellschaften. Nintendo besitzt alle Rechte am Wii Balance Board, und die Software wird ohne jegliche Haftung angeboten. Die Verwendung erfolgt auf eigene Gefahr.

Viel Spaß mit YAWiiBB, und ich freue mich über Feedback oder Vorschläge! Bei Fragen einfach ein Issue im GitHub-Repository erstellen oder mich direkt kontaktieren.
