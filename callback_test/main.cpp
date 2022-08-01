#include <iostream>
#include <chrono>
#include <csignal>
#include <portaudio.h>
#include <thread>
#include <opus/opus.h>
#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <codec2/codec2.h>

#define NUM_CHANNELS		2
#define SAMPLE_RATE			8000
#define FRAMES_PER_BUFFER	960
#define BITRATE				8000

static log4cpp::Appender *appender = new log4cpp::OstreamAppender("console", &std::cout);
static log4cpp::Category& logger = log4cpp::Category::getRoot();
static volatile sig_atomic_t sig_caught = 0;

OpusDecoder 	*opusDecoder;
OpusEncoder 	*opusEncoder;
struct CODEC2 	*codec2;

short         *buf;
unsigned char *bits;
int            nsam, nbit;
// encoded buffer
std::vector<unsigned char> encoded(FRAMES_PER_BUFFER * NUM_CHANNELS * 0.5);

/**
 * Signal interrupt handler
 *
 * @param signal the signal
 */
static void sigHandler(int signal) {
	logger.warn("caught signal %d", signal);
	sig_caught = signal;
}

/**
 * Portaudio callback 
 */
static int streamCallback( const void *inputBuffer,
        void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData) {
    (void) timeInfo; // Prevent unused variable warnings.
    (void) statusFlags;
    (void) userData;

    // Cast data to floats:
	auto *in_buff = (int16_t*)inputBuffer;
	auto *out_buff = (int16_t*)outputBuffer;
	opus_int16 enc_bytes;
	opus_int16 dec_bytes;
    
    // Clean playback
	// unsigned long i;
    // for( i = 0; i < framesPerBuffer * NUM_CHANNELS; i++ ) {
	// 	out_buff[i] = in_buff[i];
	// }

	// OPUS ENCODE & DECODE
	// if ((enc_bytes = opus_encode(opusEncoder, reinterpret_cast<opus_int16 const*>(in_buff), 
	// 							FRAMES_PER_BUFFER, 
	// 							encoded.data(), 
	// 							encoded.size())) < 0) {
	// 	std::cout << "opus_encode failed: " << enc_bytes << "\n";
	// 	sig_caught = 2;
	// }

	// if ((dec_bytes = opus_decode(opusDecoder, 
	// 							encoded.data(), 
	// 							enc_bytes,
	// 							out_buff, FRAMES_PER_BUFFER, 0)) < 0) {
	// 	std::cout << "opus_decode failed: " << dec_bytes << "\n";
	// 	sig_caught = 2;
	// }

	// CODEC2 ENCODE & DECODE
	codec2_encode(codec2, bits, in_buff);
	codec2_decode(codec2, out_buff, bits);
    return paContinue;
}

/**
 * main function
 *
 * Program flow:
 * - Parse file configuration
 * - Init PortAudio engine and open default input and output audio devices
 * - Loop PortAudio until Ctrl + C
 * - Clean up PortAudio engine
 */
int main(int argc, char *argv[]){
	///////////////////////
	// init logger
	///////////////////////
	appender->setLayout(new log4cpp::BasicLayout());
	logger.setPriority(log4cpp::Priority::INFO);
	logger.addAppender(appender);

	///////////////////////
	// init opus library
	///////////////////////
	int opusErr;
	opusEncoder = opus_encoder_create(SAMPLE_RATE, NUM_CHANNELS, OPUS_APPLICATION_AUDIO, &opusErr);
	if(opusErr != OPUS_OK) {
		std::cout <<"opus_encoder_create failed :" << opusErr << std::endl;
		return 0;
	}

	opusDecoder = opus_decoder_create(SAMPLE_RATE, NUM_CHANNELS, &opusErr);
	if(opusErr != OPUS_OK) {
		std::cout <<"opus_decoder_create failed :" << opusErr << std::endl;
		return 0;
	}
	int opus_bandwidth;
	opus_encoder_ctl(opusEncoder, OPUS_SET_VBR(0));
    opus_encoder_ctl(opusEncoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    opus_encoder_ctl(opusEncoder, OPUS_SET_COMPLEXITY(5));
    opus_encoder_ctl(opusEncoder, OPUS_SET_DTX(1));
    opus_encoder_ctl(opusEncoder, OPUS_GET_BANDWIDTH(&opus_bandwidth));
    opus_encoder_ctl(opusEncoder, OPUS_SET_INBAND_FEC(1));

	opusErr = opus_encoder_ctl(opusEncoder, OPUS_SET_BITRATE(BITRATE));
	if(opusErr != OPUS_OK) {
		std::cout <<"opus_encoder_ctl failed to set bitrate :" << opusErr << std::endl;
		return 0;
	}

	///////////////////////
	// init codec2 library
	///////////////////////
	codec2 = codec2_create(CODEC2_MODE_3200);
	nsam = codec2_samples_per_frame(codec2);
	buf = (short*)malloc(nsam*sizeof(short));
    nbit = codec2_bits_per_frame(codec2);
    bits = (unsigned char*)malloc(nbit*sizeof(char));

	///////////////////////
	// init audio library
	///////////////////////
    PaError err;
    err = Pa_Initialize();

	if(err != paNoError) {
		logger.error("PortAudio error: %s", Pa_GetErrorText(err));
        exit(-1);
	}

    // list all devices (include hostApi):
	auto deviceCount = (int) Pa_GetDeviceCount();
    logger.info("There are %d audio devices", deviceCount);
    for( int i = 0; i < deviceCount; i++ ){
        const PaDeviceInfo *devInfo;
        devInfo = Pa_GetDeviceInfo( (PaDeviceIndex)i );

        const PaHostApiInfo *host;
        host = Pa_GetHostApiInfo( devInfo->hostApi );
        logger.info("Device Id Number %d: %s : %s", i, host->name, devInfo->name);
    }
    
    PaStream *stream = nullptr;
	PaStreamParameters inputParameters{};
	PaStreamParameters outputParameters{};

    // input parameter
    inputParameters.device = Pa_GetDefaultInputDevice();
	if (inputParameters.device == paNoDevice) {
		logger.error("No default input device.");
		exit(-1);
	}
	inputParameters.channelCount = NUM_CHANNELS;
	inputParameters.sampleFormat = paInt16;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = nullptr;

	logger.info("inputParameters.suggestedLatency: %.4f", inputParameters.suggestedLatency);

    // output parameter
    outputParameters.device = Pa_GetDefaultOutputDevice();
	if(outputParameters.device == paNoDevice) {
		logger.error("No default output device.");
		exit(-1);
	}
	outputParameters.channelCount = NUM_CHANNELS;
	outputParameters.sampleFormat =  paInt16;
	outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultHighOutputLatency;
	outputParameters.hostApiSpecificStreamInfo = nullptr;

	logger.info("outputParameters.suggestedLatency: %.4f", outputParameters.suggestedLatency);

    err = Pa_OpenStream(&stream,            // the input stream
						&inputParameters,   // input params
						&outputParameters,  // output params
						SAMPLE_RATE,        // sample rate
						nsam,  				// frames per buffer
						paClipOff,          // we won't output out of range samples so don't bother clipping them
						streamCallback,     // PortAudio callback function
						nullptr);			// data pointer

	if(err != paNoError) {
	    logger.error("Failed to open input stream: %s", Pa_GetErrorText(err));
		exit(-1);
	}

    // start the streams
	err = Pa_StartStream(stream);
	if(err != paNoError) {
		logger.error("Failed to start stream: %s", Pa_GetErrorText(err));
		exit(-1);
	}

	// init signal handler
	struct sigaction action;
	action.sa_handler = sigHandler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);
	sigaction(SIGINT, &action, nullptr);
	sigaction(SIGTERM, &action, nullptr);

    while(!sig_caught) {
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
	}

    ///////////////////////////
	// clean up audio library
	///////////////////////////
	logger.warn("Cleaning up PortAudio...");
	
	// close streams
	logger.warn("Stop stream");
	err = Pa_StopStream(stream);
	if(err != paNoError) {
		logger.error("Failed to close stop: %s", Pa_GetErrorText(err));
		exit(-1);
	}

	logger.warn("Closing stream");
	err = Pa_CloseStream(stream);
	if(err != paNoError) {
		logger.error("Failed to close stream: %s", Pa_GetErrorText(err));
		exit(-1);
	}

	// terminate PortAudio engine
	logger.warn("Terminating PortAudio engine");
	err = Pa_Terminate();
	if(err != paNoError) {
		logger.error("PortAudio error: %s", Pa_GetErrorText(err));
		exit(-1);
	}

	// destroy opus
	logger.warn("Destroy opus");
	opus_decoder_destroy(opusDecoder);
	opus_encoder_destroy(opusEncoder);

	// destroy codec2
	logger.warn("Destroy codec2");
	codec2_destroy(codec2);

    return 0;
}