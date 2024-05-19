#include <QString>
#include <QDataStream>

void modifyFileName(QString & fileName){
    qsizetype indexEnd = fileName.lastIndexOf('.');
    if(indexEnd == -1){
        indexEnd = fileName.length();
    }

    qsizetype indexBegin = indexEnd - 1;
    while(indexBegin >= 0 && fileName[indexBegin].isDigit()){
        --indexBegin;
    }
    ++indexBegin;

    int value;
    if(indexBegin == 0 || fileName[indexBegin - 1] == QChar('/')){
        value = 1;
        indexBegin = indexEnd;
    } else{
        value = fileName.mid(indexBegin, indexEnd - indexBegin).toInt() + 1;
        indexBegin -= (fileName[indexBegin - 1] == QChar('_'));
    }

    fileName.replace(indexBegin, indexEnd - indexBegin, QChar('_') + QString::number(value));
}

void modifyFile(QDataStream &in, QDataStream &out, uint64_t value_for_modification){
    uint8_t *readedRawBytes = new uint8_t[8];
    while(!in.atEnd()){
        int numberReadedBytes = in.readRawData((char*)(readedRawBytes), 8);
        uint64_t readedBlock = 0;
        for(int i = 0; i < numberReadedBytes; ++i){
            readedBlock = (readedBlock << 8) + readedRawBytes[i];
        }
        readedBlock <<= (8 - numberReadedBytes) * 8;    // дозаполняем нулями недостающий конец файла до кратноти 64 битам
        out << (readedBlock ^ value_for_modification);
    }
    delete[] readedRawBytes;
}
