import character

def main():
    with open("character.bin", "rb") as f:
        data = f.read()

    # Unpack the binary directly into a Python object!
    player, _ = character.Character.unpack(data)
    
    print("Successfully decoded from binary using Python Generator!")
    print(f"Name: {player.name}")
    print(f"Level: {player.level}")
    print(f"Faction: {player.faction}")
    print(f"Position: {player.position}")
    print(f"Inventory Items: {len(player.inventory)}")
    print(f"Attributes: {player.attributes}")
    
    # Test packing it back
    packed_data = player.pack()
    print(f"Repacked to {len(packed_data)} bytes. (Matches original: {data == packed_data})")

if __name__ == "__main__":
    main()
