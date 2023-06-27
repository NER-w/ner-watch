#include "heart.h"

#include <zephyr/drivers/sensor.h>

int lowpass_fir_filter(int sample) {
    static int cbuf[32];
    static uint8_t offset = 0;
    static const uint16_t fir_coeffs[12] = {172, 321, 579, 927, 1360, 1858, 2390, 2916, 3391, 3768, 4012, 4096};
    cbuf[offset] = sample;

    int z = (long) fir_coeffs[11] * (long) cbuf[(offset - 11) & 0x1f];

    for (uint8_t i = 0; i < 11; i++) {
        z += (long) fir_coeffs[i] * (long) (cbuf[(offset - i) & 0x1f] + cbuf[(offset - 22 + i) & 0x1f]);        
    }

    offset++;
    offset %= 32;

    return z >> 15;    
}

int average_dc_estimator(int *p, uint16_t x) {
    *p += ((((long) x << 15) - *p) >> 4);
    return (*p >> 15);
}

void get_bpm(struct k_work *work) {
    struct heart_work *heart_work =
        CONTAINER_OF(work, struct heart_work, work);

    uint64_t prev_beat, cur_beat;
    uint64_t beat_ms = 0;
    size_t beat_count = 0;

    int ir_ac_max = 20;
    int ir_ac_min = -20;

    int ir_ac_signal_cur = 0;
    int ir_ac_signal_prev;
    int ir_ac_signal_min = 0;
    int ir_ac_signal_max = 0;
    int ir_avg_estim;

    bool positive_edge = false;
    bool negative_edge = false;
    int ir_avg_reg = 0;

    bool beat = false;

	struct sensor_value sample;
    
    prev_beat = k_uptime_get();
	while (1) {
		sensor_sample_fetch(heart_dev);
		sensor_channel_get(heart_dev, SENSOR_CHAN_IR, &sample);

        ir_ac_signal_prev = ir_ac_signal_cur;

        ir_avg_estim = average_dc_estimator(&ir_avg_reg, sample.val1);
        ir_ac_signal_cur = lowpass_fir_filter(sample.val1 - ir_avg_estim);

        if ((ir_ac_signal_prev < 0) && (ir_ac_signal_cur >= 0)) {
            ir_ac_max = ir_ac_signal_max;
            ir_ac_min = ir_ac_signal_min;

            positive_edge = true;
            negative_edge = false;
            ir_ac_signal_max = 0;

            if ((ir_ac_max - ir_ac_min) > 20 && (ir_ac_max - ir_ac_min) < 1000) {
                beat = true;
            }
        } else if ((ir_ac_signal_prev > 0) && (ir_ac_signal_cur <= 0)) {
            positive_edge = false;
            negative_edge = true;
            ir_ac_signal_min = 0;
        }

        if (positive_edge && (ir_ac_signal_cur > ir_ac_signal_prev)) {
            ir_ac_signal_max = ir_ac_signal_cur;
        } else if (negative_edge && (ir_ac_signal_cur < ir_ac_signal_prev)) {
            ir_ac_signal_min = ir_ac_signal_cur;
        }

        if (beat) {
            cur_beat = k_uptime_get();
            beat_ms += cur_beat - prev_beat;
            beat_count++;
            prev_beat = cur_beat;
        }
        beat = false;

        if (beat_count == 5) {
            k_sem_give(&read_bpm_finished);
            heart_work->bpm = 60000UL * beat_count / beat_ms;
            return;
        }

        k_sleep(K_MSEC(10));
    }
        
}
