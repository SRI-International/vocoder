#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <opus/opus.h>
#include <portaudio.h>

using namespace std;

int main(int argc, char **argv) {
    int opus_err;
    PaError pa_err;
    
    int const channels = 1;
    int const buf_size = 480;
    int const sample_rate = 8000;
    int const duration_secs = 10;

    opus_int32 enc_bytes;
    opus_int32 dec_bytes;
    int frames_processed = 0;

    vector<unsigned short> captured(buf_size);
    vector<unsigned short> decoded(buf_size);
    vector<unsigned char> encoded(buf_size * 2);

    OpusEncoder *encoder = opus_encoder_create(sample_rate, channels, OPUS_APPLICATION_AUDIO, &opus_err);
    OpusDecoder *decoder = opus_decoder_create(sample_rate, channels, &opus_err);

    pa_err = Pa_Initialize();
    PaStream *stream = nullptr;
    pa_err = Pa_OpenDefaultStream(&stream, channels, channels, paInt16, sample_rate, buf_size, nullptr, nullptr);
    pa_err = Pa_StartStream(stream);

    while (frames_processed < sample_rate * duration_secs) {
        pa_err = Pa_ReadStream(stream, captured.data(), buf_size);
        enc_bytes = opus_encode(encoder, reinterpret_cast<opus_int16 const *>(captured.data()), 480, encoded.data(), encoded.size());
        dec_bytes = opus_decode(decoder, encoded.data(), enc_bytes, reinterpret_cast<opus_int16 *>(decoded.data()), 480, 0);
        pa_err = Pa_WriteStream(stream, decoded.data(), buf_size);
        frames_processed += buf_size;
    }

    pa_err = Pa_StopStream(stream);
    pa_err = Pa_CloseStream(stream);
    pa_err = Pa_Terminate();

    opus_encoder_destroy(encoder);
    opus_decoder_destroy(decoder);

    return 0;
}