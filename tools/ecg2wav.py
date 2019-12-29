# vim: set ts=4 sw=4 tw=0 et pm=:
import numpy
import wave
import sys
import struct

def read(file_name):
    signal = numpy.fromfile(file_name, dtype=numpy.int16)
    return signal


signal = read(sys.argv[1])


sampleRate = 128.0   # hertz
duration = len(signal) / sampleRate   # seconds

wavef = wave.open('out.wav','w')
wavef.setnchannels(1) # mono
wavef.setsampwidth(2) 
wavef.setframerate(sampleRate)

for i in range(int(duration * sampleRate)):
    value = int(signal[i])
    data = struct.pack('<h', value)
    wavef.writeframesraw( data )


wavef.close()

