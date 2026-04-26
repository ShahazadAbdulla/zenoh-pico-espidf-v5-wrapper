import zenoh
import time
import struct
import sys

def main():
    print("--- Zenoh 1.0.0+ Python Peer ---")
    
    conf = zenoh.Config()
    conf.insert_json5("listen/endpoints", '["udp/0.0.0.0:7447"]')
    
    print("[INFO] Opening Zenoh Session...")
    try:
        session = zenoh.open(conf)
    except Exception as e:
        print(f"[ERROR] Failed to open session: {e}")
        sys.exit(1)

    def pong_handler(sample):
        raw_bytes = sample.payload.to_bytes()
        data = struct.unpack('<q', raw_bytes)[0]
        print(f"[RECV] PONG bounced back from ESP32: {data}")

    print("[INFO] Declaring Subscriber on 'pong'...")
    sub = session.declare_subscriber("pong", pong_handler)

    print("[INFO] Declaring Publisher on 'ping'...")
    pub = session.declare_publisher("ping")

    print("\n[INFO] Peer is listening. Turn on your ESP32 now.")
    print("[INFO] Waiting 5 seconds for ESP32 to connect...\n")
    time.sleep(5)

    counter = 1
    try:
        while True:
            payload = struct.pack('<q', counter)
            print(f"[SEND] Firing PING: {counter}")
            
            pub.put(payload)
            
            counter += 1
            time.sleep(1) 
            
    except KeyboardInterrupt:
        print("\n[INFO] Stopping stress test.")
    finally:
        sub.undeclare()
        pub.undeclare()
        session.close()

if __name__ == "__main__":
    main()
