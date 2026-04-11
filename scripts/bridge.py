#!/usr/bin/env python3

# Author: Kevin Leon
# Date: 2026

import argparse
import select
import signal
import socket
import string
import struct
import sys
import time

import zmq
from pluto_lora_rx import pluto_lora_rx

DEFAULT_PORT = 5009
DEFAULT_PLUTO_SOURCE = "ip:pluto.local"
DEFAULT_DOWNLINK_ADDRESS = "tcp://127.0.0.1:5009"
DEFAULT_FILE_OUTPUT = "output.bin"

DEFAULT_BRIDGE_HOST = "127.0.0.1"
DEFAULT_BRIDGE_PORT = 5008

DOWNLINK_FREQ = 916000000
DOWNLINK_BW = 250000
DOWNLINK_SF = 7

# PCAP Constants
PCAP_GLOBAL_HEADER_FORMAT = "<LHHIILL"
PCAP_PACKET_HEADER_FORMAT = "<llll"
PCAP_MAGIC_NUMBER = 0xA1B2C3D4
PCAP_VERSION_MAJOR = 2
PCAP_VERSION_MINOR = 4
PCAP_MAX_PACKET_SIZE = 0x0000FFFF


def is_hex_encoded(data: bytes) -> bool:
    if len(data) % 2 != 0:
        return False

    hex_chars = set(string.hexdigits.encode("ascii"))
    return all(byte in hex_chars for byte in data)


def pcap_header(interface=148):
    return struct.pack(
        PCAP_GLOBAL_HEADER_FORMAT,
        PCAP_MAGIC_NUMBER,
        PCAP_VERSION_MAJOR,
        PCAP_VERSION_MINOR,
        0,
        0,
        PCAP_MAX_PACKET_SIZE,
        interface,
    )


class Pcap:
    def __init__(self, packet: bytes, timestamp_seconds: float):
        self.packet = packet
        self.timestamp_seconds = timestamp_seconds
        self.pcap_packet = self.pack()

    def pack(self):
        int_timestamp = int(self.timestamp_seconds)
        timestamp_offset = int((self.timestamp_seconds - int_timestamp) * 1_000_000)
        return (
            struct.pack(
                PCAP_PACKET_HEADER_FORMAT,
                int_timestamp,
                timestamp_offset,
                len(self.packet),
                len(self.packet),
            )
            + self.packet
        )

    def get(self):
        return self.pcap_packet


def hexdump(data: bytes, width: int = 16) -> str:
    lines = []
    for offset in range(0, len(data), width):
        chunk = data[offset : offset + width]
        hex_bytes = " ".join(f"{b:02X}" for b in chunk)
        hex_bytes = hex_bytes.ljust(width * 3)
        ascii_bytes = "".join(
            chr(b) if chr(b) in string.printable and b >= 0x20 else "." for b in chunk
        )
        lines.append(f"{offset:08X}  {hex_bytes}  {ascii_bytes}")
    return "\n".join(lines)


def hex_string_to_bytes(data_bytes: bytes) -> bytes:
    try:
        clean_str = data_bytes.decode("ascii", errors="ignore").replace(" ", "").strip()
        if len(clean_str) % 2 != 0:
            clean_str = clean_str[:-1]
        return bytes.fromhex(clean_str)
    except Exception as e:
        print(f"[!] Error converting hex string: {e}")
        return b""


def show_args_config(args):
    print("========== Configuration ==========")
    for k, v in vars(args).items():
        print(f"{k:25}: {v}")
    print("----------------------------------\n")


class Controller:
    def __init__(self, args):
        self.args = args
        self.ctx = None
        self.sock = None
        self.tb = None
        self.running = False
        self.f_output = None
        self.f_pcap_output = None
        self.bridge_sock = None
        self.last_reconnect_time = 0

    def file_write_frame(self, data: bytes):
        if self.f_output is not None:
            ts = time.time_ns()
            length = len(data)
            header = struct.pack("<QH", ts, length)
            self.f_output.write(header)
            self.f_output.write(data)
            self.f_output.flush()

    def file_open(self):
        self.f_output = open(self.args.output_file, "wb")

    def file_close(self):
        if self.f_output:
            self.f_output.close()

    def pcap_write_frame(self, data: bytes):
        if self.f_pcap_output is not None:
            pcap_packet = Pcap(data, time.time()).get()
            self.f_pcap_output.write(pcap_packet)
            self.f_pcap_output.flush()

    def pcap_open(self):
        self.f_pcap_output = open(self.args.pcap_output_file, "wb")
        self.f_pcap_output.write(pcap_header())
        self.f_pcap_output.flush()

    def pcap_close(self):
        if self.f_pcap_output:
            self.f_pcap_output.close()

    def setup(self):
        if pluto_lora_rx is not None:
            self.tb = pluto_lora_rx()
            self.tb.set_frequency(float(self.args.frequency))
            self.tb.set_bandwidth(int(self.args.bandwidth))
            self.tb.set_zmq_address(self.args.address)
            self.tb.set_pluto_source(str(self.args.pluto_address))
        else:
            print(
                "[!] Warning: pluto_lora_rx module not found. Running in ZMQ-only mode."
            )

        if self.args.output_file:
            self.file_open()

        if self.args.pcap_output_file:
            self.pcap_open()

    def connect_to_bridge(self):
        now = time.time()
        if self.bridge_sock is None and (now - self.last_reconnect_time) > 2:
            self.last_reconnect_time = now
            try:
                self.bridge_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.bridge_sock.settimeout(1.0)
                self.bridge_sock.connect((self.args.bridge_host, self.args.bridge_port))
                self.bridge_sock.setblocking(False)
                print(
                    f"[*] Connected to bridge at {self.args.bridge_host}:{self.args.bridge_port}"
                )
            except (ConnectionRefusedError, socket.error):
                self.bridge_sock = None

    def send_to_bridge(self, data: bytes):
        if self.bridge_sock:
            payload = (
                bytes([0x64, 0x83])
                + struct.pack(">B", len(data))
                + data
                + bytes([0x64, 0x69])
            )
            print(payload)
            try:
                self.bridge_sock.sendall(payload)
            except socket.error:
                print("[!] Bridge connection lost")
                self.bridge_sock.close()
                self.bridge_sock = None

    def parse_bridge_data(self, data: bytes):
        if len(data) < 12:
            return
        try:
            (header, frequency, bandwidth, spreadfactor, tail) = struct.unpack_from(
                ">HIHHH", data
            )

            if header == 0x6483 and tail == 0x6469:
                print(
                    f"Header: {header} Freq: {frequency / 100} Bw: {bandwidth / 100} {spreadfactor}"
                )
            else:
                print(f"[!] Invalid Packet Magic: {header:04X} {tail:04X}")
        except struct.error as e:
            print(f"[!] Parsing error: {e}")

    def start(self):
        self.running = True
        if self.tb:
            self.tb.start()
            print("[+] GNU Radio script started")

        self.ctx = zmq.Context()
        self.sock = self.ctx.socket(zmq.SUB)
        self.sock.connect(self.args.address)
        self.sock.setsockopt(zmq.SUBSCRIBE, b"")
        print(f"[*] Subscribed to {self.args.address} (Mode: {self.args.mode.upper()})")

        self.connect_to_bridge()

    def stop(self):
        if not self.running:
            return

        print("\n[!] Stopping...")
        if self.tb:
            self.tb.stop()
            self.tb.wait()

        if self.sock:
            self.sock.close(0)

        if self.ctx:
            self.ctx.term()

        if self.bridge_sock:
            self.bridge_sock.close()

        self.file_close()
        self.pcap_close()
        print("[*] Clean exit")

    def run(self):
        def handler(sig, frame):
            self.stop()
            sys.exit(0)

        signal.signal(signal.SIGINT, handler)
        signal.signal(signal.SIGTERM, handler)

        self.setup()
        self.start()

        poller = zmq.Poller()
        poller.register(self.sock, zmq.POLLIN)

        try:
            while self.running:
                self.connect_to_bridge()

                socks = dict(poller.poll(10))
                if self.sock in socks and socks[self.sock] == zmq.POLLIN:
                    raw_zmq = self.sock.recv()
                    processed = (
                        hex_string_to_bytes(raw_zmq)
                        if self.args.mode == "tc"
                        else raw_zmq
                    )

                    if processed:
                        print(
                            f"\n=========== {self.args.mode.upper()} PACKET ==========="
                        )
                        print(f"Original Length: {len(raw_zmq)}")
                        print(f"Processed Length: {len(processed)}")
                        print(hexdump(processed))
                        self.send_to_bridge(processed)
                        self.file_write_frame(processed)
                        self.pcap_write_frame(processed)
                if self.bridge_sock:
                    try:
                        readable, _, _ = select.select([self.bridge_sock], [], [], 0.01)
                        if readable:
                            bridge_data = self.bridge_sock.recv(1024)
                            if not bridge_data:
                                print("[!] Bridge disconnected")
                                self.bridge_sock.close()
                                self.bridge_sock = None
                            else:
                                self.parse_bridge_data(bridge_data)
                                if self.tb:
                                    pass
                    except socket.error:
                        self.bridge_sock = None
        except KeyboardInterrupt:
            self.stop()


def main():
    parser = argparse.ArgumentParser(
        prog="pylora_rx",
        description="GNU Radio LoRa Receiver",
        epilog="Kevin Leon - 2026",
    )
    parser.add_argument(
        "-f", "--frequency", help="Frequency for Downlink (Hz)", default=DOWNLINK_FREQ
    )
    parser.add_argument(
        "-bw", "--bandwidth", help="Bandwidth for Downlink (Hz)", default=DOWNLINK_BW
    )
    parser.add_argument(
        "-sf",
        "--spread_factor",
        help="SpreadFactor for Downlink (Hz)",
        default=DOWNLINK_SF,
    )
    parser.add_argument(
        "-a", "--address", help="ZMQ Address for RX", default=DEFAULT_DOWNLINK_ADDRESS
    )
    parser.add_argument("-o", "--output-file", help="File output")
    parser.add_argument("-pcap", "--pcap-output-file", help="File output")
    parser.add_argument(
        "-p", "--pluto-address", help="Pluto Source", default=DEFAULT_PLUTO_SOURCE
    )

    parser.add_argument(
        "-m",
        "--mode",
        choices=["tm", "tc"],
        default="tm",
        help="Operation mode: tm (Binary) or tc (Hex String to Binary). Default: tm",
    )
    parser.add_argument("--bridge-host", default=DEFAULT_BRIDGE_HOST)
    parser.add_argument("--bridge-port", type=int, default=DEFAULT_BRIDGE_PORT)

    args = parser.parse_args()
    show_args_config(args)

    ctrl = Controller(args)
    ctrl.run()


if __name__ == "__main__":
    main()
