# Imported receiver DSP

`receiver_dsp/` is a local copy from
`C:\Users\asyra\Downloads\VS Pico\SDR-RX`, Git commit
`ceca0e46e8692d6f2d0a3b92a109d6b9292581c5`,
repository https://github.com/Asyraf-Khairul/SDR-RX.
That receiver is based on Jonathan P Dawson's PicoRX:
https://github.com/dawsonjon/PicoRX.
Existing source copyright notices are retained. FFT/filter headers identify
Copyright (c) Jonathan P Dawson 2024, License: MIT.
`ring_buffer_lib.*` retains the full MIT notice, Copyright (c) 2022 rppicomidi.
`ssd1306.*` retains its existing David Schramm MIT notice.

Local changes to imported algorithms:

- Initialize capture/AGC/PLL state used by the bench.
- Explicit squelch bypass for a reproducible unsquelched test.
- Guard IQ correction at zero energy; start with unity correction and saturate.
- Guard noise-reduction division and table indexing; widen gain arithmetic.
- Correct the second de-emphasis mode's feedback coefficient selection.
- Recompute polar samples after impulse blanking so AM/FM see the change.
- AGC peak detector considers both polarities.
- Correct negative-frequency CIC compensation sign and negative NR-bin indexing.
- Guard flat/silent spectrum scaling and initialize empty queue reads.
- Use defined wider frequency-word conversion and compile with `-fwrapv`.

AM timing changes additionally group equivalent CIC updates with local state,
retain exact spectrum logarithm results in a precomputed table, and add stage
profiling. `tests/reference_receiver/` freezes the prior adapted receiver at
`34bde79` for numerical comparisons, retaining its original notices. The 37-case
bit-identical comparison is against that frozen adapted reference, not the
original upstream receiver.

These are adapted receiver algorithms, not a bit-identical validation of the
upstream revision. Function-static filter histories remain; reboot for a clean
independent measurement. The source `SDR-RX` working tree was not edited.

MIT License (for the MIT-identified upstream portions):

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
