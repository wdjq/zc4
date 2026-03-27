#!/usr/bin/env python3
import sys
import os

def patch(so_path, key_value, target):
    with open(so_path, 'rb') as f:
        data = bytearray(f.read())
    
    magic = b"CFGBLK"
    offset = data.find(magic)
    if offset == -1:
        raise ValueError("未找到配置魔数 CFGBLK")
    
    print(f"[+] 配置区偏移: 0x{offset:x}")
    
    # KEY_VALUE: 魔数后 6 字节，长度 10
    key_offset = offset + 6
    key_bytes = key_value.encode().ljust(10, b' ')[:10]
    data[key_offset:key_offset+10] = key_bytes
    
    # TARGET: 再后 16 字节（对齐），长度 50
    target_offset = offset + 6 + 16
    target_bytes = target.encode().ljust(50, b' ')[:50]
    data[target_offset:target_offset+50] = target_bytes
    
    with open(so_path, 'wb') as f:
        f.write(data)
    
    print(f"[✓] 已修补:")
    print(f"    KEY_VALUE: {key_value}")
    print(f"    TARGET:    {target}")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print(f"用法: python3 {sys.argv[0]} <so文件> <KEY_VALUE> <目标Activity>")
        print(f"示例: python3 {sys.argv[0]} arm64-v8a.so 0x9A3F com.yourapp.MainActivity")
        sys.exit(1)
    patch(sys.argv[1], sys.argv[2], sys.argv[3])
