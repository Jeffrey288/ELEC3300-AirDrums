
#ifndef __kalman_h_
#define __kalman_h_
// stolen from https://github.com/arezaie14/ars_kalman_filter

typedef struct {
	float _err_measure;
	float _err_estimate;
	float _q;
	float _current_estimate;
	float _last_estimate;
	float _kalman_gain;
} Kalman;

#define kalman(mea_e, q) (Kalman) { ._err_measure = mea_e, ._err_estimate = mea_e, ._q = q, ._current_estimate = 0, ._last_estimate = 0};

static float kalmanUpdate(Kalman *k, float mea) {
	k->_kalman_gain = k->_err_estimate / (k->_err_estimate + k->_err_measure);
	k->_current_estimate = k->_last_estimate + k->_kalman_gain * (mea - k->_last_estimate);
	k->_err_estimate = (1.0 - k->_kalman_gain) * k->_err_estimate
			+ fabs(k->_last_estimate - k->_current_estimate) * k->_q;
	k->_last_estimate = k->_current_estimate;

	return k->_current_estimate;
}

#endif
