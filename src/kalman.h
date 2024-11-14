#ifndef KALMAN_H
#define KALMAN_H

typedef struct {
    float state_estimate;          // Zustandsschätzung
    float estimate_uncertainty;    // Unsicherheit der Schätzung
    float process_noise;           // Prozessrauschen
    float measurement_noise;       // Messrauschen
} KalmanFilter;

/**
 * @brief Initialisiert den Kalman-Filter mit den angegebenen Parametern.
 *
 * Diese Funktion setzt die anfängliche Zustandsschätzung, die Unsicherheit und
 * die Rauschparameter für den Kalman-Filter.
 *
 * @param kf Pointer auf die KalmanFilter-Struktur, die den Zustand des Filters speichert.
 * @param initial_estimate Die anfängliche Schätzung des Zustands.
 * @param initial_uncertainty Die anfängliche Unsicherheit der Schätzung.
 * @param process_noise Der Prozessrauschwert, der angibt, wie viel Unsicherheit durch das Systemmodell eingebracht wird.
 * @param measurement_noise Der Messrauschwert, der die Unsicherheit der Sensoren beschreibt.
 */
void kalman_filter_init(KalmanFilter *kf, float initial_estimate, float initial_uncertainty, float process_noise, float measurement_noise);

/**
 * @brief Aktualisiert den Kalman-Filter mit einer neuen Messung.
 *
 * Diese Funktion führt eine Schätzung basierend auf der neuen Messung durch,
 * glättet den Wert und aktualisiert die Zustands- und Unsicherheitswerte im KalmanFilter.
 *
 * @param kf Pointer auf die KalmanFilter-Struktur, die den Zustand des Filters speichert.
 * @param measurement Die neue Messung, die durch den Kalman-Filter verarbeitet werden soll.
 * @return Der geglättete Schätzwert nach Anwendung des Kalman-Filters.
 */
float kalman_filter_update(KalmanFilter *kf, float measurement);


#endif // KALMAN_H
