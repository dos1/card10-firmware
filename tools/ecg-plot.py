# vim: set ts=4 sw=4 tw=0 et pm=:
import numpy
import sys
import matplotlib.pyplot as plt


def read(file_name):
    signal = numpy.fromfile(file_name, dtype=numpy.int16)
    return signal


signal = read(sys.argv[1])
factor = -1
count = 5
offset = 0

signal = signal[offset * 128 :]

count = min(min(len(signal) / 1280 + 1, 10), count)

font = {"family": "serif", "color": "darkred", "weight": "normal", "size": 16}

title = False

for i in range(count):
    plt.subplot(count, 1, i + 1)
    sub_signal = signal[i * 1280 : (i + 1) * 1280] * factor

    # pad with 0 as needed.
    # TODO: find a better solution to visialize this
    sub_signal = numpy.pad(sub_signal, (0, 1280 - len(sub_signal)), "constant")

    time_scale = (
        numpy.array(range(i * 1280, i * 1280 + len(sub_signal))) / 128.0 + offset
    )

    plt.plot(time_scale, sub_signal, "-")
    if not title:
        plt.title("File: %s" % sys.argv[1].split("/")[-1], fontdict=font)
        title = True

plt.xlabel("time (s)", fontdict=font)
plt.show()
