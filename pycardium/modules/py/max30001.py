import sys_max30001
import interrupt
import ucollections


class MAX30001:
    def __init__(
        self,
        usb=False,
        bias=True,
        sample_rate=128,
        callback=None,
        sample_buffer_len=256,
    ):
        self.sample_rate = sample_rate
        self.callback = callback
        self.sample_buffer_len = sample_buffer_len
        self.interrupt_id = interrupt.MAX30001_ECG
        self.usb = usb
        self.bias = bias
        self._callback = callback
        self.enable_sensor()

    def enable_sensor(self):
        interrupt.disable_callback(self.interrupt_id)
        interrupt.set_callback(self.interrupt_id, self._interrupt)
        self.stream_id = sys_max30001.enable_sensor(
            self.usb, self.bias, self.sample_rate, self.sample_buffer_len
        )

        if self.stream_id < 0:
            raise ValueError("Enable sensor returned %i", self.stream_id)

        self.active = True

        if self._callback:
            interrupt.enable_callback(self.interrupt_id)

    def __enter__(self):
        return self

    def __exit__(self, _et, _ev, _t):
        self.close()

    def close(self):
        if self.active:
            self.active = False
            ret = sys_max30001.disable_sensor(self.sensor_id)

            if ret < 0:
                raise ValueError("Disable sensor returned %i", ret)

            interrupt.disable_callback(self.interrupt_id)
            interrupt.set_callback(self.interrupt_id, None)

    def read(self):
        if self.active:
            return sys_max30001.read_sensor(self.stream_id)
        return []

    def _interrupt(self, _):
        if self.active:
            data = self.read()
            if self._callback:
                self._callback(data)
