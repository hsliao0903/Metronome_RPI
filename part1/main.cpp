#include <chrono>
#include <thread>
#include <string>
using namespace std::chrono_literals;

#include <cpprest/http_msg.h>
#include <wiringPi.h>

#include "metronome.hpp"
#include "rest.hpp"

// ** Remember to update these numbers to your personal setup. **
#define LED_RED   17
#define LED_GREEN 27
#define BTN_MODE  23
#define BTN_TAP   24

#define MIN 0
#define MAX 1

// Mark as volatile to ensure it works multi-threaded.
volatile bool btn_mode_pressed = false;
volatile bool is_startup = true;
size_t blink_frequency;
metronome metro;

// Run an additional loop separate from the main one.
void blink() {
	bool on = false;
	
	// ** This loop manages LED blinking. **
	while (true) {
		// The LED state will toggle once every second.
		std::this_thread::sleep_for(std::chrono::milliseconds(blink_frequency));
		

		// Perform the blink if we are pressed,
		// otherwise just set it to off.
		if (!metro.is_timing() && !is_startup){
			digitalWrite(LED_GREEN,HIGH);
			delay(10);
			digitalWrite(LED_GREEN,LOW);
		}
	}
}

static void check_tap_button () {
	digitalWrite(LED_RED,HIGH);
	delay(10);
	digitalWrite(LED_RED,LOW);

}

//fix CORS problem
void msg_reply(web::http::http_request msg, size_t content, int mode) {
	web::http::http_response response(200);
	response.headers().add("Access-Control-Allow-Origin", "*");
	if(mode == 1)
		response.set_body(web::json::value::number(content));
	msg.reply(response);
}


void updateBPM() {
	blink_frequency = metro.get_bpm();
	if(blink_frequency != 0){
		blink_frequency = (60*1000) / blink_frequency;
		is_startup = false;
	} else
		is_startup = true;	
}


void getBPM(web::http::http_request msg) {
	//msg.reply(200, web::json::value::number(metro.get_bpm()));
	msg_reply(msg, metro.get_bpm(), 1);
}

void getMIN(web::http::http_request msg) {
	
	//msg.reply(200, web::json::value::number(metro.getMIN_MAX(MIN)));
	msg_reply(msg, metro.getMIN_MAX(MIN), 1);
}
void getMAX(web::http::http_request msg) {
	
	//msg.reply(200, web::json::value::number(metro.getMIN_MAX(MAX)));
	msg_reply(msg, metro.getMIN_MAX(MAX), 1);
}

void delMIN(web::http::http_request msg) {
	metro.delMIN_MAX(MIN);
	updateBPM();
	//msg.reply(200);
	msg_reply(msg, 0 ,0);
}

void delMAX(web::http::http_request msg) {
	metro.delMIN_MAX(MAX);
	updateBPM();
	//msg.reply(200);
	msg_reply(msg, 0 ,0);
}

void setBPM(web::http::http_request msg){
	pplx::task<utility::string_t>msg_str = msg.extract_string();
	std::string str = utility::conversions::to_utf8string(msg_str.get());
	metro.addNewBPM(std::stoul (str,nullptr,0));
	updateBPM();
	msg_reply(msg, 0 ,0);
}

// This program shows an example of input/output with GPIO, along with
// a dummy REST service.
// ** You will have to replace this with your metronome logic, but the
//    structure of this program is very relevant to what you need. **
int main() {
	// WiringPi must be initialized at the very start.
	// This setup method uses the Broadcom pin numbers. These are the
	// larger numbers like 17, 24, etc, not the 0-16 virtual ones.
	wiringPiSetupGpio();

	// Set up the directions of the pins.
	// Be careful here, an input pin set as an output could burn out.
	pinMode(LED_RED, OUTPUT);
	pinMode(BTN_MODE, INPUT);
	pinMode(LED_GREEN, OUTPUT);
	pinMode(BTN_TAP, INPUT);
	// Note that you can also set a pull-down here for the button,
	// if you do not want to use the physical resistor.
	//pullUpDnControl(BTN_MODE, PUD_DOWN);

	// Configure the rest services after setting up the pins,
	// but before we start using them.
	// ** You should replace these with the BPM endpoints. **

	auto getBPM_rest = rest::make_endpoint("/bpm");
	auto getMIN_rest = rest::make_endpoint("/bpm/min");
	auto getMAX_rest = rest::make_endpoint("/bpm/max");

	getBPM_rest.support(web::http::methods::GET, getBPM);
	getBPM_rest.support(web::http::methods::PUT, setBPM);
	getMIN_rest.support(web::http::methods::GET, getMIN);
	getMIN_rest.support(web::http::methods::DEL, delMIN);
	getMAX_rest.support(web::http::methods::GET, getMAX);
	getMAX_rest.support(web::http::methods::DEL, delMAX);
	// Start the endpoints in sequence.

	getBPM_rest.open().wait();
	getMIN_rest.open().wait();
	getMAX_rest.open().wait();
	//metronome metro;

	// Use a separate thread for the blinking.
	// This way we do not have to worry about any delays
	// caused by polling the button state / etc.
	std::thread blink_thread(blink);
	blink_thread.detach();


	//initial the blink frequency
	blink_frequency = 1000;
	

	// ** This loop manages reading button state. **
	while (true) {
		// We are using a pull-down, so pressed = HIGH.
		// If you used a pull-up, compare with LOW instead.
		//btn_mode_pressed = (digitalRead(BTN_MODE) == HIGH);
		
		//Learn Mode
		if(digitalRead(BTN_MODE) == HIGH){
			printf("Mode button pressed!\n");			
			if(!metro.is_timing())
				metro.start_timing();
			else {
				metro.stop_timing();
				updateBPM();
				
			}
			//btn_mode_pressed = !btn_mode_pressed;

			delay(200);
		}
		
		
		if(metro.is_timing()){
			if(digitalRead(BTN_TAP) == HIGH){
				printf("Tap button pressed!\n");
				metro.tap();
				check_tap_button();
				delay(200);
			}
		}
		
		std::this_thread::sleep_for(10ms);

		// ** Note that the above is for testing when the button
		// is held down. For detecting "taps", or momentary
		// pushes, you need to check for a "rising edge" -
		// this is when the button changes from LOW to HIGH... **
	}

	return 0;
}
