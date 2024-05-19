#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QString>
#include <QDataStream>

void modifyFileName(QString & fileName);

void modifyFile(QDataStream &in, QDataStream &out, uint64_t value_for_modification);

#endif // FUNCTIONS_H
