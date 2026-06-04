#!/usr/bin/env python3

from __future__ import annotations

import argparse
import socket
import sys
import time


KEYS = {
    "a": 0x01,
    "b": 0x02,
    "select": 0x04,
    "start": 0x08,
    "right": 0x10,
    "left": 0x20,
    "up": 0x40,
    "down": 0x80,
}

KEY_STUBS = (0x0203DF10, 0x0203DF28, 0x0203DF58, 0x0203DF70)


class RSPClient:
    def __init__(self, host: str, port: int):
        self.sock = socket.create_connection((host, port), timeout=5)
        self.sock.settimeout(5)

    def close(self) -> None:
        self.sock.close()

    def _checksum(self, payload: bytes) -> int:
        return sum(payload) & 0xFF

    def packet(self, payload: str) -> str:
        raw = payload.encode("ascii")
        packet = b"$" + raw + b"#" + f"{self._checksum(raw):02x}".encode("ascii")
        self.sock.sendall(packet)
        ack = self.sock.recv(1)
        if ack not in (b"+", b""):
            raise RuntimeError(f"remote rejected packet {payload!r}: {ack!r}")
        return self.recv_packet()

    def recv_packet(self) -> str:
        while True:
            byte = self.sock.recv(1)
            if byte == b"$":
                break
        payload = bytearray()
        while True:
            byte = self.sock.recv(1)
            if byte == b"#":
                break
            payload.extend(byte)
        self.sock.recv(2)
        self.sock.sendall(b"+")
        return payload.decode("ascii", errors="replace")

    def interrupt(self) -> str:
        self.sock.sendall(b"\x03")
        return self.recv_packet()

    def write(self, address: int, data: bytes) -> None:
        response = self.packet(f"M{address:x},{len(data):x}:{data.hex()}")
        if response != "OK":
            raise RuntimeError(f"write failed at 0x{address:08x}: {response}")

    def continue_for(self, seconds: float) -> None:
        self.sock.sendall(b"$c#63")
        ack = self.sock.recv(1)
        if ack not in (b"+", b""):
            raise RuntimeError(f"continue rejected: {ack!r}")
        time.sleep(seconds)
        self.interrupt()


def key_stub_bytes(mask: int) -> bytes:
    if not 0 <= mask <= 0xFF:
        raise ValueError("this helper only supports key masks encodable by Thumb movs r0, #imm8")
    return ((0x2000 | mask).to_bytes(2, "little") + (0x4770).to_bytes(2, "little"))


def set_key(client: RSPClient, mask: int) -> None:
    print(f"key mask 0x{mask:02x}", flush=True)
    data = key_stub_bytes(mask)
    for address in KEY_STUBS:
        client.write(address, data)


def tap(client: RSPClient, mask: int, hold: float, gap: float) -> None:
    set_key(client, mask)
    client.continue_for(hold)
    set_key(client, 0)
    client.continue_for(gap)


def parse_key(name: str) -> int:
    try:
        return KEYS[name.lower()]
    except KeyError as exc:
        raise argparse.ArgumentTypeError(f"unknown key {name!r}; expected one of {', '.join(KEYS)}") from exc


def main() -> int:
    parser = argparse.ArgumentParser(description="Inject DS key input through melonDS's ARM9 GDB RSP stub.")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=3333)
    parser.add_argument("--boot-wait", type=float, default=5.0)
    parser.add_argument("--hold", type=float, default=0.20)
    parser.add_argument("--gap", type=float, default=0.20)
    parser.add_argument("--tap", action="append", type=parse_key, default=[])
    parser.add_argument("--walk", type=int, default=0, help="alternate left/right this many times")
    parser.add_argument("--post-wait", type=float, default=0.0)
    args = parser.parse_args()

    client = RSPClient(args.host, args.port)
    try:
        print(f"connected to {args.host}:{args.port}", flush=True)
        client.packet("qSupported:swbreak+;hwbreak+")
        client.packet("?")
        set_key(client, 0)
        print(f"boot wait {args.boot_wait:.2f}s", flush=True)
        client.continue_for(args.boot_wait)
        for mask in args.tap:
            print(f"tap 0x{mask:02x}", flush=True)
            tap(client, mask, args.hold, args.gap)
        for _ in range(args.walk):
            print("walk left/right", flush=True)
            tap(client, KEYS["left"], args.hold, args.gap)
            tap(client, KEYS["right"], args.hold, args.gap)
        if args.post_wait > 0:
            print(f"post wait {args.post_wait:.2f}s", flush=True)
            client.continue_for(args.post_wait)
        set_key(client, 0)
    except socket.timeout:
        print("timed out waiting for RSP response", file=sys.stderr, flush=True)
        return 2
    finally:
        client.close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
