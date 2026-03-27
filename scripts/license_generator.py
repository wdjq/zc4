#!/usr/bin/env python3
import sys

def generate(machine_id, key_value):
    kv = int(key_value, 0)
    val = (machine_id // 2) + kv
    return hex(val)[2:].upper()

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"用法: python3 {sys.argv[0]} <machine_id> <key_value>")
        print(f"示例: python3 {sys.argv[0]} 123456 0x9A3F")
        sys.exit(1)
    
    mid = int(sys.argv[1])
    kv = sys.argv[2]
    code = generate(mid, kv)
    print(f"Machine ID: {mid}")
    print(f"Key Value:  {kv} ({int(kv, 0)})")
    print(f"License:    {code}")
