#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include "SerialPort.hpp"

using namespace std;

// Authorized UID 
const string AUTHORIZED_UID = "19 B4 C7 55";

const char* portName = "\\\\.\\COMx"; // initialize the port where X is the number of the port (EX COM4)

//Declare a global object
SerialPort* arduino;

#include <windows.h>
#include <string>

void launchApplication() {
    std::string appPath = "X:\\FOLDER\\APPLICATION FOLDER\\app.exe"; // copy the path of the executable (keep the double slashes \\ instead of one)

    // Convert `std::string` (ASCII) to `std::wstring` (Unicode)
    int len;
    int slength = (int)appPath.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, appPath.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, appPath.c_str(), slength, buf, len);
    std::wstring appPathW(buf);
    delete[] buf;

    // Call ShellExecute with the Unicode string
    ShellExecute(NULL, L"open", appPathW.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

string cleanString(const string& str) {
    string cleaned;
    for (char c : str) {
        if (!isspace(c)) { // Remove all whitespace characters
            cleaned += c;
        }
    }
    return cleaned;
}


int main() {

    arduino = new SerialPort(portName);
    cout << "Is connected: " << arduino->isConnected() << std::endl;

    if (!arduino->isConnected()) {
        cout << "The connection could not be realised to Arduino in the specified port ( " << portName << " )" << endl;
        return 1;
    }

    cout << "Waiting to scan the RFID..." << endl;

    while (true) {
        // Reading Arduino data
        char buffer[256];
        int bytesRead = arduino->readSerialPort(buffer, sizeof(buffer) - 1);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Finishing the string
            string data(buffer);

            cout << "Received data: " << data << endl;

            // Get UID if it is found
            if (data.find("UID tag :") != string::npos) {
                // Extract UID after "UID tag :"
                string uid = data.substr(data.find("UID tag :") + 9); // Skip "UID tag :"
                size_t endPos = uid.find("Message");
                if (endPos != string::npos) {
                    uid = uid.substr(0, endPos); // Extract only the UID part
                }

                // Remove unwanted characters (e.g., spaces, \r, \n)
                uid.erase(remove(uid.begin(), uid.end(), '\r'), uid.end());
                uid.erase(remove(uid.begin(), uid.end(), '\n'), uid.end());
                uid.erase(remove(uid.begin(), uid.end(), ' '), uid.end()); // Remove spaces

                // Normalize UID for comparison
                string cleanedUID = cleanString(uid);
                string cleanedAuthorizedUID = cleanString(AUTHORIZED_UID);

                cout << "Cleaned UID: '" << cleanedUID << "'" << endl;
                cout << "Cleaned Authorized UID: '" << cleanedAuthorizedUID << "'" << endl;

                if (cleanedUID == cleanedAuthorizedUID) {
                    cout << "Access authorized! Launching the application..." << endl;
                    launchApplication();
                    break;
                }
                else {
                    cout << "Access denied! Unknown UID." << endl;
                }
            }
        }
    }

    return 0;
}
