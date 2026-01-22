import serial
import argparse
import csv
import re
import sys
import time
from datetime import datetime

def main():
    parser = argparse.ArgumentParser(description='ESP32 Energy Monitor Datalogger')
    parser.add_argument('--port', type=str, required=True, help='Serial port (e.g., COM3 or /dev/ttyUSB0)')
    parser.add_argument('--baud', type=int, default=115200, help='Baud rate (default: 115200)')
    parser.add_argument('--output', type=str, default='datalog.csv', help='Output CSV file name (default: datalog.csv)')
    
    args = parser.parse_args()

    print(f"Connecting to {args.port} at {args.baud} baud...")
    print(f"Saving data to {args.output}")

    # Regex to parse the line: 
    # V: 127.0 V, I: 0.500 A, P: 60.0 W, E: 1.000 kWh, F: 60.0 Hz, PF: 0.95
    # Adjusted to match the specific print format in main.c
    log_pattern = re.compile(
        r"V:\s*([\d.]+)\s*V,\s*"
        r"I:\s*([\d.]+)\s*A,\s*"
        r"P:\s*([\d.]+)\s*W,\s*"
        r"E:\s*([\d.]+)\s*kWh,\s*"
        r"F:\s*([\d.]+)\s*Hz,\s*"
        r"PF:\s*([\d.]+)"
    )

    try:
        ser = serial.Serial(args.port, args.baud, timeout=1)
        
        # Open CSV file in append mode or write mode? Write mode to start fresh, append if desired.
        # usually dataloggers append. Let's append if exists? Or just new file.
        # User request didn't specify, but safer to append or just write header if new.
        
        file_exists = False
        try:
            with open(args.output, 'r') as f:
                file_exists = True
        except FileNotFoundError:
            file_exists = False
            
        with open(args.output, 'a', newline='') as csvfile:
            writer = csv.writer(csvfile)
            
            if not file_exists:
                writer.writerow(['Timestamp', 'Voltage (V)', 'Current (A)', 'Power (W)', 'Energy (kWh)', 'Frequency (Hz)', 'Power Factor'])
            
            print("Logging started. Press Ctrl+C to stop.")
            
            while True:
                try:
                    if ser.in_waiting > 0:
                        line = ser.readline().decode('utf-8', errors='replace').strip()
                        if line:
                            print(f"RX: {line}")
                            
                            # Check if line matches data format
                            match = log_pattern.search(line)
                            if match:
                                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                                voltage = match.group(1)
                                current = match.group(2)
                                power = match.group(3)
                                energy = match.group(4)
                                frequency = match.group(5)
                                pf = match.group(6)
                                
                                writer.writerow([timestamp, voltage, current, power, energy, frequency, pf])
                                csvfile.flush() # Ensure data is written
                                print(f"Logged: {timestamp} -> P: {power}W")
                                
                except UnicodeDecodeError:
                    pass # Ignore decode errors
                    
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\nLogging stopped by user.")
    except Exception as e:
        print(f"\nAn error occurred: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial port closed.")

if __name__ == "__main__":
    main()
