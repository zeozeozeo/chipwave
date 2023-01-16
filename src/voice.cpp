#include "voice.h"

std::string waveform_to_string(Waveform waveform) {
	switch (waveform) {
	case SINE:
		return "Sine";
	case SQUARE:
		return "Square";
	case NOISE:
		return "Noise";
	case TRIANGLE:
		return "Triangle";
	case SAW:
		return "Saw";
	default:
		return "Invalid";
	}
}

Voice::Voice(Waveform waveform) {
	m_waveform = waveform;
}

Voice::Voice() {

}

double Voice::waveform_value(double time, double base_freq) {
	double freq = HZ_TO_VEL(base_freq) * time + m_lfo_amplitude * base_freq * std::sin(HZ_TO_VEL(m_lfo_hz) * time);

	switch (m_waveform) {
	case SINE:
		return std::sin(freq);
	case SQUARE:
		return std::sin(freq) > 0.0 ? 1.0 : -1.0;
	case TRIANGLE:
		return std::asin(std::sin(freq)) * M_2_PI;
	case SAW: // TODO: add LFO to this
		return M_2_PI * (base_freq * M_PI * std::fmod(time, 1.0 / base_freq) - M_PI_2);
	case NOISE:
		return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;
	default:
		return 0.0;
	}
}

double Voice::next_sample(double time, double base_freq) {
	double waveform_val = this->waveform_value(time, base_freq + m_freq_offset);
	return (waveform_val * m_envelope.get_amplitude(time));
}
