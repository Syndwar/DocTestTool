#ifndef DOC_INFO_H
#define DOC_INFO_H

#include <QStringList>

struct DocInfo
{
    QString filePath;
    QString fileName;
    QStringList tags;
    QString comment;
};


#endif // DOC_INFO_H
