#include "kalman.h"

void kalman_filter_init(KalmanFilter *kf, float initial_estimate, float initial_uncertainty, float process_noise, float measurement_noise) {
    kf->state_estimate = initial_estimate;
    kf->estimate_uncertainty = initial_uncertainty;
    kf->process_noise = process_noise;
    kf->measurement_noise = measurement_noise;
}

float kalman_filter_update(KalmanFilter *kf, float measurement) {
    // Kalman-Gain berechnen
    float kalman_gain = kf->estimate_uncertainty / (kf->estimate_uncertainty + kf->measurement_noise);

    // Zustandsschätzung aktualisieren
    kf->state_estimate = kf->state_estimate + kalman_gain * (measurement - kf->state_estimate);

    // Unsicherheit aktualisieren
    kf->estimate_uncertainty = (1 - kalman_gain) * kf->estimate_uncertainty + kf->process_noise;

    // Rückgabe der neuen Schätzung
    return kf->state_estimate;
}
