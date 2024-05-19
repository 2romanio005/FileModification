#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QString>

#include <windows.h>

void modifyFileName(QString &fileName);

void modifyFile(HANDLE in, HANDLE out, uint64_t modifyingValue);

#endif // FUNCTIONS_H
