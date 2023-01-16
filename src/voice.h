#ifndef VOICE_H
#define VOICE_H

#include "envelope.h"
#include <string>

#define BASE_FREQUENCY 110.0 // A2
#define TWTH_ROOT_OF_TWO 1.0594630943592952646 // pow(2.0, 1.0 / 12.0)
#define HZ_TO_VEL(hz) hz * 2.0 * M_PI

enum Waveform {
	SINE,
	SQUARE,
	TRIANGLE,
	SAW,
	NOISE,
};

std::string waveform_to_string(Waveform waveform);
const Waveform ALL_WAVEFORMS[] = { SINE, SQUARE, TRIANGLE, SAW, NOISE };

class Voice {
public:
	Voice();
	Voice(Waveform waveform);
	double waveform_value(double time, double base_freq);
	double next_sample(double time, double base_freq);
	void set_lfo(double freq, double amplitude) { m_lfo_hz = freq; m_lfo_amplitude = amplitude; }

	double m_freq_offset = 0.0;
	double m_lfo_hz = 0.0;
	double m_lfo_amplitude = 0.0;
	Envelope m_envelope;
	Waveform m_waveform = SINE;
};

#endif // VOICE_H
