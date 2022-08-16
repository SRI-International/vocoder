# Tin Can Phone (SRI Modified)

A simple C++ program wrote to demonstrate peer-to-peer VOIP, for Linux and Windows. 
Modifications have been made to make lower Opus encoder bitrates available, as well as adjustments
to the computational complexity and the passband of the encoder. So far, these modifications have
only been implemented for use in Linux/Ubuntu.

# Precompiled binaries

If you want to have the program run on Windows, use the precompiled binary. However, you will only
be able to make and answer calls. The program modifications do not exist on Windows yet.

Windows: [tincanphone.exe](http://garysinitsin.com/tincanphone.exe) **NOT UP TO DATE**
<!-- Linux: [tincanphone_0.1-1_amd64.deb](http://garysinitsin.com/tincanphone_0.1-1_amd64.deb) -->

# How to use it

To make an outgoing call, input the IP address of another user running the program and press Call.

Your IP should be printed in the Tin Can Phone window after it starts up if UPnP is working on your network;
otherwise you may need to forward UDP port 56780 and look up your public IP yourself in order for incoming calls to work.

[Port Forwarding Guide](https://www.noip.com/support/knowledgebase/general-port-forwarding-guide/)

# Compiling on Ubuntu

Tin Can Phone has 2 external dependencies:
[opus](https://www.opus-codec.org/) and [portaudio](http://portaudio.com/).
[miniupnpc](https://github.com/miniupnp/miniupnp) is also used, but is included in `miniupnpc.zip` for convenience, and should be compiled alongside Tin Can Phone.
Gtk3 is also required on Linux.

## Installing Dependencies via CLI
* gcc - `sudo apt install build-essential`
* opus - `sudo apt-get install libopus-dev`
* Gtk3 - `sudo apt-get install libgtk-3-dev`
* pkg-config - `sudo apt install pkg-config`
* libjack - `sudo apt-get install libjack-dev`
* portaudio - `sudo apt-get install portaudio19-dev`

## Installing PortAudio (if portaudio19-dev isn't working)
PortAudio is an Audio I/O library that is available on all three major operating systems. However,
it does not have a nice package to install from, unlike the other dependencies listed above. 
These are the steps needed to install the PortAudio library on Ubuntu:

1. Install ALSA Development Kit - `sudo apt-get install libasound-dev`
2. Go to the [PortAudio Downloads Page](http://files.portaudio.com/download.html) and get the most recent .tgz
3. In the directory where you downloaded this file, run `tar xvzf <.tgz file>`.
4. `cd portaudio/`
5. `./configure && make` - if you're missing anything, `configure` will let you know before compiling.
6. `sudo make install` to install the library

## Compiling
To make compiling simple, I have included a `makefile` that will produce an executable simply called
`tincanphone`.
* Ensure you have `make` command installed.
* Type `make` in the base directory.
* To run, type `./tincanphone 2>/dev/null`

<!--
To compile on Linux, make sure the dev packages for the dependencies are installed, `unzip miniupnpc.zip`, then run `compile.sh`.
Note: on some distros you may need to `apt-get install libjack0` before `portaudio19-dev` to get the correct dev packages for compiling
(see [here](http://askubuntu.com/questions/526385/unable-to-install-libjack-dev)).
-->

# Compiling for Windows (TODO)

Otherwise, creating a project file for any IDE is pretty straightforward. Add the contents of either `src/Windows` or `src/Gtk` depending on your platform,
make sure to set up the above dependencies, and don't forget to define `MINIUPNP_STATICLIB`.


# Notes

The default port Tin Can Phone uses is UDP 56780, unless that port is already in use.
The GUI is implemented with Gtk3 on Linux and WinAPI on Windows.
Bug reports or fixes are welcome.

Although care was taken to create a usable application, some things were left out for simplicity:

* UPnP (or manual port-forwarding) is required instead of including code for other techniques such as proxying or hole-punching, which would also require a third party host.
* Network and audio I/O is all done in a synchronous fashion in a single thread. The GUI does run in a separate thread, though.
* IPv4 was assumed to make testing easier, but forward-compatible socket APIs were used.
* UPnP discovery is done every time the program starts instead of being saved so subsequent runs start up faster.


# License

MIT licensed open source; see the LICENSE file. If you use this code for something cool, letting me know would be appreciated but it's not required.
