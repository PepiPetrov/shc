# # import ctypes
# # import sys
# # import struct

# # class COFFARG:
# #     def __init__(self, value, size, include_size):
# #         self.value = value
# #         self.size = size
# #         self.include_size = include_size

# # def prepare_coff_args(args):
# #     full_size = 0
# #     arg_list = []

# #     for arg in args:
# #         if arg.startswith("str:"):
# #             # String argument
# #             str_value = arg[4:].encode('utf-8') + b'\x00'  # UTF-8 string with null terminator
# #             arg_list.append(COFFARG(str_value, len(str_value), True))
# #         elif arg.startswith("int:"):
# #             # Integer argument
# #             int_value = struct.pack("i", int(arg[4:]))
# #             arg_list.append(COFFARG(int_value, len(int_value), False))
# #         else:
# #             print(f"Unknown argument type: {arg}")
# #             return None, 0

# #         full_size += (4 if arg_list[-1].include_size else 0) + arg_list[-1].size

# #     # Allocate memory
# #     output = ctypes.create_string_buffer(full_size + 4)

# #     # Write the size of the entire block
# #     struct.pack_into("I", output, 0, full_size + 4)

# #     # Write each argument
# #     offset = 4
# #     for arg in arg_list:
# #         if arg.include_size:
# #             struct.pack_into("I", output, offset, arg.size)
# #             offset += 4
# #         ctypes.memmove(ctypes.addressof(output) + offset, arg.value, arg.size)
# #         offset += arg.size

# #     return output, full_size + 4

# # def main():
# #     if len(sys.argv) < 2:
# #         print("COFF Arguments Generator")
# #         print("Usage: {} str:<some-string> for utf8 string int:<some-int> for int. Can have multiple arguments passed".format(sys.argv[0]))
# #         return 1

# #     # Skip the first argument (program name), and pass the rest to prepare_coff_args
# #     output, size = prepare_coff_args(sys.argv[1:])

# #     if output is not None:
# #         # Print the result in the format
#         # print('LPSTR args = "', end="")
#         # for byte in output.raw[:size]:
#         #     print(f"\\x{byte:02X}", end="")
#         # print('";')
#         # print(f"SIZE_T size = {size};")

# # if __name__ == "__main__":
# #     main()

# from struct import pack, calcsize
# import binascii
# import cmd

# class BeaconPack:
#     def __init__(self):
#         self.buffer = b''
#         self.size = 0

#     def getbuffer(self):
#         return pack("<L", self.size) + self.buffer

#     def addshort(self, short):
#         self.buffer += pack("<h", short)
#         self.size += 2

#     def addint(self, dint):
#         self.buffer += pack("<i", dint)
#         self.size += 4

#     def addstr(self, s):
#         if isinstance(s, str):
#             s = s.encode("utf-8")
#         fmt = "<L{}s".format(len(s) + 1)
#         self.buffer += pack(fmt, len(s)+1, s)
#         self.size += calcsize(fmt)

#     def addWstr(self, s):
#         if isinstance(s, str):
#             s = s.encode("utf-16_le")
#         fmt = "<L{}s".format(len(s) + 2)
#         self.buffer += pack(fmt, len(s)+2, s)
#         self.size += calcsize(fmt)

# class MainLoop(cmd.Cmd):
#     def __init__(self):
#         cmd.Cmd.__init__(self)
#         self.BeaconPack = BeaconPack()
#         self.intro = "Beacon Argument Generator"
#         self.prompt = "Beacon>"

#     def do_addWString(self, text):
#         '''addWString String here
#         Append the wide string to the text.
#         '''
#         self.BeaconPack.addWstr(text)

#     def do_addString(self, text):
#         '''addString string here
#         Append the utf-8 string here.
#         '''
#         self.BeaconPack.addstr(text)

#     def do_generate(self, text):
#         '''generate
#         Generate the buffer for the BOF arguments
#         '''
#         outbuffer = self.BeaconPack.getbuffer()
#         size = self.BeaconPack.size

#         print('LPSTR args = "', end="")
#         for byte in outbuffer[:size]:
#             print(f"\\x{byte:02X}", end="")
#         print('";')
#         print(f"SIZE_T size = {size};")

#     def do_addint(self, text):
#         '''addint integer
#         Add an int32_t to the buffer
#         '''
#         try:
#             converted = int(text)
#             self.BeaconPack.addint(converted)
#         except:
#             print("Failed to convert to int\n")

#     def do_addshort(self, text):
#         '''addshort integer
#         Add an uint16_t to the buffer
#         '''
#         try:
#             converted = int(text)
#             self.BeaconPack.addshort(converted)
#         except:
#             print("Failed to convert to short\n")

#     def do_reset(self, text):
#         '''reset
#         Reset the buffer here.
#         '''
#         self.BeaconPack.buffer = b''
#         self.BeaconPack.size = 0

#     def do_exit(self, text):
#         '''exit
#         Exit the console
#         '''
#         return True

# if __name__ == "__main__":
#     cmdloop = MainLoop()
#     cmdloop.cmdloop()

from struct import pack, calcsize
import binascii
import cmd

class BeaconPack:
    def __init__(self):
        self.buffer = b''
        self.size = 0

    def getbuffer(self):
        """Returns the buffer with the total size packed at the beginning."""
        return pack("<L", self.size) + self.buffer

    def addshort(self, short):
        """Adds a 2-byte short value to the buffer."""
        self.buffer += pack("<h", short)
        self.size += 2

    def addint(self, dint):
        """Adds a 4-byte integer to the buffer."""
        self.buffer += pack("<i", dint)
        self.size += 4

    def addstr(self, s):
        """Adds a UTF-8 string to the buffer."""
        if isinstance(s, str):
            s = s.encode("utf-8")
        length = len(s) + 1  # Null-terminated string
        fmt = "<L{}sB".format(len(s))
        self.buffer += pack(fmt, length, s, 0)
        self.size += calcsize(fmt)

    def addwstr(self, s):
        """Adds a UTF-16 wide string to the buffer."""
        s_wide = s.encode("utf-16_le")  # Encode as UTF-16 LE (Little Endian)
        length = len(s_wide) + 2  # Null-terminated wide string
        fmt = "<L{}s".format(len(s_wide))
        self.buffer += pack(fmt, length, s_wide + b'\x00\x00')  # Null terminator for UTF-16
        self.size += calcsize(fmt)

    def addbin(self, data):
        """Adds a binary data block to the buffer."""
        length = len(data)
        fmt = "<L{}s".format(length)
        self.buffer += pack(fmt, length, data)
        self.size += calcsize(fmt)

    def reset(self):
        """Resets the buffer, clearing all data and resetting the size to 0."""
        self.buffer = b''
        self.size = 0

class MainLoop(cmd.Cmd):
    def __init__(self):
        cmd.Cmd.__init__(self)
        self.BeaconPack = BeaconPack()
        self.intro = "Beacon Argument Generator"
        self.prompt = "Beacon>"

    def do_addWString(self, text):
        """addWString String here
        Append the wide string to the text.
        """
        self.BeaconPack.addwstr(text)

    def do_addString(self, text):
        """addString string here
        Append the utf-8 string here.
        """
        self.BeaconPack.addstr(text)

    def do_generate(self, text):
        """generate
        Generate the buffer for the BOF arguments
        """
        outbuffer = self.BeaconPack.getbuffer()
        size = self.BeaconPack.size

        print('LPSTR args = "', end="")
        for byte in outbuffer[:size]:
            print(f"\\x{byte:02X}", end="")
        print('";')
        print(f"SIZE_T size = {size};")

    def do_addint(self, text):
        """addint integer
        Add an int32_t to the buffer
        """
        try:
            converted = int(text)
            self.BeaconPack.addint(converted)
        except:
            print("Failed to convert to int\n")

    def do_addshort(self, text):
        """addshort integer
        Add an uint16_t to the buffer
        """
        try:
            converted = int(text)
            self.BeaconPack.addshort(converted)
        except:
            print("Failed to convert to short\n")

    def do_addbin(self, text):
        """addbin data
        Add binary data to the buffer
        """
        try:
            data = binascii.unhexlify(text)
            self.BeaconPack.addbin(data)
        except:
            print("Failed to convert to binary data\n")

    def do_reset(self, text):
        """reset
        Reset the buffer here.
        """
        self.BeaconPack.reset()

    def do_exit(self, text):
        """exit
        Exit the console
        """
        return True

if __name__ == "__main__":
    cmdloop = MainLoop()
    cmdloop.cmdloop()
