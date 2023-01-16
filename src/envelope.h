#ifndef ENVELOPE_H
#define ENVELOPE_H

#include <SDL.h>

class Envelope {
public:
	Envelope();
	double m_attack_time = 0.1;
	double m_decay_time = 0.01;
	double m_release_time = 0.2;
	double m_sustain_amplitude = 0.8;
	double m_start_amplitude = 1.0;
	double m_trigger_on_time = 0.0;
	double m_trigger_off_time = 0.0;
	bool m_note_on = false;

	double get_amplitude(double time);
	void note_on(double time);
	void note_off(double time);
};

#endif // ENVELOPE_H
