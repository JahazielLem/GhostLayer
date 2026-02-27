import socket
import cmd
import threading
import sys
import struct
from spacepackets.ccsds.spacepacket import SpHeader, PacketType, CCSDS_HEADER_LEN

def hexdump(data: bytes, width: int = 16):
    lines = []
    for offset in range(0, len(data), width):
        chunk = data[offset:offset + width]
        hex_bytes = " ".join(f"{b:02x}" for b in chunk)
        ascii_repr = "".join(chr(b) if 32 <= b < 127 else "." for b in chunk)
        lines.append(f"{offset:08x}  {hex_bytes:<{width*3}}  |{ascii_repr}|")
    return "\n".join(lines)

def spp_print_packet_details(packet: bytes):
    if len(packet) < 6:
        print("Error: paquete demasiado corto SPP.")
        return

    (packet_id, sequence, length) = struct.unpack_from(">HHH", packet)

    version = (packet_id >> 13) & 0x7
    pkt_type = (packet_id >> 11) & 0x1
    sec_header = (packet_id >> 10) & 0x1
    apid = packet_id & 0x7FF

    seq_flags = (sequence >> 14) & 0x3
    seq_count = sequence & 0x3FFF

    seq_flag_str = {
        0b00: "Continuation",
        0b01: "Start",
        0b10: "End",
        0b11: "Unsegmented",
    }.get(seq_flags, "Unknown")

    print("=== Space Packet Header ===")
    print(f" Version:             {version}")
    print(f" Type:                {pkt_type:02X}")
    print(f" Secondary Header:    {sec_header}")
    print(f" APID:                0x{apid:04X}")
    print(f" Sequence Flags:      0x{seq_flags:X} ({seq_flag_str})")
    print(f" Sequence Count:      {seq_count}")
    print(f" Data Length:         {length}")

    payload = packet[6:6 + length + 1]  # CCSDS length = (N - 1)
    print("=== Payload Dump (Hex) ===")


class SPP:
    def __init__(self):
        self.sequence_tc = 0
        self.sequence_tm = 0

    def build_packet(self, payload=b"Hello", apid=0x01, seq_count=0, packet_type=PacketType.TM):
        if packet_type == PacketType.TM:
            spp_header = SpHeader.tm(apid=apid, seq_count=seq_count, data_len=len(payload))
        else:
            spp_header = SpHeader.tc(apid=apid, seq_count=seq_count, data_len=len(payload))

        packet = spp_header.pack() + payload
        return bytes(packet)

    def build_tm(self, payload=b"TC", apid=0x02, packet_type=PacketType.TM):
        self.sequence_tm += 1
        return self.build_packet(payload, apid, self.sequence_tm, packet_type)
    def build_tc(self, payload=b"TM", apid=0x01, packet_type=PacketType.TC):
        self.sequence_tc += 1
        return self.build_packet(payload, apid, self.sequence_tc, packet_type)
    def show_packet_details(self, packet):
        spp_print_packet_details(packet)
        print(hexdump(packet))


class TCPClient(cmd.Cmd):
    intro = "TCP Client interactivo. Escribe help o ? para comandos.\n"
    prompt = "(tcp-client) "

    def __init__(self):
        super().__init__()
        self.sock = None
        self.receiver_thread = None
        self.connected = False

    # ----------- CONEXION -----------

    def do_connect(self, arg):
        """
        connect <host>
        Conecta al host en el puerto 9090
        """
        if self.connected:
            print("[!] Ya estás conectado.")
            return

        host = arg.strip()
        if not host:
            print("Uso: connect <host>")
            return

        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((host, 9090))
            self.connected = True
            print(f"[+] Conectado a {host}:9090")

            # Hilo para recibir datos
            self.receiver_thread = threading.Thread(target=self.receive_loop, daemon=True)
            self.receiver_thread.start()

        except Exception as e:
            print(f"[!] Error al conectar: {e}")

    # ----------- ENVIO -----------

    def do_send(self, arg):
        """
        send <mensaje>
        Envía datos al servidor
        """
        if not self.connected:
            print("[!] No estás conectado.")
            return

        if not arg:
            print("Uso: send <mensaje>")
            return

        try:
            sof = bytes([0x64, 0x83])
            eof = bytes([0x64, 0x69])
            payload = arg.encode()

            data = sof + payload + eof
            self.sock.sendall(data)
        except Exception as e:
            print(f"[!] Error enviando datos: {e}")
            self.do_disconnect("")

    def do_spp(self, arg):
        """
        send <mensaje>
        Envía datos al servidor
        """
        if not self.connected:
            print("[!] No estás conectado.")
            return

        if not arg:
            print("Uso: send <mensaje>")
            return

        try:
            sof = bytes([0x64, 0x83])
            eof = bytes([0x64, 0x69])
            payload = arg.encode()

            spp = SPP()
            spp_packet = spp.build_tm(payload=payload)

            data = sof + spp_packet + eof
            self.sock.sendall(data)
        except Exception as e:
            print(f"[!] Error enviando datos: {e}")
            self.do_disconnect("")
    # ----------- RECEPCION -----------

    def receive_loop(self):
        try:
            while self.connected:
                data = self.sock.recv(4096)
                if not data:
                    print("\n[!] Conexión cerrada por el servidor.")
                    self.connected = False
                    break

                print(f"\n[RECV] {data.decode(errors='ignore')}")
                print(self.prompt, end="", flush=True)

        except Exception as e:
            print(f"\n[!] Error recibiendo datos: {e}")
            self.connected = False

    # ----------- DESCONEXION -----------

    def do_disconnect(self, arg):
        """
        disconnect
        Cierra la conexión actual
        """
        if self.sock:
            self.connected = False
            try:
                self.sock.close()
            except:
                pass
            self.sock = None
            print("[*] Desconectado.")

    # ----------- SALIR -----------

    def do_exit(self, arg):
        """
        exit
        Salir del cliente
        """
        print("[*] Cerrando cliente...")
        self.do_disconnect("")
        return True

    def do_EOF(self, arg):
        print()
        return self.do_exit(arg)


if __name__ == "__main__":
    try:
        TCPClient().cmdloop()
    except KeyboardInterrupt:
        print("\n[*] Interrumpido por usuario.")
        sys.exit(0)