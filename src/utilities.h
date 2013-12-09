#ifndef _utilities_H
#define _utilities_H

#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

string pathToContent(string path);

bool deleteDirectoryAndContent(string& directory);

bool rmDir(const QString &dirPath);

bool cpDir(const QString &srcPath, const QString &dstPath);

bool isStringInt(const char* word);

QString searchLocalIP();

const char* lopts(char *argv[], const char *name, const char* def);

#endif