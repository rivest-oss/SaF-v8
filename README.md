# SaF v8

A simple (lossy) audio codec.

It encodes audio as follows:

1. Converts the audio from the original samples to 16-bit signed integer samples.
2. Split the audio into 256-sample chunks.
3. Process the delta of each sample in each chunk.
4. Store the seven most commonly used "deltas" with shorter bit prefixes.
5. Adds *dither* on audio playback, to mask quantization errors.
6. *That's all Folks.*

Prefixes (from most used, to least used, and EOF):

1. `0`.
2. `10`.
3. `110`.
4. `1110`.
5. `11110`.
6. `111110`.
7. `1111110`.
8. `11111110` (signals EOF).
