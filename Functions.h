#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QDataStream>
#include <QString>

void modifyFileName(QString &fileName);

void modifyFile(QDataStream &in, QDataStream &out, uint64_t modifyingValue);

#endif // FUNCTIONS_H
