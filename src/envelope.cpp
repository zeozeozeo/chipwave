#include "envelope.h"

Envelope::Envelope() {
}

double Envelope::get_amplitude(double time) {
	double amplitude = 0.0;
	double lifetime = time - m_trigger_on_time;

	if (m_note_on) {
		// attack, decay, sustain
		if (lifetime <= m_attack_time)
			amplitude = (lifetime / m_attack_time) * m_start_amplitude;

		if (lifetime > m_attack_time && lifetime <= (m_attack_time + m_decay_time))
			amplitude = ((lifetime - m_attack_time) / m_decay_time) * (m_sustain_amplitude - m_start_amplitude) + m_start_amplitude;

		if (lifetime > (m_attack_time + m_decay_time))
			amplitude = m_sustain_amplitude;
	} else {
		// release
		amplitude = ((time - m_trigger_off_time) / m_release_time) * (0.0 - m_sustain_amplitude) + m_sustain_amplitude;
	}

	// filter garbage from the envelope, we won't hear it anyway
	// and it might cause some issues
	if (amplitude <= 0.0001)
		amplitude = 0.0;

	return amplitude;
}

void Envelope::note_on(double time) {
	m_trigger_on_time = time;
	m_note_on = true;
}

void Envelope::note_off(double time) {
	m_trigger_off_time = time;
	m_note_on = false;
}
