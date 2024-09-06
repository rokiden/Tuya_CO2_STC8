import serial
import time

log = bytearray()


def get_bytes(n):
    d = ser.read(n)
    log.extend(d)
    # print(d.hex())
    return d


def log_flush():
    print(len(log), log.hex())
    log.clear()


def get_byte():
    return get_bytes(1)[0]


def get_int(n):
    return int.from_bytes(get_bytes(n))


prev_time = None


def get_time():
    global prev_time
    t = time.time()
    if prev_time is None:
        d = 0
    else:
        d = t - prev_time
    prev_time = t
    return f'[{d:.03f}]'


class Tuya:
    CMDS = {
        0x1: 'QPI',
        0x2: 'RNS',
        0x3: 'CZM',
        0x6: 'RSP',
        0x20: 'QZS'
    }

    @staticmethod
    def wait_start():
        b = None
        i = 0
        while True:
            bp = b
            b = get_byte()
            i += 1
            if (bp, b) == (0x55, 0xaa):
                skipped = i - 2
                if skipped:
                    print('skipped', skipped)
                return

    @staticmethod
    def main():
        while True:
            Tuya.wait_start()
            ver = get_byte()
            assert ver == 2
            seq = get_int(2)
            cmd = get_byte()
            len = get_int(2)
            data = get_bytes(len)

            csum = sum(log) % 256

            check = get_byte()

            cmd_name = Tuya.CMDS.get(cmd, '')
            if cmd == 0x6 and len == 8:
                parsed = f'CO2:{int.from_bytes(data[-4:])}'
            elif cmd == 0x1 and len == 28:
                parsed = data.decode()
            elif cmd == 0x2 and len == 1:
                parsed = {0: 'not paired', 1: 'paired', 2: 'exc', 3: 'pairing'}[data[0]]
            else:
                parsed = ''
            log_flush()
            print(f'{seq:04X} {cmd:02X}:{cmd_name} {len:3d}[{data.hex()}] {parsed} {csum == check}')


class Sensor:
    @staticmethod
    def wait_start():
        b = None
        i = 0
        while True:
            bp = b
            b = get_byte()
            i += 1
            if (bp, b) == (0x42, 0x4d):
                skipped = i - 2
                if skipped:
                    print('skipped', skipped)
                return

    @staticmethod
    def main():
        while True:
            Sensor.wait_start()
            inst = get_byte()
            assert inst == 0xA0
            cmd = get_int(2)
            len = get_int(2)
            data = get_bytes(len)
            csum = sum(log)
            check = get_int(2)
            cmd_name = Sensor.CMDS.get(cmd, '')
            if cmd == 0x3 and len == 5:
                parsed = f'CO2:{int.from_bytes(data[:-1])} V:{data[-1]}'
            else:
                parsed = ''
            log_flush()
            print(f'{get_time()} {cmd:02X}:{cmd_name} {len:3d}[{data.hex()}] {parsed} {csum == check}')


class Trim:
    @staticmethod
    def main():
        tp = None
        ts = None
        avg_sum = 0
        avg_num = 0
        while True:
            b = get_byte()
            t = time.time_ns()
            if ts is None:
                ts = t
            if tp is not None:
                d = t - tp
                avg_sum += d
                avg_num += 1
                avg = avg_sum / avg_num
                avg2 = (t - ts) / avg_num
                print(d, avg, avg2, avg_num, b)
            tp = t


if __name__ == '__main__':
    ser = serial.Serial('/dev/ttyUSB1', baudrate=9600)

    Sensor.main()
    # Tuya.main()
