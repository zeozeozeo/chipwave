#ifndef MIXER_H
#define MIXER_H

#include "node.h"
#include <SDL.h>
#include <vector>

#define SAMPLE_RATE 44100
class Mixer {
public:
	Mixer();
	~Mixer();
	void init_audio(int sample_rate);
	void close();
	int sample_rate() const { return m_sample_rate; }
	Uint64 sample_num() const { return m_sample_num; }
	bool did_init() const { return m_did_init; }
	void play();
	void pause();
	double next_sample();
	Node* add_node();
	Node* get_node(Uint64 index);
	Uint64 num_nodes() { return m_nodes.size(); }
	double get_time() { return m_time; }
private:
	Uint64 m_sample_num = 0;
	int m_sample_rate = 0;
	bool m_did_init = false;
	double m_time = 0.0;
	std::vector<Node> m_nodes;
};

#endif // MIXER_H
