# import gzip
# import io
# import nbtlib

# # Path to your level.dat file
# level_dat_file_path = 'level.dat'

# # Read and decompress the gzipped file
# with gzip.open(level_dat_file_path, 'rb') as f:
#     decompressed_data = f.read()

# # Use a BytesIO stream to pass the decompressed data to nbtlib
# nbt_data = nbtlib.File.parse(io.BytesIO(decompressed_data))

# # Access the player's inventory
# player_data = nbt_data.get('Data', {}).get('Player', {})
# inventory = player_data.get('Inventory', [])

# # Print the inventory details including enchantments
# for item in inventory:
#     item_id = item.get('id', 'unknown')
#     item_count = item.get('Count', 1)
#     item_slot = item.get('Slot', 'unknown')

#     # Check for enchantments in the "tag" field
#     tag = item.get('tag', {})
#     enchantments = tag.get('Enchantments', [])

#     print(f"Item ID: {item_id}, Count: {item_count}, Slot: {item_slot}")

#     if enchantments:
#         print("  Enchantments:")
#         for enchantment in enchantments:
#             ench_id = enchantment.get('id', 'unknown')
#             lvl = enchantment.get('lvl', 0)
#             print(f"    {ench_id} (Level: {lvl})")
#     else:
#         print("  No Enchantments")

import gzip
import io
import nbtlib

# Path to your level.dat file
level_dat_file_path = 'level.dat'

# Read and decompress the gzipped file
with gzip.open(level_dat_file_path, 'rb') as f:
    decompressed_data = f.read()

# Use a BytesIO stream to pass the decompressed data to nbtlib
nbt_data = nbtlib.File.parse(io.BytesIO(decompressed_data))

# Function to recursively dump NBT data
def dump_nbt_data(data, indent=0):
    spacing = "  " * indent
    if isinstance(data, dict):
        for key, value in data.items():
            print(f"{spacing}{key}:")
            dump_nbt_data(value, indent + 1)
    elif isinstance(data, list):
        for index, item in enumerate(data):
            print(f"{spacing}[{index}]:")
            dump_nbt_data(item, indent + 1)
    else:
        print(f"{spacing}{data}")

# Access the player's inventory
player_data = nbt_data.get('Data', {}).get('Player', {})
inventory = player_data.get('Inventory', [])

# Dump all attributes and properties of each item in the inventory
for i, item in enumerate(inventory):
    print(f"Item {i+1}:")
    dump_nbt_data(item)
    print("\n" + "="*40 + "\n")
