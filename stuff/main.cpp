#include <stuff.hpp>
#include <Windows.h>
#include <iostream>
using namespace std;

int main()
{
    stuff::hash::hash_t aaa("aAMAMA", true);
    // aaa.print();

    stuff::hash::hash_t bbb("aAMAMA", false);
    // bbb.print();
    // hash_t aaa("amaAAA", true);
    // cout << aaa << endl;

    // hash_t aaba("amaAAA", false);
    // cout << aaa << endl;

    stuff::ldr::module Mod = stuff::ldr::module("kernel32.dll");

    Mod.FindFunction("ExitProcess");
    printf("%p", Mod.FindFunction("ExitProcess"));
    printf("\n%p", stuff::ldr::module("ntdll.dll").FindFunction("NtTerminateProcess"));

    Mod.CallFunction("LoadLibraryA", "user32.dll");
    stuff::ldr::module("user32.dll").CallFunction("MessageBoxA", 0, "aka", "akak", 0);

    return 0;
}
