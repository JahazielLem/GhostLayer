#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Deamon Bridge
# GNU Radio version: 3.10.12.0

from gnuradio import blocks
from gnuradio import filter
from gnuradio.filter import firdes
from gnuradio import gr
from gnuradio.fft import window
import sys
import signal
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import iio
from gnuradio import zeromq
import gnuradio.lora_sdr as lora_sdr
import threading




class bridge(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "Deamon Bridge", catch_exceptions=True)
        self.flowgraph_started = threading.Event()

        ##################################################
        # Variables
        ##################################################
        self.samp_mult = samp_mult = 2
        self.bandwidth = bandwidth = 250000
        self.tx_bandwidth = tx_bandwidth = 250000
        self.samp_rate = samp_rate = bandwidth*samp_mult
        self.zmq_address = zmq_address = "tcp://0.0.0.0:5009"
        self.tx_zmq_address = tx_zmq_address = "tcp://0.0.0.0:5007"
        self.tx_spread_factor = tx_spread_factor = 7
        self.tx_samp_rate = tx_samp_rate = int(tx_bandwidth)
        self.tx_gain = tx_gain = 0
        self.tx_frequency = tx_frequency = 918000000
        self.spread_factor = spread_factor = 7
        self.pluto_source = pluto_source = "ip:pluto.local"
        self.impl_header = impl_header = False
        self.has_crc = has_crc = True
        self.frequency = frequency = 916000000
        self.coding_rate = coding_rate = 1
        self.bandpass = bandpass = firdes.complex_band_pass(1.0, samp_rate, -bandwidth/2, bandwidth/2, bandwidth/50, window.WIN_HAMMING, 6.76)

        ##################################################
        # Blocks
        ##################################################

        self.zeromq_pull_msg_source_0 = zeromq.pull_msg_source(tx_zmq_address, 100, True)
        self.zeromq_pub_sink_0 = zeromq.pub_sink(gr.sizeof_char, 1, zmq_address, 100, False, 1000, '', True, True)
        self.low_pass_filter_0 = filter.fir_filter_ccf(
            1,
            firdes.low_pass(
                1,
                tx_samp_rate,
                100000,
                5000,
                window.WIN_HAMMING,
                6.76))
        self.lora_tx_0 = lora_sdr.lora_sdr_lora_tx(
            bw=tx_bandwidth,
            cr=1,
            has_crc=True,
            impl_head=False,
            samp_rate=tx_samp_rate,
            sf=tx_spread_factor,
         ldro_mode=2,frame_zero_padd=10000,sync_word=[0x12] )
        self.lora_rx_0 = lora_sdr.lora_sdr_lora_rx( bw=bandwidth, cr=1, has_crc=True, impl_head=False, pay_len=255, samp_rate=samp_rate, sf=spread_factor, sync_word=[0x12], soft_decoding=True, ldro_mode=2, print_rx=[False,False])
        self.iio_pluto_source_0 = iio.fmcomms2_source_fc32(pluto_source if pluto_source else iio.get_pluto_uri(), [True, True], 22768)
        self.iio_pluto_source_0.set_len_tag_key('packet_len')
        self.iio_pluto_source_0.set_frequency(frequency)
        self.iio_pluto_source_0.set_samplerate(samp_rate)
        self.iio_pluto_source_0.set_gain_mode(0, 'slow_attack')
        self.iio_pluto_source_0.set_gain(0, 64)
        self.iio_pluto_source_0.set_quadrature(True)
        self.iio_pluto_source_0.set_rfdc(True)
        self.iio_pluto_source_0.set_bbdc(True)
        self.iio_pluto_source_0.set_filter_params('Auto', '', 0, 0)
        self.iio_pluto_sink_0 = iio.fmcomms2_sink_fc32(pluto_source if pluto_source else iio.get_pluto_uri(), [True, True], 1000, False)
        self.iio_pluto_sink_0.set_len_tag_key('')
        self.iio_pluto_sink_0.set_bandwidth(tx_bandwidth)
        self.iio_pluto_sink_0.set_frequency(tx_frequency)
        self.iio_pluto_sink_0.set_samplerate(tx_samp_rate)
        self.iio_pluto_sink_0.set_attenuation(0, tx_gain)
        self.iio_pluto_sink_0.set_filter_params('Auto', '', 0, 0)
        self.freq_xlating_fir_filter_xxx_0 = filter.freq_xlating_fir_filter_ccc(1, bandpass, 0, samp_rate)
        self.blocks_multiply_const_vxx_0 = blocks.multiply_const_cc(1)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.zeromq_pull_msg_source_0, 'out'), (self.lora_tx_0, 'in'))
        self.connect((self.blocks_multiply_const_vxx_0, 0), (self.low_pass_filter_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.lora_rx_0, 0))
        self.connect((self.iio_pluto_source_0, 0), (self.freq_xlating_fir_filter_xxx_0, 0))
        self.connect((self.lora_rx_0, 0), (self.zeromq_pub_sink_0, 0))
        self.connect((self.lora_tx_0, 0), (self.blocks_multiply_const_vxx_0, 0))
        self.connect((self.low_pass_filter_0, 0), (self.iio_pluto_sink_0, 0))


    def get_samp_mult(self):
        return self.samp_mult

    def set_samp_mult(self, samp_mult):
        self.samp_mult = samp_mult
        self.set_samp_rate(self.bandwidth*self.samp_mult)

    def get_bandwidth(self):
        return self.bandwidth

    def set_bandwidth(self, bandwidth):
        self.bandwidth = bandwidth
        self.set_bandpass(firdes.complex_band_pass(1.0, self.samp_rate, -self.bandwidth/2, self.bandwidth/2, self.bandwidth/50, window.WIN_HAMMING, 6.76))
        self.set_samp_rate(self.bandwidth*self.samp_mult)

    def get_tx_bandwidth(self):
        return self.tx_bandwidth

    def set_tx_bandwidth(self, tx_bandwidth):
        self.tx_bandwidth = tx_bandwidth
        self.set_tx_samp_rate(int(self.tx_bandwidth))
        self.iio_pluto_sink_0.set_bandwidth(self.tx_bandwidth)

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_bandpass(firdes.complex_band_pass(1.0, self.samp_rate, -self.bandwidth/2, self.bandwidth/2, self.bandwidth/50, window.WIN_HAMMING, 6.76))
        self.iio_pluto_source_0.set_samplerate(self.samp_rate)

    def get_zmq_address(self):
        return self.zmq_address

    def set_zmq_address(self, zmq_address):
        self.zmq_address = zmq_address

    def get_tx_zmq_address(self):
        return self.tx_zmq_address

    def set_tx_zmq_address(self, tx_zmq_address):
        self.tx_zmq_address = tx_zmq_address

    def get_tx_spread_factor(self):
        return self.tx_spread_factor

    def set_tx_spread_factor(self, tx_spread_factor):
        self.tx_spread_factor = tx_spread_factor
        self.lora_tx_0.set_sf(self.tx_spread_factor)

    def get_tx_samp_rate(self):
        return self.tx_samp_rate

    def set_tx_samp_rate(self, tx_samp_rate):
        self.tx_samp_rate = tx_samp_rate
        self.iio_pluto_sink_0.set_samplerate(self.tx_samp_rate)
        self.low_pass_filter_0.set_taps(firdes.low_pass(1, self.tx_samp_rate, 100000, 5000, window.WIN_HAMMING, 6.76))

    def get_tx_gain(self):
        return self.tx_gain

    def set_tx_gain(self, tx_gain):
        self.tx_gain = tx_gain
        self.iio_pluto_sink_0.set_attenuation(0,self.tx_gain)

    def get_tx_frequency(self):
        return self.tx_frequency

    def set_tx_frequency(self, tx_frequency):
        self.tx_frequency = tx_frequency
        self.iio_pluto_sink_0.set_frequency(self.tx_frequency)

    def get_spread_factor(self):
        return self.spread_factor

    def set_spread_factor(self, spread_factor):
        self.spread_factor = spread_factor

    def get_pluto_source(self):
        return self.pluto_source

    def set_pluto_source(self, pluto_source):
        self.pluto_source = pluto_source

    def get_impl_header(self):
        return self.impl_header

    def set_impl_header(self, impl_header):
        self.impl_header = impl_header

    def get_has_crc(self):
        return self.has_crc

    def set_has_crc(self, has_crc):
        self.has_crc = has_crc

    def get_frequency(self):
        return self.frequency

    def set_frequency(self, frequency):
        self.frequency = frequency
        self.iio_pluto_source_0.set_frequency(self.frequency)

    def get_coding_rate(self):
        return self.coding_rate

    def set_coding_rate(self, coding_rate):
        self.coding_rate = coding_rate

    def get_bandpass(self):
        return self.bandpass

    def set_bandpass(self, bandpass):
        self.bandpass = bandpass
        self.freq_xlating_fir_filter_xxx_0.set_taps(self.bandpass)




def main(top_block_cls=bridge, options=None):
    tb = top_block_cls()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        sys.exit(0)

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    tb.start()
    tb.flowgraph_started.set()

    tb.wait()


if __name__ == '__main__':
    main()
