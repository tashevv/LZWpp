#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <windows.h>
#include <functional>
#include <iomanip>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <clocale>
#include <codecvt>

// Utility functions for various common tasks
class Utility {

public:
    // Converts a UTF-8 encoded std::string to a UTF-32 encoded std::u32string
    static std::u32string stringToU32String(const std::string& str) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        return converter.from_bytes(str);
    }

    // Converts a UTF-32 encoded std::u32string to a wide string (std::wstring)
    static std::wstring u32stringToWstring(const std::u32string& u32str) {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        std::string utf8_str = converter.to_bytes(u32str);

        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> utf16_converter;
        return utf16_converter.from_bytes(utf8_str);
    }

    // Converts a wide string (std::wstring) to a UTF-32 encoded std::u32string
    static std::u32string wstringToU32string(const std::wstring& wstr) {
        // Convert std::wstring (wide string) to UTF-8 encoded std::string
        std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_converter;
        std::string utf8_str = utf8_converter.to_bytes(wstr);

        // Convert UTF-8 encoded std::string to UTF-32 encoded std::u32string
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> u32_converter;
        return u32_converter.from_bytes(utf8_str);
    }

    // Reads a file into a vector<uint8_t>
    static void readFileToVector(const std::u32string& filename, std::vector<uint8_t>& data) {
        std::wstring wfilename = u32stringToWstring(filename); // Convert UTF-32 string path to wide string

        std::ifstream file(wfilename, std::ios::binary | std::ios::ate); // Open file and seek to the end
        if (!file) {
            throw std::exception("Error opening file for reading.");
        }

        // Get the size of the file
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg); // Move back to the beginning of the file

        // Resize the vector to accommodate the data
        data.resize(static_cast<size_t>(size));

        // Read the vector data from the file
        if (size > 0) {
            file.read(reinterpret_cast<char*>(data.data()), size);
        }

        file.close();
    }

    // Writes a vector<uint8_t> to a file
    static void writeVectorToFile(const std::u32string& filename, const std::vector<uint8_t>& data) {
        std::wstring wfilename = u32stringToWstring(filename); // Convert UTF-32 string path to wide string

        std::ofstream file(wfilename, std::ios::binary);
        if (!file) {
            throw std::exception("Error opening file for writing.");
        }

        // Write the vector data to the file
        if (!data.empty()) {
            file.write(reinterpret_cast<const char*>(data.data()), data.size());
        }

        file.close();
    }

    // Appends the contents of 'src' vector to the end of 'dest' vector
    static void appendVector(std::vector<uint8_t>& dest, const std::vector<uint8_t>& src) {
        // Use the insert method to concatenate src to dest
        dest.insert(dest.end(), src.begin(), src.end());
    }

    // Function to split input into file path without extension, extension, and option
    static std::vector<std::u32string> splitInputFilePathAndOption(const std::u32string& input) {
        std::vector<std::u32string> result;

        // Find the last space character to separate the option
        std::u32string::size_type spacePos = input.rfind(U' ');

        if (spacePos == std::u32string::npos) {
            throw std::exception("Cannot parse input: no space found.");
        }

        // Extract the file path part (before the option)
        std::u32string filePathPart = input.substr(0, spacePos);
        // Extract the option part (after the last space)
        std::u32string option = input.substr(spacePos + 1);

        // Convert UTF-32 file path to UTF-8 string for easier manipulation
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
        std::string filePathUtf8 = converter.to_bytes(filePathPart);

        // Find the last dot character to separate the extension
        std::size_t dotPos = filePathUtf8.find_last_of('.');

        // Initialize file path without extension and extension
        std::u32string filePathWithoutExtension;
        std::u32string extension;

        if (dotPos == std::string::npos) {
            // No extension found
            filePathWithoutExtension = filePathPart;
            extension = U"";
        }
        else {
            // Convert back to UTF-32 strings for the result
            filePathWithoutExtension = converter.from_bytes(filePathUtf8.substr(0, dotPos)); // Path without extension
            extension = converter.from_bytes(filePathUtf8.substr(dotPos)); // Extension, including the dot
        }

        // Add the results to the vector
        result.push_back(filePathWithoutExtension); // File path without extension
        result.push_back(extension);                // Extension
        result.push_back(option);                   // Option

        return result;
    }

    // Checks if an index is within the bounds of a vector
    template <typename T>
    static bool isIndexInRange(const std::vector<T>& vec, size_t n) {
        return n < vec.size();
    }

    // Checks if a file exists at the given UTF-32 encoded path
    static bool fileExists(const std::u32string& path) {
        std::wstring wpath = u32stringToWstring(path); // Convert UTF-32 string path to wide string

        struct _stat buffer;
        return (_wstat(wpath.c_str(), &buffer) == 0);
    }

    // Function to verify if the input option is valid
    static bool isValidOption(const std::u32string& option) {
        // Check if the extension starts with a dot
        if (option == U"-c" || option == U"-d") return true;
        else return false;
    }

    // Retrieves the size of a file without opening it
    static std::uintmax_t getFileSize(const std::u32string& filePath) {
        // Convert the UTF-32 file path to a wide string (wstring)
        std::wstring wfilePath = u32stringToWstring(filePath);

        struct _stat stat_buf;
        int rc = _wstat(wfilePath.c_str(), &stat_buf);
        return rc == 0 ? static_cast<std::uintmax_t>(stat_buf.st_size) : static_cast<std::uintmax_t>(0);
    }

    // Function to convert a string to a vector of uint8_t
    static std::vector<uint8_t> stringToBytes(const std::u32string& str) {
        // Create a vector and copy the bytes of the string into it
        std::vector<uint8_t> byteVector(str.begin(), str.end());
        return byteVector;
    }

    // Function to convert a vector of uint8_t back to a string
    static std::u32string bytesToString(const std::vector<uint8_t>& byteVector) {
        // Create a string from the byte vector
        return std::u32string(byteVector.begin(), byteVector.end());
    }

    // Function to find the last occurrence of the '.' character in a vector of bytes
    static std::ptrdiff_t findLastDot(const std::vector<uint8_t>& data) {
        // The byte value for the '.' character in ASCII/UTF-8
        uint8_t dotChar = '.';

        // Iterate from the end to the beginning to find the last occurrence
        for (std::ptrdiff_t i = data.size() - 1; i >= 0; --i) {
            if (data[i] == dotChar) {
                return i; // Return the index of the last dot
            }
        }

        // Return -1 if the dot is not found
        return -1;
    }

    // Efficient extaction of sub-vectors
    static std::vector<uint8_t> extractSubVector(const std::vector<uint8_t>& source, std::size_t start, std::size_t length) {
        if (start >= source.size()) {
            throw std::out_of_range("Start index is out of range.");
        }
        // Ensure the length does not exceed the bounds of the source vector
        std::size_t end = start + length < source.size() ? start + length : source.size(); // Set to the minimum
        return std::vector<uint8_t>(source.begin() + start, source.begin() + end);
    }
};

// Graphic User Interface (GUI) handling various display functions
class GUI {

public:
    // Clears the console screen by filling it with spaces and resetting the cursor position
    static void clearScreen() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD coordScreen = { 0, 0 };    // Coordinates for the top-left corner of the screen
        DWORD charsWritten;
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        DWORD consoleSize;

        // Retrieve the current screen buffer information
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        consoleSize = csbi.dwSize.X * csbi.dwSize.Y;

        // Fill the entire screen buffer with spaces
        FillConsoleOutputCharacter(hConsole, ' ', consoleSize, coordScreen, &charsWritten);

        // Restore the screen attributes (colors and other attributes)
        FillConsoleOutputAttribute(hConsole, csbi.wAttributes, consoleSize, coordScreen, &charsWritten);

        // Move the cursor back to the top-left corner of the screen
        SetConsoleCursorPosition(hConsole, coordScreen);
    }

    // Returns the ASCII art for the Main screen of the GUI
    static const std::string& drawMainScreen() {
        static const std::string mainScreen =
            "  __________________________________________________________________ \n"
            " /                                                                  \\ \n"
            " | General purpose Lempel-Ziv-Welch compressor                      | \n"
            " |                                                                  | \n"
            " | Compress     D:\\Folder\\File.xyz -c -> D:\\Folder\\File.bin         | \n"
            " | Decompress   D:\\Folder\\File.bin -d -> D:\\Folder\\File Decoded.xyz | \n"
            " |__________________________________________________________________| \n"
            " |                                                                  | \n"
            " | Input:                                                           | \n"
            " \\__________________________________________________________________/ \n"
            "  __________________________________________________________________   \n"
            " /                                                                  \\  \n"
            " | Progress: [--------------------] 0 %       Time: 00:00:00        | \n"
            " |                                                                  | \n"
            " | Size: - / -                               Speed: -               | \n"
            " |__________________________________________________________________| \n"
            " |                                                                  | \n"
            " | Output: -                                                        | \n"
            " \\__________________________________________________________________/ \n"
            "                                                                       "
            ;

        return mainScreen;
    }

    // Returns the ASCII art for the Error screen of the GUI
    static const std::string& drawErrorScreen() {
        static const std::string errorScreen =
            "  __________________________________________________________________   \n"
            " /                                                                  \\  \n"
            " | Error:                                                           | \n"
            " \\__________________________________________________________________/ \n"
            "                                                                       "
            ;

        return errorScreen;
    }

    // Generates a progress bar as a string representation
    // The progress bar is visualized with '#' characters for completed progress and '-' for remaining progress
    static std::string drawProgressBar(double progressBar) {
        // Create the progress bar using '#' and '-' characters
        return "[" + std::string(static_cast<int>(std::round(progressBar * 20)), '#') +
            std::string(static_cast<int>(std::round((1 - progressBar) * 20)), '-') +
            "] " + std::to_string(static_cast<int>(std::round(progressBar * 100))) + " %";
    }

    // Formats the size of a file or data for display
    // Sizes are formatted as Bytes, KB, MB, or GB depending on the magnitude
    static std::string drawSizeField(size_t size) {
        std::ostringstream oss;

        // Format size with appropriate units
        if (size < 1024) {
            oss << size << " Bytes";
        }
        else if (size < 1024 * 1024) {
            oss << std::fixed << std::setprecision(2) << (size / 1024.0) << " KB";
        }
        else if (size < 1024 * 1024 * 1024) {
            oss << std::fixed << std::setprecision(2) << (size / (1024.0 * 1024.0)) << " MB";
        }
        else {
            oss << std::fixed << std::setprecision(2) << (size / (1024.0 * 1024.0 * 1024.0)) << " GB";
        }

        return oss.str();
    }

    // Formats the data transfer speed for display
    // Speeds are formatted as Bytes/s, KB/s, MB/s, or GB/s depending on the magnitude
    static std::string drawSpeed(size_t size, double duration) {
        if (duration <= 0) {
            duration = 1; // Handle invalid duration input
        }

        double speed = (size / duration); // Calculate speed in Bytes per second
        std::ostringstream oss;

        // Format speed with appropriate units
        if (speed < 1024) {
            oss << std::fixed << std::setprecision(2) << speed << " Bytes/s";
        }
        else if (speed < 1024 * 1024) {
            oss << std::fixed << std::setprecision(2) << (speed / 1024.0) << " KB/s";
        }
        else if (speed < 1024 * 1024 * 1024) {
            oss << std::fixed << std::setprecision(2) << (speed / (1024.0 * 1024.0)) << " MB/s";
        }
        else {
            oss << std::fixed << std::setprecision(2) << (speed / (1024.0 * 1024.0 * 1024.0)) << " GB/s";
        }

        return oss.str();
    }
};

// Cursor manipulation for managing console cursor and screen interactions
class Cursor {

public:
    // Retrieves the coordinates of the last non-space character on the console screen
    static COORD getLastConsoleChar() {
        // Obtain the handle to the console
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        // Initialize structure to store console screen buffer info
        CONSOLE_SCREEN_BUFFER_INFO csbi;

        // Get information about the console screen buffer
        if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
            // Calculate total number of characters in the console screen buffer
            DWORD consoleSize = csbi.dwSize.X * csbi.dwSize.Y;

            // Allocate buffer to store console screen contents
            CHAR_INFO* buffer = new CHAR_INFO[consoleSize];

            COORD bufferSize = csbi.dwSize;
            COORD bufferCoord = { 0, 0 };
            SMALL_RECT readRegion = { 0, 0, csbi.dwSize.X - 1, csbi.dwSize.Y - 1 };

            // Read console screen buffer into the allocated buffer
            if (ReadConsoleOutput(hConsole, buffer, bufferSize, bufferCoord, &readRegion)) {
                // Search buffer in reverse to find the last non-space character
                for (int i = consoleSize - 1; i >= 0; --i) {
                    if (buffer[i].Char.AsciiChar != ' ') {
                        // Determine the coordinates of the last non-space character
                        COORD lastCharPos = {
                            static_cast<SHORT>(i % csbi.dwSize.X),
                            static_cast<SHORT>(i / csbi.dwSize.X)
                        };

                        delete[] buffer;
                        return lastCharPos;
                    }
                }
            }

            delete[] buffer;
        }

        // Return {0, 0} if no non-space character is found
        return { 0, 0 };
    }

    // Moves the cursor to the specified (x, y) position in the console
    static void goTo(int x, int y) {
        COORD coord = { x, y };
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
    }

    // Reads input from the console with support for line wrapping
    // If maxLineLength is reached, wraps to the next line
    // Optionally triggers a callback when the line changes
    static std::u32string readInputStream(int maxLineLength, std::function<void(bool direction)> lineChangeCallback = nullptr) {
        COORD initialPos = getCurrentCursorPosition();

        int startX = initialPos.X;
        int startY = initialPos.Y;
        int currentX = startX;
        int currentY = startY;

        int lineCount = 1;

        std::wstring input;
        wchar_t ch;

        while (true) {
            ch = _getwch();  // Read wide character input

            if (ch == L'\r') {  // Enter key
                break;
            }
            else if (ch == L'\b' || ch == 8) {  // Backspace key
                if (!input.empty()) {
                    input.pop_back();

                    if (currentX <= startX) {
                        // Move back to the previous line if the current line is empty
                        lineCount--;
                        currentX = startX + maxLineLength - 1;
                        currentY--;

                        goTo(currentX, currentY);
                        std::wcout << L' ' << L' '; // Erase character

                        if (lineChangeCallback) lineChangeCallback(0); // Notify line change

                        goTo(currentX, currentY);
                    }
                    else
                    {
                        goTo(--currentX, currentY);
                        std::wcout << L' ' << L' ';  // Erase character

                        goTo(currentX, currentY);
                    }
                }
            }
            else if (ch == L'\"') { // Skip quote characters
                continue;
            }
            else {
                if ((float)input.size() / lineCount >= maxLineLength) {
                    // Move to the next line if the max length is reached
                    lineCount++;
                    currentX = startX;
                    currentY++;

                    if (lineChangeCallback) lineChangeCallback(1); // Notify line change

                    goTo(currentX, currentY);
                }
                input += ch;
                std::wcout << ch;

                currentX++;
            }
        }

        goTo(0, currentY + 2);  // Move cursor to a new line for subsequent output

        return Utility::wstringToU32string(input); // Convert input to UTF-32 string
    }

    // Writes output to the console with support for line wrapping
    // If maxLineLength is reached, wraps to the next line
    // Optionally triggers a callback when the line changes
    static std::u32string writeOutputStream(const std::u32string& text, int maxLineLength, std::function<void()> lineChangeCallback = nullptr) {
        COORD initialPos = getCurrentCursorPosition();

        int startX = initialPos.X;
        int startY = initialPos.Y;
        int currentX = startX;
        int currentY = startY;

        int lineCount = 1;

        std::u32string output;
        size_t pos = 0;
        size_t textLength = text.length();

        while (pos < textLength) {
            wchar_t ch = text[pos++];

            if ((float)output.length() / lineCount >= maxLineLength - 1) {
                // Move to the next line if the max length is reached
                lineCount++;
                currentX = startX;
                currentY++;

                if (lineChangeCallback) lineChangeCallback(); // Notify line change

                goTo(currentX, currentY);
            }

            output += ch;
            std::wcout << ch;

            currentX++;
        }

        // Move cursor to a new line after writing the output
        goTo(0, currentY + 2);

        return output;
    }

    // Searches for a text string in the console buffer and returns its coordinates
    // Returns {-1, -1} if the text is not found
    static COORD findTextInConsole(const std::string& searchString) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);

        int consoleWidth = csbi.dwSize.X;
        int consoleHeight = csbi.dwSize.Y;

        CHAR_INFO* buffer = new CHAR_INFO[consoleWidth * consoleHeight];
        COORD bufferSize = { consoleWidth, consoleHeight };
        COORD bufferCoord = { 0, 0 };
        SMALL_RECT readRegion = { 0, 0, consoleWidth - 1, consoleHeight - 1 };

        // Read the console screen buffer into the allocated buffer
        if (ReadConsoleOutput(hConsole, buffer, bufferSize, bufferCoord, &readRegion)) {
            // Search for the text in the console buffer
            for (int y = 0; y < consoleHeight; ++y) {
                for (int x = 0; x <= consoleWidth - searchString.size(); ++x) {
                    bool match = true;
                    for (size_t i = 0; i < searchString.size(); ++i) {
                        if (buffer[y * consoleWidth + x + i].Char.AsciiChar != searchString[i]) {
                            match = false;
                            break;
                        }
                    }
                    if (match) {
                        delete[] buffer;
                        return { static_cast<SHORT>(x + searchString.length()), static_cast<SHORT>(y) };
                    }
                }
            }
        }

        delete[] buffer;

        // Return {-1, -1} if text is not found
        return { -1, -1 };
    }

    // Pauses execution and waits for user input before exiting
    // Moves the cursor to a position below the last character
    static void pause() {
        COORD lastChar = Cursor::getLastConsoleChar();
        Cursor::goTo(0, lastChar.Y + 2);

        system("pause");  // Wait for user input
    }

private:
    // Retrieves the current cursor position
    static COORD getCurrentCursorPosition() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

        return csbi.dwCursorPosition;
    }
};

// Class for packing and unpacking vector data into a bit-packed format
class Bitpacker {

public:
    // Packs 'nin' integers from the 'input' vector into a bit-packed format and stores it in the 'output' vector.
    // 'n' specifies the number of bits used to represent each integer.
    // Returns the total number of bits written to the 'output' vector.
    static int pack(const std::vector<int>& input, std::vector<uint8_t>& output, int n) {
        int nin = input.size(); // Number of integers to pack
        int64_t inmask = 0; // Mask to isolate bits from the input integers
        int obit = 0; // Bit position in the current byte of the output vector
        int nout = 0; // Total number of bits written to the output vector

        // Calculate the maximum number of bytes needed to store the packed data
        int maxBytes = (nin * n + 7) / 8;
        output.resize(maxBytes); // Resize the output vector to fit the packed data
        memset(output.data(), 0, maxBytes); // Initialize the output buffer to zero

        // Iterate through each integer in the input vector
        for (int i = 0; i < nin; i++) {
            inmask = (int64_t)1 << (n - 1); // Initialize mask for the current integer

            // Extract bits from the current integer and store them in the output vector
            for (int k = 0; k < n; k++) {
                output[nout / 8] |= (((input[i] & inmask) >> (n - k - 1)) << (7 - obit));
                inmask >>= 1;
                obit++;
                nout++;

                // If a byte is fully filled, reset the bit position
                if (obit == 8) {
                    obit = 0;
                }
            }
        }
        return nout; // Return the total number of bits packed
    }

    // Unpacks 'nbitsin' bits from the 'input' vector and stores the resulting integers in the 'output' vector.
    // 'n' specifies the number of bits used to represent each integer.
    // Returns the number of integers written to the 'output' vector.
    static int unpack(const std::vector<uint8_t>& input, int nbitsin, std::vector<int>& output, int n) {
        int inbit = 0; // Bit position in the current byte of the input vector
        int nout = 0; // Total number of bits processed
        int numInts = nbitsin / n; // Number of integers to unpack
        output.resize(numInts); // Resize the output vector to fit the unpacked integers

        // Iterate through each integer to be unpacked
        for (int i = 0; i < numInts; i++) {
            int64_t value = 0; // Temporary variable to accumulate the unpacked integer value

            // Extract bits from the input vector to form the integer
            for (int j = 0; j < n; j++) {
                if (inbit == 8) { // Move to the next byte if needed
                    inbit = 0;
                }
                int bit = (input[nout / 8] >> (7 - inbit)) & 1;
                value = (value << 1) | bit;
                inbit++;
                nout++;
            }
            output[i] = static_cast<int>(value); // Store the unpacked integer in the output vector
        }
        return numInts; // Return the number of integers unpacked
    }

    // Converts an integer value to a vector of uint8_t with a specified number of bytes.
    static std::vector<uint8_t> intToBytes(int value, size_t n) {
        std::vector<uint8_t> bytes(n);
        // Copy the bytes of the integer into the vector
        std::memcpy(bytes.data(), &value, sizeof(value) < n ? sizeof(value) : n);
        return bytes;
    }

    // Converts a vector of uint8_t back into an integer.
    static int bytesToInt(const std::vector<uint8_t>& bytes) {
        int value = 0;
        // Copy the bytes from the vector into the integer
        std::memcpy(&value, bytes.data(), sizeof(value) < bytes.size() ? sizeof(value) : bytes.size());
        return value;
    }
};

// Lempel-Ziv-Welch algorithm
class LZW {
    // Type definition for a progress callback function
    using ProgressCallback = std::function<void(double)>;

public:
    static std::vector<uint8_t> encode(std::vector<uint8_t>& input, ProgressCallback progressCallback = nullptr) {
        // Dictionary initialization with std::unordered_map
        std::unordered_map<std::vector<uint8_t>, int32_t, VectorHash> dictionary;
        int32_t dictSize = 256; // Standard dictionary size for single-byte values
        for (int i = 0; i < dictSize; ++i) {
            dictionary[{static_cast<uint8_t>(i)}] = i;
        }

        std::vector<int32_t> codes;
        std::vector<uint8_t> currentSeq;

        int32_t i = 0;
        // Calculate interval for progress updates (1/6 of input length)
        int32_t interval = (input.size() / 3);

        for (uint8_t byte : input) {
            currentSeq.push_back(byte);
            if (dictionary.find(currentSeq) == dictionary.end()) {
                // Add the sequence to the dictionary
                std::vector<uint8_t> seq = currentSeq;
                seq.pop_back();
                int32_t code = dictionary[seq];
                codes.push_back(code);
                dictionary[currentSeq] = dictSize++;
                currentSeq = { byte };
            }
            i++;

            // Report progress periodically (every 1/3 of input length)
            if (progressCallback && input.size() >= 3 && i % interval == 0) {
                progressCallback(static_cast<double>(i) / input.size() * .81); // Report progress 
            }
        }

        // Output the last sequence
        if (!currentSeq.empty()) {
            codes.push_back(dictionary[currentSeq]);
        }

        uint8_t bitWidth = std::ceil(std::log2(dictSize));

        // Compress the output codes and append metadata
        std::vector<uint8_t> compressedVec;
        int nbits = Bitpacker::pack(codes, compressedVec, bitWidth);

        // Convert the number of bits used for packing into 4 bytes and append
        std::vector<uint8_t> nbitsVec = Bitpacker::intToBytes(nbits, 4);
        Utility::appendVector(compressedVec, nbitsVec);

        // Append the bit-width used for encoding
        compressedVec.push_back(bitWidth);

        if (progressCallback) progressCallback(1.0); // Report progress

        return compressedVec;
    }

    static std::vector<uint8_t> decode(std::vector<uint8_t>& compressed, ProgressCallback progressCallback = nullptr) {
        // Extract bit-width from the end
        int bitWidth = static_cast<int>(compressed.back());
        // Extract number of bits used for packing
        std::vector<uint8_t> nbitsVec = Utility::extractSubVector(compressed, compressed.size() - 5, 4);
        int nbits = Bitpacker::bytesToInt(nbitsVec);

        // Resize to remove the metadata
        compressed.resize(compressed.size() - 5);

        // Unpack the compressed data into a vector of codes
        std::vector<int32_t> unpackedVec;
        Bitpacker::unpack(compressed, nbits, unpackedVec, bitWidth);

        // Dictionary initialization with std::map
        std::unordered_map<int32_t, std::vector<uint8_t>> dictionary;
        int32_t dictSize = 256; // Standard dictionary size for single-byte values
        for (int i = 0; i < dictSize; ++i) {
            dictionary[i] = { static_cast<uint8_t>(i) };
        }

        std::vector<uint8_t> output;
        std::vector<uint8_t> currentSeq;
        int32_t oldCode = unpackedVec[0];
        output.insert(output.end(), dictionary[oldCode].begin(), dictionary[oldCode].end());
        currentSeq = dictionary[oldCode];

        // Calculate interval for progress updates (1/6 of input length)
        int32_t interval = (unpackedVec.size() / 3);
        for (size_t i = 1; i < unpackedVec.size(); ++i) {
            int32_t code = unpackedVec[i];
            std::vector<uint8_t> entry;

            if (dictionary.find(code) != dictionary.end()) {
                entry = dictionary[code];
            }
            else if (code == dictSize) {
                entry = currentSeq;
                entry.push_back(currentSeq[0]);
            }
            else {
                throw std::runtime_error("Bad compressed code.");
            }

            output.insert(output.end(), entry.begin(), entry.end());

            // Add new sequence to dictionary
            currentSeq.push_back(entry[0]);
            dictionary[dictSize++] = currentSeq;
            currentSeq = entry;

            // Report progress periodically (every 1/3 of input length)
            if (progressCallback && unpackedVec.size() >= 3 && i % interval == 0) {
                progressCallback(static_cast<double>(i) / unpackedVec.size() * 1); // Report progress 
            }
        }

        if (progressCallback) progressCallback(1.0); // Report progress

        return output;
    }

private:
    // Custom hash function for std::vector<uint8_t>
    struct VectorHash {
        std::size_t operator()(const std::vector<uint8_t>& vec) const {
            std::size_t seed = vec.size();
            for (const auto& i : vec) {
                seed ^= static_cast<std::size_t>(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
};

// Class to manage thread-safe access to shared resources using a mutex lock
class SharedResource {

public:
    // Outputs a message to the console at a specific position
    // Ensures that multiple threads do not write to the console simultaneously
    // by acquiring a mutex lock before printing.
    // This prevents interleaved or garbled output from concurrent threads.
    // 
    // Parameters:
    // - message: The string message to be printed.
    // - position: The console coordinates where the message will be displayed.
    static void printMessage(const std::string& message, COORD position) {
        // Lock the mutex to ensure exclusive access to the console output
        std::lock_guard<std::mutex> guard(mutex_);
        Cursor::goTo(position.X, position.Y); // Move cursor to the specified position
        std::cout << message; // Print the message to the console
    }

    // Reset the std::once_flag
    static void resetOnceFlag() {
        // Destroy the current once_flag and reinitialize it
        flag_.~once_flag();             // Explicitly call destructor
        new (&flag_) std::once_flag();  // Placement new to reconstruct the flag
    }

    // Static members to manage synchronization
    static std::mutex mutex_;  // Mutex to control access to the shared resource
    static std::once_flag flag_; // Flag for ensuring single-time initialization with std::call_once
};
// Static member definitions
std::mutex SharedResource::mutex_; // Definition of the static mutex for controlling access
std::once_flag SharedResource::flag_; // Definition of the static once_flag for one-time initialization

// Singleton class to manage and track progress updates
class ProgressTracker {

public:
    // Get the singleton instance of ProgressTracker
    // Ensures that only one instance of ProgressTracker exists
    static ProgressTracker& getInstance() {
        static ProgressTracker instance; // Static instance for singleton pattern
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copying
    // Ensures that the singleton instance cannot be copied or assigned
    ProgressTracker(const ProgressTracker&) = delete;
    ProgressTracker& operator=(const ProgressTracker&) = delete;

    // Start the progress tracking process
    // Adds the current instance to the list of active trackers
    // Ensures that progress can be updated if the tracker is running
    void start() {
        if (running) return;  // Exit if the tracker is already running

        running = true;
        std::lock_guard<std::mutex> lock(instancesMutex); // Lock to protect concurrent access
        instances.push_back(this); // Add this instance to the list of active trackers
    }

    // Stop the progress tracking process
    // Disables further progress updates
    void stop() {
        running = false;
    }

    // Update the displayed progress value
    // Only updates if the tracker is currently running
    // Displays a progress bar in the console at the location of the "Progress: " text
    // 
    // Parameters:
    // - progress: A double value representing the current progress (0.0 to 1.0)
    void updateProgress(double progress) {
        if (!running) return;  // Only update if the tracker is running

        COORD progressField = Cursor::findTextInConsole("Progress: "); // Find the location to update
        SharedResource::printMessage(GUI::drawProgressBar(progress), progressField); // Update progress display
    }

    // Stop and clean up all active progress trackers
    // Ensures that all trackers are stopped and removed from the list
    // This method should be called to clean up before program exit or when trackers are no longer needed
    static void shutdownAll() {
        std::lock_guard<std::mutex> lock(instancesMutex); // Lock to protect concurrent access
        for (auto* tracker : instances) {
            tracker->stop(); // Stop each tracker
        }
        instances.clear(); // Clear the list of trackers
    }

    // Destructor
    // Ensures proper cleanup of all active trackers when the instance is destroyed
    ~ProgressTracker() {
        shutdownAll(); // Clean up all trackers
    }

private:
    ProgressTracker() : running(false) {} // Constructor initializes the tracker as not running
    std::atomic<bool> running; // Atomic flag to indicate if the tracker is currently running

    static std::vector<ProgressTracker*> instances; // Static list of active ProgressTracker instances
    static std::mutex instancesMutex; // Mutex to synchronize access to the static list
};
// Static member definitions
std::vector<ProgressTracker*> ProgressTracker::instances; // Define the static list of instances
std::mutex ProgressTracker::instancesMutex; // Define the static mutex

// Manages and tracks elapsed time in a separate thread
class Timer {

public:
    // Constructor
    // Initializes the timer and adds it to the static list of instances
    Timer() : running(false), stop_thread(false) {
        std::lock_guard<std::mutex> lock(instancesMutex); // Lock to ensure thread-safe access to static members
        instances.push_back(this); // Add this instance to the list of active timers
    }

    // Destructor
    // Ensures proper cleanup by stopping the timer and cleaning up resources
    ~Timer() {
        shutdown(); // Stop the timer if running
    }

    // Start the timer
    // Begins tracking elapsed time in a new thread if not already running
    void start() {
        if (running) return; // Exit if the timer is already running

        running = true;
        timerThread = std::thread(&Timer::printElapsedTime, this); // Launch the timer thread
    }

    // Stop the timer
    // Stops the elapsed time tracking and joins the timer thread
    void stop() {
        running = false; // Stop the timer thread from running
        if (timerThread.joinable()) {
            timerThread.join(); // Wait for the timer thread to finish
        }
    }

    // Restart the timer
    // Stops the current timer and starts a new one
    void restart() {
        stop(); // Stop the current timer
        start(); // Start a new timer
    }

    // Shutdown the timer
    // Alias for stop, to stop the timer and clean up resources
    void shutdown() {
        stop(); // Stop the timer and wait for the thread to join
    }

    // Stop and clean up all active timers
    // Sets the stop flag and stops each timer, then clears the list of timers
    static void shutdownAll() {
        std::lock_guard<std::mutex> lock(instancesMutex); // Lock to ensure thread-safe access
        for (auto* timer : instances) {
            timer->stop_thread = true; // Signal all timers to stop
            timer->stop(); // Stop the timer
        }
        instances.clear(); // Clear the list of active timers
    }

    // Getter for the duration in seconds
    long getDuration() const {
        return duration;
    }

private:
    std::atomic<bool> running; // Flag to indicate if the timer is running
    std::atomic<bool> stop_thread; // Flag to signal the timer thread to stop
    std::thread timerThread; // Thread for tracking elapsed time
    static std::vector<Timer*> instances; // Static list of active Timer instances
    static std::mutex instancesMutex; // Mutex to synchronize access to the list of instances
    std::atomic<long> duration; // Variable to store the duration in seconds

    // Function executed by the timer thread to track and display elapsed time
    void printElapsedTime() {
        using namespace std::chrono;
        auto start = steady_clock::now(); // Record the start time

        while (running && !stop_thread) {
            auto now = steady_clock::now(); // Get the current time
            auto elapsed = duration_cast<seconds>(now - start).count(); // Calculate elapsed time in seconds

            // Find the location in the console to display the time
            COORD sizeFieldCoord = Cursor::findTextInConsole("Time: ");

            // Format the elapsed time as HH:MM:SS
            std::ostringstream oss;
            oss << std::setw(2) << std::setfill('0') << elapsed / 3600 << ":"        // Hours
                << std::setw(2) << std::setfill('0') << (elapsed % 3600) / 60 << ":" // Minutes
                << std::setw(2) << std::setfill('0') << elapsed % 60;                // Seconds

            std::string formattedTime = oss.str(); // Get the formatted time string
            SharedResource::printMessage(formattedTime, sizeFieldCoord); // Update the console display

            std::this_thread::sleep_for(seconds(1)); // Wait for one second before updating again
        }
        duration = duration_cast<seconds>(steady_clock::now() - start).count() - 1; // Calculate elapsed time in milliseconds ( -1 to remove the last second of sleep)
    }
};
// Static member definitions for Timer
std::vector<Timer*> Timer::instances; // Initialize the static list of Timer instances
std::mutex Timer::instancesMutex; // Initialize the static mutex for synchronizing access to the list

// Manages exception handling and error behavior
class ExceptionHandler {

public:
    // Handles exceptions by logging the error and performing necessary cleanups
    static void ExceptionHandle(const std::exception& e) {
        // Ensure that error handling occurs only once to avoid redundant outputs
        std::call_once(SharedResource::flag_, [&]() {
            // Lock to ensure thread-safe access to shared resources
            std::lock_guard<std::mutex> guard(SharedResource::mutex_);

            // Get the current cursor position and move to a new line for error output
            COORD pos = Cursor::getLastConsoleChar();
            Cursor::goTo(0, pos.Y + 2);

            // Convert exception message to a Unicode string and print it at the console
            Cursor::writeOutputStream(Utility::stringToU32String(e.what()), 58, errorCallback());

            // Shut down all active timers and progress trackers
            Timer::shutdownAll();
            ProgressTracker::shutdownAll();

            // Pause the cursor to stop any further console updates
            Cursor::pause();
            });
    }

    // Configures settings to suppress assertion pop-ups in debug builds
    static void DisableAssertionPopups() {
        // Set a custom report hook to handle CRT reports
        _CrtSetReportHook(CustomReportHook);

        // Set report modes to redirect CRT assertions and errors to a file instead of showing pop-ups
        _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
        _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
        _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    }

private:
    // Custom report hook function to handle CRT reports
    static int CustomReportHook(int reportType, char* message, int* returnValue) {
        // Create an exception from the report message and handle it
        std::exception e(message);
        ExceptionHandle(e);

        return 1; // Return 1 to indicate that the report was handled
    }

    // Creates a callback function to update the console display with error information
    static std::function<void()> errorCallback() {
        // Get the current end position of the console and move to a new line
        COORD endOfConsole = Cursor::getLastConsoleChar();
        Cursor::goTo(0, endOfConsole.Y + 1);

        // Draw an error screen to provide visual feedback on the error
        std::cout << GUI::drawErrorScreen();

        // Locate the error field in the console and move the cursor to it
        COORD errorField = Cursor::findTextInConsole("Error: ");
        Cursor::goTo(errorField.X, errorField.Y);

        // Define a lambda function to update the console display for the output field
        auto lineChangeCallbackOutput = []() {
            std::string screenUpdate = {
                    "\n |                                                                  |"
                    "\n \\__________________________________________________________________/"
            };
            std::cout << screenUpdate; // Output the update to the console
        };

        return lineChangeCallbackOutput; // Return the lambda function as the callback
    }
};

// Provides multi-threaded functionality for encoding and decoding algorithms
class Parallelization {

public:
    using ProgressCallback = std::function<void(double)>;

    // Parallel encoding of a Unicode string
    // Splits the input into chunks, encodes each chunk in parallel, and combines the results
    // O(n) where n is the number of threads
    static std::vector<uint8_t> parallelEncode(const std::vector<uint8_t>& input, int n, ProgressCallback progressCallback = nullptr) {

        // Split input into chunks
        std::vector<std::vector<uint8_t>> chunks(n);
        size_t chunkSize = input.size() / n;
        std::vector<std::vector<uint8_t>> encodedChunks(n);
        size_t maxChunkSize = 0;

        for (int i = 0; i < n; ++i) {
            size_t start = i * chunkSize;
            size_t end = (i == n - 1) ? input.size() : start + chunkSize;
            chunks[i] = std::vector<uint8_t>(input.begin() + start, input.begin() + end);
        }

        std::vector<std::thread> threads;
        std::mutex resultMutex;

        // Lambda function to encode each chunk
        auto processTask = [&](int index) {
            std::vector<uint8_t> encoded = encodeChunk(chunks[index], index == n - 1 ? progressCallback : nullptr);
            std::lock_guard<std::mutex> lock(resultMutex);
            encodedChunks[index] = encoded;
            maxChunkSize = maxChunkSize > encoded.size() ? maxChunkSize : encoded.size(); // Track maximum chunk size
        };

        // Launch encoding threads
        for (int i = 0; i < n; ++i) {
            threads.emplace_back(processTask, i);
        }

        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }

        // Combine all encoded chunks into a final result
        std::vector<uint8_t> finalResult;
        concatenateWithMetadata(encodedChunks, finalResult);

        return finalResult;
    }

    // Parallel decoding of a binary vector
    // Splits the input into chunks, decodes each chunk in parallel, and combines the results
    // O(n) where n is the number of threads
    static std::vector<uint8_t> parallelDecode(const std::vector<uint8_t>& input, int n, ProgressCallback progressCallback = nullptr) {

        std::vector<uint8_t> originalVec(input.begin(), input.end());
        // Split input into chunks
        std::vector<std::vector<uint8_t>> chunks(n);
        splitWithMetadata(originalVec, chunks);

        std::vector<std::vector<uint8_t>> results(n);
        std::vector<std::thread> threads;
        std::mutex resultMutex;

        // Lambda function to decode each chunk
        auto processTask = [&](int index) {
            std::vector<uint8_t> decoded = decodeChunk(chunks[index], index == n - 1 ? progressCallback : nullptr);
            std::lock_guard<std::mutex> lock(resultMutex);
            results[index] = decoded;
        };

        // Launch decoding threads
        for (int i = 0; i < n; ++i) {
            threads.emplace_back(processTask, i);
        }

        // Wait for all threads to complete
        for (auto& thread : threads) {
            thread.join();
        }

        // Combine all decoded results into a final std::vector<uint8_t>
        std::vector<uint8_t> finalResult;
        for (const auto& result : results) {
            Utility::appendVector(finalResult, result);
        }

        return finalResult;
    }

private:
    // Encodes a single chunk of input data
    static std::vector<uint8_t> encodeChunk(std::vector<uint8_t>& chunk, ProgressCallback progressCallback = nullptr) {
        try {
            return LZW::encode(chunk, progressCallback);
        }
        catch (const std::exception& e) {
            ExceptionHandler::ExceptionHandle(e);
            return {}; // Return empty vector in case of error
        }
    }

    // Decodes a single chunk of encoded data
    static std::vector<uint8_t> decodeChunk(std::vector<uint8_t>& chunk, ProgressCallback progressCallback = nullptr) {
        try {
            return LZW::decode(chunk, progressCallback);
        }
        catch (const std::exception& e) {
            ExceptionHandler::ExceptionHandle(e);
            return {}; // Return empty string in case of error
        }
    }

    // Concatenates multiple vectors into a single vector and includes metadata for each chunk.
    // Metadata includes the size of each chunk and the total number of chunks.
    static void concatenateWithMetadata(const std::vector<std::vector<uint8_t>>& vectors, std::vector<uint8_t>& result) {
        // Append all vectors to the result
        for (const auto& vec : vectors) {
            result.insert(result.end(), vec.begin(), vec.end());
        }
        // Append the size of each vector as a 4-byte integer
        for (const auto& vec : vectors) {
            std::vector<uint8_t> sizeVec = Bitpacker::intToBytes(static_cast<int>(vec.size()), 4);
            result.insert(result.end(), sizeVec.begin(), sizeVec.end());
        }
        // Append the total count of vectors as a single byte
        result.push_back(static_cast<uint8_t>(vectors.size()));
    }

    // Splits a concatenated vector into original chunks based on size metadata.
    // The input vector should contain metadata indicating the size of each chunk and the total count.
    static void splitWithMetadata(std::vector<uint8_t>& data, std::vector<std::vector<uint8_t>>& output) {
        // Retrieve the total number of chunks from the last byte of the data vector
        int count = static_cast<int>(data[data.size() - 1]);
        output.resize(count);

        // Calculate the start position of the size metadata
        size_t sizeMetadataStart = data.size() - (count * 4 + 1);

        // Extract sizes of each chunk from metadata
        std::vector<int> sizeIntVec(count);
        for (size_t i = 0; i < count; ++i) {
            std::vector<uint8_t> sizeBytes(data.begin() + sizeMetadataStart + i * 4, data.begin() + sizeMetadataStart + (i + 1) * 4);
            sizeIntVec[i] = Bitpacker::bytesToInt(sizeBytes);
        }

        // Split the data vector into chunks based on the extracted sizes
        size_t start = 0;
        for (size_t i = 0; i < output.size(); ++i) {
            output[i] = std::vector<uint8_t>(data.begin() + start, data.begin() + start + sizeIntVec[i]);
            start += sizeIntVec[i];
        }
    }
};

// Main function for encoding or decoding files based on user input
int main() {
    try {
        // Set the global locale to the user's preferred locale
        std::setlocale(LC_ALL, "");
        std::wcout.imbue(std::locale(""));

        // Disable assertion pop-ups that cannot be caught by standard exception handling
        ExceptionHandler::DisableAssertionPopups();

        // Draw the main screen of the GUI
        std::cout << GUI::drawMainScreen();

        // Find and set cursor position for the input field
        COORD inputField = Cursor::findTextInConsole("Input: ");
        Cursor::goTo(inputField.X, inputField.Y);

        // Lambda to handle screen updates for input field based on line changes
        auto lineChangeCallbackInput = [](bool direction) {
            if (direction) {
                std::string screenUpdate = {
                    "\n |                                                                  |"
                    "\n \\__________________________________________________________________/ "
                    "\n  __________________________________________________________________   "
                    "\n /                                                                  \\  "
                    "\n | Progress: [--------------------] 0 %       Time: 00:00:00        | "
                    "\n |                                                                  | "
                    "\n | Size: - / -                               Speed: -               | "
                    "\n |__________________________________________________________________| "
                    "\n |                                                                  | "
                    "\n | Output: -                                                        | "
                    "\n \\__________________________________________________________________/ "
                    "\n                                                                       "
                };
                std::cout << screenUpdate;
            }
            else {
                std::string screenUpdate = {
                    "\n \\__________________________________________________________________/ "
                    "\n  __________________________________________________________________   "
                    "\n /                                                                  \\  "
                    "\n | Progress: [--------------------] 0 %       Time: 00:00:00        | "
                    "\n |                                                                  | "
                    "\n | Size: - / -                               Speed: -               | "
                    "\n |__________________________________________________________________| "
                    "\n |                                                                  | "
                    "\n | Output: -                                                        | "
                    "\n \\__________________________________________________________________/ "
                    "\n                                                                       "
                };
                std::cout << screenUpdate;
            }
        };

        // Read input from the user and update the screen accordingly
        std::u32string userInput = Cursor::readInputStream(57, lineChangeCallbackInput);

        // Split the input into separate arguments
        std::vector<std::u32string> userInputVec = Utility::splitInputFilePathAndOption(userInput);

        // Determine input path and option based on input arguments
        const std::u32string INPUT_PATH = Utility::isIndexInRange(userInputVec, 0) ? userInputVec[0] : U"-0";
        const std::u32string INPUT_EXT = Utility::isIndexInRange(userInputVec, 1) ? userInputVec[1] : U"-1";
        const std::u32string INPUT_OPTION = Utility::isIndexInRange(userInputVec, 2) ? userInputVec[2] : U"-2";

        // Check if the input file exists
        if (!Utility::fileExists(INPUT_PATH + INPUT_EXT)) throw std::exception("File not found at input path.");
        // Check if the input option is valid
        if (!Utility::isValidOption(INPUT_OPTION)) throw std::exception("Input option not valid.");

        // Read the input file bytes
        std::vector<uint8_t> input;
        Utility::readFileToVector(INPUT_PATH + INPUT_EXT, input);

        // Determine output path based on input option
        std::u32string OUTPUT_PATH = INPUT_PATH;
        if (INPUT_OPTION == U"-c") {
            OUTPUT_PATH += U".bin";
        }
        else if (INPUT_OPTION == U"-d") {
            if (INPUT_EXT != U".bin") throw std::exception("Cannot decompress non .bin files.");

            ptrdiff_t indexOfDot = Utility::findLastDot(input); // Start index of the extension
            std::vector<uint8_t> extensionVec = Utility::extractSubVector(input, indexOfDot, input.size() - indexOfDot); // Efficient sub-vector extarction
            OUTPUT_PATH += U" Decoded" + Utility::bytesToString(extensionVec); // Add the file extension to the output path
            input.resize(indexOfDot); // Remove the file extension from the compressed data
        }

        // Find and set cursor position for the size field, then display the input file size
        COORD sizeField = Cursor::findTextInConsole("Size: - / ");
        Cursor::goTo(sizeField.X, sizeField.Y);
        std::cout << GUI::drawSizeField(Utility::getFileSize(INPUT_PATH + INPUT_EXT));

        // Find and set cursor position for the output field
        COORD outputField = Cursor::findTextInConsole("Output: ");
        Cursor::goTo(outputField.X, outputField.Y);

        // Lambda to handle screen updates for the output field
        auto lineChangeCallbackOutput = []() {
            std::string screenUpdate = {
                "\n |                                                                  | "
                "\n \\__________________________________________________________________/ "
            };
            std::cout << screenUpdate;
        };

        // Display the output file path
        std::u32string outputString = Cursor::writeOutputStream(OUTPUT_PATH, 57, lineChangeCallbackOutput);

        // Callback to update progress
        auto progressCallback = [](double progress) {
            ProgressTracker::getInstance().updateProgress(progress);
        };

        // Initialize progress tracker and timer
        ProgressTracker& progressTracker = ProgressTracker::getInstance();
        progressTracker.start();
        Timer timer;
        timer.start();

        // Perform encoding or decoding based on the input option
        if (INPUT_OPTION == U"-c") { // Encoding
            std::vector<uint8_t> encodedData = Parallelization::parallelEncode(input, 4, progressCallback);
            Utility::appendVector(encodedData, Utility::stringToBytes(INPUT_EXT)); // Add the extension as metadata
            Utility::writeVectorToFile(outputString, encodedData); // Final write to .bin file
        }
        else { // Decoding
            std::vector<uint8_t> decodedData = Parallelization::parallelDecode(input, 4, progressCallback);
            Utility::writeVectorToFile(outputString, decodedData); // Final write to original file
        }

        // Stop the progress tracker and timer
        progressTracker.stop();
        timer.stop();

        // Update size fields with sizes of input and output files
        sizeField = Cursor::findTextInConsole("Size: ");
        Cursor::goTo(sizeField.X, sizeField.Y);
        int outputSize = Utility::getFileSize(OUTPUT_PATH);
        int inputSize = Utility::getFileSize(INPUT_PATH + INPUT_EXT);
        std::cout << GUI::drawSizeField(outputSize) << " / " << GUI::drawSizeField(inputSize);

        // Display the compression ratio
        double compressionRatio = static_cast<double>(outputSize) / inputSize;
        std::cout << " (" << std::fixed << std::setprecision(4) << compressionRatio << ")";

        // Display the speed
        COORD speedField = Cursor::findTextInConsole("Speed: ");
        Cursor::goTo(speedField.X, speedField.Y);
        std::cout << GUI::drawSpeed(inputSize, timer.getDuration());

        // Pause the cursor to prevent further input
        Cursor::pause();
    }
    catch (const std::exception& e) {
        // Handle any exceptions that occurred during processing
        ExceptionHandler::ExceptionHandle(e);
    }

    // Clear the screen and restart the main function (for repeated runs)
    GUI::clearScreen();
    SharedResource::resetOnceFlag();
    main();
    return 0;
}
