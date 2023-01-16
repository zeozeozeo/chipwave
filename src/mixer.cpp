#include "mixer.h"

Mixer::Mixer() {

}

double Mixer::next_sample() {
	m_sample_num++;
	m_time = (double)m_sample_num / (double)m_sample_rate;
	double val = 0.0;

	// mix value of nodes
	for (auto& node : m_nodes)
		val += node.next_sample(m_time) / m_nodes.size();

	// clamp value between -1.0 and 1.0
	if (val > 1.0)
		val = 1.0;
	if (val < -1.0)
		val = -1.0;

	return val;
}

void audio_callback(void *user_data, Uint8 *raw_buffer, int bytes) {
	Mixer* mixer = (Mixer*)user_data;

	Sint16 *buffer = (Sint16*)raw_buffer;
	int length = bytes / 2; // 2 bytes per sample for AUDIO_S16SYS

	for(int i = 0; i < length; i++) {
		double val = mixer->next_sample();
		buffer[i] = (Sint16)(val * 0x7fff); // convert to signed short
	}
}

void Mixer::init_audio(int sample_rate) {
	SDL_AudioSpec want;
	m_sample_rate = sample_rate;
	want.freq = sample_rate; // sample rate
	want.format = AUDIO_S16SYS; // sample type (signed short)
	want.channels = 1; // channels amount
	want.samples = 2048; // buffer size
	want.callback = audio_callback;
	want.userdata = this;

	SDL_AudioSpec have;
	if (SDL_OpenAudio(&want, &have) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "failed to open audio: %s", SDL_GetError());
	}
	if (want.format != have.format) {
		SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "failed to get desired SDL_AudioSpec");
	}

	m_did_init = true;
}

void Mixer::play() {
	if (m_did_init)
		SDL_PauseAudio(0);
}

void Mixer::pause() {
	if (m_did_init)
		SDL_PauseAudio(1);
}

Mixer::~Mixer() {
	if (m_did_init)
		SDL_CloseAudio();
}

Node* Mixer::add_node() {
	Node node;
	m_nodes.push_back(node);
	return &m_nodes[m_nodes.size()-1];
}

Node* Mixer::get_node(Uint64 index) {
	if (index >= m_nodes.size())
		return nullptr;
	return &m_nodes[index];
}
