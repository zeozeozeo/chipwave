#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include "mixer.h"
#include <stdlib.h>
#include <time.h>
#include <sstream>

void show_tooltip(const char* text) {
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("%s", text);
}

void show_voice(Voice* voice, Node* node, int voice_idx) {
	if (ImGui::TreeNode("General Settings")) {
		// waveform type combo box
		std::string waveform_str = waveform_to_string(voice->m_waveform);
		if (ImGui::BeginCombo("Waveform Type", waveform_str.c_str())) {
			for (int i = 0; i < IM_ARRAYSIZE(ALL_WAVEFORMS); i++) {
				Waveform waveform = ALL_WAVEFORMS[i];
				bool is_selected = waveform == voice->m_waveform;
				waveform_str = waveform_to_string(waveform);

				if (ImGui::Selectable(waveform_str.c_str(), is_selected))
					voice->m_waveform = ALL_WAVEFORMS[i];
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::TreePop();
	}

	// envelopes
	double temp_min = 0.0;
	double temp_max = DBL_MAX;

	if (ImGui::TreeNode("Envelope Settings")) {
		// this is ugly, ugh
		// and yes, i copied it from wikipedia :^)
		Envelope* env = &voice->m_envelope;
		ImGui::DragScalar("Attack", ImGuiDataType_Double, &env->m_attack_time, 0.001, &temp_min, &temp_max, "%.3f");
		show_tooltip("The time taken for initial run-up of amplitude from zero to peak, beginning when the key is pressed");
		ImGui::DragScalar("Decay", ImGuiDataType_Double, &env->m_decay_time, 0.001, &temp_min, &temp_max, "%.3f");
		show_tooltip("The time taken for the subsequent run down from the attack amplitude to the designated sustain amplitude");
		ImGui::DragScalar("Release", ImGuiDataType_Double, &env->m_release_time, 0.001, &temp_min, &temp_max, "%.3f");
		show_tooltip("The time taken for the amplitude to decay from the sustain amplitude to zero after the key is released");
		ImGui::DragScalar("Sustain Amplitude", ImGuiDataType_Double, &env->m_sustain_amplitude, 0.001, &temp_min, &temp_max, "%.3f");
		show_tooltip("The amplitude during the main sequence of the sound's duration, until the key is released");
		ImGui::DragScalar("Start Amplitude", ImGuiDataType_Double, &env->m_start_amplitude, 0.001, &temp_min, &temp_max, "%.3f");
		show_tooltip("The amplitude at the start of the attack sequence");
		ImGui::TreePop();
	}

	// lfo
	if (ImGui::TreeNode("LFO Settings")) {
		ImGui::DragScalar(
					"LFO Frequency",
					ImGuiDataType_Double,
					&voice->m_lfo_hz,
					0.001, 0, 0, "%.3f");
		ImGui::DragScalar(
					"LFO Amplitude",
					ImGuiDataType_Double,
					&voice->m_lfo_amplitude,
					0.001, 0, 0, "%.3f");
		ImGui::TreePop();
	}

	if (ImGui::Button("Remove Voice"))
		node->remove_voice(voice_idx);
	show_tooltip("Removes this voice (no confirmation and you can't restore it!)");
}

void show_node_editor(Node *node, Mixer* mixer, double time) {
	ImGui::SetNextWindowSize(ImVec2(700, 500), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Node Editor")) {
		// waveform
		ImVec2 max = ImGui::GetContentRegionAvail();
		ImGui::PlotLines(
					"##waveform",
					node->m_last_samples,
					WAVEFORM_SIZE,
					1, // skip the first sample
					nullptr,
					node->m_normalize_waveform ? FLT_MAX : -1.0,
					node->m_normalize_waveform ? FLT_MAX : 1.0,
					ImVec2(max.x, 100));

		ImGui::Checkbox("Normalize Waveform", &node->m_normalize_waveform);
		show_tooltip("Normalizes the waveform (-1.0..1.0)");

		ImGui::Separator();

		// trigger/release button
		if (node->m_note_on) {
			if (ImGui::Button("Release##notebutton"))
				node->note_off(time);
		} else {
			if (ImGui::Button("Trigger##notebutton"))
				node->note_on(time);
		}

		show_tooltip("Triggers or releases the note. You can also use the keys on your keyboard");

		// note frequency
		ImGui::SameLine();
		ImGui::Text("%fhz", node->m_freq);

		// volume slider
		double temp_min = 0.0;
		double temp_max = 1.0;
		ImGui::SliderScalar(
					"Volume",
					ImGuiDataType_Double,
					&node->m_volume,
					&temp_min, &temp_max,
					"%.4f");
		show_tooltip("Volume level between -1.0 and 1.0");
		ImGui::Separator();

		// voices
		int idx = 0;
		if (ImGui::CollapsingHeader("Voices")) {
			for (auto& voice : node->m_voices) {

				std::ostringstream treenode_str;
				treenode_str << "Voice " << idx + 1;

				// display the voice
				if (ImGui::TreeNode(treenode_str.str().c_str())) {
					show_voice(&voice, node, idx);
					ImGui::TreePop();
				}

				idx++;
			}

			// add new voice button
			if (ImGui::Button("Add Voice"))
				node->add_voice();
		}
	}
	ImGui::End();
}

int main() {
	// setup SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "failed to initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	srand(time(nullptr)); // seed PRNG

	// setup audio
	Mixer mixer;
	mixer.init_audio(SAMPLE_RATE);

	// setup window
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_Window* window = SDL_CreateWindow(
				"chipwave",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				1280, 720,
				SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1); // enable vsync

	// setup imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = nullptr; // prevent imgui from creating imgui.ini
	ImGui::StyleColorsDark();

	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL2_Init();

	// main loop
	bool running = true;
	mixer.play();

	Node* node = mixer.add_node();
	node->add_voice();
	// voice->set_lfo(5.0, 0.01);
	node->set_volume(0.1);

	while (running) {
		// process SDL events
		SDL_Event event;
		double time = mixer.get_time();

		while (SDL_PollEvent(&event)) {
			ImGui_ImplSDL2_ProcessEvent(&event);

			if (event.type == SDL_QUIT) {
				running = false;
				break;
			}
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
				running = false;
				break;
			}

			char* key = nullptr;
			bool is_pressed;
			bool can_input = !io.WantCaptureKeyboard;

			switch (event.type) {
			case SDL_KEYUP:
				if (event.key.repeat || !can_input)
					break;
				key = (char*)SDL_GetKeyName(event.key.keysym.sym);
				is_pressed = false;
				break;
			case SDL_KEYDOWN:
				if (event.key.repeat || !can_input)
					break;
				key = (char*)SDL_GetKeyName(event.key.keysym.sym);
				is_pressed = true;
				break;
			}

			if (key == nullptr)
				continue;

			for (int k = 0; k < 15; k++) {
				// keymap
				if (*key == "ZSXCFVGBNJMK,L./"[k]) {
					node->set_semitone(k);

					if (is_pressed)
						node->note_on(time);
					else
						node->note_off(time);
					break;
				}
			}
		}

		// start a new imgui frame
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		// draw ui
		show_node_editor(node, &mixer, time);

		// render
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(0, 0, 0, 255);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(window);
	}

	// cleanup
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
