# OPUS Audio Samples

**Description**: The shell script in this directory takes in a `.ogg` file as input and compresses it into
                 various different bitrates and frame sizes. 

## Form of the File Names
output_XXbXXm.opus
* Number before `b` indicates the **bitrate** in *kbit/s*
* Number before `m` indicates the **frame size** in *ms*

## Decoding/Playing
From the command-line, there are a couple options for playing back the audio samples.
* `ffplay <filename>`
* `opusdec --force-wav --quiet <filename> - | aplay`