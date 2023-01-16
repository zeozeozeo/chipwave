#ifndef NODE_H
#define NODE_H

#include "voice.h"
#include <SDL.h>
#include <string>
#include <vector>

#define WAVEFORM_SIZE 256
#define WAVEFORM_SCALE 8

class Node {
public:
	Node();
	double next_sample(double time);
	void set_semitone(int semitone);
	Voice* add_voice(Voice voice);
	Voice* add_voice();
	int voices_amount() { return m_voices.size(); }
	void remove_voice(int index);
	Voice* get_voice(int index);
	void note_on(double time);
	void note_off(double time);
	void set_volume(double volume) { m_volume = volume; }

	double m_volume = 1.0;
	bool m_note_on = false;
	std::vector<Voice> m_voices;
	double m_freq = BASE_FREQUENCY;
	bool m_normalize_waveform = true; // UI
	Uint64 m_sample_num = 0;
	float m_last_samples[WAVEFORM_SIZE]; // UI
};

#endif // NODE_H
