#include <stddef.h> // For size_t
#include <stdint.h> // For uint32_t
#include <stdio.h>  // For printf (optional)
#include <Native.h>

namespace stuff
{
    namespace hash
    {
        class hash_t
        {
        public:
            using value_t = uint32_t;

            // Converting Constructor
            template <typename CharT>
            hash_t(const CharT *string, bool case_sensitive = false)
                : m_value(0), m_case_sensitive(case_sensitive)
            {
                generate(string);
            }

            // Explicit Constructor for Precomputed Hash Values
            constexpr hash_t(value_t hash, bool case_sensitive = false)
                : m_value(hash), m_case_sensitive(case_sensitive) {}

            hash_t() : m_value(0), m_case_sensitive(false) {}

            // Get Hash Value
            value_t get() const { return m_value; }

            // Comparison Operators
            bool operator==(const hash_t &other) const { return m_value == other.m_value; }
            bool operator!=(const hash_t &other) const { return !(*this == other); }

        private:
            // Hash Generation Method
            template <typename CharT>
            void generate(const CharT *string)
            {
                m_value = 2166136261u; // FNV-1a offset basis
                while (*string)
                {
                    CharT c = m_case_sensitive ? *string : to_lower(*string);
                    m_value ^= static_cast<value_t>(c);
                    m_value *= 16777619u; // FNV-1a prime
                    ++string;
                }
            }

            // Helper Method to Convert Character to Lowercase
            template <typename CharT>
            CharT to_lower(CharT c) const
            {
                if constexpr (sizeof(CharT) == 1)
                {
                    return (c >= 'A' && c <= 'Z') ? (c + 32) : c;
                }
                else if constexpr (sizeof(CharT) > 1)
                {
                    return (c >= L'A' && c <= L'Z') ? (c + 32) : c;
                }
                return c;
            }

            value_t m_value; // Hash value
            bool m_case_sensitive;
        };
    }

    namespace ldr
    {
        // Utility Function to Traverse the Module List
        template <typename Func>
        PLDR_DATA_TABLE_ENTRY traverse_module_list(Func callback)
        {
            PLIST_ENTRY First = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
            PLIST_ENTRY Current = First->Flink;

            while (Current != First)
            {
                PLDR_DATA_TABLE_ENTRY Entry = (PLDR_DATA_TABLE_ENTRY)Current;
                if (callback(Entry))
                {
                    return Entry;
                }
                Current = Current->Flink;
            }

            return nullptr;
        }

        PVOID FindModule(hash::hash_t Module)
        {
            return traverse_module_list([&](PLDR_DATA_TABLE_ENTRY Entry)
                                        { return hash::hash_t(Entry->BaseDllName.Buffer) == Module; })
                ->DllBase;
        }

        class module
        {
        public:
            // Constructor
            explicit module(PVOID Base) : base(Base) {}
            explicit module(const hash::hash_t &Name) : base(FindModule(Name)) {}

            PIMAGE_DOS_HEADER GetDosHdr()
            {
                return (PIMAGE_DOS_HEADER)base;
            }

            PIMAGE_NT_HEADERS GetNtHdr()
            {
                return (PIMAGE_NT_HEADERS)((UINT_PTR)base + GetDosHdr()->e_lfanew);
            }

            PVOID FindFunction(hash::hash_t Function)
            {
                PIMAGE_EXPORT_DIRECTORY ExportDir = (PIMAGE_EXPORT_DIRECTORY)((UINT_PTR)base + GetNtHdr()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
                PDWORD AddressOfNames = (PDWORD)((UINT_PTR)base + ExportDir->AddressOfNames);
                PDWORD AddressOfFunctions = (PDWORD)((UINT_PTR)base + ExportDir->AddressOfFunctions);
                PWORD AddressOfNameOrdinals = (PWORD)((UINT_PTR)base + ExportDir->AddressOfNameOrdinals);

                for (int i = 0; i < ExportDir->NumberOfNames; i++)
                {
                    PCHAR Name = (PCHAR)((UINT_PTR)base + AddressOfNames[i]);

                    if (hash::hash_t(Name) == Function)
                    {
                        return (PVOID)((UINT_PTR)base + AddressOfFunctions[AddressOfNameOrdinals[i]]);
                    }
                }

                return nullptr;
            }

            template <typename... Args>
            PVOID CallFunction(hash::hash_t Name, Args... args)
            {
                PVOID Function = FindFunction(Name);
                return ((PVOID(*)(...))Function)(args...);
            }

            PVOID base; // Base address of the module
        };

        inline module current()
        {
            return module(NtCurrentPeb()->ImageBaseAddress);
        }

    }
}
