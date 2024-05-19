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
    uint64_t *readedRawBytes = new uint64_t;
    DWORD numberReadedBytes = 1;
    while (ReadFile(in, reinterpret_cast<uint8_t *>(readedRawBytes), 8, &numberReadedBytes, NULL) && numberReadedBytes != 0) {
        *readedRawBytes ^= modifyingValue;

        WriteFile(out, reinterpret_cast<uint8_t *>(readedRawBytes), numberReadedBytes, NULL, NULL);
    }
    delete readedRawBytes;
}
