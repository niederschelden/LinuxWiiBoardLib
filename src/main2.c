#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include "kalman.h"  // Einbindung der Kalman-Bibliothek
//compile: gcc main2.c kalman.c -o main2.out


volatile int running = 1;  // Steuerungsflag für Threads
pthread_mutex_t lock;  // Mutex zum Schutz der Variablen (nur für die Ausgabe)

// Strukturen zur Speicherung der Sensordaten
typedef struct {
    float raw_value;               // Rohwert vom Sensor
    float kalman_value;            // Kalman-geglätteter Wert
    int offset;                    // Offset für den Sensor
    float calibration_factor_pos;  // Kalibrierungsfaktor für positive Werte
    float calibration_factor_neg;  // Kalibrierungsfaktor für negative Werte
} SensorData;

SensorData sensor_data[3];  // Array zur Speicherung der Daten für 3 Werte
KalmanFilter kalman_filters[3];  // Kalman-Filter für 3 Sensorwerte

void handle_signal(int signal) {
    running = 0;  // Steuerungsflag setzen, um Schleifen zu beenden
}

void* output_thread(void* arg) {
    while (running) {
        // Zugriff auf die Variablen sichern
        pthread_mutex_lock(&lock);
        printf("Sensor 3: %-10f Sensor 4: %-10f Sensor 5: %-10f\r", 
               sensor_data[0].raw_value, sensor_data[1].raw_value, sensor_data[2].raw_value);
        //printf("Sensor 3: %-10f Sensor 4: %-10f Sensor 5: %-10f\r", 
        //       sensor_data[0].kalman_value, sensor_data[1].kalman_value, sensor_data[2].kalman_value);
        fflush(stdout);  // Sicherstellen, dass die Ausgabe sofort erfolgt
        pthread_mutex_unlock(&lock);
        usleep(100000);  // 0,1 Sekunden warten
    }
    return NULL;
}

float apply_calibration(SensorData *sensor, const int value) {
    int corrected_value = value + sensor->offset; // Offset abziehen
    float calibration_factor = (corrected_value >= 0) ? sensor->calibration_factor_pos : sensor->calibration_factor_neg;
    float calibrated_value = (float)corrected_value * calibration_factor; // Kalibrierung anwenden
    return calibrated_value; // Gibt den kalibrierten Wert zurück
}

int main(int argc, char **argv) {
    signal(SIGINT, handle_signal);
    setbuf(stdout, NULL);
    int fd, bytes, index;
    struct input_event ev;

    // Initialisierung der Sensordaten (Beispielwerte)
    sensor_data[0].offset = 8;
    sensor_data[0].calibration_factor_neg = 1.0f;
    sensor_data[0].calibration_factor_pos = 1.0f;
    sensor_data[1].offset = 9;
    sensor_data[1].calibration_factor_neg = 1.0f;
    sensor_data[1].calibration_factor_pos = 1.0f;
    sensor_data[2].offset = -91;
    sensor_data[2].calibration_factor_neg = 1.0f;
    sensor_data[2].calibration_factor_pos = 1.0f;


    // Mutex initialisieren
    pthread_mutex_init(&lock, NULL);

    // Initialisierung der Kalman-Filter
    for (int i = 0; i < 3; i++) {
        kalman_filter_init(&kalman_filters[i], 0.0f, 1.0f, 0.1f, 0.5f);
    }

    char device_path[] = "/dev/input/event7";
    fd = open(device_path, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    if (ioctl(fd, EVIOCGRAB, 1) < 0) {
        perror("Failed to get exclusive access to device");
        close(fd);
        return 1;
    }

    pthread_t output_thread_id;
    if (pthread_create(&output_thread_id, NULL, output_thread, NULL) != 0) {
        perror("Failed to create output thread");
        running = 0;
        close(fd);
        pthread_mutex_destroy(&lock);
        return 1;
    }

    // Hauptschleife
    while (running) {
        bytes = read(fd, &ev, sizeof(struct input_event));
        if (bytes < (int)sizeof(struct input_event)) {
            perror("Failed to read input event");
            running = 0; // Beendet das Laufen bei Lesefehler
            break;
        }

        index = ev.code - 3;
        if (index >= 0 && index < 3) {
            // Kein Locking hier erforderlich, da Hauptschleife exklusiv auf die Daten zugreift
            sensor_data[index].raw_value = apply_calibration(&sensor_data[index], ev.value);
            // Kalman-Filterung
            sensor_data[index].kalman_value = kalman_filter_update(&kalman_filters[index], sensor_data[index].raw_value);
        }
    }

    // Thread beenden
    running = 0;
    pthread_join(output_thread_id, NULL);

    // Mutex zerstören
    pthread_mutex_destroy(&lock);

    ioctl(fd, EVIOCGRAB, 0);
    close(fd);
    printf("\nOrdentlich beendet\n");
    return 0;
}
