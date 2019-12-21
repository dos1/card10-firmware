poor-profiler
-------------
This is a (rather poor) attempt at building a profiler for card10.  The
idea is based on the [poor man's profiler](https://poormansprofiler.org/).
Essentially, it uses stack-traces gathered using GDB.  This means,
execution will be significantly slowed down which might make certain
features of the firmware misbehave.  Use with care!

That said, here is how to use it:

1. Configure the profiler by adjusting `nsamples` and `sleeptime` in
   [`poor-profiler.sh`](https://git.card10.badge.events.ccc.de/card10/firmware/blob/master/tools/poor-profiler/poor-profiler.sh).
   (If anyone wants to send a patch which makes this script use proper
   commandline args, please do!)
2. Start profiling!  Just call
   ```bash
   ./tools/poor-profiler/poor-profiler.sh >/tmp/samples
   ```
   from the firmware repository root.
3. Next, collapse the samples using the
   [`poor-collapse.sh`](https://git.card10.badge.events.ccc.de/card10/firmware/blob/master/tools/poor-profiler/poor-collapse.sh)
   script:
   ```bash
   $ ./tools/poor-profiler/poor-collapse.sh /tmp/samples >/tmp/samples.collapsed
   ```
4. Finally, feed the collapsed samples into the Brendan Gregg's
   [FlameGraph](https://github.com/brendangregg/FlameGraph) script which
   will create an svg you can view (and interact with!) in your browser:
   ```bash
   $ /path/to/FlameGraph/flamegraph.pl /tmp/samples.collapsed >/tmp/flamegraph.svg
   $ firefox /tmpflamegraph.svg
   ```
   (If you feel that a perl script is not modern enough, you can also use
   [`inferno-flamegraph`](https://github.com/jonhoo/inferno) which is
   written in Rust ...)

Happy profiling!
