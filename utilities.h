#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <QtGui>
#include <string>

using namespace std;

string pathToContent(string path);

bool deleteDirectoryAndContent(string& directory);

bool rmDir(const QString &dirPath);

bool cpDir(const QString &srcPath, const QString &dstPath);

bool isStringInt(const char* word);

#endif