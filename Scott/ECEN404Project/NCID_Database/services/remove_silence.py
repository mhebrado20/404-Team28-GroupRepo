import numpy as np
from scipy.io import wavfile
from pathlib import Path


def remove_silence(folder_location: str, file_name: str):

    # Maintain naming convention for code I had already written. This came about because when I wrote the code to create
    # a new directory and avoid overwriting data (which I learned the hard way), I needed the file and folder locations
    # split so that I could append "_processed"
    file_location = folder_location + file_name

    # Segmentation
    fs, signal = wavfile.read(file_location + ".wav")
    # print(fs)
    signal = signal / (2 ** 15)
    signal_len = len(signal)
    segment_size_t = 1  # segment size in seconds
    segment_size = segment_size_t * fs  # segment size in samples
    # Break signal into list of sprint segments in a single-line Python code
    segments = np.array([signal[x:x + segment_size] for x in np.arange(0, signal_len, segment_size, dtype=object)])

    # Remove pauses using an energy threshold = 50% of the median energy:
    energies = [(s ** 2).sum() / len(s) for s in segments]
    # (attention: integer overflow would occur without normalization here!)
    threshold = 0.5 * np.median(energies)
    index_of_segments_to_keep = (np.where(energies > threshold)[0])
    # get segments that have energies higher than the threshold:
    segments2 = segments[index_of_segments_to_keep]
    # concatenate segments to signal:
    new_signal = np.concatenate(segments2)

    if fs == 9600:  # hard coded bit rate of files we know will be mono audio
        np.reshape(new_signal, new_signal.size)

    # create a new directory for the processed signal so that the original is not overwritten and the quantity of files
    # in the original is also maintained, this ensures you can upload the same data many times without errors
    if not Path(folder_location + "_processed").exists():
        Path(folder_location + "_processed").mkdir()

    # write the data to the new file location with the appended string "_processed" to indicate that it is the processed
    # file
    # fs = fs/2
    wavfile.write(folder_location + "_processed" + file_name + "_processed" + ".wav", fs,
                  new_signal.astype(np.float32))

    return new_signal


print('Starting Amy\'s file')
remove_silence('C:/Users/sky20/Desktop/serialrecording/Original training files', '/Amy_Orig')
print('Done with Amy\'s file')

print('Starting Scott\'s file')
remove_silence('C:/Users/sky20/Desktop/serialrecording/Original training files', '/Scott_Orig')
print('Done with Scott\'s file')

print('Starting Matthew\'s file')
remove_silence('C:/Users/sky20/Desktop/serialrecording/Original training files', '/Matthew_Orig')
print('Done with Matthew\'s file')
