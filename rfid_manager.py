import serial
import time
import os
import threading

PORT = "COM6"
BAUD = 9600
current_file = ""

def load_cards():
    if not current_file or not os.path.exists(current_file):
        return []
    with open(current_file, "r") as f:
        return [line.strip() for line in f if line.strip()]

def save_card(card_uid):
    with open(current_file, "a") as f:
        f.write(card_uid + "\n")

def display_all():
    cards = load_cards()
    print("\n" + "=" * 50)
    print(f"Current Database: [{current_file}] ({len(cards)} cards)")
    if not cards:
        print("[The file is empty or no cards registered yet]")
    else:
        for idx, card in enumerate(cards, 1):
            print(f"  {idx}. {card}")
    print("=" * 50 + "\n")

def serial_listener(ser):
    while True:
        try:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line and "Card UID:" in line:
                card_uid = line.split("Card UID:")[1].strip()
                cards = load_cards()
                
                if card_uid in cards:
                    print(f"\n[SENSOR] Card already registered in [{current_file}]: [ {card_uid} ] (Access Granted)\n> ", end="")
                else:
                    save_card(card_uid)
                    print(f"\n[SENSOR] >> New card detected and saved to [{current_file}]: [ {card_uid} ]\n> ", end="")
        except Exception:
            break

def main():
    global current_file

    print("\n" + "#" * 50)
    print("        RFID Access Management System")
    print("#" * 50)

    user_file = input("Enter database file name (e.g. you.log or cards.txt): ").strip()
    if not user_file:
        current_file = "database.txt"
    else:
        current_file = user_file

    print(f"--> Using file: {current_file}")
    display_all()

    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        time.sleep(1)
        print(f"Successfully connected to the board via {PORT}.\n")
    except Exception:
        print(f"Error: Unable to open {PORT}! Ensure PuTTY and AVRDUDESS are closed.")
        return

    t = threading.Thread(target=serial_listener, args=(ser,), daemon=True)
    t.start()

    print("Available Commands:")
    print(" - Enter UID to search (e.g. 23:CA:61:F5)")
    print(" - Type 'list' to view all registered cards")
    print(" - Type 'exit' to quit the application")
    print("-" * 50)

    while True:
        try:
            cmd = input("> ").strip()
            if not cmd:
                continue
            if cmd.lower() == 'exit':
                break
            elif cmd.lower() == 'list':
                display_all()
            else:
                cards = load_cards()
                if cmd.upper() in [c.upper() for c in cards]:
                    print(f"Search Result: Card [ {cmd} ] is ALREADY registered in {current_file}.")
                else:
                    print(f"Search Result: Card [ {cmd} ] NOT found in {current_file}!")
        except KeyboardInterrupt:
            break

    ser.close()
    print("Connection closed. Goodbye!")

if __name__ == "__main__":
    main()