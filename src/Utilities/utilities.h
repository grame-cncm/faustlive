#ifndef _utilities_H
#define _utilities_H

#include <string>

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

using namespace std;

void writeFile(const QString& filePath, const QString& content);

QString readFile(const QString& filePath);

QString pathToContent(const QString& path);

void touchFolder(const QString& path);

bool executeInstruction(const QString& instruct, QString& errorMsg);

void deleteDirectoryAndContent(const QString& directory);

bool tarFolder(const QString& folder, QString& errorMsg);
bool untarFolder(const QString& folder, QString& errorMsg);

bool rmDir(const QString &dirPath);
bool cpDir(const QString &srcPath, const QString &dstPath);

bool isStringInt(const char* word);

QString searchLocalIP();

const char* lopts(char *argv[], const char *name, const char* def);

int get_numberParameters(const QString& compilOptions);

string parse_compilationParams(QString& compilOptions);

bool parseKey(vector<string> options, const string& key, int& position);

bool addKeyIfExisting(vector<string>& options, vector<string>& newoptions, const string& key, const string& defaultKey, int& position);

void addKeyValueIfExisting(vector<string>& options, vector<string>& newoptions, const string& key, const string& defaultValue);

vector<string> reorganizeCompilationOptionsAux(vector<string>& options);

string FL_reorganize_compilation_options(QString compilationParams);

string FL_generate_sha1(const string& dsp_content);

void centerOnPrimaryScreen(QWidget* w);

#endif
