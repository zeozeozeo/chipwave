#include "node.h"
#include <stdlib.h>

Node::Node() {

}

double Node::next_sample(double time) {
	// mix all voices
	m_sample_num++;
	double val = 0.0;

	for (auto& voice : m_voices)
		val += voice.next_sample(time, m_freq) / m_voices.size();

	val *= m_volume;

	if (val > 1.0)
		val = 1.0;
	if (val < -1.0)
		val = -1.0;

	if (m_sample_num % WAVEFORM_SCALE == 0)
		m_last_samples[(m_sample_num / WAVEFORM_SCALE) % WAVEFORM_SIZE] = val;

	return val;
}

void Node::set_semitone(int semitone) {
	m_freq = BASE_FREQUENCY * std::pow(TWTH_ROOT_OF_TWO, semitone);
}

Voice* Node::add_voice(Voice voice) {
	m_voices.push_back(voice);
	return get_voice(m_voices.size() - 1);
}

Voice* Node::add_voice() {
	Voice voice;
	m_voices.push_back(voice);
	return get_voice(m_voices.size() - 1);
}

void Node::remove_voice(int index) {
	if (index > voices_amount() - 1 || index < 0)
		return;
	m_voices.erase(m_voices.begin() + index);
}

Voice* Node::get_voice(int index) {
	return &m_voices.at(index);
}

void Node::note_on(double time) {
	for (auto& voice : m_voices)
		voice.m_envelope.note_on(time);
	m_note_on = true;
}

void Node::note_off(double time) {
	for (auto& voice : m_voices)
		voice.m_envelope.note_off(time);
	m_note_on = false;
}
