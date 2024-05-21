#include <QString>
#include <windows.h>

void modifyFileName(QString &fileName)
{
    qsizetype indexEnd = fileName.lastIndexOf('.');
    if (indexEnd == -1) {
        indexEnd = fileName.length();
    }

    qsizetype indexBegin = indexEnd - 1;
    while (indexBegin >= 0 && fileName[indexBegin].isDigit()) {
        --indexBegin;
    }
    ++indexBegin;

    int value;
    if (indexBegin == 0 || fileName[indexBegin - 1] == '/') {
        value = 1;
        indexBegin = indexEnd;
    } else {
        value = fileName.mid(indexBegin, indexEnd - indexBegin).toInt() + 1;
        indexBegin -= (fileName[indexBegin - 1] == '_');
    }

    fileName.replace(indexBegin, indexEnd - indexBegin, '_' + QString::number(value));
}

void modifyFile(HANDLE in, HANDLE out, uint64_t modifyingValue)
{
    // обратите внимание что это читает и сохраняет в little-endian
    uint64_t readedRawBytes;
    DWORD numberReadedBytes;
    while (ReadFile(in, &readedRawBytes, 8, &numberReadedBytes, NULL) && numberReadedBytes != 0) {
        readedRawBytes ^= modifyingValue;

        DWORD numberWritedBytes;
        WriteFile(out, &readedRawBytes, numberReadedBytes, &numberWritedBytes, NULL);
    }

    // uint8_t *readedRawBytes = new uint8_t[8];
    // DWORD numberReadedBytes;
    // while (ReadFile(in, readedRawBytes, 8, &numberReadedBytes, NULL) && numberReadedBytes != 0) {
    //     uint64_t resultValue = 0;
    //     for (int i = 0; i < numberReadedBytes; ++i) {
    //         resultValue = (resultValue << 8) + readedRawBytes[i];
    //     }
    //     resultValue <<= 8 * (8 - numberReadedBytes);

    //     resultValue ^= modifyingValue;

    //     DWORD numberWritedBytes;
    //     WriteFile(out, &resultValue, numberReadedBytes, &numberWritedBytes, NULL);
    // }
    // delete[] readedRawBytes;
}
