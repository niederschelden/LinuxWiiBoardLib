#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/hidraw.h>
#include <sys/ioctl.h>
#include <libudev.h>
//compile: gcc -o main.out main.c -ludev

#define MAX_DEVICES 100 // Maximale Anzahl der Geräte

typedef struct {
    int id;                        // Laufende Nummer
    char event_path[256];          // Event-Dateipfad
    char hidraw_path[256];         // HIDRAW-Dateipfad
    char name[256];                // Name des Geräts
} DeviceInfo;

// Vorwärtsdeklarationen der Funktionen
int list_event_devices(DeviceInfo *devices);
int list_hidraw_devices(DeviceInfo *devices);
int select_device(DeviceInfo* selected_device);

/**
 * @brief Auflistung der 'event'-Geräte.
 *
 * Diese Funktion durchsucht das 'input'-Subsystem nach 'event'-Geräten und speichert
 * die Informationen in einem statischen Array.
 *
 * @param[out] devices Ein statisches Array zur Speicherung der Geräteinformationen.
 * @return Die Anzahl der gefundenen Geräte.
 */
int list_event_devices(DeviceInfo *devices) {
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices_list, *entry;
    int count = 0;

    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Fehler bei udev-Erstellung\n");
        return 0;
    }

    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "input");
    udev_enumerate_scan_devices(enumerate);
    devices_list = udev_enumerate_get_list_entry(enumerate);

    udev_list_entry_foreach(entry, devices_list) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *device = udev_device_new_from_syspath(udev, path);
        const char *devnode = udev_device_get_devnode(device);

        if (devnode && strstr(devnode, "event") && count < MAX_DEVICES) {
            strncpy(devices[count].event_path, devnode, sizeof(devices[count].event_path) - 1);

            struct udev_device *parent = udev_device_get_parent_with_subsystem_devtype(
                device, "input", NULL);
            if (parent) {
                snprintf(devices[count].name, sizeof(devices[count].name), "%s",
                         udev_device_get_sysattr_value(parent, "name"));
            } else {
                snprintf(devices[count].name, sizeof(devices[count].name), "Unbekanntes Gerät");
            }

            devices[count].id = count + 1;
            count++;
        }

        udev_device_unref(device);
    }

    udev_enumerate_unref(enumerate);
    udev_unref(udev);

    return count;
}

/**
 * @brief Auflistung der 'hidraw'-Geräte durch direkten Zugriff auf /dev/.
 *
 * Diese Funktion durchsucht das Verzeichnis '/dev/' nach 'hidraw'-Geräten und verwendet
 * ioctl-Aufrufe, um den Namen des Geräts abzurufen.
 *
 * @param[out] devices Ein statisches Array zur Speicherung der Geräteinformationen.
 * @return Die Anzahl der gefundenen 'hidraw'-Geräte.
 */
int list_hidraw_devices(DeviceInfo *devices) {
    struct dirent *entry;
    DIR *dp = opendir("/dev/");
    int count = 0;

    if (dp == NULL) {
        perror("Fehler beim Öffnen von /dev/");
        return 0;
    }

    while ((entry = readdir(dp))) {
        if (strncmp(entry->d_name, "hidraw", 6) == 0 && count < MAX_DEVICES) {
            char device_path[256];
            snprintf(device_path, sizeof(device_path), "/dev/%s", entry->d_name);

            // Öffnen des Geräts
            int fd = open(device_path, O_RDONLY | O_NONBLOCK);
            if (fd < 0) {
                perror("Fehler beim Öffnen des Geräts");
                continue;
            }

            // Gerätenamen über ioctl abfragen
            char device_name[256] = "Unbekannt";
            if (ioctl(fd, HIDIOCGRAWNAME(sizeof(device_name)), device_name) < 0) {
                perror("Fehler beim Abrufen des Gerätenamens");
                strncpy(device_name, "Unbekannt", sizeof(device_name) - 1);
            }

            // Gerätedaten in die Struktur speichern
            devices[count].id = count + 1;
            strncpy(devices[count].hidraw_path, device_path, sizeof(devices[count].hidraw_path) - 1);
            strncpy(devices[count].name, device_name, sizeof(devices[count].name) - 1);

            close(fd);
            count++;
        }
    }

    closedir(dp);
    return count;
}


/**
 * @brief Wrapper-Funktion zur Auswahl von 'event'- und 'hidraw'-Geräten.
 *
 * Diese Funktion listet die verfügbaren 'event'- und 'hidraw'-Geräte auf, ermöglicht
 * die Auswahl durch den Benutzer und speichert die ausgewählten Geräteinformationen
 * in der übergebenen Zielstruktur.
 *
 * @param[out] selected_device Ein Zeiger auf die Zielstruktur zur Speicherung der ausgewählten Geräteinformationen.
 * @return 0 bei erfolgreicher Auswahl, 1 bei ungültiger Auswahl.
 */
int select_device(DeviceInfo* selected_device) {
    DeviceInfo event_devices[MAX_DEVICES] = {0};
    DeviceInfo hidraw_devices[MAX_DEVICES] = {0};
    int event_count = list_event_devices(event_devices);
    int hidraw_count = list_hidraw_devices(hidraw_devices);

    if (event_count == 0) {
        printf("Keine 'event'-Geräte gefunden.\n");
        return 0;
    }

    // Ausgabe der 'event'-Geräte
    printf("Liste der 'event'-Geräte:\n");
    for (int i = 0; i < event_count; i++) {
        printf("ID: %d, Name: %s, Event Path: %s\n", event_devices[i].id,
               event_devices[i].name, event_devices[i].event_path);
    }

    // Benutzerwahl eines 'event'-Geräts
    printf("\nBitte geben Sie die ID des 'event'-Geräts ein, das Sie auswählen möchten (0 zum Beenden): ");
    int event_selection;
    if (scanf("%d", &event_selection) != 1 || event_selection < 0 || event_selection > event_count) {
        fprintf(stderr, "Ungültige Auswahl.\n");
        return 1;
    }

    if (event_selection == 0) {
        printf("Programm beendet.\n");
        return 0;
    }

    // Kopieren der Auswahl in die Zielstruktur
    *selected_device = event_devices[event_selection - 1];

    // Ausgabe der 'hidraw'-Geräte
    if (hidraw_count > 0) {
        printf("\nListe der 'hidraw'-Geräte:\n");
        for (int i = 0; i < hidraw_count; i++) {
            printf("ID: %d, Name: %s, HIDRAW Path: %s\n", hidraw_devices[i].id,
                   hidraw_devices[i].name, hidraw_devices[i].hidraw_path);
        }

        // Benutzerwahl eines 'hidraw'-Geräts zur Zuordnung
        printf("\nBitte geben Sie die ID des 'hidraw'-Geräts ein, das Sie zuordnen möchten (0 zum Beenden): ");
        int hidraw_selection;
        if (scanf("%d", &hidraw_selection) != 1 || hidraw_selection < 0 || hidraw_selection > hidraw_count) {
            fprintf(stderr, "Ungültige Auswahl.\n");
            return 1;
        }

        if (hidraw_selection == 0) {
            printf("Programm beendet.\n");
            return 0;
        }

        // Zuordnung des HIDRAW-Pfads zum ausgewählten 'event'-Gerät
        strncpy(selected_device->hidraw_path, hidraw_devices[hidraw_selection - 1].hidraw_path, sizeof(selected_device->hidraw_path) - 1);
    } else {
        printf("Keine 'hidraw'-Geräte gefunden.\n");
    }

    return 0;
}

int main() {
    DeviceInfo selected_device = {0};
    select_device(&selected_device);
    // Anzeige der zusammengeführten Gerätedaten
    printf("\nSie haben folgendes Gerät ausgewählt und zugeordnet:\n");
    printf("Name: %s\n", selected_device.name);
    printf("Event Path: %s\n", selected_device.event_path);
    printf("HIDRAW Path: %s\n", selected_device.hidraw_path);

    return 0;
}