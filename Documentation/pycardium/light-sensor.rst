.. py:module:: light_sensor

``light_sensor`` - Ambient Brightness
=====================================
On the harmonic board, there is an IR-LED which can be used in reverse as a
crude brightness sensor.  Values returned are in no particular unit but seem to
be fairly stable.

.. py:function:: start()

   Turn on the ADC and start reading brightness values.  (In past this function must be
   called before any measurements can be taken.)

.. py:function:: get_reading()

   Get an ambient brightness reading.  The returned value is in no particular
   unit, though it seems to be fairly stable. The value could be between 0 and 400.  Common values:

   - ~8: Very dark are
   - ~17: Typical hackerspace brightness
   - >200: Direct sunlight

   :returns: A brightness reading in no particular unit

.. py:function:: stop()

   Stop the ADC.

.. py:function:: read()

   Direct readout of the light-sensor.

   Use this function for low latency readout.  The time between polls can have
   an effect on the values measures.  If you do not need low latency, prefer
   :py:func:`light_sensor.get_reading`.

   .. versionadded:: 1.8
