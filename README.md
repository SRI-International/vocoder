# Vocoder Research
## Voice Recording & Encoding

**Author**: Matt Oyales, Intern

This repository stores potential software solutions for recording, compressing, and sending audio
as they pertain to the TMIC project. In this repo you will find a program to record software with
the SFML library, audio samples, a VoIP example, and a Python script to download audio samples from
YouTube.

I sectioned off particular components of functionality into their own directories. The goal is to develop
a solution that will link the functionality into a single program for the project to run. Sections may be added
or removed as implementation research continues.

## Contents of Repo
* `py_downloader/` : YouTube audio downloader script in Python
    * `audio_downloader.py`: Prompts user for a valid YouTube URL and downloads an audio file to
                             the same directory where this script resides.
* `sfml_record/` : Recording software solution via SFML (**S**imple **F**ast **M**ultimedia **L**ibrary)
    * `arial.ttf` : Arial font loaded for text-rendering
    * `main.cpp` : Audio recording program source code.
    * `makefile` : Simple, single-source makefile for compilation. Type `make` in the terminal to compile.
* `samples/` : Audio samples at various compression levels.