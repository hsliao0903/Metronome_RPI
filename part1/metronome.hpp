#pragma once

#include <cstddef>

class metronome
{
public:
	//sotre 4 sample beats
	enum { beat_samples = 4 };

public:
	metronome()
	: m_timing(false), m_beat_count(0) {
		// initial the beat samples all to 0
		for(int i = 0; i < beat_samples ; i++)
			m_beats[i] = 0;
	}
	~metronome() {}

public:
	// Call when entering "learn" mode
	void start_timing(){
		printf("Start timing\n");
		m_timing = true;
		firstTap = lastTap = 0;
		m_beat_count = 0;
	}
	// Call when leaving "learn" mode
	void stop_timing(){
		printf("Stop timing\n");
		m_timing = false;
		
		//at least 4 taps to get new BPM measurment
		if(m_beat_count >= 4) {
			for(int i = 0 ; i < beat_samples-1 ; i++) {
				m_beats[i] = m_beats[i+1];
			}			
			//calculate BPM?
			double seconds = ((double)(lastTap - firstTap) / (double) 1000);
			if(seconds <= 0){
				printf("something is wrong:lastTap=%u, firstTap=%u, seconds=%u\n",lastTap, firstTap, seconds);
				m_beats[beat_samples-1] = 0;
			} else
				m_beats[beat_samples-1] = size_t ((double) m_beat_count * ((double)60 /  seconds));
			printf("New BPM = %u\n", m_beats[beat_samples-1]);
		} else {
			printf("\nFailed to record new BPM!!\nAt lest four taps for new BPM measurment\n\n");
		}
	}

	// Should only record the current time when timing
	// Insert the time at the next free position of m_beats
	void tap(){
		size_t time_stamp = millis();
		if(firstTap == 0)
			firstTap = time_stamp;
		lastTap = time_stamp;
		m_beat_count++;

		//log for showing all the beat samples
		printf("\nbeat count: %d\n", m_beat_count);
		printf("m_beats=[");
		for(int j = 0 ; j < beat_samples ; j++)
			printf("%u, ", m_beats[j]);
		printf("]\n");
		
	}

	bool is_timing() const { return m_timing; }
	// Calculate the BPM from the deltas between m_beats
	// Return 0 if there are not enough samples
	size_t get_bpm() const {
		//get the latest BPM record
		printf("Current bpm = %u\n", m_beats[beat_samples-1]);
		return m_beats[beat_samples-1];
		
	}

	size_t getMIN_MAX(int mode) const {
		size_t ans;
		ans = m_beats[0];
		//mode 0 for get MIN, mode 1 for get MAX
		if (mode == 0) {
			for(int i = 1 ; i < beat_samples ; i++){
				if(m_beats[i] < ans) {
					ans = m_beats[i];
				}
			}
			return ans;
		} else {
			for(int i = 1 ; i < beat_samples ; i++){
				if(m_beats[i] > ans) {
					ans = m_beats[i];
				}
			}
			return ans;
		}
		return 0;
	}

	void delMIN_MAX(int mode) {
		size_t ans = getMIN_MAX(mode);
		for (int i = 0; i< beat_samples ; i++){
			if(m_beats[i] == ans)
				m_beats[i] = 0;
		}
	}

	//replace the new BPM with the oldest one
	void addNewBPM(size_t val) {
		for(int i = 0 ; i < beat_samples-1 ; i++) {
				m_beats[i] = m_beats[i+1];
		}
		m_beats[beat_samples-1] = val;
	}
	

private:
	bool m_timing;
	size_t firstTap;
	size_t lastTap;
	// Insert new samples at the end of the array, removing the oldest
	size_t m_beats[beat_samples];
	size_t m_beat_count;
};

